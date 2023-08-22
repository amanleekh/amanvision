

#include <pthread.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <linux/videodev2.h>

#include "common_def.h"
#include "video_sync.h"
#if VIDEOSYNC_MODULE_TEST == 0
#include "g2d.h"
#endif

// in desktop app, we do not need video sync, but use video file instead
#if GLOBAL_RUN_ENV_DESKTOP == 0

typedef struct _videosync_chn_struct_
{
    int reg;    /**< is this chan register to save video frame*/
    int single_mem; /**< single queue ele mem size */
    int mem_size;                   /**< pre alloced mem size */
    int write_idx;  /**< src data write idx */
    int lock_idx_gpu;   /**< the frame idx being used by gpu process. def is -1, means no frame is locked */
    int lock_idx_sync;   /**< the frame idx being synced */
    unsigned int seq;   /**< each frame have seq inc+1 */
    /**< how many vaild frame in queue, max is VIDEOSYNC_MAX_QUEUE_NUM. */
    /**< discard frame is also vaild */
    int vaild_frame_num;    
    videosync_frame_s sync_frame[VIDEOSYNC_MAX_QUEUE_NUM];   /**< video frame */
    struct g2d_buf *g2d_buffer[VIDEOSYNC_MAX_QUEUE_NUM];     /**< pre alloced g2d mem struct */
}videosync_chn_s;


typedef struct _video_sync_state_struct_
{
    pthread_mutex_t lock;
    pthread_cond_t cond;
    int sync_det;
    videosync_chn_s chns[VIDEOSYNC_MAX_FRAME];  /**< channel frame data */
}video_sync_state_s;

/**
* state struct
*/
static video_sync_state_s *g_pvideosync = NULL;


int videosync_init(int sync_det)
{
    int i, j;
    video_sync_state_s *p_handler 
        = (video_sync_state_s *)malloc(sizeof(video_sync_state_s));
    if (NULL == p_handler)
    {
        VSYNC_ERR("videosync_init. alloc state mem faild\n");
        return VIDEOSYNC_ERR;
    }
    memset(p_handler, 0, sizeof(video_sync_state_s));
    // VIDEOSYNC_MAX_QUEUE_NUM must at least 4
    // 1 lock_gpu, 1 lock_sync, 1 cur_write_idx, 1 next write idx
    if ((VIDEOSYNC_MAX_FRAME != 4) || (VIDEOSYNC_MAX_QUEUE_NUM < 4))
    {
        VSYNC_ERR("videosync_init. param error\n");
        return VIDEOSYNC_ERR;        
    }

    pthread_mutex_init(&(p_handler->lock), NULL);
    pthread_cond_init(&(p_handler->cond), NULL);
    p_handler->sync_det = sync_det;
    for (i = 0; i < VIDEOSYNC_MAX_FRAME; i++)
    {
        videosync_chn_s *p_chn = &(p_handler->chns[i]);

        p_chn->reg = 0;
        p_chn->single_mem = 0;
        p_chn->mem_size = 0;
        p_chn->write_idx = 0;
        p_chn->lock_idx_gpu = -1;
        p_chn->lock_idx_sync = -1;
        p_chn->seq = 0;
        p_chn->vaild_frame_num = 0;

        for (j = 0; j < VIDEOSYNC_MAX_QUEUE_NUM; j++)
        {
            p_chn->sync_frame[j].idx = i;
            p_chn->sync_frame[j].queue_idx = j;
            p_chn->sync_frame[j].frame_seq = 0;
            p_chn->sync_frame[j].vaild = 0;  
            p_chn->sync_frame[j].discard = 0;
            p_chn->sync_frame[j].is_synced = -1;
            p_chn->sync_frame[j].is_cache = 0;
            p_chn->sync_frame[j].paddr = 0;
            p_chn->sync_frame[j].vaddr = NULL;
            p_chn->sync_frame[j].g2d_format = G2D_YUYV;
            p_chn->sync_frame[j].time_cap_ms = 0;
            p_chn->sync_frame[j].time_cvt_ms = 0;
            p_chn->sync_frame[j].time_wait_ms = 0;
            p_chn->g2d_buffer[j] = NULL;
        }
    }
    g_pvideosync = p_handler;

    VSYNC_LOG("videosync_init success, sync_det=%d ms, chn=%d, chn queue=%d\n",
        p_handler->sync_det, VIDEOSYNC_MAX_FRAME, VIDEOSYNC_MAX_QUEUE_NUM);
    return VIDEOSYNC_OK;
}


int videosync_register(int idx)
{
    video_sync_state_s *p_handler = g_pvideosync;

    if ((NULL == p_handler) || (idx < 0) || (idx > VIDEOSYNC_MAX_FRAME))
    {
        VSYNC_ERR("videosync_register. param error\n");
        return VIDEOSYNC_ERR;
    }
    pthread_mutex_lock(&(p_handler->lock));
    p_handler->chns[idx].reg = 1;
    pthread_mutex_unlock(&(p_handler->lock));

    VSYNC_LOG("videosync_register success. sync chn idx=%d\n", idx);
    return VIDEOSYNC_OK;
}


int videosync_unregister(int idx)
{
    video_sync_state_s *p_handler = g_pvideosync;

    if ((NULL == p_handler) || (idx < 0) || (idx > VIDEOSYNC_MAX_FRAME))
    {
        VSYNC_ERR("videosync_unregister. param error\n");
        return VIDEOSYNC_ERR;
    }

    pthread_mutex_lock(&(p_handler->lock));
    p_handler->chns[idx].reg = 0;
    pthread_mutex_unlock(&(p_handler->lock));

    VSYNC_LOG("videosync_unregister success. idx=%d\n", idx);
    return VIDEOSYNC_OK;
}


int videosync_get_register_state(int idx)
{
    video_sync_state_s *p_handler = g_pvideosync;

    if ((NULL == p_handler) || (idx < 0) || (idx > VIDEOSYNC_MAX_FRAME))
    {
        VSYNC_ERR("videosync_get_register_state. param error\n");
        return VIDEOSYNC_ERR;
    }

    return p_handler->chns[idx].reg;
}


static unsigned int videosync_get_time_ms()
{
    struct timeval  tv;
    gettimeofday(&tv, NULL);

    unsigned int time_in_mill 
        = (tv.tv_sec) * 1000 + (tv.tv_usec) / 1000;
    return time_in_mill;
}


static int videosync_update_widx(videosync_chn_s *p_chn)
{
    int i;
    for (i = 0; i < VIDEOSYNC_MAX_QUEUE_NUM - 1; i++)
    {
        p_chn->write_idx++;
        if (p_chn->write_idx >= VIDEOSYNC_MAX_QUEUE_NUM)
        {
            p_chn->write_idx = 0;
        }
        if (p_chn->write_idx == p_chn->lock_idx_gpu)
        {
            continue;
        }
        videosync_frame_s *p_tmp_frame = &(p_chn->sync_frame[p_chn->write_idx]);
        if (p_tmp_frame->vaild)
        {
            p_tmp_frame->discard = 1;
            p_tmp_frame->is_synced = -1;
        }
        return 0;
    }

    return 1;
}

/**
* check if other channel sync with ref channel's frame, and set lock_sync flag
*/
static int videosync_do_synce_process(int ref_chn, int queue_idx)
{
    int i, j;
    video_sync_state_s *p_handler = g_pvideosync;
    int sync_queue_idx[VIDEOSYNC_MAX_FRAME];

    // first set sync_queue_idx = -1
    for (i = 0; i < VIDEOSYNC_MAX_FRAME; i++)
    {
        sync_queue_idx[i] = -1;
    }
    sync_queue_idx[ref_chn] = queue_idx;

    // check all channel
    for (i = 0; i < VIDEOSYNC_MAX_FRAME; i++)
    {
        videosync_chn_s *p_chn = &(p_handler->chns[i]);
        if ((i == ref_chn) || (0 == p_chn->reg))
        {
            continue;
        }
        // compare timestamps from the latest to the oldest
        // once it meets the requirements, the loop exits. 
        // we are looking for the latest & timestamp matching frames.
        int match_ok = 0;
        int newest_idx = p_chn->write_idx;
        for (j = 0; j < p_chn->vaild_frame_num; j++)
        {
            newest_idx--;
            if (newest_idx < 0)
            {
                newest_idx = VIDEOSYNC_MAX_QUEUE_NUM - 1;
            }         
            // skip lst locked frame
            if (newest_idx == p_chn->lock_idx_gpu)
            {
                continue;
            }
            // skip discard frame
            videosync_frame_s *p_ref_frame = &(p_handler->chns[ref_chn].sync_frame[queue_idx]);
            videosync_frame_s *p_frame = &(p_chn->sync_frame[newest_idx]);   
            if ((0 == p_frame->vaild) || (1 == p_frame->discard) || (1 == p_frame->is_synced))
            {
                continue;
            }
            unsigned int ts_ref = p_ref_frame->time_cap_ms;
            unsigned int ts_cur = p_frame->time_cap_ms;
            unsigned int det;
            if (ts_ref > ts_cur)
            {
                det = ts_ref - ts_cur;
            }
            else
            {
                det = ts_cur - ts_ref;
            }
            if (det <= (unsigned int)(p_handler->sync_det))
            {
                sync_queue_idx[i] = newest_idx;
                match_ok = 1;
                break;
            }
        }
        // if one channel fails, return
        if (0 == match_ok)
        {
            return 1;
        }  
    }

    // all chan is synced
    for (i = 0; i < VIDEOSYNC_MAX_FRAME; i++)
    {
        // set chn's lock sync flag
        videosync_chn_s *p_chn = &(p_handler->chns[i]);       
        p_chn->lock_idx_sync = sync_queue_idx[i];

        // set frame's is_synced flag
        p_chn->sync_frame[sync_queue_idx[i]].is_synced = 1;
        
        // set discard flag, which older than synced
        int discard_flag = 0;
        int newest_idx = p_chn->write_idx;
        for (j = 0; j < p_chn->vaild_frame_num; j++)
        {
            newest_idx--;
            if (newest_idx < 0)
            {
                newest_idx = VIDEOSYNC_MAX_QUEUE_NUM - 1;
            }
            if (newest_idx == p_chn->lock_idx_sync)
            {
                discard_flag = 1;
                continue;
            }
            videosync_frame_s *p_frame = &(p_chn->sync_frame[newest_idx]);          
            if ((1 == discard_flag) 
                && (1 == p_frame->vaild)
                && (newest_idx != p_chn->lock_idx_gpu))
            {
                p_frame->discard = 1;
                p_frame->is_synced = -1;
            }
        }
    }

    return 0;
}


/**
* videosync_reserve_frames
* resered frames for new input frame, should be called before input_src_data
* @param idx, channel
* @param p_data, input array
* @param p_arr_size, input and output, input array size must at least VIDEOSYNC_MAX_QUEUE_NUM
*        return filled reserved frame num, [0, VIDEOSYNC_MAX_QUEUE_NUM]
*/
int videosync_reserve_frames(int idx, videosync_frame_s *p_data, int *p_arr_size)
{
    int i;
    video_sync_state_s *p_handler = g_pvideosync; 

    if ((NULL == p_handler) || (idx < 0) || (idx >= VIDEOSYNC_MAX_FRAME)
        || (NULL == p_data) || (NULL == p_arr_size))
    {
        VSYNC_ERR("videosync_reserve_frames. param is not correct\n");
        return VIDEOSYNC_ERR;    
    }
    if (*p_arr_size < VIDEOSYNC_MAX_QUEUE_NUM)
    {
        VSYNC_ERR("videosync_reserve_frames. p_arr_size=%d error\n", *p_arr_size);
        return VIDEOSYNC_ERR;            
    }
    if (0 == p_handler->chns[idx].reg)
    {
        VSYNC_ERR("videosync_reserve_frames. idx %d is not register\n", idx);
        return VIDEOSYNC_ERR;
    }
    *p_arr_size = 0;

    pthread_mutex_lock(&(p_handler->lock));
    // remove all frame which have discard flag
    videosync_chn_s *p_chn = &(p_handler->chns[idx]);
    int frame_idx = 0;
    for (i = 0; i < VIDEOSYNC_MAX_QUEUE_NUM; i++)
    {
        videosync_frame_s *p_frame = &(p_chn->sync_frame[i]);
        if (p_frame->discard && p_frame->vaild)
        {
            p_frame->discard = 0;
            p_frame->vaild = 0;
            p_frame->is_synced = -1;
            p_chn->vaild_frame_num--;
            memcpy(&(p_data[frame_idx]), p_frame, sizeof(videosync_frame_s));
            frame_idx++;
        }
    }
    *p_arr_size = frame_idx;
#if VIDEOSYNC_MODULE_TEST == 1    
    VSYNC_LOG("videosync_reserve_frames. idx %d, %d frame reserved\n", idx, frame_idx);
    VSYNC_LOG("videosync_reserve_frames. idx %d, write_idx=%d, valid_num=%d, lock_gpu=%d, lock_sync=%d\n", 
        idx, p_chn->write_idx, p_chn->vaild_frame_num, p_chn->lock_idx_gpu, p_chn->lock_idx_sync);
#endif    
    pthread_mutex_unlock(&(p_handler->lock));

    return VIDEOSYNC_OK;
}


int videosync_input_src_data(int idx, videosync_src_data_s *p_data)
{
    video_sync_state_s *p_handler = g_pvideosync;

#if VIDEOSYNC_MODULE_TEST == 0
    if ((NULL == p_handler) || (idx < 0) || (idx >= VIDEOSYNC_MAX_FRAME)
        || (NULL == p_data) || (NULL == p_data->p_private)
        || (p_data->width <= 0) || (p_data->height <= 0))
    {
        VSYNC_ERR("videosync_input_src_data. param is not correct\n");
        return VIDEOSYNC_ERR;
    }
#endif

    if (0 == p_handler->chns[idx].reg)
    {
        VSYNC_ERR("videosync_input_src_data. idx %d is not register\n", idx);
        return VIDEOSYNC_ERR;
    }
   
    pthread_mutex_lock(&(p_handler->lock));

    // if vsync queue is full, return error
    // in order to ensure that the data can be placed success
    // videosync_reserve_frames function must be called before this
    videosync_chn_s *p_chn = &(p_handler->chns[idx]);
    if (p_chn->vaild_frame_num >= VIDEOSYNC_MAX_QUEUE_NUM)
    {
        pthread_mutex_unlock(&(p_handler->lock));
        VSYNC_ERR("videosync_input_src_data. idx %d is full\n", idx);
        return VIDEOSYNC_ERR;
    }
    // set sync frame info
    int queue_widx = p_chn->write_idx;
    videosync_frame_s *p_frame = &(p_chn->sync_frame[queue_widx]);    
    if (p_frame->vaild)
    {
        pthread_mutex_unlock(&(p_handler->lock));
        VSYNC_ERR("videosync_input_src_data. idx %d, queue %d have data\n", idx, queue_widx);
        return VIDEOSYNC_ERR;
    }
    p_frame->idx = idx;
    p_frame->queue_idx = queue_widx;
    p_frame->frame_seq = p_handler->chns[idx].seq + 1;
    p_frame->vaild = 1;
    p_frame->discard = 0;
    p_frame->is_synced = -1;
    p_frame->width = p_data->width;
    p_frame->height = p_data->height;
    p_frame->is_cache = p_data->is_cache;
#if VIDEOSYNC_MODULE_TEST == 0    
    p_frame->paddr = ((struct g2d_buf *)p_data->p_private)->buf_paddr;
    p_frame->vaddr = (unsigned char *)(((struct g2d_buf *)p_data->p_private)->buf_vaddr);
#endif    
    p_frame->g2d_format = p_data->dst_g2d_format;
    p_frame->time_cap_ms = p_data->cap_time_ms;
    p_frame->time_cvt_ms = videosync_get_time_ms();
    memcpy(&(p_frame->v4l2_frame), &(p_data->v4l2_frame), sizeof(struct v4l2_buffer));
    p_chn->seq++;
    p_chn->vaild_frame_num++;
    // update write idx
    int update_widx_err = videosync_update_widx(p_chn);
    if (update_widx_err != 0)
    {
        pthread_mutex_unlock(&(p_handler->lock));
        VSYNC_ERR("videosync_input_src_data. idx %d, update write idx faild\n", idx);
        return VIDEOSYNC_ERR;        
    } 
    // check other chan, find frame idx which ts in limit
    int sync_error = videosync_do_synce_process(idx, queue_widx);

#if VIDEOSYNC_MODULE_TEST == 1
    VSYNC_LOG("+videosync_input_src_data. idx=%d, cap_time=%u\n", idx, p_frame->time_cap_ms);
    VSYNC_LOG("+old widx=%d, cur widx=%d, vaild_frame_num=%d, seq=%u. sync_error=%d, lock_gpu_idx=%d, lock_sync_idx=%d\n",
        queue_widx, p_chn->write_idx, p_chn->vaild_frame_num, p_chn->seq, sync_error, 
        p_chn->lock_idx_gpu, p_chn->lock_idx_sync);
#endif
    if (0 == sync_error)
    {
        pthread_cond_signal(&(p_handler->cond));
    }
	pthread_mutex_unlock(&(p_handler->lock));

    return VIDEOSYNC_OK;
}


int videosync_clear_queue()
{
    int i, j;
    video_sync_state_s *p_handler = g_pvideosync;

    pthread_mutex_lock(&(p_handler->lock));
    
    for (i = 0; i < VIDEOSYNC_MAX_FRAME; i++)
    {
        videosync_chn_s *p_chn = &(p_handler->chns[i]);

        p_chn->write_idx = 0;
        p_chn->lock_idx_gpu = -1;
        p_chn->lock_idx_sync = -1;
        for (j = 0; j < VIDEOSYNC_MAX_QUEUE_NUM; j++)
        {
            videosync_frame_s *p_frame = &(p_chn->sync_frame[j]);
            if (p_frame->vaild)
            {
                p_frame->discard = 1;
            }
        }
    }
    pthread_mutex_unlock(&(p_handler->lock));

    return 0;
}


int videosync_get_queue_num(int *p_queue_num)
{
    if (NULL == p_queue_num)
    {
        VSYNC_ERR("videosync_get_queue_num. param error\n");
    }
    *p_queue_num = VIDEOSYNC_MAX_QUEUE_NUM;
    return VIDEOSYNC_OK;
}


int videosync_get_frame_info(int idx, int queue_idx, videosync_frame_s *p_single_frame)
{
    video_sync_state_s *p_handler = g_pvideosync;

    if ((NULL == p_handler) || (NULL == p_single_frame) 
        || (idx < 0) || (idx >= VIDEOSYNC_MAX_FRAME)
        || (queue_idx < 0) || (queue_idx >= VIDEOSYNC_MAX_QUEUE_NUM))
    {
        VSYNC_ERR("videosync_lookat_single_frame. param error\n");
        return VIDEOSYNC_ERR; 
    }
    if ((0x00 == p_handler->chns[idx].sync_frame[queue_idx].paddr)
        || (NULL == p_handler->chns[idx].sync_frame[queue_idx].vaddr))
    {
        VSYNC_ERR("videosync_lookat_single_frame. idx=%d addr is null\n",
            idx);
        return VIDEOSYNC_ERR;
    }

    pthread_mutex_lock(&(p_handler->lock));
    memcpy(p_single_frame, 
        &(p_handler->chns[idx].sync_frame[queue_idx]), 
        sizeof(videosync_frame_s));
    pthread_mutex_unlock(&(p_handler->lock));

    return VIDEOSYNC_OK;
}

static int videosync_check_synced_vaild()
{
    int i;
    video_sync_state_s *p_handler = g_pvideosync;

    for (i = 0; i < VIDEOSYNC_MAX_FRAME; i++)
    {
        videosync_chn_s *p_chn = &(p_handler->chns[i]);
        if (p_chn->reg)
        {
            if ((p_chn->lock_idx_sync < 0) || (p_chn->vaild_frame_num <= 0))
            {
                return 0;
            }
        }
    }

    return 1;
}

int videosync_wait_synced_frame(videosync_sync_frame_s *p_sync_frame, int force_release)
{
    int i;
    video_sync_state_s *p_handler = g_pvideosync;

    if ((NULL == p_handler) || (NULL == p_sync_frame))
    {
        VSYNC_ERR("videosync_wait_synced_frame. param error\n");
        return VIDEOSYNC_ERR;
    }

    pthread_mutex_lock(&(p_handler->lock));
    while (1)
    {
        pthread_cond_wait(&(p_handler->cond), &(p_handler->lock));
        // if fake cond single, continue to wait
        int sync_vaild = videosync_check_synced_vaild();
        if (sync_vaild)
        {
            break;
        }
    }
    unsigned int time_ms = videosync_get_time_ms();
    for (i = 0; i < VIDEOSYNC_MAX_FRAME; i++)
    {
        videosync_chn_s *p_chn = &(p_handler->chns[i]);
        if (0 == p_chn->reg)
        {
            p_sync_frame->frame[i].vaild = 0;
            continue;
        }
        if (p_chn->lock_idx_gpu >= 0)
        {
            if (force_release)
            {
                p_chn->lock_idx_gpu = p_chn->lock_idx_sync;
            }
            else
            {
                pthread_mutex_unlock(&(p_handler->lock));
                //VSYNC_ERR("videosync_wait_synced_frame. sync gpu frame is not released\n");
                return VIDEOSYNC_ERR_RELEASE;
            }
        }
        else
        {
            p_chn->lock_idx_gpu = p_chn->lock_idx_sync;
        } 
        p_chn->lock_idx_sync = -1;
        memcpy(&(p_sync_frame->frame[i]), 
            &(p_handler->chns[i].sync_frame[p_chn->lock_idx_gpu]),
            sizeof(videosync_frame_s));
        p_sync_frame->frame[i].time_wait_ms = time_ms;
        
    }
    pthread_mutex_unlock(&(p_handler->lock));

    return VIDEOSYNC_OK;    
}



int videosync_release_synced_rame()
{
    int i;
    video_sync_state_s *p_handler = g_pvideosync;
    
    if (NULL == p_handler)
    {
        VSYNC_ERR("videosync_release_synced_rame. param error\n");
        return VIDEOSYNC_ERR;
    }

    pthread_mutex_lock(&(p_handler->lock));
    for (i = 0; i < VIDEOSYNC_MAX_FRAME; i++)
    {
        videosync_chn_s *p_chn = &(p_handler->chns[i]);
        if (0 == p_chn->reg)
        {
            continue;
        }
        p_chn->lock_idx_gpu = -1;
    }
    pthread_mutex_unlock(&(p_handler->lock));

    return 0;
}


int videosync_print_info()
{
    int i, j;
    video_sync_state_s *p_handler = g_pvideosync;

    static int s_cnt = 0;

    printf("---videosync_print_info,sync_det=%d,prt_cnt=%d---\n", p_handler->sync_det, s_cnt++);
    for (i = 0; i < VIDEOSYNC_MAX_FRAME; i++)
    {
        videosync_chn_s *p_chn = &(p_handler->chns[i]);
        printf("+chan=%d,w_idx=%d,lock_idx_gpu=%d,lock_idx_sync=%d,seq=%d,vaild_num=%d\n",
            i, p_chn->write_idx, p_chn->lock_idx_gpu, p_chn->lock_idx_sync, p_chn->seq,
            p_chn->vaild_frame_num);
        for (j = 0; j < VIDEOSYNC_MAX_QUEUE_NUM; j++)
        {
            videosync_frame_s *p_frame = &(p_chn->sync_frame[j]);
            printf("+--queue=%d,frame_seq=%d,vaild=%d,discard=%d,is_synced=%d,time_cap=%u\n",
                p_frame->queue_idx, p_frame->frame_seq,
                p_frame->vaild, p_frame->discard, p_frame->is_synced, p_frame->time_cap_ms);
        }
    }

    return 0;
}


int videosync_deinit()
{
    int i, j;
    video_sync_state_s *p_handler = g_pvideosync;

    if (NULL == p_handler)
    {
        VSYNC_ERR("videosync_deinit. param error\n");
        return VIDEOSYNC_ERR;
    }
#if VIDEOSYNC_MODULE_TEST == 0
    for (i = 0; i < VIDEOSYNC_MAX_FRAME; i++)
     {
         if (p_handler->chns[i].reg)
         {
             for (j = 0; j < VIDEOSYNC_MAX_QUEUE_NUM; j++)
             {
                g2d_free(p_handler->chns[i].g2d_buffer[j]);
             }
         }
    }
#endif
    free(p_handler);

    return 0;
}

#endif
