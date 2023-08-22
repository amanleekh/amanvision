

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdint.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <linux/videodev2.h>
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>


#include "common_def.h"
#include "g2d.h"
#include "video_capture.h"
#include "video_sync.h"
#include "comu_cmd.h"
#include "comu_cmd_define.h"

// ioctl for max9286_chrdev, it must be the same as max9286 driver
#define MAX9286_CHRDEV_IOC_MAGIC 'M'
#define MAX9286_CHRDEV_RESTART      _IOR(MAX9286_CHRDEV_IOC_MAGIC, 1, int)
#define MAX9286_CHRDEV_CAM_PWR      _IOW(MAX9286_CHRDEV_IOC_MAGIC, 2, int)
#define MAX9286_CHRDEV_READ_REG     _IO(MAX9286_CHRDEV_IOC_MAGIC, 3)
#define MAX9286_CHRDEV_READ_STATUS  _IOR(MAX9286_CHRDEV_IOC_MAGIC, 4, int)
#define MAX9286_CHRDEV_READ_LINKSTA  _IOR(MAX9286_CHRDEV_IOC_MAGIC, 5, int)
#define MAX9286_CHRDEV_READ_SYNCSTA  _IOR(MAX9286_CHRDEV_IOC_MAGIC, 6, int)



static int g_is_video_ok = 1;       /**< is video signal ok */

static int g_vaild_mask = 0xFF;     /**< if a idx video is vaild*/

static int g_is_cam_pwr_closed = 0; /**< is camera power closed */

// 0-7 bit, max9286 0x49 reg, video link and config link 
// 8-11 bit, max9286 0x27 reg last 4 bit, means vsync detected
// 12-15 bit
// 16-19 bit, 0 ok, 0xff read max9286 faild
static unsigned int g_all_link_status = 0xFFFF;  /**< all link status get from kernel */

static unsigned int g_cam_link_status = 0xF;  /**< all link status get from kernel */
static unsigned int g_cam_sync_status = 0xF;  /**< all link status get from kernel */



// in desktop app, we do not need camera capture, use video file instead
#if GLOBAL_RUN_ENV_DESKTOP == 0

#define VIDEOCAP_NUMBER_BUFFERS     (6)  // num of v4l2's frame buffer


typedef struct _videocap_buffer_struct_
{
	unsigned char *start;   /**< mem vaddr */
	size_t offset;          /**< mem paddr */
	unsigned int length;
}videocap_buffer_s;


/**
* all videocap val
*/
typedef struct _videocap_state_struct_
{
    videocap_init_s init;   /**< init param */

    char cap_dev_str[36];   /**< /dev/videox string */
    int fd_cap_dev_v4l2;    /**< /dev/videox open handler */
    int v4l2_in_width;      /**< frame width get from v4l2 */
    int v4l2_in_height;

    int g2d_frame_size;     /**< alloc g2d mem's single frame size*/
    int g2d_fmt;            /**< equal as video V4L2_PIX format, but g2d format enum */

    int num_cap_buffers;    /**< is VIDEOCAP_NUMBER_BUFFERS */
    struct g2d_buf *g2d_cap_buffer[VIDEOCAP_NUMBER_BUFFERS];
    videocap_buffer_s cap_buffer[VIDEOCAP_NUMBER_BUFFERS];

    unsigned int last_ms;   /**< */
    unsigned int total_ms;  
    unsigned int total_cnt;
    unsigned int total_cnt_since_start;
    unsigned int v4l2_dq_err_cnt;
    float fps;
    unsigned int mem_used;
    int prt_cnt;    /**< print video fps serval times */

    int err_flag;   /**< if error occur when dq frame */
    int reopen_stream_flag; /**< if process reoprn stream flag */
	int alive_flag;
}videocap_state_s;


static int videocap_cap_setup(videocap_state_s *p_handler)
{
    // check cap abality
    struct v4l2_capability cap;
	if (ioctl (p_handler->fd_cap_dev_v4l2, VIDIOC_QUERYCAP, &cap) < 0) 
	{
		if (EINVAL == errno) 
		{
		    VIDEOCAP_ERR("videocap_cap_setup. %s is no V4L2 device\n", p_handler->cap_dev_str);
			return VIDEO_CAPTURE_ERR;
		} 
		else 
		{
		    VIDEOCAP_ERR("videocap_cap_setup. %s is not V4L devide, unknow error\n", 
		        p_handler->cap_dev_str);
			return VIDEO_CAPTURE_ERR;
		}
	}
	if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE)) 
	{
	    VIDEOCAP_ERR("videocap_cap_setup. %s is no video capture device\n",
	        p_handler->cap_dev_str);
		return VIDEO_CAPTURE_ERR;
	}
	if (!(cap.capabilities & V4L2_CAP_STREAMING)) 
	{
		VIDEOCAP_ERR("videocap_cap_setup. %s does not support streaming i/o\n",
			p_handler->cap_dev_str);
		return VIDEO_CAPTURE_ERR;
	}

    // set input
    int input = 1;
	if (ioctl(p_handler->fd_cap_dev_v4l2, VIDIOC_S_INPUT, &input) < 0) 
	{
		VIDEOCAP_ERR("videocap_cap_setup. VIDIOC_S_INPUT failed\n");
		close(p_handler->fd_cap_dev_v4l2);
		return VIDEO_CAPTURE_ERR;
	}

    // set video crop
    struct v4l2_cropcap cropcap;
    struct v4l2_crop crop;
	memset(&cropcap, 0, sizeof(cropcap));
	cropcap.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	if (ioctl (p_handler->fd_cap_dev_v4l2, VIDIOC_CROPCAP, &cropcap) < 0) 
	{
		crop.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		crop.c = cropcap.defrect; /* reset to default */

        if (ioctl (p_handler->fd_cap_dev_v4l2, VIDIOC_S_CROP, &crop) < 0) 
		{
			switch (errno) {
				case EINVAL:
					/* Cropping not supported. */
					VIDEOCAP_ERR("%s  doesn't support crop\n",
					    p_handler->cap_dev_str);
					break;
				default:
					/* Errors ignored. */
					break;
			}
		}
	} else {
		/* Errors ignored. */
	}	

    // set video param
    struct v4l2_streamparm parm;
	parm.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	parm.parm.capture.timeperframe.numerator = 1;
	parm.parm.capture.timeperframe.denominator = 0;
	parm.parm.capture.capturemode = 0;
	if (ioctl(p_handler->fd_cap_dev_v4l2, VIDIOC_S_PARM, &parm) < 0) 
	{
		VIDEOCAP_ERR("videocap_cap_setup. VIDIOC_S_PARM failed\n");
		close(p_handler->fd_cap_dev_v4l2);
		return VIDEO_CAPTURE_ERR;
	}

    // set video format
    if (p_handler->init.format != V4L2_PIX_FMT_UYVY)
    {
        VIDEOCAP_ERR("warning:videocap_cap_setup. avm project current "
            "color format is UYUV\n");
    }
    if (p_handler->init.format == V4L2_PIX_FMT_YUYV)
    {
        VIDEOCAP_ERR("warning: in avm project. "
            "cam foramt is YUYV ,but v4l2 must be UYUV\n");
    }
    struct v4l2_format fmt;
	memset(&fmt, 0, sizeof(fmt));

	fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	fmt.fmt.pix.width = 0;
	fmt.fmt.pix.height = 0;
	fmt.fmt.pix.pixelformat = p_handler->init.format;
	fmt.fmt.pix.field = V4L2_FIELD_ANY;
	if (ioctl(p_handler->fd_cap_dev_v4l2, VIDIOC_S_FMT, &fmt) < 0)
	{
		VIDEOCAP_ERR("videocap_cap_setup. %s iformat not supported\n",
			p_handler->cap_dev_str);
		return VIDEO_CAPTURE_ERR;
	}

	/* Note VIDIOC_S_FMT may change width and height. */

	/* Buggy driver paranoia. */
	unsigned int min;
	
	min = fmt.fmt.pix.width * 2;
	if (fmt.fmt.pix.bytesperline < min)
	{
		fmt.fmt.pix.bytesperline = min;
    }
	min = fmt.fmt.pix.bytesperline * fmt.fmt.pix.height;
	if (fmt.fmt.pix.sizeimage < min)
	{
		fmt.fmt.pix.sizeimage = min;
	}
	if (ioctl(p_handler->fd_cap_dev_v4l2, VIDIOC_G_FMT, &fmt) < 0) 
	{
		VIDEOCAP_ERR("videocap_cap_setup. VIDIOC_G_FMT failed\n");
		close(p_handler->fd_cap_dev_v4l2);
		return VIDEO_CAPTURE_ERR;
	}
    p_handler->v4l2_in_width = fmt.fmt.pix.width;
    p_handler->v4l2_in_height = fmt.fmt.pix.height;

    // set reqbuf as userspace mem
    struct v4l2_requestbuffers req;
	memset(&req, 0, sizeof (req));
	req.count = p_handler->num_cap_buffers;
	req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	req.memory = V4L2_MEMORY_USERPTR;
	if (ioctl(p_handler->fd_cap_dev_v4l2, VIDIOC_REQBUFS, &req) < 0) 
	{
		if (EINVAL == errno) 
		{
			VIDEOCAP_ERR("videocap_cap_setup. %s does not support "
					 "memory mapping\n", p_handler->cap_dev_str);
			return VIDEO_CAPTURE_ERR;
		} 
		else 
		{
			VIDEOCAP_ERR("videocap_cap_setup. %s does not support "
					 "memory mapping, unknow error\n", p_handler->cap_dev_str);
			return VIDEO_CAPTURE_ERR;
		}
	}
    if ((int)(req.count) != (int)(p_handler->num_cap_buffers)) {
		VIDEOCAP_ERR("videocap_cap_setup. Insufficient buffer memory on %s\n",
            p_handler->cap_dev_str);
		return VIDEO_CAPTURE_ERR;
	}

    VIDEOCAP_LOG("videocap_cap_setup. dev_idx=%d, v4l2_in_width=%d, v4l2_in_height=%d\n",
        p_handler->init.dev_index, p_handler->v4l2_in_width, p_handler->v4l2_in_height);
    return VIDEO_CAPTURE_OK;
}


/**
* alloc g2d mem and set to cap_buffer
*/
static int videocap_g2d_setup(videocap_state_s *p_handler)
{
    int i;
    
    // set g2d gormat and frame size
    switch (p_handler->init.format) 
    {
        case V4L2_PIX_FMT_RGB565:
            p_handler->g2d_frame_size 
                = p_handler->v4l2_in_width * p_handler->v4l2_in_height * 2;
            p_handler->g2d_fmt = G2D_RGB565;
            break;

        // avm project select this
        case V4L2_PIX_FMT_UYVY:
            p_handler->g2d_frame_size 
                = p_handler->v4l2_in_width * p_handler->v4l2_in_height * 2;
            // p_handler->g2d_fmt = G2D_UYVY;
            // FIXME: cap video is YUYV, but mipi not support this format, so set v4l2 format as UYUY
            // but in g2d convert, we must select actual pix format
            p_handler->g2d_fmt = G2D_YUYV;
            break;
    
        case V4L2_PIX_FMT_YUYV:
            p_handler->g2d_frame_size 
                = p_handler->v4l2_in_width * p_handler->v4l2_in_height * 2;
            p_handler->g2d_fmt = G2D_YUYV;
            break;
    
        case V4L2_PIX_FMT_YUV420:
            p_handler->g2d_frame_size 
                = p_handler->v4l2_in_width * p_handler->v4l2_in_height * 3 / 2;
            p_handler->g2d_fmt = G2D_I420;
            break;
    
        case V4L2_PIX_FMT_NV12:
            p_handler->g2d_frame_size 
                = p_handler->v4l2_in_width * p_handler->v4l2_in_height * 3 / 2;
            p_handler->g2d_fmt = G2D_NV12;
            break;
    
        default:
            VIDEOCAP_ERR("videocap_g2d_setup. unsupported format.\n");
            return VIDEO_CAPTURE_ERR;
    }

    // alloc g2d buffer and set to cap buffer
    for (i = 0; i < p_handler->num_cap_buffers; i++) 
    {
        //alloc physical contiguous memory for source image data
        if (p_handler->init.is_cache)
        {
            p_handler->g2d_cap_buffer[i] = g2d_alloc(p_handler->g2d_frame_size, 1);
        }
        else
        {
            p_handler->g2d_cap_buffer[i] = g2d_alloc(p_handler->g2d_frame_size, 0);
        }
        if (!p_handler->g2d_cap_buffer[i])
        {
            VIDEOCAP_ERR("videocap_g2d_setup. "
                "Fail to allocate physical memory for image buffer\n");
            return VIDEO_CAPTURE_ERR;
        }

        // start for vaddr, offset for paddr
        p_handler->cap_buffer[i].start = p_handler->g2d_cap_buffer[i]->buf_vaddr;
        p_handler->cap_buffer[i].offset = p_handler->g2d_cap_buffer[i]->buf_paddr;
        p_handler->cap_buffer[i].length = p_handler->g2d_frame_size;

        p_handler->mem_used += p_handler->g2d_frame_size;
    }

    return VIDEO_CAPTURE_OK;
}


int videocap_init(videocap_init_s *p_init, void **handler, int* input_width, int* input_height)
{
    int ret;
    
    if ((NULL == p_init) || (NULL == handler)
        || (NULL == input_width)
        || (NULL == input_height))
    {
        VIDEOCAP_ERR("videocap_init faild. param err\n");
        return VIDEO_CAPTURE_ERR;
    }
    if ((p_init->dev_index < 0) || (p_init->dev_index > 32)
        || (NULL == p_init->getback_frame_t)
        || (NULL == p_init->send_frame_t))
    {
        VIDEOCAP_ERR("videocap_init. param error\n");
        return VIDEO_CAPTURE_ERR;
    }

    videocap_state_s *p_handler 
        = (videocap_state_s *)malloc(sizeof(videocap_state_s));
    if (NULL == p_handler)
    {
        VIDEOCAP_ERR("videocap_init. malloc state struct faild\n");
        return VIDEO_CAPTURE_ERR;
    }
    memset(p_handler, 0, sizeof(videocap_state_s));
    p_handler->mem_used += sizeof(videocap_state_s);

    // cpy init param
    memcpy(&(p_handler->init), p_init, sizeof(videocap_init_s));
    // set cap num buffers as 4
    p_handler->num_cap_buffers = VIDEOCAP_NUMBER_BUFFERS; 

    // open video device
    sprintf(p_handler->cap_dev_str, "/dev/video%d", p_init->dev_index);
    p_handler->fd_cap_dev_v4l2 = open(p_handler->cap_dev_str, O_RDWR, 0);
    if (p_handler->fd_cap_dev_v4l2 < 0)
    {
        VIDEOCAP_ERR("videocap_init. unable to open %s\n", p_handler->cap_dev_str);
        return VIDEO_CAPTURE_ERR;
    }
    
    // video cap setup
    ret = videocap_cap_setup(p_handler);
    if (ret != VIDEO_CAPTURE_OK)
    {
        VIDEOCAP_ERR("videocap_init. videocap_cap_setup faild\n");
        return VIDEO_CAPTURE_ERR;
    }

    // set g2d init param. alloc g2d mem
    ret = videocap_g2d_setup(p_handler);
    if (ret != VIDEO_CAPTURE_OK)
    {
        close(p_handler->fd_cap_dev_v4l2);
        VIDEOCAP_ERR("videocap_init. videocap_cap_setup faild\n");
        return VIDEO_CAPTURE_ERR;
    }

    // status val init
    p_handler->last_ms = 0;
    p_handler->total_ms = 0;
    p_handler->total_cnt = 0;
    p_handler->total_cnt_since_start = 0;
    p_handler->v4l2_dq_err_cnt = 0;
    p_handler->prt_cnt = 0;

    p_handler->err_flag = 0;
    
    *handler = p_handler;
    *input_width = p_handler->v4l2_in_width;
    *input_height = p_handler->v4l2_in_height;

    VIDEOCAP_LOG("videocap_init success. video dev idx=%d\n", p_handler->init.dev_index);
    return VIDEO_CAPTURE_OK;
}



int videocap_start(void *handler)
{
    if (NULL == handler)
    {
        VIDEOCAP_ERR("videocap_start faild. param error\n");
        return VIDEO_CAPTURE_ERR;
    }
    videocap_state_s *p_handler = (videocap_state_s *)handler;

    // set all v4l2 userspace mem
    int i;
    struct v4l2_buffer buf;
    for (i = 0; i < p_handler->num_cap_buffers; i++) 
    {
        memset(&buf, 0, sizeof (buf));
        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_USERPTR;
        buf.index = i;
		buf.length = p_handler->cap_buffer[i].length;
		buf.m.userptr = (unsigned long)p_handler->cap_buffer[i].offset; // p_addr
		
        if (ioctl(p_handler->fd_cap_dev_v4l2, VIDIOC_QUERYBUF, &buf) < 0) 
        {
            VIDEOCAP_ERR("videocap_start. VIDIOC_QUERYBUF error\n");
            return VIDEO_CAPTURE_ERR;
        }
	}

    // queue all v4l2 buffer
    for (i = 0; i < p_handler->num_cap_buffers; i++) 
    {
        memset(&buf, 0, sizeof (buf));
        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_USERPTR;
        buf.index = i;
        // here should be physical address, good news is that this value is not used in the driver
        //buf.m.offset = (unsigned int)p_handler->cap_buffer[i].start;
        buf.m.offset = (unsigned int)p_handler->cap_buffer[i].offset;
        buf.length = (unsigned int)p_handler->cap_buffer[i].length;
        if (ioctl(p_handler->fd_cap_dev_v4l2, VIDIOC_QBUF, &buf) < 0) 
        {
            VIDEOCAP_ERR("videocap_start. VIDIOC_QBUF error\n");
            return VIDEO_CAPTURE_ERR;
        }
    }

    enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if (ioctl (p_handler->fd_cap_dev_v4l2, VIDIOC_STREAMON, &type) < 0) 
    {
        VIDEOCAP_ERR("videocap_start. VIDIOC_STREAMON error\n");
        return VIDEO_CAPTURE_ERR;
    }

    VIDEOCAP_LOG("videocap_start success. dev_idx=%d\n", p_handler->init.dev_index);
    return VIDEO_CAPTURE_OK;
}


static unsigned int videocap_get_time_ms()
{
    struct timeval  tv;
    gettimeofday(&tv, NULL);

    unsigned int time_in_mill 
        = (tv.tv_sec) * 1000 + (tv.tv_usec) / 1000;
    return time_in_mill;
}


static void videocap_inc_vcap_hb_code(int index)
{
    unsigned char *p_mem = NULL;
    int memsize = 0;
    comucmd_get_ext_share_mem(&p_mem, &memsize);
    if (NULL == p_mem)
    {
        return;
    }
    if ((index < 0) || (index > 4))
    {
        return;
    }
    ext_share_mem_s *p_share_s = (ext_share_mem_s *)p_mem;
    p_share_s->vcap_hb_code[index]++;
}


static int videocap_reopen_stream(videocap_state_s *p_handler)
{
    //enum v4l2_buf_type type;
    //type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    //ioctl(p_handler->fd_cap_dev_v4l2, VIDIOC_STREAMOFF, &type);

    // close capture v4l2 dev
    close(p_handler->fd_cap_dev_v4l2);
    // reopen v4l2 dev
    p_handler->fd_cap_dev_v4l2 = open(p_handler->cap_dev_str, O_RDWR, 0);
    if (p_handler->fd_cap_dev_v4l2 < 0)
    {
        VIDEOCAP_ERR("videocap_reopen_stream. unable to open %s\n", p_handler->cap_dev_str);
        return VIDEO_CAPTURE_ERR;
    }

    videocap_cap_setup(p_handler);

    videocap_start((void *)p_handler);

    return 0;
}


int videocap_process(void *handler)
{
    int i;

    if (NULL == handler)
    {
        VIDEOCAP_ERR("videocap_process. param error\n");
        return VIDEO_CAPTURE_ERR;
    }
    videocap_state_s *p_handler = (videocap_state_s *)handler;
    videocap_inc_vcap_hb_code(p_handler->init.dev_index);

    if (p_handler->err_flag)
    {
        usleep(10*1000);
        return VIDEO_CAPTURE_ERR;
    }
    if (p_handler->reopen_stream_flag)
    {
        VIDEOCAP_LOG("videocap_process. reopen stream idx=%d\n", p_handler->init.dev_index);
        videocap_reopen_stream(p_handler);
        p_handler->reopen_stream_flag = 0;
    }

    // get v4l2 frame back, and release it
    videocap_frame_s back_frame[6];
    int arr_size = 6;
    int getback_frame_error = 0;
    (*p_handler->init.getback_frame_t)(back_frame, &arr_size, &getback_frame_error);
    for (i = 0; i < arr_size; i++)
    {
        if (ioctl(p_handler->fd_cap_dev_v4l2, VIDIOC_QBUF, &(back_frame[i].v4l2_frame)) < 0) 
        {
            VIDEOCAP_ERR("videocap_process. getback_frame, VIDIOC_QBUF failed\n");
            return VIDEO_CAPTURE_ERR;
        }
    }

    // get v4l2 video frame
    struct v4l2_buffer capture_buf;
    memset(&capture_buf, 0, sizeof(capture_buf));
    capture_buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    capture_buf.memory = V4L2_MEMORY_USERPTR;
    if (ioctl(p_handler->fd_cap_dev_v4l2, VIDIOC_DQBUF, &capture_buf) < 0) 
    {
        VIDEOCAP_ERR("videocap_process. VIDIOC_DQBUF failed. dev idx=%d, errno=%d\n",
            p_handler->init.dev_index, errno);
        p_handler->v4l2_dq_err_cnt++;
        p_handler->err_flag++;

        return VIDEO_CAPTURE_ERR;
    }

    // status calc
    unsigned int cur_ms = videocap_get_time_ms();
    
    p_handler->total_cnt_since_start++;
    if (0 == p_handler->last_ms)
    {
        p_handler->last_ms = cur_ms;
        p_handler->total_ms = 0;
        p_handler->total_cnt = 0;
    }
    else
    {
        int det = cur_ms - p_handler->last_ms;
        if ((det > 0) && (det < 10*1000))  // 10*1000 for error
        {
            p_handler->total_ms += det;
            p_handler->total_cnt++;
            if (p_handler->total_ms >= 5000)    // every 5s stat once
            {
                p_handler->fps = p_handler->total_cnt * 1000.0 / p_handler->total_ms;
                p_handler->total_ms = 0;
                p_handler->total_cnt = 0;
#if VIDEO_CAPTURE_PRT_STAT == 1
                VIDEOCAP_LOG("idx=%d, fps=%f.. total_cnt=%d\n",
                    p_handler->init.dev_index,
                    p_handler->fps,
                    p_handler->total_cnt_since_start);
#else
                if (p_handler->prt_cnt++ <= 2)
                {
                    VIDEOCAP_LOG("idx=%d, fps=%f, total_cnt=%d\n",
                        p_handler->init.dev_index,
                        p_handler->fps,
                        p_handler->total_cnt_since_start);                    
                }
#endif
            }
        }
        else
        {
            p_handler->total_ms = 0;
            p_handler->total_cnt = 0;
        }
        p_handler->last_ms = cur_ms;
    }

    // call callback func
    videocap_frame_s frame;
    frame.width = p_handler->v4l2_in_width;
    frame.height = p_handler->v4l2_in_height;
    frame.format = p_handler->init.format;
    frame.cap_ts_ms = cur_ms;
    frame.is_chache = p_handler->init.is_cache;
    // set struct g2d_buf * as private data
    frame.p_private = (void *)(p_handler->g2d_cap_buffer[capture_buf.index]);
    memcpy(&(frame.v4l2_frame), &capture_buf, sizeof(struct v4l2_buffer));

    int send_frame_error = 0;
    (*p_handler->init.send_frame_t)(&frame, &send_frame_error);
    if (send_frame_error != 0)
    {
        VIDEOCAP_ERR("videocap_process. send_frame_t failed, error=%d\n", send_frame_error);
        // release v4l2 frame
        if (ioctl(p_handler->fd_cap_dev_v4l2, VIDIOC_QBUF, &capture_buf) < 0) 
        {
            VIDEOCAP_ERR("videocap_process. VIDIOC_QBUF failed\n");
            return VIDEO_CAPTURE_ERR;
        }
    }

    return VIDEO_CAPTURE_OK;
}


/**
* tmp suspend cap process
* @param pwr_status, 0 for shutdown cam power, 1 to setup camera power
*/
int videocap_pause_process(int pwr_status)
{
    int fd = -1;

    if (0 == pwr_status)
    {
        g_is_video_ok = 0;
        g_is_cam_pwr_closed = 1;
    }
    
    fd = open("/dev/max9286_chdev", O_RDWR);
    if (-1 == fd) 
    {
        VIDEOCAP_ERR("videocap_pause_process. open max9286_chdev device failed!\n");
        return 1;
    }
    ioctl(fd, MAX9286_CHRDEV_CAM_PWR, &pwr_status);
    close(fd);

    if (pwr_status)
    {
        g_is_cam_pwr_closed = 0;
        // g_is_video_ok is unknow yet
    }

    VIDEOCAP_LOG("videocap_pause_process. set cam power=%d, cur g_is_video_ok=%d\n", 
        pwr_status, g_is_video_ok);
    return 0;
}


int videocap_watchdog_process(void **handler, int dev_num)
{
    int i;
    unsigned int all_vaild_mask = 0x00u;
    
    if ((NULL == handler) || (dev_num <= 0))
    {
        VIDEOCAP_ERR("videocap_watchdog_process. param error\n");
        return VIDEO_CAPTURE_ERR;
    }
    for (i = 0; i < dev_num; i++)
    {
        all_vaild_mask |= (0x01u << i);
        if (NULL == handler[i])
        {
            VIDEOCAP_ERR("videocap_watchdog_process. param error2\n");
            return VIDEO_CAPTURE_ERR;
        }
    }

    while (1)
    {
        // read max9286 status reg
        //int fdd = open("/dev/max9286_chdev", O_RDWR);
        //ioctl(fdd, MAX9286_CHRDEV_READ_REG);
        //close(fdd);
        //usleep(1000*1000);
        //continue;

        // check if at least one camera is not vaild
        unsigned int lost_mask = 0x00;
        for (i = 0; i < dev_num; i++)
        {
            videocap_state_s *p_handler = (videocap_state_s *)(handler[i]);
            if (p_handler->err_flag)
            {
                lost_mask |= (0x01 << i);
            }
        }
        // if channel err_flag, or direct video_ok=0, will lead restart
        // g_is_video_ok flag is set when the camera is closed by manual close command(power off or show error image)
        if ((0x00 == lost_mask) && (g_is_video_ok))
        {
            usleep(1000*1000);
            continue;
        }
        g_is_video_ok = 0;
        if (0x00 == lost_mask)
        {
            VIDEOCAP_LOG("videocap_watchdog_process. camera is power off\n");  
        }
        else
        {
            VIDEOCAP_LOG("videocap_watchdog_process. video loss detected. "
                "lost_mask is 0x%x, is_video_ok=%d\n", lost_mask, g_is_video_ok);
        }
        
        // close all video stream first
        for (i = 0; i < dev_num; i++)
        {
            videocap_state_s *p_handler = (videocap_state_s *)(handler[i]);
            enum v4l2_buf_type type;
            type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
            ioctl(p_handler->fd_cap_dev_v4l2, VIDIOC_STREAMOFF, &type);
        }

        // wait all camera cap process is actual stopped
        while (1)
        {
            int stop_num = 0;
            for (i = 0; i < dev_num; i++)
            {
                videocap_state_s *p_handler = (videocap_state_s *)(handler[i]);
                if (p_handler->err_flag)
                {
                    stop_num++;
                }
            }
            if (stop_num == dev_num)
            {
                break;
            }
            else
            {
                usleep(100*1000);
            }
        }
        VIDEOCAP_LOG("videocap_watchdog_process. all stream is actual stoped\n");

        // first check if we manualy close camera power
        // manual power off is achieved through comu console command
        // in this case, will not do restart process until power is up
        if (g_is_cam_pwr_closed)
        {
            VIDEOCAP_LOG("videocap_watchdog_process. is_cam_pwr_closed=%d, "
                "wait until power up\n", g_is_cam_pwr_closed);
        }
        while (1)
        {
            if (g_is_cam_pwr_closed)
            {
                usleep(100*1000);
                continue;
            }
            break;
        }
        VIDEOCAP_LOG("videocap_watchdog_process. is_cam_pwr_closed=%d\n", g_is_cam_pwr_closed);

        // try reinit max9286 repeatedly until all sensor is detected
        usleep(100*1000);   // wait 100ms before restart process. in case cam power up, wait voaltge stable
        int fd = -1;
        int sensor_mask = 0;
        unsigned int all_link_status = 0x00;
        int reset_time_cnt = 0;

        int cam_link_status = 0x0;
        int cam_sync_status = 0x0;
        int check_error_cnt = 0;
        
        fd = open("/dev/max9286_chdev", O_RDWR);
        if (-1 == fd) 
        {
            VIDEOCAP_ERR("videocap_watchdog_process. open max9286_chdev device failed!\n");
            return 1;
        }
        unsigned int prt_intv_cnt = 0;
        while (1)
        {
            // dump max9286 reg values every 10 times
            if (prt_intv_cnt++ % 10 == 0)
            {
                ioctl(fd, MAX9286_CHRDEV_READ_REG);
            }
            
            ioctl(fd, MAX9286_CHRDEV_RESTART, &sensor_mask);
            ioctl(fd, MAX9286_CHRDEV_READ_STATUS, &all_link_status);
            VIDEOCAP_LOG("videocap_watchdog_process. sensor_mask = 0x%x."
                "all_link_status = 0x%x.\n", sensor_mask, all_link_status);

            //read max9286 status.
            if(check_error_cnt++ % 3 == 0)
            {
                usleep(500*1000);
                ioctl(fd, MAX9286_CHRDEV_READ_LINKSTA, &cam_link_status);
                ioctl(fd, MAX9286_CHRDEV_READ_SYNCSTA, &cam_sync_status);
                g_cam_link_status = cam_link_status & 0x0F;
                g_cam_sync_status = cam_sync_status & 0x0F;
                VIDEOCAP_LOG("videocap_watchdog_process. enter check cam status. cam_link_status = 0x%x."
                    "cam_sync_status = 0x%x.\n", cam_link_status, cam_sync_status);
            }

            g_all_link_status = all_link_status;
            if (sensor_mask != 0x0F)
            {
                g_vaild_mask = sensor_mask;
            }
            else
            {
                sensor_mask = (g_all_link_status >> 8) & 0x0F;
                g_vaild_mask = sensor_mask;
            }
            
            if ((unsigned int)sensor_mask != all_vaild_mask)
            {
                reset_time_cnt++;
                usleep(2000*1000);
                if (reset_time_cnt % 5 == 0)
                {
                    VIDEOCAP_LOG("videocap_watchdog_process. reset %d s but camera not vaild."
                        " current sensor_mask=%d\n", reset_time_cnt * 2, sensor_mask);
                }
                continue;
            }
            close(fd);

            //if cam is ok. set status ok
            g_cam_link_status = (g_all_link_status & 0x0F) | ((g_all_link_status >> 4) & 0x0F);
            g_cam_sync_status = (g_all_link_status >> 8) & 0x0F;
            VIDEOCAP_LOG("videocap_watchdog_process. exit check cam status. g_cam_link_status = 0x%x."
                    "g_cam_sync_status = 0x%x.\n", g_cam_link_status, g_cam_sync_status);

            VIDEOCAP_LOG("videocap_watchdog_process. reset success. time=%d s, sensor_mask=0x%x\n",
                reset_time_cnt * 2, sensor_mask);
            break;
        }
        
        // clear sync queue
        videosync_clear_queue();

        // start cap process again
        for (i = 0; i < dev_num; i++)
        {
            videocap_state_s *p_handler = (videocap_state_s *)(handler[i]);
            p_handler->reopen_stream_flag = 1;
            p_handler->err_flag = 0;
        }
        
        g_is_video_ok = 1;
    }
    
    return 0;
}


int videocap_get_devidx(void *handler, int *p_devidx)
{
    videocap_state_s *p_handler = (videocap_state_s *)handler;

    if ((NULL == p_handler) || (NULL == p_devidx))
    {
        VIDEOCAP_ERR("videocap_get_devidx. param error\n");
        return VIDEO_CAPTURE_ERR;
    }

    *p_devidx = p_handler->init.dev_index;
    return VIDEO_CAPTURE_OK;
}


int videocap_get_status(void *handler, videocap_status_s *p_status)
{
    videocap_state_s *p_handler = (videocap_state_s *)handler;

    if ((NULL == p_handler) || (NULL == p_status))
    {  
        VIDEOCAP_ERR("videocap_get_status. param error\n");
        return VIDEO_CAPTURE_ERR;
    }

    p_status->dev_idx = p_handler->init.dev_index;
    p_status->fps = p_handler->fps;
    p_status->frame_cnt = p_handler->total_cnt_since_start;
    p_status->v4l2_dq_err_cnt = p_handler->v4l2_dq_err_cnt;
    p_status->mem_kb = (int)(p_handler->mem_used / 1024.0);

    return VIDEO_CAPTURE_OK;
}


int videocap_deinit(void *handler)
{
    int i;
    videocap_state_s *p_handler = (videocap_state_s *)handler;

    if (NULL == p_handler)
    {  
        VIDEOCAP_ERR("videocap_deinit. param error\n");
        return VIDEO_CAPTURE_ERR;
    }

    // close capture v4l2 dev
    close(p_handler->fd_cap_dev_v4l2);

    // free g2d buffer
    for (i = 0; i < p_handler->num_cap_buffers; i++)
    {
        g2d_free(p_handler->g2d_cap_buffer[i]);
    }

    // free state mem
    free(p_handler);

    return VIDEO_CAPTURE_OK;
}

#endif


void videocap_get_video_ok(int *p_is_ok, unsigned int *p_mask, int *p_cam_pwr)
{
    if (p_is_ok != NULL)
    {
        *p_is_ok = g_is_video_ok;
    }
    if (p_mask != NULL)
    {
        *p_mask = g_vaild_mask;
    }
    if (p_cam_pwr != NULL)
    {
        if (g_is_cam_pwr_closed)
        {
            *p_cam_pwr = 0;
        }
        else
        {
            *p_cam_pwr = 0;
        }
    }
}

void videocap_set_video_ok(int flag)
{
    g_is_video_ok = flag;
}

void videocap_get_cam_link_status(unsigned int *p_status)
{
    if (p_status != NULL)
    {
        *p_status = g_cam_link_status;
    }
}

void videocap_get_cam_sync_status(unsigned int *p_status)
{
    if (p_status != NULL)
    {
        *p_status = g_cam_sync_status;
    }
}



