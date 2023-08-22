

#ifndef _VIDEO_SYNC_H_
#define _VIDEO_SYNC_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <linux/videodev2.h>
#include "stddef.h"
#include "common_def.h"

// in desktop app, we do not use capture video, and do not need video sync
#if GLOBAL_RUN_ENV_DESKTOP == 0

#define VSYNC_LOG(...)   AVM_LOG(__VA_ARGS__)
#define VSYNC_ERR(...)   AVM_ERR(__VA_ARGS__)


#define VIDEOSYNC_OK            (0)
#define VIDEOSYNC_ERR           (1)
#define VIDEOSYNC_ERR_RELEASE   (2)     // get new frame before release last frame


#define VIDEOSYNC_MODULE_TEST   (0)     // if in linux test this module, will not alloc g2d mem

#define VIDEOSYNC_MAX_FRAME     (4)     // can sync max num frame
#define VIDEOSYNC_MAX_QUEUE_NUM (4)     // each sync chn have a frame queue.

/*
eg

chn1 (frame queue 5)
chn2 (frame queue 5)
chn3 (frame queue 5)
chn4 (frame queue 5)

4 chn for frame input, each chn have 5 queue for data input. queue have write idx and read idx
a synced frame include 4 chn frame
*/


/**
* single video source's frame info
* is videosync_input_src_data() input param
*/
typedef struct _videosync_src_data_struct_
{
    int width;  /**< input video's res */
    int height;
    int src_v4l2_format;    /**< input video's v4l2 format */
    unsigned int cap_time_ms;   /**< time capture model v4l2 get this frame */
    int is_cache;           /**< is input video's frame data is cached mem or not */
    int dst_g2d_format;      /**< will trans input frame from src format to dst format, dst format is g2d format */
    void *p_private;        /**< private data pointer. (must be struct g2d_buf) */
    struct v4l2_buffer v4l2_frame;  /**< v4l2 frame */
}videosync_src_data_s;

/**
* single channel synced video frame
* one videosync_sync_frame_s have VIDEOSYNC_MAX_FRAME number channel frame
*/
typedef struct _videosync_frame_struct_
{
    int idx;    /**< chn idx of synced video frame, [0, VIDEOSYNC_MAX_FRAME) */
    int queue_idx;  /**< queue idx of this frame in chn's queue. [0, VIDEOSYNC_MAX_QUEUE_NUM) */
    unsigned int frame_seq;   /**< each frame have a seq, which always inc+1 */
    int vaild;  /**< have frame data */
    int discard;   /**< frame have discard flag will not used for sync */
    int is_synced;      /**< is this frame have being synced before */
    int width;  
    int height;
    int is_cache;   /**< is video frame data cached */
    size_t paddr;   /**< video data's phy addr */
    unsigned char *vaddr;   /**< video data's virt addr */
    int g2d_format; /**< video data's g2d format */
    unsigned int time_cap_ms;   /**< time capture module'v4l2 get this frame */
    unsigned int time_cvt_ms;   /**< time that hw g2d convert over, from src to dst format */
    unsigned int time_wait_ms;  /**< time that wait synced time */
    struct v4l2_buffer v4l2_frame;  /**< v4l2 frame */
}videosync_frame_s;


/**
* sync frame struct. include all synced frame
*/
typedef struct _videosync_sync_frame_struct_
{
    videosync_frame_s frame[VIDEOSYNC_MAX_FRAME];
}videosync_sync_frame_s;


/**
* alloc mem, this mem not include image frame data
* @param sync_det sync frame's time det, -1 not care
*/
int videosync_init(int sync_det);


/**
* register a sync channel
* if a idx chn is not register, src input data also copy data
* a register chn mean that sync frame signal comes from it
*
* @param idx [0, VIDEOSYNC_MAX_FRAME)
*/
int videosync_register(int idx);


/**
* unregister sync chn
*/
int videosync_unregister(int idx);


/**
* get chn register state
*/
int videosync_get_register_state(int idx);


/**
* channel input data
* will always put data no matter this chn is register or not
*/
int videosync_input_src_data(int idx, videosync_src_data_s *p_data);


/**
* videosync_reserve_frames
* resered frames for new input frame, should be called before input_src_data
* @param idx, channel
* @param p_data, input array
* @param p_arr_size, input and output, input array size must at least VIDEOSYNC_MAX_QUEUE_NUM
*        return filled reserved frame num, [0, VIDEOSYNC_MAX_QUEUE_NUM]
*/
int videosync_reserve_frames(int idx, videosync_frame_s *p_data, int *p_arr_size);


/**
* clear queue
*/
int videosync_clear_queue();


/**
* get sync chn queue num
* *p_queue_num=VIDEOSYNC_MAX_QUEUE_NUM
*/
int videosync_get_queue_num(int *p_queue_num);


/**
* get frame info, for check paddr and vaddr
*/
int videosync_get_frame_info(int idx, int queue_idx, videosync_frame_s *p_single_frame);


/**
* wait for a syned frame
* block until frame is ready
*/
int videosync_wait_synced_frame(videosync_sync_frame_s *p_sync_frame, int force_release);


int videosync_release_synced_rame();


int videosync_print_info();

/**
* deinit
*/
int videosync_deinit();

#endif


#ifdef __cplusplus
}
#endif

#endif



