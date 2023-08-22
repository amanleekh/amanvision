

#ifndef _VIDEO_CAPTURE_H_
#define _VIDEO_CAPTURE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <linux/videodev2.h>
#include "common_def.h"

// in desktop app, we do not need camera capture, use video file instead
#if GLOBAL_RUN_ENV_DESKTOP == 0

#define VIDEOCAP_LOG(...) AVM_LOG(__VA_ARGS__)
#define VIDEOCAP_ERR(...) AVM_ERR(__VA_ARGS__)

#define VIDEO_CAPTURE_ERR   (1)
#define VIDEO_CAPTURE_OK    (0)

#define VIDEO_CAPTURE_PRT_STAT  (0) // if printf status, for debug

/**
* as process callback func's param
*/
typedef struct _videocap_frame_struct_
{
    int width;
    int height;
    int format;         /**< V4L2_PIX_FMT_ */
    unsigned int cap_ts_ms; /**< the time ms when v4l2 cap this video frame*/
    int is_chache;      /**< is frame data mem cache or not. (g2d mem) */
    void *p_private;    /**< user define data (struct g2d_buf *) */
    struct v4l2_buffer v4l2_frame;  /**< v4l2 frame */
}videocap_frame_s;


/**
* init param
*/
typedef struct _videocap_init_struct_
{
    int dev_index;      /**< will open dev/videox */
    int format;         /**< V4L2_PIX_FMT_ */
    int is_cache;       /**< v4l2 will alloc userspace mem, indicate this mem is cache or not */
    /**< error=0, frame is ok, error=1 no frame return, error others, error */
    void (*getback_frame_t)(videocap_frame_s *p_frame, int *p_arr_num, int *p_error);  
    /**< error=0, frame is ok, error others, error */
    void (*send_frame_t)(videocap_frame_s *p_frame, int *p_error);
}videocap_init_s;


/**
* capture status
*/
typedef struct _videocap_status_struct_
{
    int dev_idx;    /**< dev index */
    float fps;      /**< capture fps */
    unsigned int frame_cnt; /**< total frame cnt since start */
    int v4l2_dq_err_cnt;    /**< v4l2 dqbuffer ioctl error cnt */
    int mem_kb;     /**< module mem used */
}videocap_status_s;


/**
* cam init, open video device
*/
int videocap_init(videocap_init_s *p_init, void **handler, int* input_width, int* input_height);


/**
* start videox capture
*/
int videocap_start(void *handler);


/**
* cap_process. should be in thread since it's block process
*/
int videocap_process(void *handler);


/**
* tmp suspend cap process
* @param pwr_status, 0 for shutdown cam power, 1 to setup camera power
*/
int videocap_pause_process(int pwr_status);


/**
* watch dog process
@param handler, handler array
@param dev_num, array num
*/
int videocap_watchdog_process(void **handler, int dev_num);


/**
* deinit. close fd and free mem
*/
int videocap_deinit(void *handler);


/**
 * @brief videocap_get_devidx
 * @param p_devidx
 * @return
 */
int videocap_get_devidx(void *handler, int *p_devidx);


/**
* get capture status
*/
int videocap_get_status(void *handler, videocap_status_s *p_status);


#endif

/**
 * @brief videocap_get_video_ok, get if video is vaild
 * @param p_is_ok
 */
void videocap_get_video_ok(int *p_is_ok, unsigned int *p_mask, int *p_cam_pwr);

/**
 * @brief videocap_set_video_ok, set video vaild
 * this func only can be used in desktop app
 * @param flag
 */
void videocap_set_video_ok(int flag);

void videocap_get_cam_link_status(unsigned int *p_status);
void videocap_get_cam_sync_status(unsigned int *p_status);


#ifdef __cplusplus
}
#endif

#endif


