
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <linux/videodev2.h>

#include "common_def.h"
#include "video_capture.h"
#include "video_sync.h"
#include "video_adapt.h"
#include "thread.h"
#include "g2d.h"
//#include "minIni.h"
#include "ini_config.h"

// in desktop app, we do not need camera capture, use video file instead
#if GLOBAL_RUN_ENV_DESKTOP == 0

#define THREAD_VIDEO_CAP_WD             (50)
#define THREAD_VIDEO_CAP_THREAD_SUP     (55)
#define THREAD_VIDEO_CAP_PRI_BASE       (70)
#define THREAD_VIDEO_FETCH_PRI          (69)


/**
* video capture process callback function
* @param dev_idx /video/devx
* @param capture frame info struct
*/
void video_cap_cbfunc_comm(int dev_idx, videocap_frame_s *p_frame, int *p_error)
{
    videosync_src_data_s sync_in_data;

    sync_in_data.width = p_frame->width;
    sync_in_data.height = p_frame->height;
    sync_in_data.src_v4l2_format = p_frame->format;
    sync_in_data.cap_time_ms = p_frame->cap_ts_ms;
    sync_in_data.is_cache = p_frame->is_chache;
    sync_in_data.dst_g2d_format = G2D_RGBA8888;
    sync_in_data.p_private = p_frame->p_private; // struct g2d_buffer
    memcpy(&(sync_in_data.v4l2_frame), &(p_frame->v4l2_frame), sizeof(struct v4l2_buffer));

    // video/devx -> sync channel x
    *p_error = videosync_input_src_data(dev_idx, &sync_in_data);
}


/**
* video capture process callback function. idx = 0
*/
static void video_cap_cbfunc_dev0(videocap_frame_s *p_frame, int *p_error)
{
    return video_cap_cbfunc_comm(0, p_frame, p_error);
}


/**
* video capture process callback function. idx = 1
*/
static void video_cap_cbfunc_dev1(videocap_frame_s *p_frame, int *p_error)
{
    return video_cap_cbfunc_comm(1, p_frame, p_error);
}


/**
* video capture process callback function. idx = 2
*/
static void video_cap_cbfunc_dev2(videocap_frame_s *p_frame, int *p_error)
{
    return video_cap_cbfunc_comm(2, p_frame, p_error);
}


/**
* video capture process callback function. idx = 3
*/
static void video_cap_cbfunc_dev3(videocap_frame_s *p_frame, int *p_error)
{
    return video_cap_cbfunc_comm(3, p_frame, p_error);
}


void video_getback_cbfunc_comm(int dev_idx, videocap_frame_s *p_frame, int *p_arr_num, int *p_error)
{
    int i;
    videosync_frame_s sync_frame[VIDEOSYNC_MAX_QUEUE_NUM];

    *p_error = videosync_reserve_frames(dev_idx, sync_frame, p_arr_num);
    if (*p_error != 0)
    {
        VIDEOAPT_ERR("video_getback_cbfunc_comm. dev_idx=%d faild\n", dev_idx);
        return;
    }
    for (i = 0; i < *p_arr_num; i++)
    {
        p_frame[i].width = sync_frame[i].width;
        p_frame[i].height = sync_frame[i].height;
        p_frame[i].format = 0;
        p_frame[i].cap_ts_ms = sync_frame[i].time_cap_ms;
        p_frame[i].is_chache = 0;
        p_frame[i].p_private = NULL;
        memcpy(&(p_frame[i].v4l2_frame), &(sync_frame[i].v4l2_frame),
            sizeof(struct v4l2_buffer));
    }
}


void video_getback_cbfunc_dev0(videocap_frame_s *p_frame, int *p_arr_num, int *p_error)
{
    video_getback_cbfunc_comm(0, p_frame, p_arr_num, p_error);
}


void video_getback_cbfunc_dev1(videocap_frame_s *p_frame, int *p_arr_num, int *p_error)
{
    video_getback_cbfunc_comm(1, p_frame, p_arr_num, p_error);
}


void video_getback_cbfunc_dev2(videocap_frame_s *p_frame, int *p_arr_num, int *p_error)
{
    video_getback_cbfunc_comm(2, p_frame, p_arr_num, p_error);
}


void video_getback_cbfunc_dev3(videocap_frame_s *p_frame, int *p_arr_num, int *p_error)
{
    video_getback_cbfunc_comm(3, p_frame, p_arr_num, p_error);
}


/**
* video capture thread function
*/
void *video_cap_thread(void *arg)
{
    int ret;

    if (NULL == arg)
    {
        VIDEOAPT_ERR("videocap_thread. param error\n");
        return NULL;
    }

    void *cap_handler = *((void **)arg);
    int dev_idx = -1;
    ret = videocap_get_devidx(cap_handler, &dev_idx);
    if (ret != VIDEO_CAPTURE_OK)
    {
        VIDEOAPT_ERR("video_cap_thread. videocap_get_devidx faild\n");
        return NULL;
    }
    VIDEOAPT_LOG("video_cap_thread start. dev_idx=%d\n", dev_idx);

    ret = videocap_start(cap_handler);
    if (ret != VIDEO_CAPTURE_OK)
    {
        VIDEOAPT_ERR("video_cap_thread. videocap_start faild\n");
        return NULL;
    }

    // video cap process. will call cb_func to put frame to vsync chn
    while (1)
    {
        videocap_process(cap_handler);
    }

    return NULL;
}


/**
* video watch dog thread
*/
void *video_cap_wd_thread(void *arg)
{
    int ret = 0;

    if (NULL == arg)
    {
        VIDEOAPT_ERR("video_cap_wd_thread. param error\n");
        return NULL;
    }
    void **cap_handler_arr = (void **)arg;

    while (1)
    {
        ret = videocap_watchdog_process(cap_handler_arr, 4);
        if (ret != 0)
        {
            return NULL;
        }
    }

    return NULL;
}


/*
* video capture and video sync input
*/
int video_init(int *p_chn_vaild, int* input_width, int* input_height, int cap_v4l2_format)
{
    int i;
    int ret;

    if((NULL == input_width)||(NULL == input_height))
    {
        VIDEOAPT_ERR("video_init. input pointer is NULL\n");
        return 1;
    }

    VIDEOAPT_LOG("video_init. camera exist config.[0]=%d, [1]=%d, [2]=%d, [3]=%d\n",
        p_chn_vaild[0], p_chn_vaild[1], p_chn_vaild[2], p_chn_vaild[3]);

    // videosync init, set max sync det(ms)
    ret = videosync_init(10);
    if (ret != VIDEOSYNC_OK)
    {
        VIDEOAPT_ERR("video_init. videosync_init faild\n");
        return 1;
    }

    for (i = 0; i < 4; i++)
    {
        // register channel videosync
        // if all vsync chn registered ignoring chn availability, get sync frame could faild as
        // it wait all frame ready
        if (p_chn_vaild[i])
        {
            ret = videosync_register(i);
            if (ret != VIDEOSYNC_OK)
            {
                VIDEOAPT_ERR("video_init. videosync_register faild, idx=%d\n", i);
                return 1;
            }
        }
    }

    // init and create thread for video capture
    // capture video data will send to videosync channel
    static void *s_cap_handler[4] = {NULL};
    for (i = 0; i < 4; i++)
    {
        if (0 == p_chn_vaild[i])
        {
            continue;
        }

        // cap init
        videocap_init_s init;

        init.dev_index = i;
        init.format = cap_v4l2_format;
        init.is_cache = 0;
        if (0 == i)
        {
            init.getback_frame_t = &video_getback_cbfunc_dev0;
            init.send_frame_t = &video_cap_cbfunc_dev0;
        }
        else if (1 == i)
        {
            init.getback_frame_t = &video_getback_cbfunc_dev1;
            init.send_frame_t = &video_cap_cbfunc_dev1;
        }
        else if (2 == i)
        {
            init.getback_frame_t = &video_getback_cbfunc_dev2;
            init.send_frame_t = &video_cap_cbfunc_dev2;
        }
        else
        {
            init.getback_frame_t = &video_getback_cbfunc_dev3;
            init.send_frame_t = &video_cap_cbfunc_dev3;
        }
        ret = videocap_init(&init, &(s_cap_handler[i]), input_width, input_height);
        if (ret != VIDEO_CAPTURE_OK)
        {
            VIDEOAPT_ERR("video_init. videocap_init faild\n");
            return 1;
        }

        // create video capture thread
        char new_name[16] = {'\0'};
        sprintf(new_name, "vcap_%d", i);
        ret = thread_create_name(THREAD_VIDEO_CAP_PRI_BASE + i,
            new_name,
            &video_cap_thread,
            &(s_cap_handler[i]));
        if (ret != THREAD_OK)
        {
            VIDEOAPT_ERR("video_init. thread_create cap faild. idx=%d\n", i);
            return 1;
        }
    }

    // create video watch dog thread
    ret = thread_create_name(THREAD_VIDEO_CAP_WD, "vcap_dog",
        &video_cap_wd_thread, s_cap_handler);
    if (ret != THREAD_OK)
    {
        VIDEOAPT_ERR("video_init. video_cap_wd_thread faild.\n");
        return 1;
    }

    return 0;
}

int video_adapt(int* input_width, int* input_height)
{
    int ret;
    int cap_chn_vaild[4] = {1, 1, 1, 1};
    unsigned int camera_v4l2_format = V4L2_PIX_FMT_UYVY;  /**< actual format is YUYV, but imx6 mipi can only receive UYVY format */

    if((NULL == input_width)||(NULL == input_height))
    {
        VIDEOAPT_ERR("video_adapt input pointer is NULL\n");
        return 1;
    }

    // read camera vaild config file, if this file not exist, use default value
    int tmp_val[4];
    tmp_val[0] = ini_get_idx0();
    tmp_val[1] = ini_get_idx1();
    tmp_val[2] = ini_get_idx2();
    tmp_val[3] = ini_get_idx3();
    if ((-1 == tmp_val[0]) || (-1 == tmp_val[1]) 
        || (-1 == tmp_val[2]) || (-1 == tmp_val[3]))
    {
        VIDEOAPT_LOG("video_adapt. open camera config file faild, use default value\n");
    }
    else
    {
        cap_chn_vaild[0] = tmp_val[0];
        cap_chn_vaild[1] = tmp_val[1];
        cap_chn_vaild[2] = tmp_val[2];
        cap_chn_vaild[3] = tmp_val[3];

        VIDEOAPT_LOG("video_adapt. read camera config file."
            "cam0=%d,cam1=%d,cam2=%d,cam3=%d\n",
            cap_chn_vaild[0], cap_chn_vaild[1],
            cap_chn_vaild[2], cap_chn_vaild[3]);
    }

    // init video capture and video sync
    ret = video_init(cap_chn_vaild, 
        input_width, input_height, camera_v4l2_format);
    if (ret != 0)
    {
        VIDEOAPT_ERR("video_init faild\n");
        return 1;
    }

    return 0;
}

#endif
