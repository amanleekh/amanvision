#ifndef SYNC_VIDEO_GETTER_H
#define SYNC_VIDEO_GETTER_H

#include <QObject>

#include "common_def.h"
#include "video_sync.h"


#define SYNCVDGET_LOG(...)  AVM_LOG(__VA_ARGS__)
#define SYNCVDGET_ERR(...)  AVM_ERR(__VA_ARGS__)


// in desktop app, we do not need camera capture, use video file instead
#if GLOBAL_RUN_ENV_DESKTOP == 0

/**
 * @brief The SyncVideoGetter class
 * a work thread which get synced video, and send a signal to surround widget to update gl context
 */
class SyncVideoGetter : public QObject
{
    Q_OBJECT

public:
    SyncVideoGetter(int isPrt);

    /**
     * @brief setPrtFlag
     * @param isPrt
     * @return
     */
    int setPrtFlag(int isPrt);

    /**
    @ saveCamPic save current cam pic to dir
    */
    static void saveCamPic()
    {
        SyncVideoGetter::s_save_cam_pic = 1;
    }

    static int getSaveCamPicFlag()
    {
        return SyncVideoGetter::s_save_cam_pic;
    }

    /**
    * @brief get sync chn's image info
    *        the image has been acquired in the worker thread, 
    *        this func just get some information, not do actual get action
    *        you can call this interface multiple times to get the image, 
    *        which is actually the same frame, until call releaseImage
    */
    static int getChnImgAddr_ex(int sync_chn_idx, unsigned int *paddr, unsigned char **p_vaddr, unsigned int *p_ts);

    /**< release image */
    static int releaseImage();

public slots:
    /**
     * @brief fetchSyncVideoProcess, a slots run while(1) to wait synced video. start when thread start
     */
    void fetchSyncVideoProcess();

signals:
    /**
     * @brief finishGetOneFrame, signal send to surround widget when get one frame, will let surround widget to update gpu context
     */
    void finishGetOneFrame();

private:
    /**
     * @brief getTimeMs
     * @return
     */
    unsigned int getTimeMs();

    /**
     * @brief statusCalc
     */
    void statusCalc();

    /**< save pic process */
    void savePicProcess(videosync_sync_frame_s *p_sync_frame);

    /**< status calc pre process. check if get image func return ok */
    void statusCalc_pre(int func_ret);

private:
    int m_isPrt;    /**< is print status */
    unsigned int m_last_ms; /**< last ms time */
    unsigned int m_total_ms;    /**< total ms time */
    int m_total_cnt;    /**< total cnt */
    int m_intv; /**< stat intv is 5S */
    int m_get_cnt;          /**< total get frame cnt */
    int m_continue_cnt;     /**< get frame func return err cnt */
    float m_fps;
    int m_prt_cnt;    /**< */
    static int s_save_cam_pic;
    static int s_camera_seq[4];   /**< right,front,left,back image is v4l2's which vide idx */
    static unsigned int s_ts[VIDEOSYNC_MAX_FRAME];
    static unsigned int s_paddr[VIDEOSYNC_MAX_FRAME];
    static unsigned char *s_vaddr[VIDEOSYNC_MAX_FRAME];
};

#endif

#endif // SYNC_VIDEO_GETTER_H
