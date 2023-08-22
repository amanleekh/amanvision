
#include <stdio.h>
#include <sys/time.h>
#include <unistd.h>

#include "avm_gpu.h"
#include "video_sync.h"
#include "sync_video_getter.h"

// in desktop app, we do not need camera capture, use video file instead
#if GLOBAL_RUN_ENV_DESKTOP == 0

#include "minIni.h"
#include "g2d.h"
#include <opencv2/opencv.hpp>


int SyncVideoGetter::s_camera_seq[4] = {0};
int SyncVideoGetter::s_save_cam_pic = 0;
unsigned int SyncVideoGetter::s_ts[VIDEOSYNC_MAX_FRAME] = {0};
unsigned int SyncVideoGetter::s_paddr[VIDEOSYNC_MAX_FRAME] = {0};
unsigned char *SyncVideoGetter::s_vaddr[VIDEOSYNC_MAX_FRAME] = {NULL};


/**
 * @brief SyncVideoGetter::SyncVideoGetter
 * @param isPrt
 */
SyncVideoGetter::SyncVideoGetter(int isPrt)
{
    // read config file
    // right,front,left,back image is v4l2's which vide idx
    s_camera_seq[0] = ini_getl("CAMERA_SEQ", "cam_right", -1, "avm_qt_app_res/config.ini");
    s_camera_seq[1] = ini_getl("CAMERA_SEQ", "cam_front", -1, "avm_qt_app_res/config.ini");
    s_camera_seq[2] = ini_getl("CAMERA_SEQ", "cam_left", -1, "avm_qt_app_res/config.ini");
    s_camera_seq[3] = ini_getl("CAMERA_SEQ", "cam_rear", -1, "avm_qt_app_res/config.ini");
    if ((s_camera_seq[0] < 0) || (s_camera_seq[1] < 0)
        || (s_camera_seq[2] < 0) || (s_camera_seq[3] < 0))
    {
        SYNCVDGET_ERR("SyncVideoGetter::SyncVideoGetter. read config  file faild, use default\n");
        s_camera_seq[0] = 0;
        s_camera_seq[1] = 1;
        s_camera_seq[2] = 2;
        s_camera_seq[3] = 3;
    }
    
    // init value
    m_isPrt = isPrt;

    m_last_ms = 0;
    m_total_ms = 0;
    m_total_cnt = 0;
    m_get_cnt = 0;
    m_continue_cnt = 0;
    m_intv = 5000;
    m_fps = 0.0f;
    m_prt_cnt = 0;
    //s_save_cam_pic = 0;
}


/**
 * @brief SyncVideoGetter::setPrtFlag
 * @param isPrt
 * @return
 */
int SyncVideoGetter::setPrtFlag(int isPrt)
{
    m_isPrt = isPrt;

    if (1 == isPrt)
    {
        m_prt_cnt = 0;
    }

    return 0;
}

/**
 * @brief SyncVideoGetter::getTimeMs, get time ms
 */
unsigned SyncVideoGetter::getTimeMs()
{
    struct timeval  tv;
    gettimeofday(&tv, NULL);

    unsigned int time_in_mill
        = (tv.tv_sec) * 1000 + (tv.tv_usec) / 1000;
    return time_in_mill;
}


/**< status calc pre process. check if get image func return ok */
void SyncVideoGetter::statusCalc_pre(int func_ret)
{
    m_get_cnt++;
    if (func_ret != 0)
    {
        m_continue_cnt++;
    }
}


/**
 * @brief SyncVideoGetter::statusCalc
 */
void SyncVideoGetter::statusCalc()
{
    unsigned int cur_ms = getTimeMs();

    if (0 == m_last_ms)
    {
        m_last_ms = cur_ms;
        m_total_ms = 0;
        m_total_cnt = 0;
    }
    else
    {
        int det = cur_ms - m_last_ms;
        if ((det > 0) && (det < 10*1000))  // 10*1000 for error
        {
            m_total_ms += det;
            m_total_cnt++;
            if (m_total_ms >= (unsigned int)(m_intv))    // every %d s stat once
            {
                m_fps = m_total_cnt * 1000.0 / m_total_ms;
                if ((m_isPrt) && (m_prt_cnt++ <= 2))
                {
                    SYNCVDGET_LOG("SyncVideoGetter::statusCalc. fps=%.2f(intv=%dms), frm_drop_rate=%.2f(%d/%d)\n",
                        m_fps, m_intv, m_continue_cnt * 1.0 / m_get_cnt,
                        m_continue_cnt, m_get_cnt);
                }
                m_continue_cnt = 0;
                m_get_cnt = 0;
                m_total_ms = 0;
                m_total_cnt = 0;
            }
        }
        else
        {
            m_total_ms = 0;
            m_total_cnt = 0;
        }
        m_last_ms = cur_ms;
    }
}


/**< save pic process */
void SyncVideoGetter::savePicProcess(videosync_sync_frame_s *p_sync_frame)
{
    int i, j;
    
    // is save pic flag=0, return
    if (0 == s_save_cam_pic)
    {
        return;
    }
    
    unsigned int time_start = getTimeMs();
    for (i = 0; i < 4; i++)
    {
        if (0 == p_sync_frame->frame[i].vaild)
        {
            continue;
        }
    
        int width = p_sync_frame->frame[i].width;
        int height = p_sync_frame->frame[i].height;
        unsigned char *p_vaddr = p_sync_frame->frame[i].vaddr;
        if (p_sync_frame->frame[i].g2d_format == G2D_RGBA8888)
        {
            cv::Mat rgba_img(height, width, CV_8UC2);
            cv::Mat dst_bgr_img(height, width, CV_8UC3);
            memcpy(rgba_img.data, p_vaddr, width * height * 2);
            cv::cvtColor(rgba_img, dst_bgr_img, CV_YUV2BGR_YUYV);
            for (j = 0; j < 4; j++)
            {
                if (i == s_camera_seq[j])
                {
                    if (0 == j)     // j=0 means that right image is v4l2 idx i
                    {
                        imwrite("tmp_cap_right.jpg", dst_bgr_img);
                    }
                    else if (1 == j)
                    {
                        imwrite("tmp_cap_front.jpg", dst_bgr_img);
                    }
                    else if (2 == j)
                    {
                        imwrite("tmp_cap_left.jpg", dst_bgr_img);
                    }
                    else
                    {
                        imwrite("tmp_cap_rear.jpg", dst_bgr_img);
                    }
                }
            }
        }
        else
        {
            SYNCVDGET_ERR("SyncVideoGetter::fetchSyncVideoProcess. save tmp image faild\n");
        }
    }
    unsigned int time_end = getTimeMs();
    s_save_cam_pic = 0;

    SYNCVDGET_LOG("SyncVideoGetter::fetchSyncVideoProcess. save tmp image success\n");
    SYNCVDGET_LOG("SyncVideoGetter::fetchSyncVideoProcess. cost %u ms\n", time_end - time_start);
}


/**
 * @brief fetchSyncVideoProcess, a slots run while(1) to wait synced video. 
 *        start when worker thread start and send signal
 *
 *        SyncVideoGetter *syncVideoGetter = new SyncVideoGetter(1);
 *        syncVideoGetter->moveToThread(m_workerThread);
 */
void SyncVideoGetter::fetchSyncVideoProcess()
{
    SYNCVDGET_LOG("SyncVideoGetter::fetchSyncVideoProcess. thread. start to wait synced video\n");

    while (1)
    {
        // block to get an synced frame
        videosync_sync_frame_s sync_frame;
        int ret = videosync_wait_synced_frame(&sync_frame, 0);
        statusCalc_pre(ret);
        // if last frame is not released, bypass this frame
        if (VIDEOSYNC_ERR_RELEASE == ret)
        {
            continue;
        }
        // other error, continue to next wait
        else if (ret != VIDEOSYNC_OK)
        {
            usleep(10*1000);
            SYNCVDGET_ERR("SyncVideoGetter::fetchSyncVideoProcess. wait frame faild\n");
            continue;
        }

        // get frame success set address
        for (int i = 0; i < VIDEOSYNC_MAX_FRAME; i++)
        {
            SyncVideoGetter::s_ts[i] = sync_frame.frame[i].time_cap_ms;
            SyncVideoGetter::s_paddr[i] = (unsigned int)(sync_frame.frame[i].paddr);
            SyncVideoGetter::s_vaddr[i] = sync_frame.frame[i].vaddr;
        }

        // calc time status
        statusCalc();

        // check if necessory, save camera pic to dir
        savePicProcess(&sync_frame);

        emit finishGetOneFrame();
    }
}


/**< release image */
int SyncVideoGetter::releaseImage()
{
    videosync_release_synced_rame();
    return 0;
}


int SyncVideoGetter::getChnImgAddr_ex(int sync_chn_idx, unsigned int *paddr, unsigned char **p_vaddr, unsigned int *p_ts)
{
    *paddr = SyncVideoGetter::s_paddr[sync_chn_idx];
    *p_vaddr = SyncVideoGetter::s_vaddr[sync_chn_idx];
    *p_ts = SyncVideoGetter::s_ts[sync_chn_idx];

    return 0;
}


#endif
