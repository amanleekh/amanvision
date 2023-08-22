
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <float.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <malloc.h>

#include "common_def.h"
#include "gpu_egl.h"
#include "gpu_shader.h"
#include "video_sync.h"
#include "aman_gpu.h"
#include "aman_gpu_table.h"
//#include "minIni.h"
#include "ini_config.h"

// Include GLM core features
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
#include <glm/trigonometric.hpp>

// Include GLM extensions
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#if GLOBAL_RUN_ENV_DESKTOP == 1
#include <opencv2/opencv.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#endif

// we usd txtviv in imx6, not in desktop env
#if GLOBAL_RUN_ENV_DESKTOP == 0

#include "g2d.h"

#define aman_GPU_FUNC_SET(_val, type, _str)    \
    if (_val == NULL)    \
    {   \
        _val = (type)   \
            eglGetProcAddress(_str);    \
        if (_val == NULL)    \
        {   \
            amanGPU_ERR("amanGpu::aman_gpu_env_init.Required extension not supported.\n");  \
            return 1;   \
        }   \
    }

typedef void (GL_APIENTRY *PFNGLTEXDIRECTVIV) (GLenum Target, GLsizei Width, GLsizei Height, GLenum Format, GLvoid ** Pixels);
typedef void (GL_APIENTRY *PFNGLTEXDIRECTVIVMAP)(GLenum Target,GLsizei Width, GLsizei Height, GLenum Format, GLvoid ** Logical, const GLuint *Physical);
typedef void (GL_APIENTRY *PFNGLTEXDIRECTINVALIDATEVIV) (GLenum Target);

static PFNGLTEXDIRECTVIV g_pFNglTexDirectVIV = NULL;
static PFNGLTEXDIRECTVIVMAP g_pFNglTexDirectVIVMap = NULL;
static PFNGLTEXDIRECTINVALIDATEVIV g_pFNglTexDirectInvalidateVIV = NULL;

#endif

#define IF_IN_RANGE(x, begin, end)	((x>=begin)&&(x<end))

// load and set aman data
#define OBJ_aman_SET_MEM(_str1, _str2)                       \
            *p_mem_start++ = (float)st._str1.x;             \
            *p_mem_start++ = (float)st._str1.y;             \
            *p_mem_start++ = (float)st._str1.z;             \
            *p_mem_start++ = (float)st._str1.Tex_x;         \
            *p_mem_start++ = (float)st._str1.Tex_y;         \
            *p_mem_start++ = (float)st._str2.Tex_x;         \
            *p_mem_start++ = (float)st._str2.Tex_y;         \
            *p_mem_start++ = (float)st._str2.alpha;         \
            *p_mem_start++ = (float)st.PosID + 0.1f

// rotate z min deg
#define Z_DEG_LIMIT_MIN             (2.0)


/**
* load aman table.db into this struct
*/
typedef struct _aman_buffer_data_struct_
{
    float x;    /**< vert x,y,z */
    float y;
    float z;
    float tex_x;    /**< tex, xy */
    float tex_y;    
    float tex_ex_x;
    float tex_ex_y;
    float alpha_ex;         /**< tex_ex's mix alpha */
    float choose_tex_id;    /**< alg choose tex id val */
}aman_buffer_data_s;


typedef struct _camera_data_struct_
{
    float x;
    float y;
    float z;
    float tex_x;
    float tex_y;
    float choose_tex_id;
}camera_data_s;


/**< static data init*/
QMutex amanGpu::m_s_mutex;
int amanGpu::m_s_inst_id = 0;
int amanGpu::m_s_is_share_lb = 0; 
int amanGpu::m_s_is_share_lb_load_sts = 0;
int amanGpu::m_s_is_share_vbo = 0;
int amanGpu::m_s_is_share_vbo_load_sts = 0;
int amanGpu::m_s_is_share_tex = 0;
int amanGpu::m_s_is_share_tex_load_sts = 0;
amangpu_share_vbo_s amanGpu::m_s_hare_vbo;
amangpu_share_texure_s amanGpu::m_s_share_tex;
amangpu_share_ltbalance_s amanGpu::m_s_share_lb;


/**
* @param car_type, car type, 1 for lk01, 2 for 02, 3 for 03
* @param proj_width, opengl projection marrix width, usually the width of parent's widget
* @param proj_height
* @param tex_width, input texture image width, usually camera image width
* @param tex_height
* @param p_get_func, get tex image func and release func
* @param is_share_context_vbo, is param widget instances is share with each other, in this case, vbo & texture only create and init once
* @param is_share_context_texture, texture share
*/
amanGpu::amanGpu(int car_type, int proj_width, int proj_height, 
    int proj_wideangle_width, int proj_wideangle_height,
    int tex_width, int tex_height,
    get_chn_image_addr_cbfuc *p_get_func,
    rel_chn_image_addr_cbfuc *p_release_func,
    int is_share_context_vbo, int is_share_context_texture)
{
    int i;

    // stitch gles program attr and uniform loc
    m_shader_handler = NULL;
    m_loc_vertices = -1;
    m_loc_texcoord = -1;
    m_loc_texcoord_ex = -1;
    m_loc_choose_tex_id = -1;
    m_loc_alpha_ex = -1;
    m_loc_transform_mat = -1;
    m_loc_sampler = -1;
    m_loc_sampler1 = -1;
    m_loc_sampler2 = -1;
    m_loc_sampler3 = -1;
    m_loc_colork = -1;

    // ingle or two camera, gles program attr and uniform loc
    m_cam_shader_handler = NULL;
    m_cam_loc_vertices = -1;
    m_cam_loc_texcoord = -1;
    m_cam_loc_sampler = -1;
    m_cam_loc_sampler1 = -1;
    m_cam_loc_choose_tex_id = -1;
    m_cam_loc_transform_mat = -1;

    // view or rotate or mvp matrix
    m_gl_proj = glm::mat4(1.0f);
    m_gl_view = glm::mat4(1.0f);
    m_gl_model = glm::mat4(1.0f);
    m_mvp = m_gl_proj * m_gl_view * m_gl_model;
    m_projWidth = proj_width;
    m_projHeight = proj_height;
    m_proj_wideangle_width = proj_wideangle_width;
    m_proj_wideangle_height = proj_wideangle_height;
    m_current_view = aman_VIEW_UNKNOW;
#if GLOBAL_RUN_ENV_DESKTOP == 1
    m_is_use_bmp = 1;
#else
    m_is_use_bmp = ini_get_device_use_bmp_file();
#endif
    if (1 == m_is_use_bmp)
    {
        amanGPU_LOG("amanGpu::amanGpu. is use image file=%d\n", m_is_use_bmp);
    }
    m_car_type = car_type;

    m_rotate_center_x = 0.0f;
    m_rotate_center_y = 0.0f;
    m_rotate_top_r = 0.0f;
    m_rotate_bottom_r = 0.0f;
    m_rotate_max_z_angle = 0.0f;
    m_rotate_top_height = 0.0f;
    m_rotate_bottom_height = 0.0f;
    m_rotate_auto_top_r = 0.0f;
    m_rotate_auto_bottom_r = 0.0f;
    m_rotate_auto_top_height = 0.0f;
    m_rotate_auto_bottom_height = 0.0f;
    m_2d_front_det = 0.0f;
    m_2d_rear_det = 0.0f;
    m_2d_height = 0.0f;
    m_curview_rot_ang = 0.0f;
    m_curview_rot_ang_z = 0.0f;
    m_rotate_r = 0.0f;

    // 3d view synmic trans
    m_is_in_view_trans = 0;  
    m_dyn_trans_dst_view = aman_VIEW_UNKNOW; 
    m_dyn_trans_dst_deg = 0.0f;
    m_dyn_trans_frame_cnt = 0;

    if (ini_get_d2_vrate(&m_2d_f_clip_rate, &m_2d_r_clip_rate) != 0)
    {
        m_2d_f_clip_rate = 1.0f;
        m_2d_r_clip_rate = 1.0f;
        amanGPU_LOG("amanGpu::amanGpu. get clip faild, use default\n");
    }

    // init texture init value
    m_texture_width = tex_width;
    m_texture_height = tex_height;
    for (i = 0; i < 4; i++)
    {
        m_texture_id[i] = -1;
        m_texture_vaddr[i] = NULL;
        m_texture_paddr[i] = 0;
        m_camera_uniform2tex[i] = -1;
        m_camera_uniform2tex_bmp[i] = -1;
        m_camera_tex2uniform[i] = -1;
        m_camera_tex2uniform_bmp[i] = -1;
    }
    m_num_2_texture_enum[0] = GL_TEXTURE0;
    m_num_2_texture_enum[1] = GL_TEXTURE1;
    m_num_2_texture_enum[2] = GL_TEXTURE2;
    m_num_2_texture_enum[3] = GL_TEXTURE3;
    m_tex_uid_l = 0;
    m_tex_uid_h = 0;
    m_p_get_imgfunc = p_get_func;
    m_p_rel_imgfunc = p_release_func;

    // stitch view .data file name and mem
    m_p_db_file_name = (char *)("aman_qt_app_res/data/Table.db");
    m_p_vshader_name = (char *)("aman_qt_app_res/shader/vshader_gpu.glsl");
    m_p_fshader_name = (char *)("aman_qt_app_res/shader/fshader_gpu.glsl");
    m_p_aman_mem = NULL;
    m_p_aman_idx_mem = NULL;
    m_aman_mem_st_num = 0;

    // single or two camea view, .data file name
    m_p_cam_vshader_name = (char *)("aman_qt_app_res/shader/vshader_camera.glsl");
    m_p_cam_fshader_name = (char *)("aman_qt_app_res/shader/fshader_camera.glsl");
    m_p_front_db_file_name = (char *)("aman_qt_app_res/data/table_cam_front.db");
    m_p_rear_db_file_name = (char *)("aman_qt_app_res/data/table_cam_rear.db");
    m_p_two_db_file_name = (char *)("aman_qt_app_res/data/table_cam_two.db");
    m_p_fisheye_file_name = (char *)("aman_qt_app_res/data/fisheye_clip.db");

    // opengl vbo
    m_aman_table_vbo = -1; 
    m_aman_idx_vbo = -1;
    m_aman_single_front_camera_vbo = -1;
    m_aman_single_rear_camera_vbo = -1;
    m_aman_two_camera_vbo = -1;
    m_aman_fisheye_front_vbo = -1; 
    m_aman_fisheye_rear_vbo = -1;
    m_aman_fisheye_clip_front_vbo = -1; 
    m_aman_fisheye_clip_rear_vbo = -1;

    // single or two camera mem
    m_p_front_cam_mem = NULL;
    m_p_rear_cam_mem = NULL;
    m_p_two_cam_mem = NULL;
    m_front_cam_point_num = 0;
    m_rear_cam_point_num = 0;
    m_two_cam_point_num = 0;

#if GLOBAL_RUN_ENV_DESKTOP == 1
    m_is_use_bmp = 1;
#else
    m_is_use_bmp = ini_get_device_use_bmp_file();
#endif
    if (1 == m_is_use_bmp)
    {
        amanGPU_LOG("amanGpu::amanGpu. is use image file=%d\n", m_is_use_bmp);
    }

    // for 3d stitch view light balance
    for (i = 0; i < 8; i++)
    {
        m_p_light_area[i] = NULL;
        m_light_area_num[i] = 0;
        m_light_avg_rgb[i][0] = 0.0;
        m_light_avg_rgb[i][1] = 0.0;
        m_light_avg_rgb[i][2] = 0.0;
        m_light_avg_y[i] = 0.0;
    }
    m_light_prc_cnt = 0;

    // light balance
    for (i = 0; i < 4; i++)
    {
        m_color_k[i] = 1.0f;
        m_dst_color_k[i] = 1.0f;
    }
    m_color_limit_det = 0.3f;
    m_is_do_lb_process = 0;

    // hide param
    m_is_hide_change = 0;
    m_is_hide_right = 0;
    m_is_hide_front = 0;
    m_is_hide_left = 0;
    m_is_hide_rear = 0;

    // fisheye
    m_fisheye_front.xl = 0;
    m_fisheye_front.xr = tex_width;
    m_fisheye_front.yt = 0;
    m_fisheye_front.yb = tex_height;
    m_fisheye_front.act_xl = 0;
    m_fisheye_front.act_xr = tex_width;
    m_fisheye_front.cx = tex_width / 2;
    m_fisheye_front.cy = tex_height / 2;
    m_fisheye_rear.xl = 0;
    m_fisheye_rear.xr = tex_width;
    m_fisheye_rear.yt = 0;
    m_fisheye_rear.yb = tex_height;
    m_fisheye_rear.act_xl = 0;
    m_fisheye_rear.act_xr = tex_width;
    m_fisheye_rear.cx = tex_width / 2;
    m_fisheye_rear.cy = tex_height / 2;    
    // -1.0f, 1.0f, -1.0f, 1.0f
    // fish view, front and rear
    m_fisheye_front_orth[0] = -1.0f;
    m_fisheye_front_orth[1] = 1.0f;
    m_fisheye_front_orth[2] = -1.0f;
    m_fisheye_front_orth[3] = 1.0f;
    m_fisheye_rear_orth[0] = -1.0f;
    m_fisheye_rear_orth[1] = 1.0f;
    m_fisheye_rear_orth[2] = -1.0f;
    m_fisheye_rear_orth[3] = 1.0f;  
    // fish_clip view
    m_fisheye_clip_front_orth[0] = -1.0f;
    m_fisheye_clip_front_orth[1] = 1.0f;
    m_fisheye_clip_front_orth[2] = -1.0f;
    m_fisheye_clip_front_orth[3] = 1.0f;
    m_fisheye_clip_rear_orth[0] = -1.0f;
    m_fisheye_clip_rear_orth[1] = 1.0f;
    m_fisheye_clip_rear_orth[2] = -1.0f;
    m_fisheye_clip_rear_orth[3] = 1.0f;

    // etc
    m_inst_id = amanGpu::m_s_inst_id++;

    // static val init
    if (0 == m_inst_id)
    {
        amanGpu::m_s_is_share_vbo = is_share_context_vbo;
        amanGpu::m_s_is_share_tex = is_share_context_texture;
        amanGpu::m_s_is_share_lb = 1;   // default light balance is share
        amanGpu::m_s_is_share_vbo_load_sts = 0;
        amanGpu::m_s_is_share_tex_load_sts = 0;
        amanGpu::m_s_is_share_lb_load_sts = 0;
        // lb
        for (i = 0; i < 4; i++)
        {
            amanGpu::m_s_share_lb.m_color_k[i] = 1.0f;
        }
        amanGpu::m_s_share_lb.m_fps = DEFAULT_FPS;
        // vbo
        amanGpu::m_s_hare_vbo.m_aman_table_vbo = -1;
        amanGpu::m_s_hare_vbo.m_aman_idx_vbo = -1;
        amanGpu::m_s_hare_vbo.m_aman_single_front_camera_vbo = -1;
        amanGpu::m_s_hare_vbo.m_aman_single_rear_camera_vbo = -1;
        amanGpu::m_s_hare_vbo.m_aman_two_camera_vbo = -1;
        amanGpu::m_s_hare_vbo.m_aman_fisheye_front_vbo = -1;
        amanGpu::m_s_hare_vbo.m_aman_fisheye_rear_vbo = -1;
        amanGpu::m_s_hare_vbo.m_aman_fisheye_clip_front_vbo = -1;
        amanGpu::m_s_hare_vbo.m_aman_fisheye_clip_rear_vbo = -1;
        amanGpu::m_s_hare_vbo.m_aman_mem_st_num = 0;
        // tex
        for (i = 0; i < 4; i++)
        {
            amanGpu::m_s_share_tex.m_texture_id[i] = -1;
            amanGpu::m_s_share_tex.m_texture_vaddr[i] = NULL;
            amanGpu::m_s_share_tex.m_texture_paddr[i] = 0;
        }
        amanGpu::m_s_share_tex.m_tex_uid_h = 0;
        amanGpu::m_s_share_tex.m_tex_uid_l = 0;
        amanGpu::m_s_share_tex.m_tex_updated = 0;
        for (i = 0; i < 4; i++)
        {
            amanGpu::m_s_share_tex.m_video_fc[i] = 1;
        }
    }

    aman_LOG("amanGpu::amanGpu. share_vbo=%d, share_tex=%d, share_lb=%d, inst_id=%d\n",
        amanGpu::m_s_is_share_vbo, amanGpu::m_s_is_share_tex, amanGpu::m_s_is_share_lb,
        m_inst_id);
}


/**
 * @brief amanGpu::~amanGpu
 */
amanGpu::~amanGpu()
{
}


/** 
* @brief load .data file, init opengl env, etc 
* @param is_use_fbo, if 1, use opengl fbo texture as the output of the gl program, and then output to the FB
*        if 1, gl output->fbo texture->linux fb, this is usefull when you want to get gl output for post process(eg, alg use stitch image as input)
*        if 0, gl output->linux fb
*/
int amanGpu::amangpu_init()
{
    int i;
    int ret;
    
    // env init, compile shader and create opengl program
    ret = aman_gpu_env_init();
    if (ret != 0)
    {
        amanGPU_ERR("amanGpu::amangpu_init. aman_gpu_env_init faild\n");
        return 1;
    }

    // gen vbo
    ret = aman_gpu_vbo_init();
    if (ret != 0)
    {
        amanGPU_ERR("amanGpu::amangpu_init. aman_gpu_vbo_init faild\n");
        return 1;
    }

    // gen texture
    ret = aman_gpu_texture_init();
    if (ret != 0)
    {
        amanGPU_ERR("amanGpu::amangpu_init. aman_gpu_texture_init faild\n");
        return 1;
    }

    // load fisheye data, fish eye data is not used for vbo, but for opengl es clip matrix
    // so fisheye data can't share between context
    ret = aman_gpu_load_db_fisheye(m_p_fisheye_file_name);
    if (ret != 0)
    {
        amanGPU_ERR("amanGpu::amangpu_init. load fisheye faild\n");
        return 1;
    }
    
	// load image data
	if (1 == m_is_use_bmp)
	{
        m_img1.load("aman_qt_app_res/data/cap_right.jpg");
        m_img2.load("aman_qt_app_res/data/cap_front.jpg");
        m_img3.load("aman_qt_app_res/data/cap_left.jpg");
        m_img4.load("aman_qt_app_res/data/cap_rear.jpg");
        m_img1 = m_img1.convertToFormat(QImage::Format_RGBA8888);
        m_img2 = m_img2.convertToFormat(QImage::Format_RGBA8888);
        m_img3 = m_img3.convertToFormat(QImage::Format_RGBA8888);
        m_img4 = m_img4.convertToFormat(QImage::Format_RGBA8888);

        QImage *p_qimage[4] = {&m_img1, &m_img2, &m_img3, &m_img4};
        for (i = 0; i < 4; i++)
        {
            m_texture_vaddr[i] = (*p_qimage[i]).bits();
            m_texture_paddr[i] = 0;
        }
	}
    
    // set init eye pos
    ret = aman_gpu_postion_init();
    if (ret != 0)
    {
        amanGPU_ERR("amanGpu::amangpu_init. aman_gpu_postion_init faild\n");
        return 1;
    }

    // light balance init
    ret = aman_gpu_light_balance_init();
    if (ret != 0)
    {
        amanGPU_ERR("amanGpu::amangpu_init. aman_gpu_light_balance_init faild\n");
        return 1;
    }

    amanGPU_LOG("amanGpu::amangpu_init success, inst_id=%d\n", m_inst_id);
    return 0;
}



int amanGpu::aman_gpu_init_aman_table_data()
{
    int ret;

    unsigned int start_time = get_time_ms();

    // load aman stitch table data
    ret = load_db_table(m_p_db_file_name, &m_p_aman_mem, 
        &m_p_aman_idx_mem, &m_aman_mem_st_num);
    if (ret != 0)
    {
        amanGPU_ERR("amanGpu::aman_gpu_init_aman_table_data. load stitch data for stitch faild\n");
        return 1;
    }

    // load single camera data(with fish fisheye correction)
#if SUPPORT_FRONT_REAR_CALIB == 1
    ret = aman_gpu_load_db_camera(m_p_front_db_file_name, &m_p_front_cam_mem, &m_front_cam_point_num);
    if (ret != 0)
    {
        amanGPU_ERR("amanGpu::aman_gpu_init_aman_table_data. load cam front(fisheye correction) faild\n");
        return 1;
    }
    ret = aman_gpu_load_db_camera(m_p_rear_db_file_name, &m_p_rear_cam_mem, &m_rear_cam_point_num);
    if (ret != 0)
    {
        amanGPU_ERR("amanGpu::aman_gpu_init_aman_table_data. load cam rear(fisheye correction) faild\n");
        return 1;
    }
#endif

    // load two cam data(left and right, with fish fisheye correction)
    ret = aman_gpu_load_db_camera(m_p_two_db_file_name, &m_p_two_cam_mem, &m_two_cam_point_num);
    if (ret != 0)
    {
        amanGPU_ERR("amanGpu::aman_gpu_init_aman_table_data. load cam two(fisheye correction) faild\n");
        return 1;
    }

    unsigned int end_time = get_time_ms();
    int det_time = end_time - start_time;
    
    aman_LOG("amanGpu::aman_gpu_init_aman_table_data. inst_id=%d, cost %d ms, struct num=%d\n",
        m_inst_id, det_time, m_aman_mem_st_num);

    return 0;
}


unsigned int amanGpu::get_time_ms()
{
    struct timeval  tv;
    gettimeofday(&tv, NULL);

    unsigned int time_in_mill 
        = (tv.tv_sec) * 1000 + (tv.tv_usec) / 1000;
    return time_in_mill;
}


/**< u64++ */
void amanGpu::uint64_inc(unsigned int *p_h, unsigned int *p_l)
{
    (*p_l)++;
    if (0x00 == *p_l)
    {
        (*p_h)++;
    }
}


/**< u64 comp */
int amanGpu::uint64_cmp(unsigned int *p1_h, unsigned int *p1_l, unsigned int *p2_h, unsigned int *p2_l)
{
    if (*p1_h > *p2_h)
    {
        return 1;
    }
    else if (*p1_h < *p2_h)
    {
        return -1;
    }
    else if (*p1_l > *p2_l)
    {
        return 1;
    }
    else if (*p1_l < *p2_l)
    {
        return -1;
    }
    else
    {
        return 0;
    }
}


/**< u64 set */
void amanGpu::uint64_set(unsigned int *p1_h, unsigned int *p1_l, unsigned int *p2_h, unsigned int *p2_l)
{
    *p1_h = *p2_h;
    *p1_l = *p2_l;
}


/**< lock */
void amanGpu::share_lock()
{
    //qDebug("--lock %s\n", __FUNCTION__);
    amanGpu::m_s_mutex.lock();
}


/**< unlock */
void amanGpu::share_unlock()
{
    //qDebug("--unlock %s\n", __FUNCTION__);
    amanGpu::m_s_mutex.unlock();
}


/**
 * @brief amanGpu::aman_gpu_env_init
 * @return
 */
int amanGpu::aman_gpu_env_init()
{
    int ret;

    // get func ptr
#if GLOBAL_RUN_ENV_DESKTOP == 0
#define aman_GPU_FUNC_SET(_val, type, _str)    \
    if (_val == NULL)    \
    {   \
        _val = (type)   \
            eglGetProcAddress(_str);    \
        if (_val == NULL)    \
        {   \
            amanGPU_ERR("amanGpu::aman_gpu_env_init.Required extension not supported.\n");  \
            return 1;   \
        }   \
    }

    aman_GPU_FUNC_SET(g_pFNglTexDirectVIV, PFNGLTEXDIRECTVIV, "glTexDirectVIV")
    aman_GPU_FUNC_SET(g_pFNglTexDirectVIVMap, PFNGLTEXDIRECTVIVMAP, "glTexDirectVIVMap")
    aman_GPU_FUNC_SET(g_pFNglTexDirectInvalidateVIV, PFNGLTEXDIRECTINVALIDATEVIV, "glTexDirectInvalidateVIV")
#undef aman_GPU_FUNC_SET
#endif

    // load shader and compile for stitch 3d view, and fisheye correction view
    ret = shader_load(m_p_vshader_name, m_p_fshader_name, &m_shader_handler);  
    if (ret != 0)
    {
        amanGPU_ERR("amanGpu::aman_gpu_env_init. shader_load_str faild.\n");
        return 1;
    }

    GLuint hProgram = shader_get_program(m_shader_handler);

    // get gles value location
    m_loc_vertices = glGetAttribLocation(hProgram, "vsVertex");
    m_loc_texcoord = glGetAttribLocation(hProgram, "vsTexcoord");
    m_loc_texcoord_ex = glGetAttribLocation(hProgram, "vsTexcoord_ex");
    m_loc_alpha_ex = glGetAttribLocation(hProgram, "vsAlpha_ex");
    m_loc_choose_tex_id = glGetAttribLocation(hProgram, "vsChooseTexId");

    m_loc_transform_mat = glGetUniformLocation(hProgram, "vsTransformMatrix");

    m_loc_sampler = glGetUniformLocation(hProgram, "fsSampler");
    m_loc_sampler1 = glGetUniformLocation(hProgram, "fsSampler1");
    m_loc_sampler2 = glGetUniformLocation(hProgram, "fsSampler2");
    m_loc_sampler3 = glGetUniformLocation(hProgram, "fsSampler3");
    m_loc_colork = glGetUniformLocation(hProgram, "colorK");

	// load camera shader and compile
    ret = shader_load(m_p_cam_vshader_name, m_p_cam_fshader_name, &m_cam_shader_handler);  
    if (ret != 0)
    {
        amanGPU_ERR("amanGpu::aman_gpu_env_init. shader_load_str camera faild.\n");
        return 1;
    }

	GLuint hCamProgram = shader_get_program(m_cam_shader_handler);

	// get gles value location
	m_cam_loc_vertices = glGetAttribLocation(hCamProgram, "vsVertex");
	m_cam_loc_texcoord = glGetAttribLocation(hCamProgram, "vsTexcoord");
	m_cam_loc_choose_tex_id = glGetAttribLocation(hCamProgram, "vsChooseTexId");
	m_cam_loc_sampler = glGetUniformLocation(hCamProgram, "fsSampler");
	m_cam_loc_sampler1 = glGetUniformLocation(hCamProgram, "fsSampler1");

	m_cam_loc_transform_mat = glGetUniformLocation(hCamProgram, "vsTransformMatrix");

    // enable gl feature
    glEnable(GL_DEPTH_TEST);    // for deep test
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    return 0;
}



int amanGpu::aman_gpu_load_data_vbo()
{
    // aman stitch data vert vbo
    if (NULL == m_p_aman_mem)
    {
        amanGPU_ERR("amanGpu::aman_gpu_load_data_vbo. m_p_aman_mem is null\n");
        return 1;
    }
    glGenBuffers(1, &m_aman_table_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, m_aman_table_vbo);
    glBufferData(GL_ARRAY_BUFFER, m_aman_mem_st_num * sizeof(aman_buffer_data_s), m_p_aman_mem, GL_STATIC_DRAW);

    // aman index buffer
    if (NULL == m_p_aman_idx_mem)
    {
        amanGPU_ERR("amanGpu::aman_gpu_load_data_vbo. m_p_aman_idx_mem is null\n");
        return 1;
    }
    glGenBuffers(1, &m_aman_idx_vbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_aman_idx_vbo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_aman_mem_st_num / 4 * 6 * sizeof(unsigned int), m_p_aman_idx_mem, GL_STATIC_DRAW);

    // single front camera
#if SUPPORT_FRONT_REAR_CALIB == 1
    if (NULL == m_p_front_cam_mem)
    {
        amanGPU_ERR("amanGpu::aman_gpu_load_data_vbo. m_p_front_cam_mem(for fisheye correction) is null\n");
        return 1;
    }
    glGenBuffers(1, &m_aman_single_front_camera_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, m_aman_single_front_camera_vbo);
    glBufferData(GL_ARRAY_BUFFER, m_front_cam_point_num * sizeof(camera_data_s), m_p_front_cam_mem, GL_STATIC_DRAW);
#endif

    // single rear camera vbo buffer
#if SUPPORT_FRONT_REAR_CALIB == 1
    if (NULL == m_p_rear_cam_mem)
    {
        amanGPU_ERR("amanGpu::aman_gpu_load_data_vbo. m_p_rear_cam_mem(for fisheye correction) is null\n");
        return 1;
    }
    glGenBuffers(1, &m_aman_single_rear_camera_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, m_aman_single_rear_camera_vbo);
    glBufferData(GL_ARRAY_BUFFER, m_rear_cam_point_num * sizeof(camera_data_s), m_p_rear_cam_mem, GL_STATIC_DRAW);     
#endif

    // two camera, left camera and right camera
    if (NULL == m_p_two_cam_mem)
    {
        amanGPU_ERR("amanGpu::aman_gpu_load_data_vbo. m_p_two_cam_mem is null\n");
        return 1;
    }
    glGenBuffers(1, &m_aman_two_camera_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, m_aman_two_camera_vbo);
    glBufferData(GL_ARRAY_BUFFER, m_two_cam_point_num * sizeof(camera_data_s), m_p_two_cam_mem, GL_STATIC_DRAW);

    // fisheye front data(wide angle)
    // opengl texture coord  origin is left-bottom, so this array tex coord is upside down
    static camera_data_s fisheye_data_front[] = 
    {
        {-1.0f, -1.0f,  0.0f,  0.0,  1.0,  0.1f},
        { 1.0f, -1.0f,  0.0f,  1.0,  1.0,  0.1f},
        {-1.0f,  1.0f,  0.0f,  0.0,  0.0,  0.1f},

        { 1.0f, -1.0f,  0.0f,  1.0,  1.0,  0.1f},
        { 1.0f,  1.0f,  0.0f,  1.0,  0.0,  0.1f},
        {-1.0f,  1.0f,  0.0f,  0.0,  0.0,  0.1f}
    };
    glGenBuffers(1, &m_aman_fisheye_front_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, m_aman_fisheye_front_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(fisheye_data_front), fisheye_data_front, GL_STATIC_DRAW);

    // fisheye rear data 
    static camera_data_s fisheye_data_rear[] = 
    {
        {-1.0f, -1.0f,  0.0f,  0.0,  1.0,  2.1f},
        { 1.0f, -1.0f,  0.0f,  1.0,  1.0,  2.1f},
        {-1.0f,  1.0f,  0.0f,  0.0,  0.0,  2.1f},

        { 1.0f, -1.0f,  0.0f,  1.0,  1.0,  2.1f},
        { 1.0f,  1.0f,  0.0f,  1.0,  0.0,  2.1f},
        {-1.0f,  1.0f,  0.0f,  0.0,  0.0,  2.1f}
    };
    glGenBuffers(1, &m_aman_fisheye_rear_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, m_aman_fisheye_rear_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(fisheye_data_rear), fisheye_data_rear, GL_STATIC_DRAW);

    // fisheye clip front
    static camera_data_s fisheye_clip_data_front[] = 
    {
        {-1.0f, -1.0f,  0.0f,  0.0,  1.0,  0.1f},
        { 1.0f, -1.0f,  0.0f,  1.0,  1.0,  0.1f},
        {-1.0f,  1.0f,  0.0f,  0.0,  0.0,  0.1f},

        { 1.0f, -1.0f,  0.0f,  1.0,  1.0,  0.1f},
        { 1.0f,  1.0f,  0.0f,  1.0,  0.0,  0.1f},
        {-1.0f,  1.0f,  0.0f,  0.0,  0.0,  0.1f}
    };    
    glGenBuffers(1, &m_aman_fisheye_clip_front_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, m_aman_fisheye_clip_front_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(fisheye_clip_data_front), fisheye_clip_data_front, GL_STATIC_DRAW);

    // fisheye clip rear
    static camera_data_s fisheye_clip_data_rear[] = 
    {
        {-1.0f, -1.0f,  0.0f,  0.0,  1.0,  2.1f},
        { 1.0f, -1.0f,  0.0f,  1.0,  1.0,  2.1f},
        {-1.0f,  1.0f,  0.0f,  0.0,  0.0,  2.1f},

        { 1.0f, -1.0f,  0.0f,  1.0,  1.0,  2.1f},
        { 1.0f,  1.0f,  0.0f,  1.0,  0.0,  2.1f},
        {-1.0f,  1.0f,  0.0f,  0.0,  0.0,  2.1f}
    };
    glGenBuffers(1, &m_aman_fisheye_clip_rear_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, m_aman_fisheye_clip_rear_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(fisheye_clip_data_rear), fisheye_clip_data_rear, GL_STATIC_DRAW);

    return 0;
}


/**< @brief opengl vbo init */
int amanGpu::aman_gpu_vbo_init()
{
    int ret;
    
    // check vbo share status
    share_lock();
    if (amanGpu::m_s_is_share_vbo)
    {
        // have not load vbo
        if (0 == amanGpu::m_s_is_share_vbo_load_sts)
        {
            // load .data file to mem
            ret = aman_gpu_init_aman_table_data();
            if (ret != 0)
            {
                amanGpu::m_s_is_share_vbo_load_sts = -1;
                share_unlock();
                amanGPU_ERR("amanGpu::amangpu_init. aman_gpu_init_aman_table_data faild\n");
                return 2;
            }   
            // create vert VBO buffer
            ret = aman_gpu_load_data_vbo();
            if (ret != 0)
            {
                amanGpu::m_s_is_share_vbo_load_sts = -1;
                share_unlock();
                amanGPU_ERR("amanGpu::amangpu_init. aman_gpu_load_data_vbo faild\n");
                return 3;
            }
            amanGpu::m_s_hare_vbo.m_aman_table_vbo = m_aman_table_vbo;
            amanGpu::m_s_hare_vbo.m_aman_idx_vbo = m_aman_idx_vbo;
            amanGpu::m_s_hare_vbo.m_aman_single_front_camera_vbo = m_aman_single_front_camera_vbo;
            amanGpu::m_s_hare_vbo.m_aman_single_rear_camera_vbo = m_aman_single_rear_camera_vbo;
            amanGpu::m_s_hare_vbo.m_aman_two_camera_vbo = m_aman_two_camera_vbo;
            amanGpu::m_s_hare_vbo.m_aman_fisheye_front_vbo = m_aman_fisheye_front_vbo;
            amanGpu::m_s_hare_vbo.m_aman_fisheye_rear_vbo = m_aman_fisheye_rear_vbo;
            amanGpu::m_s_hare_vbo.m_aman_fisheye_clip_front_vbo = m_aman_fisheye_clip_front_vbo;
            amanGpu::m_s_hare_vbo.m_aman_fisheye_clip_rear_vbo = m_aman_fisheye_clip_rear_vbo;          
            amanGpu::m_s_hare_vbo.m_aman_mem_st_num = m_aman_mem_st_num;
            amanGpu::m_s_hare_vbo.m_front_cam_point_num = m_front_cam_point_num;
            amanGpu::m_s_hare_vbo.m_rear_cam_point_num = m_rear_cam_point_num;
            amanGpu::m_s_hare_vbo.m_two_cam_point_num = m_two_cam_point_num;
            amanGpu::m_s_is_share_vbo_load_sts = 1;
            share_unlock();
            amanGPU_LOG("amanGpu::aman_gpu_vbo_init. inst_id=%d, act gen vbo success\n", m_inst_id);
        }
        // already load vbo success
        else if (1 == amanGpu::m_s_is_share_vbo_load_sts)
        {
            m_aman_table_vbo = amanGpu::m_s_hare_vbo.m_aman_table_vbo;
            m_aman_idx_vbo = amanGpu::m_s_hare_vbo.m_aman_idx_vbo;
            m_aman_single_front_camera_vbo = amanGpu::m_s_hare_vbo.m_aman_single_front_camera_vbo;
            m_aman_single_rear_camera_vbo = amanGpu::m_s_hare_vbo.m_aman_single_rear_camera_vbo;
            m_aman_two_camera_vbo = amanGpu::m_s_hare_vbo.m_aman_two_camera_vbo;
            m_aman_fisheye_front_vbo = amanGpu::m_s_hare_vbo.m_aman_fisheye_front_vbo;
            m_aman_fisheye_rear_vbo = amanGpu::m_s_hare_vbo.m_aman_fisheye_rear_vbo;
            m_aman_fisheye_clip_front_vbo = amanGpu::m_s_hare_vbo.m_aman_fisheye_clip_front_vbo;
            m_aman_fisheye_clip_rear_vbo = amanGpu::m_s_hare_vbo.m_aman_fisheye_clip_rear_vbo;
            m_aman_mem_st_num = amanGpu::m_s_hare_vbo.m_aman_mem_st_num;
            m_front_cam_point_num = amanGpu::m_s_hare_vbo.m_front_cam_point_num;
            m_rear_cam_point_num = amanGpu::m_s_hare_vbo.m_rear_cam_point_num;
            m_two_cam_point_num = amanGpu::m_s_hare_vbo.m_two_cam_point_num;
            share_unlock();
            amanGPU_LOG("amanGpu::aman_gpu_vbo_init. inst_id=%d, share vbo success\n", m_inst_id);
        }
        // prev load vbo faild, return error
        else
        {
            share_unlock();
            amanGPU_ERR("amanGpu::amangpu_init. inst_id=%d, aman_gpu_init_aman_table_data faild, since prev load faild\n", m_inst_id);
            return 3;
        }
    }
    else
    {
        share_unlock();
        
        ret = aman_gpu_init_aman_table_data();
        if (ret != 0)
        {
            amanGPU_ERR("amanGpu::amangpu_init. aman_gpu_init_aman_table_data faild\n");
            return 2;
        }   
        // create vert VBO buffer
        ret = aman_gpu_load_data_vbo();
        if (ret != 0)
        {
            amanGPU_ERR("amanGpu::amangpu_init. aman_gpu_load_data_vbo faild\n");
            return 3;
        }
        amanGPU_LOG("amanGpu::aman_gpu_vbo_init. inst_id=%d, act gen vbo success\n", m_inst_id);
    }

    return 0;
}


/**< @brief aman_gpu_texture_init, create textureb*/
int amanGpu::aman_gpu_texture_init()
{
    int i, j;
    int texture_num_gen = 4;    // camera number

    share_lock();
    if (amanGpu::m_s_is_share_tex)
    {
        // have not generate tex
        if (0 == amanGpu::m_s_is_share_tex_load_sts)
        {
            glGenTextures(texture_num_gen, m_texture_id);
            for (i = 0; i < texture_num_gen; i++)
            {
                glActiveTexture(m_num_2_texture_enum[i]);     // 0->GL_TEXTURE0
                glBindTexture( GL_TEXTURE_2D, m_texture_id[i]); // glGenTextures output
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            }
            for (i = 0; i < texture_num_gen; i++)
            {
                amanGpu::m_s_share_tex.m_texture_id[i] = m_texture_id[i];
            }
            amanGpu::m_s_is_share_tex_load_sts = 1;
            share_unlock();
            amanGPU_LOG("amanGpu::aman_gpu_texture_init. inst_id=%d, act gen tex success\n", m_inst_id);
        }
        // have generated
        else if (1 == amanGpu::m_s_is_share_tex_load_sts)
        {
            for (i = 0; i < texture_num_gen; i++)
            {
                m_texture_id[i] = amanGpu::m_s_share_tex.m_texture_id[i];
            }
            share_unlock();
            amanGPU_LOG("amanGpu::aman_gpu_texture_init. inst_id=%d, share tex success\n", m_inst_id);
        }
        // prev gentex faild
        else
        {
            share_unlock();
            amanGPU_ERR("amanGpu::aman_gpu_texture_init faild. prev gen faild\n");
            return 1;
        }
    }
    else
    {
        share_unlock();
        
        glGenTextures(texture_num_gen, m_texture_id);
        for (i = 0; i < texture_num_gen; i++)
        {
            glActiveTexture(m_num_2_texture_enum[i]);     // 0->GL_TEXTURE0
            glBindTexture( GL_TEXTURE_2D, m_texture_id[i]); // glGenTextures output
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        }
        amanGPU_LOG("amanGpu::aman_gpu_texture_init. inst_id=%d, act gen tex success\n", m_inst_id);
    }

    // read config file to get tex->uniform val 
    m_camera_uniform2tex[0] = ini_get_cam_right();
    m_camera_uniform2tex[1] = ini_get_cam_front();
    m_camera_uniform2tex[2] = ini_get_cam_left();
    m_camera_uniform2tex[3] = ini_get_cam_rear();
    if ((m_camera_uniform2tex[0] < 0) || (m_camera_uniform2tex[1] < 0)
        || (m_camera_uniform2tex[2] < 0) || (m_camera_uniform2tex[3] < 0))
    {
        amanGPU_ERR("amanGpu::aman_gpu_texture_init. read config for uniform2tex file faild, use default\n");
        m_camera_uniform2tex[0] = 0;
        m_camera_uniform2tex[1] = 1;
        m_camera_uniform2tex[2] = 2;
        m_camera_uniform2tex[3] = 3;
    }
    // i for tex, j for uniform
    for (i = 0; i < texture_num_gen; i++)
    {
        for (j = 0; j < texture_num_gen; j++)
        {
            if (i == m_camera_uniform2tex[j])
            {
                m_camera_tex2uniform[i] = j;
                break;
            }
        }
        amanGPU_LOG("amanGpu::aman_gpu_texture_init. inst_id=%d, tex chn[%d] - uniform[%d]\n",
            m_inst_id, i, j);
    }

    // if is use bmp, uniform[0]-[3] means right, front, left, rear is exactly tex [0-3]
    for (i = 0; i < 4; i++)
    {
        m_camera_uniform2tex_bmp[i] = i;
        m_camera_tex2uniform_bmp[i] = i;
    }

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_texture_id[0]);
    //glEnable(GL_TEXTURE_2D); // texture 2d id deprecated in opengl es 2

    return 0;
}


/**
 * @brief amanGpu::aman_gpu_postion_init
 * @param obj_width
 * @param obj_height
 * @return
 */
int amanGpu::aman_gpu_postion_init()
{
    // rotate center is not the coord (0, y, 0)
    float rotate_center_x = ini_get_rotate_center_x();
    float rotate_center_y = ini_get_rotate_center_y();

    if ((rotate_center_x > 100.0f) || (rotate_center_y > 100.0f))
    {
        amanGPU_ERR("amanGpu::aman_gpu_postion_init. read config file faild\n");
        return 1;
    }
    m_rotate_center_x = rotate_center_x;
    m_rotate_center_y = rotate_center_y;

    // 3d view. rotate view matrix radius
    if (3 == m_car_type)
    {
        m_rotate_top_r = 4.5f;
        m_rotate_bottom_r = 0.75f;
        m_rotate_top_height = 6.0f;
        m_rotate_bottom_height = -1.0f;
        m_rotate_max_z_angle = glm::degrees(glm::atan(m_rotate_top_r, m_rotate_top_height));
    }
    else
    {
        m_rotate_top_r = 4.8f;
        m_rotate_bottom_r = 0.8f;
        m_rotate_top_height = 6.0f;
        m_rotate_bottom_height = -1.0f;
        m_rotate_max_z_angle = glm::degrees(glm::atan(m_rotate_top_r, m_rotate_top_height));    
    }
    // auto view. rotate matrix radius
    m_rotate_auto_top_r = 0.05f;
    m_rotate_auto_bottom_r = 0.0f;
    m_rotate_auto_top_height = 14.5f;
    m_rotate_auto_bottom_height = 0.0f;

    // m_2d view
    m_2d_front_det = 2.8f;
    m_2d_rear_det = -2.8f;
    m_2d_height = 6.0f;

	//Todo if m_rotate_top_height/m_rotate_top_r != m_rotate_bottom_height/m_rotate_bottom_r
    m_curview_rot_ang_z = m_rotate_max_z_angle;
	m_rotate_r = sqrt(m_rotate_top_r*m_rotate_top_r+m_rotate_top_height*m_rotate_top_height);

    return 0;
}


/**< load llight balance data */
int amanGpu::aman_gpu_light_balance_init()
{
    int ret;
    
    share_lock();
    if (amanGpu::m_s_is_share_lb)
    {
        // prev not load
        if (0 == amanGpu::m_s_is_share_lb_load_sts)
        {
        	// load light data, also from stitch table data file
            ret = aman_gpu_load_db_light(m_p_db_file_name);
            if (ret != 0)
            {
                amanGpu::m_s_is_share_lb_load_sts = -1;
                share_unlock();
                amanGPU_ERR("amanGpu::aman_gpu_light_balance_init. load stitch data for light balance faild\n");
                return 1;
            }
            // this instance do light balance calc process, other direct use share color_k param
            m_is_do_lb_process = 1;
            amanGpu::m_s_is_share_lb_load_sts = 1;
            share_unlock();
            amanGPU_LOG("amanGpu::aman_gpu_light_balance_init. inst_id=%d, load lb success\n", m_inst_id);
        }
        // prev load faild
        else if (-1 == amanGpu::m_s_is_share_lb_load_sts)
        {
            share_unlock();
            amanGPU_ERR("amanGpu::aman_gpu_light_balance_init. prev load data faild\n");
            return 1;            
        }
        // prev load success
        else
        {
            share_unlock();
            amanGPU_LOG("amanGpu::aman_gpu_light_balance_init. inst_id=%d, will share lb\n", m_inst_id);
        }
    }
    else
    {
        share_unlock();
    	// load light data, also from stitch table data file
        ret = aman_gpu_load_db_light(m_p_db_file_name);
        if (ret != 0)
        {
            amanGPU_ERR("amanGpu::aman_gpu_light_balance_init. load stitch data for light balance faild\n");
            return 1;
        }
        amanGPU_LOG("amanGpu::aman_gpu_light_balance_init. inst_id=%d, load lb success\n", m_inst_id);
    }

    return 0;
}


int amanGpu::aman_gpu_set_view(int view_enum, int is_force)
{
    if ((view_enum == m_current_view) && (0 == is_force))
    {
        return 0;
    }
    m_current_view = view_enum;
    
    // single and two camera with matrix 1.0
    if (aman_VIEW_CAMERA_FRONT == view_enum)
    {
        float w = (float)((m_projWidth * 1.0 / m_projHeight * m_texture_height*m_2d_f_clip_rate) / m_texture_width);
        float h = -1.0f + (2.0f - m_2d_f_clip_rate*2.0f);
    
        m_gl_model = glm::mat4(1.0f);
        m_gl_view = glm::mat4(1.0f);
        m_gl_proj = glm::ortho(-1.0f * w, w, h, 1.0f, -1.0f, 0.0f);
        m_mvp = m_gl_proj;      // 2d space not use model matrix and view matrix
    }
    else if (aman_VIEW_CAMERA_REAR == view_enum)
    {
        float w = (float)((m_projWidth * 1.0 / m_projHeight * m_texture_height*m_2d_r_clip_rate) / m_texture_width);
        float h = -1.0f + (2.0f - m_2d_r_clip_rate*2.0f);

        m_gl_model = glm::mat4(1.0f);
        m_gl_view = glm::mat4(1.0f);
        m_gl_proj = glm::ortho(-1.0f * w, w, h, 1.0f, -1.0f, 0.0f);
        m_mvp = m_gl_proj;      // 2d space not use model matrix and view matrix
    }
    else if ((aman_VIEW_CAMERA_LEFTRIGHT == view_enum)
        || (aman_VIEW_CAMERA_LEFTRIGHT_CPY == view_enum))
    {
        // left and right image is tex_height*2 : tex_width, but have black border, use 1.0 instead
        //float w = (float)((m_projWidth * 1.0 / m_projHeight * m_texture_width) / (m_texture_height*2));
        float w = 1.0;

        m_gl_model = glm::mat4(1.0f);
        m_gl_view = glm::mat4(1.0f);
        m_gl_proj = glm::ortho(-1.0f * w, w, -1.0f, 1.0f, -1.0f, 0.0f);
        m_mvp = m_gl_proj;      // 2d space not use model matrix and view matrix
    }
    // fisheye view
    else if (aman_VIEW_CAMERA_FISH_FRONT == view_enum)
    {
        m_gl_model = glm::mat4(1.0f);
        m_gl_view = glm::mat4(1.0f);
        m_gl_proj = glm::ortho(m_fisheye_front_orth[0], 
            m_fisheye_front_orth[1], 
            m_fisheye_front_orth[2], 
            m_fisheye_front_orth[3], -1.0f, 0.0f);
        m_mvp = m_gl_proj;      // 2d space not use model matrix and view matrix    
    }
    else if (aman_VIEW_CAMERA_FISH_REAR == view_enum)
    {  
        m_gl_model = glm::mat4(1.0f);
        m_gl_view = glm::mat4(1.0f);
        m_gl_proj = glm::ortho(m_fisheye_rear_orth[0], 
            m_fisheye_rear_orth[1], 
            m_fisheye_rear_orth[2], 
            m_fisheye_rear_orth[3], -1.0f, 0.0f);
        m_mvp = m_gl_proj;      // 2d space not use model matrix and view matrix
    }
    else if (aman_VIEW_CAMERA_FISH_CLIP_FRONT == view_enum)
    { 
        m_gl_model = glm::mat4(1.0f);
        m_gl_view = glm::mat4(1.0f);
        m_gl_proj = glm::ortho(m_fisheye_clip_front_orth[0], 
            m_fisheye_clip_front_orth[1], 
            m_fisheye_clip_front_orth[2], 
            m_fisheye_clip_front_orth[3], -1.0f, 0.0f);
        m_mvp = m_gl_proj;      // 2d space not use model matrix and view matrix    
    }
    else if (aman_VIEW_CAMERA_FISH_CLIP_REAR == view_enum)
    {
        m_gl_model = glm::mat4(1.0f);
        m_gl_view = glm::mat4(1.0f);
        m_gl_proj = glm::ortho(m_fisheye_clip_rear_orth[0], 
            m_fisheye_clip_rear_orth[1], 
            m_fisheye_clip_rear_orth[2], 
            m_fisheye_clip_rear_orth[3], -1.0f, 0.0f);
        m_mvp = m_gl_proj;      // 2d space not use model matrix and view matrix      
    }
    else
    {
        // set model matrix
        m_gl_model = glm::mat4(1.0f);
        //m_gl_model = glm::rotate(m_gl_model, glm::radians(m_curview_rot_ang), glm::vec3(0, 1, 0));
        m_gl_model = glm::rotate(m_gl_model, glm::radians(-90.0f), glm::vec3(1, 0, 0));
        m_gl_model = glm::rotate(m_gl_model, glm::radians(-180.0f), glm::vec3(0, 0, 1));

        float aspect = m_projWidth * 1.0f / m_projHeight;
        m_gl_proj = glm::perspective(glm::radians(GPU_PERSPECTIVE_ANGLE), aspect, 1.0f, 1000.0f);

        if ((aman_VIEW_AUTO == view_enum)
            || (aman_VIEW_3D_FRONT == view_enum)
            || (aman_VIEW_3D_REAR == view_enum)
            || (aman_VIEW_3D_LEFT == view_enum)
            || (aman_VIEW_3D_RIGHT == view_enum)
            || (aman_VIEW_3D_LEFT_FRONT == view_enum)
            || (aman_VIEW_3D_LEFT_CENTER == view_enum)
            || (aman_VIEW_3D_RIGHT_FRONT == view_enum)
            || (aman_VIEW_3D_RIGHT_CENTER == view_enum))
        {
            float view_deg = aman_gpu_get_view_deg(view_enum);
            aman_gpu_set_rotate(1, 0, view_deg);            
        }
        else if (aman_VIEW_2D_FRONT == view_enum)
        {
            m_gl_view = glm::lookAt(
                glm::vec3(m_rotate_center_x, m_2d_height, m_rotate_center_y + m_2d_front_det),
                glm::vec3(m_rotate_center_x, 0.0f, m_rotate_center_y + m_2d_front_det + 0.1f),
                glm::vec3(0.0f, 1.0f, 0.0f));
            m_mvp = m_gl_proj * m_gl_view * m_gl_model;
        }
        else if (aman_VIEW_2D_REAR == view_enum)
        {
            m_gl_view = glm::lookAt(
                glm::vec3(m_rotate_center_x, m_2d_height, m_rotate_center_y + m_2d_rear_det),
                glm::vec3(m_rotate_center_x, 0.0f, m_rotate_center_y + m_2d_rear_det + 0.1f),
                glm::vec3(0.0f, 1.0f, 0.0f));
            m_mvp = m_gl_proj * m_gl_view * m_gl_model;
        }
    }

    return 0;
}


float amanGpu::aman_gpu_get_view_deg(int view_enum)
{
    if (aman_VIEW_3D_FRONT == view_enum)
    {
        return 90.0f;
    }
    else if (aman_VIEW_3D_LEFT == view_enum)
    {
        return -45.0f;
    }
    else if (aman_VIEW_3D_RIGHT == view_enum)
    {
        return -135.0f;
    }
    else if (aman_VIEW_3D_REAR == view_enum)
    {
        return -90.0f;
    }
    else if (aman_VIEW_3D_LEFT_FRONT == view_enum)
    {
        return 45.0f;
    }
    else if (aman_VIEW_3D_LEFT_CENTER == view_enum)
    {
        return 0.0f;
    }
    else if (aman_VIEW_3D_RIGHT_FRONT == view_enum)
    {
        return 135.0f;
    }
    else if (aman_VIEW_3D_RIGHT_CENTER == view_enum)
    {
        return -180.0f;
    }
    // top-down view's deg is same as rear view
    else if (aman_VIEW_AUTO == view_enum)
    {
        return -90.0f;
    }

    return m_curview_rot_ang;
}


int amanGpu::aman_gpu_check_in_3d_view(int view_enum)
{
    if ((view_enum != aman_VIEW_3D_FRONT)
        && (view_enum != aman_VIEW_3D_LEFT)
        && (view_enum != aman_VIEW_3D_RIGHT)
        && (view_enum != aman_VIEW_3D_REAR)
        && (view_enum != aman_VIEW_3D_LEFT_FRONT)
        && (view_enum != aman_VIEW_3D_LEFT_CENTER)
        && (view_enum != aman_VIEW_3D_RIGHT_FRONT)
        && (view_enum != aman_VIEW_3D_RIGHT_CENTER)
        && (view_enum != aman_VIEW_2D_FRONT)
        && (view_enum != aman_VIEW_2D_REAR)
        && (view_enum != aman_VIEW_3D_FREE))
    {
        return 0;
    }
    else
    {
        return 1;
    }
}


int amanGpu::aman_gpu_check_in_pure_3d_view(int view_enum)
{
    if ((view_enum != aman_VIEW_3D_FRONT)
        && (view_enum != aman_VIEW_3D_LEFT)
        && (view_enum != aman_VIEW_3D_RIGHT)
        && (view_enum != aman_VIEW_3D_FREE)
        && (view_enum != aman_VIEW_3D_REAR)
        && (view_enum != aman_VIEW_3D_LEFT_FRONT)
        && (view_enum != aman_VIEW_3D_LEFT_CENTER)
        && (view_enum != aman_VIEW_3D_RIGHT_FRONT)
        && (view_enum != aman_VIEW_3D_RIGHT_CENTER))
    {
        return 0;
    }
    else
    {
        return 1;
    }
}


void amanGpu::aman_gpu_hide_stitch(int is_hide_right, int is_hide_front,
    int is_hide_left, int is_hide_rear)
{
    // if value less than 0, hide state unchanged
    if (is_hide_right < 0)
    {
        is_hide_right = m_is_hide_right;
    }
    if (is_hide_front < 0)
    {
        is_hide_front = m_is_hide_front;
    }
    if (is_hide_left < 0)
    {
        is_hide_left = m_is_hide_left; 
    }
    if (is_hide_rear < 0)
    {
        is_hide_rear = m_is_hide_rear;
    }

    if ((is_hide_right != m_is_hide_right)
        || (is_hide_front != m_is_hide_front)
        || (is_hide_left != m_is_hide_left)
        || (is_hide_rear != m_is_hide_rear))
    {
        m_is_hide_right = is_hide_right;
        m_is_hide_front = is_hide_front;
        m_is_hide_left = is_hide_left;
        m_is_hide_rear = is_hide_rear;
        
        m_is_hide_change++;
    }
}


int amanGpu::aman_gpu_set_view_dyn_trans(int view_enum)
{
    if ((m_current_view == view_enum) && (0 == m_is_in_view_trans))
    {
        return 1;
    }
    if (aman_gpu_check_in_pure_3d_view(view_enum) != 1)
    {
        return 1;
    }
    m_dyn_trans_dst_view = view_enum;
    float target_deg = aman_gpu_get_view_deg(view_enum); /* [csj 20180929-8] */
    float abs_det_deg = fabs(target_deg - m_curview_rot_ang);
    if (abs_det_deg > 180.0f)
    {
        if (target_deg > 0.0f)
        {
            m_dyn_trans_dst_deg = abs_det_deg - 360.0;
        }
        else
        {
            m_dyn_trans_dst_deg = 360.0 - abs_det_deg;
        }
    }
	else
	{
		m_dyn_trans_dst_deg = target_deg - m_curview_rot_ang;
	}

    m_dyn_trans_frame_cnt = 25;     /**< in (m_dyn_trans_frame_cnt) frame complete this action */
    m_is_in_view_trans = 1;

    return 0;
}


int amanGpu::aman_gpu_view_dyn_process()
{
    if (0 == m_is_in_view_trans)
    {
        return 0;
    }
    // check if already direct change view 
    if (aman_gpu_check_in_pure_3d_view(m_current_view) != 1)
    {
        m_is_in_view_trans = 0;
        return 0;
    }

    // every time change deg
    float single_angle = m_dyn_trans_dst_deg * 0.3; /* [csj 20180929-8] */
    if ((single_angle >= 0.0f) && (single_angle < 1.0f))
    {
        single_angle = 1.0f;
    }
    else if ((single_angle <= 0.0f) && (single_angle > -1.0f))
    {
        single_angle = -1.0f;
    }
    if ((m_dyn_trans_frame_cnt <= 0) || (fabs(m_dyn_trans_dst_deg) < 1.0f))
    {
        float view_deg = aman_gpu_get_view_deg(m_dyn_trans_dst_view);
        aman_gpu_set_rotate(1, 0, view_deg);
        m_current_view = m_dyn_trans_dst_view;
        m_dyn_trans_frame_cnt = 0;
        m_is_in_view_trans = 0;
    }
    else
    {
        m_dyn_trans_frame_cnt--;
        aman_gpu_set_rotate_det_angle(single_angle);
		m_dyn_trans_dst_deg -= single_angle;
    }

    return 0;
}


int amanGpu::aman_gpu_set_rotate(int is_angle, int x_det, float view_angle)
{
    if ((0 == is_angle) && (0 == x_det))
    {
        return 0;
    }

    // single and two camera can't rotate
    if ((aman_VIEW_CAMERA_FRONT == m_current_view)
        || (aman_VIEW_CAMERA_REAR == m_current_view)
        || (aman_VIEW_CAMERA_LEFTRIGHT == m_current_view)
        || (aman_VIEW_CAMERA_LEFTRIGHT_CPY == m_current_view)
        || (aman_VIEW_2D_FRONT == m_current_view)
        || (aman_VIEW_2D_REAR == m_current_view)
        || (aman_VIEW_CAMERA_FISH_FRONT == m_current_view)
        || (aman_VIEW_CAMERA_FISH_REAR == m_current_view)
        || (aman_VIEW_CAMERA_FISH_CLIP_FRONT == m_current_view)
        || (aman_VIEW_CAMERA_FISH_CLIP_REAR == m_current_view))        
    {
        return 0;
    }

    if (is_angle)
    {
        m_curview_rot_ang = view_angle;
    }
    else
    {
        float angle = x_det * 1.0 / m_projWidth * 180;  // proj_width as 180 deg
        m_curview_rot_ang = m_curview_rot_ang + angle;
    }

    // 3d view rotate
    if (1 == aman_gpu_check_in_pure_3d_view(m_current_view))
    {
        // direct rotate model. comment for view rotate instead
        //m_gl_model = glm::mat4(1.0f);
        //m_gl_model = glm::rotate(m_gl_model, glm::radians(m_curview_rot_ang), glm::vec3(0, 1, 0));
        //m_gl_model = glm::rotate(m_gl_model, glm::radians(-90.0f), glm::vec3(1, 0, 0));
        //m_gl_model = glm::rotate(m_gl_model, glm::radians(-180.0f), glm::vec3(0, 0, 1));

        float top_x = m_rotate_top_r * glm::cos(glm::radians(m_curview_rot_ang)) + m_rotate_center_x;
        float top_y = m_rotate_top_r * glm::sin(glm::radians(m_curview_rot_ang)) + m_rotate_center_y;
        float bottom_x = m_rotate_bottom_r * glm::cos(glm::radians(m_curview_rot_ang + 180.0f)) + m_rotate_center_x;
        float bottom_y = m_rotate_bottom_r * glm::sin(glm::radians(m_curview_rot_ang + 180.0f)) + m_rotate_center_y;

        m_gl_view = glm::lookAt(glm::vec3(top_x, m_rotate_top_height, top_y),
            glm::vec3(bottom_x, m_rotate_bottom_height, bottom_y), 
            glm::vec3(0.0f, 1.0f, 0.0f));

        m_mvp = m_gl_proj * m_gl_view * m_gl_model;
    }
    // auto view's rotate
    else if (aman_VIEW_AUTO == m_current_view)
    {
        float top_x = m_rotate_auto_top_r * glm::cos(glm::radians(m_curview_rot_ang)) + m_rotate_center_x;
        float top_y = m_rotate_auto_top_r * glm::sin(glm::radians(m_curview_rot_ang)) + m_rotate_center_y;
        float bottom_x = m_rotate_auto_bottom_r * glm::cos(glm::radians(m_curview_rot_ang)) + m_rotate_center_x;
        float bottom_y = m_rotate_auto_bottom_r * glm::sin(glm::radians(m_curview_rot_ang)) + m_rotate_center_y;

        m_gl_view = glm::lookAt(glm::vec3(top_x, m_rotate_auto_top_height, top_y),
            glm::vec3(bottom_x, m_rotate_auto_bottom_height, bottom_y),
            glm::vec3(0.0f, 1.0f, 0.0f));

        m_mvp = m_gl_proj * m_gl_view * m_gl_model;
    }

	//convert to [-180 ~ 180) /* [csj 20180929-8] */
    if (m_curview_rot_ang < -180.0)
    {
        m_curview_rot_ang += 360.0;
    }
    else if (m_curview_rot_ang >= 180.0)
    {
        m_curview_rot_ang -= 360.0;
    }

    return 0;
}


/**
* @brief rotation 3d view, both horizontal and vertical directions can be rotated
* @param x_det
*/
int amanGpu::aman_gpu_omni_directional_rotate(int x_det, int y_det)
{
	// single and two camera can't rotate
    if ((aman_VIEW_CAMERA_FRONT == m_current_view)
        || (aman_VIEW_CAMERA_REAR == m_current_view)
        || (aman_VIEW_CAMERA_LEFTRIGHT == m_current_view)
        || (aman_VIEW_CAMERA_LEFTRIGHT_CPY == m_current_view)
        || (aman_VIEW_2D_FRONT == m_current_view)
        || (aman_VIEW_2D_REAR == m_current_view)
        || (aman_VIEW_CAMERA_FISH_FRONT == m_current_view)
        || (aman_VIEW_CAMERA_FISH_REAR == m_current_view)
        || (aman_VIEW_CAMERA_FISH_CLIP_FRONT == m_current_view)
        || (aman_VIEW_CAMERA_FISH_CLIP_REAR == m_current_view))        
    {
        return -1;
    }

#define Z_DEG_LIMIT_MIN     (2.0)

	//vertical deg
	float angle = y_det * 1.0 / m_projHeight * 180;
    float tempRslt = m_curview_rot_ang_z + angle;
    if(IF_IN_RANGE(tempRslt, Z_DEG_LIMIT_MIN, m_rotate_max_z_angle))
	{
        m_curview_rot_ang_z = tempRslt;
	}

	//horizontal deg
	angle = x_det * 1.0 / m_projWidth * 180;  // proj_width as 180 deg
    m_curview_rot_ang += angle;

	if (1 == aman_gpu_check_in_pure_3d_view(m_current_view))
	{
		float top_r = m_rotate_r * glm::sin(glm::radians(m_curview_rot_ang_z));
        float bottom_r = fabs(m_rotate_bottom_height) * glm::tan(glm::radians(m_curview_rot_ang_z));
		float top_x = top_r * glm::cos(glm::radians(m_curview_rot_ang)) + m_rotate_center_x;
        float top_y = top_r * glm::sin(glm::radians(m_curview_rot_ang)) + m_rotate_center_y;
        float bottom_x = bottom_r * glm::cos(glm::radians(m_curview_rot_ang + 180.0f)) + m_rotate_center_x;
        float bottom_y = bottom_r * glm::sin(glm::radians(m_curview_rot_ang + 180.0f)) + m_rotate_center_y;
		float top_height = m_rotate_r * glm::cos(glm::radians(m_curview_rot_ang_z));
		float bottom_height = m_rotate_bottom_height;

        m_gl_view = glm::lookAt(glm::vec3(top_x, top_height, top_y),
            glm::vec3(bottom_x, bottom_height, bottom_y), 
            glm::vec3(0.0f, 1.0f, 0.0f));

        m_mvp = m_gl_proj * m_gl_view * m_gl_model;
    }

    // change view to aman_3D_FREE
    m_current_view = aman_VIEW_3D_FREE;

    //convert to [-180, 180)
    if (m_curview_rot_ang < -180.0)
    {
        m_curview_rot_ang += 360.0;
    }
    else if (m_curview_rot_ang >= 180.0)
    {
        m_curview_rot_ang -= 360.0;
    }

    return 0;

#undef Z_DEG_LIMIT_MIN
}


int amanGpu::aman_gpu_set_rotate_det_angle(float det_angle)
{
    // float angle = x_det * 1.0 / m_projWidth * 180;  // proj_width as 180 deg
    int det = (int)(det_angle / 180.0 * m_projWidth);
    return aman_gpu_set_rotate(0, det, 0);
}


void amanGpu::aman_gpu_get_v_p(glm::mat4 *p_v, glm::mat4 *p_p)
{
    if (p_v != NULL)
    {
        *p_v = m_gl_view;
    }
    if (p_p != NULL)
    {
        *p_p = m_gl_proj;
    }
}


/**< update viv texture */
int amanGpu::aman_gpu_update_viv_gltex()
{
#if GLOBAL_RUN_ENV_DESKTOP == 0  
    int i;
    
    for (i = 0; i < 4; i++)
    {
        glActiveTexture(m_num_2_texture_enum[i]);
        glBindTexture(GL_TEXTURE_2D, m_texture_id[i]);

        // texDirectViv map and inv
        (*g_pFNglTexDirectVIVMap)(GL_TEXTURE_2D, 
            m_texture_width, m_texture_height, GL_VIV_YUY2,
            (void **)(&(m_texture_vaddr[i])), &(m_texture_paddr[i])); 
        (*g_pFNglTexDirectInvalidateVIV)(GL_TEXTURE_2D);
    } 

    // release image
    (*m_p_rel_imgfunc)();
#endif

    return 0;
}


/**< update image tex */
int amanGpu::aman_gpu_update_bmp_gltex()
{
    int i;
    
    // update image gltex
    for (i = 0; i < 4; i++)
    {
        glActiveTexture(m_num_2_texture_enum[i]);
        glBindTexture(GL_TEXTURE_2D, m_texture_id[i]);
		glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, m_texture_width, m_texture_height,
			0, GL_RGBA, GL_UNSIGNED_BYTE, (void *)(m_texture_vaddr[i]));
    }    

    return 0;
}


/**< 
*@brief get new image address
*       image is released after texture is generated(not here) 
*/
int amanGpu::aman_gpu_update_dev_img_addr()
{
#if GLOBAL_RUN_ENV_DESKTOP == 0  
    int i;
    int ret;
    
    for (i = 0; i < 4; i++)
    {
        unsigned int paddr = 0;
        unsigned char *p_vaddr = NULL;
        unsigned int ts_ms = 0;
        ret = (*m_p_get_imgfunc)(i, &paddr, &p_vaddr, &ts_ms);
        if (ret != 0)
        {
            return 1;
        }
        m_texture_vaddr[i] = p_vaddr;
        m_texture_paddr[i] = paddr;
    }
#endif

    return 0;
}


/**< update image tex */
int amanGpu::aman_gpu_update_pcenv_img_addr(int is_bmp_seq)
{
    // if use same static image, do nothing, since image is load in init process
    if (0 == is_bmp_seq)
    {
        return 0;
    }

    // else, load video image frame
    int i;
    char str[4][128];
    const char *str_path[4] = {
        "video_right/right_%d.jpg",
        "video_front/front_%d.jpg",
        "video_left/left_%d.jpg",
        "video_rear/rear_%d.jpg"
    };
    QImage *p_qimage[4] = {&m_img1, &m_img2, &m_img3, &m_img4};

    for (i = 0; i < 4; i++)
    {
        QImage tmp_img;
        memset(str[i], 0, 128);
        sprintf(str[i], str_path[i], amanGpu::m_s_share_tex.m_video_fc[i]);
        if (false == tmp_img.load(str[i]))
        {
            continue;
        }
        if ((tmp_img.width() != m_texture_width) 
            || (tmp_img.height() != m_texture_height))
        {
            amanGPU_ERR("amanGpu::aman_gpu_update_pcenv_img_addr. tex res not match\n");
            continue;
        }
        (*p_qimage[i]) = tmp_img;
        (*p_qimage[i]) = (*p_qimage[i]).convertToFormat(QImage::Format_RGBA8888);
        amanGpu::m_s_share_tex.m_video_fc[i]++;
    }
    for (i = 0; i < 4; i++)
    {
        m_texture_vaddr[i] = (*p_qimage[i]).bits();
        m_texture_paddr[i] = 0; 
    }
    
    return 0;
}


int amanGpu::aman_gpu_update_img_addr()
{
    int i;

    share_lock();
    if (amanGpu::m_s_is_share_tex)
    {
        // cur tex uid should be val 'm_tex_uid++'
        uint64_inc(&m_tex_uid_h, &m_tex_uid_l);
        // cur share image is not the newest cnt(since m_tex_uid > share.tex_uid)
        if (uint64_cmp(&m_tex_uid_h, &m_tex_uid_l, 
            &amanGpu::m_s_share_tex.m_tex_uid_h, 
            &amanGpu::m_s_share_tex.m_tex_uid_l) > 0)
        {
            if (0 == m_is_use_bmp)
            {
                // only imx6 env can use cam image        
                int ret = aman_gpu_update_dev_img_addr();
                if (ret != 0)
                {
                    share_unlock();
                    amanGPU_ERR("amanGpu::aman_gpu_update_img_addr. update image faild\n");
                }
            }
            else
            {
                aman_gpu_update_pcenv_img_addr(0);
            }
            for (i = 0; i < 4; i++)
            {
                amanGpu::m_s_share_tex.m_texture_vaddr[i] = m_texture_vaddr[i];
                amanGpu::m_s_share_tex.m_texture_paddr[i] = m_texture_paddr[i];
            }
            // new image address is set, but opengl texture is not updated
            amanGpu::m_s_share_tex.m_tex_updated = 0;
            uint64_set(&amanGpu::m_s_share_tex.m_tex_uid_h, 
                &amanGpu::m_s_share_tex.m_tex_uid_l, 
                &m_tex_uid_h, &m_tex_uid_l);
            share_unlock();
        }
        // share iamge is updated
        else
        {       
            for (i = 0; i < 4; i++)
            {
                m_texture_vaddr[i] = amanGpu::m_s_share_tex.m_texture_vaddr[i];
                m_texture_paddr[i] = amanGpu::m_s_share_tex.m_texture_paddr[i];
            }
            uint64_set(&m_tex_uid_h, &m_tex_uid_l,
                &amanGpu::m_s_share_tex.m_tex_uid_h, 
                &amanGpu::m_s_share_tex.m_tex_uid_l);
            share_unlock();
        }
    }
    else
    {
        share_unlock();

        if (0 == m_is_use_bmp)
        {
            aman_gpu_update_dev_img_addr();
        }
        else
        {
            aman_gpu_update_pcenv_img_addr(0);
        }
    }

    // check image address
    for (i = 0; i < 4; i++)
    {
        if ((NULL == m_texture_vaddr[i]) && (0 == m_texture_paddr[i]))
        {
            amanGPU_LOG("amanGpu::aman_gpu_update_img_addr. image address is null\n");
            return 1;
        }
    }

    return 0;
}


/**< generate opengl texture */
int amanGpu::aman_gpu_process_pre_texture()
{
    int i;
    
    share_lock();
    if (amanGpu::m_s_is_share_tex)
    {
        // texture is not updated
        if (0 == amanGpu::m_s_share_tex.m_tex_updated)
        {
            if (0 == m_is_use_bmp)
            {
                aman_gpu_update_viv_gltex();
            }
            else
            {
                aman_gpu_update_bmp_gltex();
            }
            amanGpu::m_s_share_tex.m_tex_updated = 1;
        }
        // texture is updated, just use it
        else
        {
            for (i = 0; i < 4; i++)
            {
                glActiveTexture(m_num_2_texture_enum[i]);
                glBindTexture(GL_TEXTURE_2D, m_texture_id[i]);
            } 
        }
        share_unlock();
    }
    else
    {
        share_unlock();

        if (0 == m_is_use_bmp)
        {
            aman_gpu_update_viv_gltex();
        }
        else
        {
            aman_gpu_update_bmp_gltex();
        }
    }

    return 0;
}


/**
@param ext_param 0 for surround widget, 1 for summwidget
*/
int amanGpu::aman_gpu_process()
{
    // update texture image source address
    if (aman_gpu_update_img_addr() != 0)
    {
        return 1;
    }

    // in 3d view, when change view state, dyn change rotate angle
    aman_gpu_view_dyn_process();

    // light balance process, calc light param K
    aman_gpu_light_balance_process();

    if ((1 == aman_gpu_check_in_3d_view(m_current_view)) 
        || (aman_VIEW_AUTO == m_current_view))
    {
        aman_gpu_stitch_process();
    }
    else if (aman_VIEW_CAMERA_FRONT == m_current_view)
    {
#if SUPPORT_FRONT_REAR_CALIB == 0
        amanGPU_ERR("amanGpu::aman_gpu_process. aman_VIEW_CAMERA_FRONT is not support\n");
        return 1;
#else
        aman_gpu_normal_front_process(0);
#endif      
    }
    else if (aman_VIEW_CAMERA_REAR == m_current_view)
    {
#if SUPPORT_FRONT_REAR_CALIB == 0    
        amanGPU_ERR("amanGpu::aman_gpu_process. aman_VIEW_CAMERA_FRONT is not support\n");
        return 1;
#else
        aman_gpu_normal_rear_process(0);
#endif
    }
    else if ((aman_VIEW_CAMERA_LEFTRIGHT == m_current_view)
        || (aman_VIEW_CAMERA_LEFTRIGHT_CPY == m_current_view))
    {
        aman_gpu_normal_two_process();
    }
    else if (aman_VIEW_CAMERA_FISH_FRONT == m_current_view)
    {
        aman_gpu_normal_front_process(1);
    }
    else if (aman_VIEW_CAMERA_FISH_REAR == m_current_view)
    {
        aman_gpu_normal_rear_process(1);
    }
    else if (aman_VIEW_CAMERA_FISH_CLIP_FRONT == m_current_view)
    {
        aman_gpu_normal_front_process(2);
    }
    else if (aman_VIEW_CAMERA_FISH_CLIP_REAR == m_current_view)
    {
        aman_gpu_normal_rear_process(2);
    }
    else
    {
        amanGPU_ERR("amanGpu::aman_gpu_process. view is not vaild\n");
        return 1;
    }	

    return 0;
}


int amanGpu::aman_gpu_stitch_process()
{
    glUseProgram(shader_get_program(m_shader_handler));

    // load texture
    int ret = aman_gpu_process_pre_texture();
    if (ret != 0)
    {
        amanGPU_ERR("amanGpu::aman_gpu_process. aman_gpu_process_pre_texture faild\n");
        return 1;
    }
    GLint sampler_array[4] = {m_loc_sampler, m_loc_sampler1, m_loc_sampler2, m_loc_sampler3};
    int *p_tex2uniform_arr = NULL;
    if (0 == m_is_use_bmp)
    {
        p_tex2uniform_arr = m_camera_tex2uniform;
    }
    else
    {
        p_tex2uniform_arr = m_camera_tex2uniform_bmp;
    }
    for (int i = 0; i < 4; i++)
    {
        glUniform1i(sampler_array[p_tex2uniform_arr[i]], i);
    }

    glUniform1fv(m_loc_colork, 4, m_color_k);
    glUniformMatrix4fv(m_loc_transform_mat, 1, GL_FALSE, glm::value_ptr(m_mvp));

    glBindBuffer(GL_ARRAY_BUFFER, m_aman_table_vbo);
    glVertexAttribPointer(m_loc_vertices, 3, GL_FLOAT, GL_FALSE,
        sizeof(aman_buffer_data_s), (GLvoid *)(0 * sizeof(float)));
    glVertexAttribPointer(m_loc_texcoord, 2, GL_FLOAT, GL_FALSE,
        sizeof(aman_buffer_data_s), (GLvoid *)(3 * sizeof(float)));
    glVertexAttribPointer(m_loc_texcoord_ex, 2, GL_FLOAT, GL_FALSE,
        sizeof(aman_buffer_data_s), (GLvoid *)(5 * sizeof(float)));
    glVertexAttribPointer(m_loc_alpha_ex, 1, GL_FLOAT, GL_FALSE,
        sizeof(aman_buffer_data_s), (GLvoid *)(7 * sizeof(float)));
    glVertexAttribPointer(m_loc_choose_tex_id, 1, GL_FLOAT, GL_FALSE,
        sizeof(aman_buffer_data_s), (GLvoid *)(8 * sizeof(float)));
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_aman_idx_vbo);
    glEnableVertexAttribArray(m_loc_vertices);
    glEnableVertexAttribArray(m_loc_texcoord);
    glEnableVertexAttribArray(m_loc_texcoord_ex);
    glEnableVertexAttribArray(m_loc_alpha_ex);
    glEnableVertexAttribArray(m_loc_choose_tex_id);
    glDrawElements(GL_TRIANGLES, m_aman_mem_st_num / 4 * 6 , GL_UNSIGNED_INT, (void *)0);

    return 0;
}


/**
* @param type 0 for cam calib, 1 for fisheye, 2 for fisheye clip
*/
int amanGpu::aman_gpu_normal_front_process(int type)
{
    if (type < 0 || type > 2)
    {
        amanGPU_ERR("amanGpu::aman_gpu_normal_front_process. invild param\n");
        return 1;
    }

    glUseProgram(shader_get_program(m_cam_shader_handler));

    // load texture
    int ret = aman_gpu_process_pre_texture();
    if (ret != 0)
    {
        amanGPU_ERR("amanGpu::aman_gpu_normal_front_process. aman_gpu_process_pre_texture faild\n");
        return 1;
    }
    // uniform 0 right, 1 front, 2 left, 3 rear
    int tex_id = 0;
    if (0 == m_is_use_bmp)
    {
        tex_id = m_camera_uniform2tex[1];
    }
    else
    {
        tex_id = m_camera_uniform2tex_bmp[1];
    }
    glUniform1i(m_cam_loc_sampler, tex_id);

    if (0 == type)
    {
#if SUPPORT_FRONT_REAR_CALIB == 0
        return 1;
#else
        glBindBuffer(GL_ARRAY_BUFFER, m_aman_single_front_camera_vbo);
#endif
    }
    else if (1 == type)
    {
        glBindBuffer(GL_ARRAY_BUFFER, m_aman_fisheye_front_vbo);
    }
    else
    {
        glBindBuffer(GL_ARRAY_BUFFER, m_aman_fisheye_clip_front_vbo);
    }
    glVertexAttribPointer(m_cam_loc_vertices, 3, GL_FLOAT, GL_FALSE,
        sizeof(camera_data_s), (GLvoid *)(0 * sizeof(float)));
    glVertexAttribPointer(m_cam_loc_texcoord, 2, GL_FLOAT, GL_FALSE,
        sizeof(camera_data_s), (GLvoid *)(3 * sizeof(float)));
    glVertexAttribPointer(m_cam_loc_choose_tex_id, 1, GL_FLOAT, GL_FALSE,
        sizeof(camera_data_s), (GLvoid *)(5 * sizeof(float)));

    glUniformMatrix4fv(m_cam_loc_transform_mat, 1, GL_FALSE, glm::value_ptr(m_mvp));

    glEnableVertexAttribArray(m_cam_loc_vertices);
    glEnableVertexAttribArray(m_cam_loc_texcoord);
    glEnableVertexAttribArray(m_cam_loc_choose_tex_id);
    if (0 == type)
    {
        glDrawArrays(GL_TRIANGLES, 0, m_front_cam_point_num);
    }
    else if (1 == type)
    {
        glDrawArrays(GL_TRIANGLES, 0, 6);
    }
    else
    {
        glDrawArrays(GL_TRIANGLES, 0, 6);
    }

    return 0;
}


/**
* @brief rear camera view, include fisheye correction, fisheye wide angle, fisheye normal view
* @param type, 0 for fisheye correction view, 1 for fisheye wide angle without coord, 2 for fisheye without coord
*/
int amanGpu::aman_gpu_normal_rear_process(int type)
{
    if (type < 0 || type > 2)
    {
        amanGPU_ERR("amanGpu::aman_gpu_normal_rear_process. invild param\n");
        return 1;
    }

    glUseProgram(shader_get_program(m_cam_shader_handler));

    // load texture
    int ret = aman_gpu_process_pre_texture();
    if (ret != 0)
    {
        amanGPU_ERR("amanGpu::aman_gpu_normal_rear_process. aman_gpu_process_pre_texture faild\n");
        return 1;
    }

    // uniform 0 right, 1 front, 2 left, 3 rear
    int tex_id = 0;
    if (0 == m_is_use_bmp)
    {
        tex_id = m_camera_uniform2tex[3];    
    }
    else
    {
        tex_id = m_camera_uniform2tex_bmp[3];
    }
    glUniform1i(m_cam_loc_sampler, tex_id);

    if (0 == type)
    {
#if SUPPORT_FRONT_REAR_CALIB == 0
        return 1;
#else
        glBindBuffer(GL_ARRAY_BUFFER, m_aman_single_rear_camera_vbo);
#endif     
    }
    else if (1 == type)
    {
        glBindBuffer(GL_ARRAY_BUFFER, m_aman_fisheye_rear_vbo);
    }
    else
    {
        glBindBuffer(GL_ARRAY_BUFFER, m_aman_fisheye_clip_rear_vbo);
    }
    glVertexAttribPointer(m_cam_loc_vertices, 3, GL_FLOAT, GL_FALSE,
        sizeof(camera_data_s), (GLvoid *)(0 * sizeof(float)));
    glVertexAttribPointer(m_cam_loc_texcoord, 2, GL_FLOAT, GL_FALSE,
        sizeof(camera_data_s), (GLvoid *)(3 * sizeof(float)));
    glVertexAttribPointer(m_cam_loc_choose_tex_id, 1, GL_FLOAT, GL_FALSE,
        sizeof(camera_data_s), (GLvoid *)(5 * sizeof(float)));

    glUniformMatrix4fv(m_cam_loc_transform_mat, 1, GL_FALSE, glm::value_ptr(m_mvp));

    glEnableVertexAttribArray(m_cam_loc_vertices);
    glEnableVertexAttribArray(m_cam_loc_texcoord);
    glEnableVertexAttribArray(m_cam_loc_choose_tex_id);
    if (0 == type)
    {
        glDrawArrays(GL_TRIANGLES, 0, m_rear_cam_point_num);
    }
    else if (1 == type)
    {
        glDrawArrays(GL_TRIANGLES, 0, 6);
    }
    else
    {
        glDrawArrays(GL_TRIANGLES, 0, 6);
    }

    return 0;
}


int amanGpu::aman_gpu_normal_two_process()
{
    glUseProgram(shader_get_program(m_cam_shader_handler));

    // load texture
    int ret = aman_gpu_process_pre_texture();
    if (ret != 0)
    {
        amanGPU_ERR("amanGpu::aman_gpu_normal_rear_process. aman_gpu_process_pre_texture faild\n");
        return 1;
    }

    // uniform 0 right, 1 front, 2 left, 3 rear
    int tex_id_left = 0;
    int tex_id_right = 0;
    if (0 == m_is_use_bmp)
    {
        tex_id_left = m_camera_uniform2tex[2];
        tex_id_right = m_camera_uniform2tex[0];
    }
    else
    {
        tex_id_left = m_camera_uniform2tex_bmp[2];
        tex_id_right = m_camera_uniform2tex_bmp[0];
    }
    glUniform1i(m_cam_loc_sampler, tex_id_left);
    glUniform1i(m_cam_loc_sampler1, tex_id_right);

    glBindBuffer(GL_ARRAY_BUFFER, m_aman_two_camera_vbo);
    glVertexAttribPointer(m_cam_loc_vertices, 3, GL_FLOAT, GL_FALSE,
        sizeof(camera_data_s), (GLvoid *)(0 * sizeof(float)));
    glVertexAttribPointer(m_cam_loc_texcoord, 2, GL_FLOAT, GL_FALSE,
        sizeof(camera_data_s), (GLvoid *)(3 * sizeof(float)));
    glVertexAttribPointer(m_cam_loc_choose_tex_id, 1, GL_FLOAT, GL_FALSE,
        sizeof(camera_data_s), (GLvoid *)(5 * sizeof(float)));

    glUniformMatrix4fv(m_cam_loc_transform_mat, 1, GL_FALSE, glm::value_ptr(m_mvp));

    glEnableVertexAttribArray(m_cam_loc_vertices);
    glEnableVertexAttribArray(m_cam_loc_texcoord);
    glEnableVertexAttribArray(m_cam_loc_choose_tex_id);
    glDrawArrays(GL_TRIANGLES, 0, m_two_cam_point_num);

    return 0;
}


/**< 
* @brief load db data to struct(aman_buffer_data_s) array 
*        file have '*p_st_arr_num/4' num of {quad} struct
*        p_buffer_arr size is *p_st_arr_num
*        p_idx_arr size is *p_st_arr_num/4*6
* @param p_file, data file name, store 'quad' struct data. file:{quad}{quad}{quad}...
* @param p_buffer_arr, output, store 'aman_buffer_data_s' struct
* @param p_idx_arr, output, store opegl idx info.
* @param p_st_arr_num, output, file have num {quad} struct
*/
int amanGpu::load_db_table(char *p_file, float **p_buffer_arr,
        unsigned int **p_idx_arr, int *p_st_arr_num)
{
    if ((NULL == p_buffer_arr) || (NULL == p_idx_arr) || (NULL == p_st_arr_num))
    {
        aman_ERR("amanGpu::load_db_table. param error\n");
        return 1;
    }
    *p_buffer_arr = NULL;
    *p_idx_arr = NULL;
    *p_st_arr_num = 0;

    // alloc mem
    struct stat statbuff;
    if (stat(p_file, &statbuff) < 0)
    {
        aman_ERR("amanGpu::load_db_table. stat file faild\n");
        return 1;
    }
    unsigned long filesize = statbuff.st_size;
    int quad_num = filesize / sizeof(quad);
    float *p_mem = (float *)memalign(128, quad_num * 4 * sizeof(aman_buffer_data_s));
    if (NULL == p_mem)
    {
        aman_ERR("amanGpu::load_db_table. alloc aman mem faild\n");
        return 1;
    }
    unsigned int *p_idx_mem = (unsigned int *)memalign(128, quad_num * 6 * sizeof(unsigned int));
    if (NULL == p_idx_mem)
    {
        aman_ERR("amanGpu::load_db_table. alloc aman idx mem faild\n");
        return 1;
    }
    *p_buffer_arr = p_mem;
    *p_idx_arr = p_idx_mem;
    //*p_st_arr_num = quad_num * 4;

    // read db table and fill mem
    FILE *file_r = fopen(p_file, "rb");
    if (file_r == NULL)
    {
        aman_ERR("amanGpu::load_db_table. fopen failed\n");
        return 1;
    }
    int i;
    quad st;
    int vaild_quad_num = 0;
    float *p_mem_start = p_mem;
    p_mem_start = p_mem_start;
    unsigned int *p_idx_mem_start = p_idx_mem;
    for (i = 0; i < quad_num; i++)
    {
        fread(&st, sizeof(st), 1, file_r);

        // id=[0,1,2,3,4,5,6,7]
        if ((st.PosID < 0) || (st.PosID > 7))
        {
            continue;
        }
        vaild_quad_num++;

#define OBJ_aman_SET_MEM(_str1, _str2)                       \
            *p_mem_start++ = (float)st._str1.x;             \
            *p_mem_start++ = (float)st._str1.y;             \
            *p_mem_start++ = (float)st._str1.z;             \
            *p_mem_start++ = (float)st._str1.Tex_x;         \
            *p_mem_start++ = (float)st._str1.Tex_y;         \
            *p_mem_start++ = (float)st._str2.Tex_x;         \
            *p_mem_start++ = (float)st._str2.Tex_y;         \
            *p_mem_start++ = (float)st._str2.alpha;         \
            *p_mem_start++ = (float)st.PosID + 0.1f

        OBJ_aman_SET_MEM(LeftUp, _LeftUp);
        OBJ_aman_SET_MEM(LeftDown, _LeftDown);
        OBJ_aman_SET_MEM(RightDown, _RightDown);
        OBJ_aman_SET_MEM(RightUp, _RightUp);

        *p_idx_mem_start++ = vaild_quad_num*4 + 1;
        *p_idx_mem_start++ = vaild_quad_num*4 + 2;
        *p_idx_mem_start++ = vaild_quad_num*4;
        *p_idx_mem_start++ = vaild_quad_num*4 + 2;
        *p_idx_mem_start++ = vaild_quad_num*4 + 3;
        *p_idx_mem_start++ = vaild_quad_num*4;

#undef OBJ_aman_SET_MEM
    }
    fclose(file_r);

    *p_st_arr_num = vaild_quad_num * 4;

    amanGPU_LOG("amanGpu::load_db_table. inst_id=%d, file='%s', data mem have 'aman_buffer_data_s' num=%d\n", 
        m_inst_id, p_file, *p_st_arr_num);

    return 0;
}


int amanGpu::aman_gpu_load_db_light(char *p_file)
{
    struct stat statbuff;
    if (stat(p_file, &statbuff) < 0)
    {
        aman_ERR("amanGpu::aman_gpu_load_db_light. stat file faild\n");
        return 1;
    }
    unsigned long filesize = statbuff.st_size;
    int quad_num = filesize / sizeof(quad);

    // read file
    FILE *file_r = fopen(p_file, "rb");
    if (file_r == NULL)
    {
        aman_ERR("amanGpu::aman_gpu_load_db_light. fopen failed\n");
        return 1;
    }
    int i;
    quad st;
    int point_num[4] = {0};
    for (i = 0; i < quad_num; i++)
    {
        fread(&st, sizeof(st), 1, file_r);

        if (1 == st.PosID)
        {
            point_num[0]++;
        }
        else if (3 == st.PosID)
        {
            point_num[1]++;
        }
        else if (5 == st.PosID)
        {
            point_num[2]++;
        }
        else if (7 == st.PosID)
        {
            point_num[3]++;
        }        
    }
    aman_LOG("amanGpu::aman_gpu_load_db_light. [1]=%d, [2]=%d, [3]=%d, [4]=%d\n",
        point_num[0], point_num[1], point_num[2], point_num[3]);

    // alloc mem for light balance calc
    for (i = 0; i < 8; i++)
    {
        m_p_light_area[i] = (int *)malloc(point_num[i/2] * sizeof(int) * 2);
        if (NULL == m_p_light_area[i])
        {
            aman_ERR("amanGpu::aman_gpu_load_db_light. alloc %d light area mem faild\n", i);
            return 1;
        }
        m_light_area_num[i] = point_num[i/2] * 2;
    }

    // set point value
    float len_limit = 0.1f;
    float car_half_width = ini_get_car_w_length() + len_limit;      // 0.9+0.1
    float car_half_height = (ini_get_car_l_length() + ini_get_car_tt_length()
        + ini_get_car_tb_length()) / 2.0f + len_limit;  // (3.5+0.3+0.3)/2
    float rotate_center_y = ini_get_rotate_center_y();  // -1.4f
    float check_width = 2.0f;   // 
    int act_num[8] = {0};
    // p_start[0] for right_front image's right image coord part
    // p_start[1] for right_front image's front image coord part
    // etc
    fseek(file_r, 0, SEEK_SET);
    int *p_start[8];
    for (i = 0; i < 8; i++)
    {
        p_start[i] = m_p_light_area[i];
    }
    for (i = 0; i < quad_num; i++)
    {
        fread(&st, sizeof(st), 1, file_r);

        if ((st.PosID != 1) && (st.PosID != 3) && (st.PosID != 5) && (st.PosID != 7))
        {
            continue;
        }
        
        double x = st.RightDown.x;
        double y = st.RightDown.y;
        int x1 = (int)(st.RightDown.Tex_x * m_texture_width + 0.5f);
        int y1 = (int)(st.RightDown.Tex_y * m_texture_height + 0.5f);
        int x2 = (int)(st._RightDown.Tex_x * m_texture_width + 0.5f);
        int y2 = (int)(st._RightDown.Tex_y * m_texture_height + 0.5f);

        if (x1 < 0)
        {
            x1 = 0;
        }
        else if (x1 >= m_texture_width) 
        {
            x1 = m_texture_width - 1;
        }
        
        if (y1 < 0)
        {
            y1 = 0;
        }
        else if (y1 >= m_texture_height)
        {
            y1 = m_texture_height - 1;
        }
        
        if (x2 < 0)
        {
            x2 = 0;
        }        
        else if (x2 >= m_texture_width)
        {
            x2 = m_texture_width - 1;
        }

        if (y2 < 0)
        {
            y2 = 0;
        }
        else if (y2 >= m_texture_height)
        {
            y2 = m_texture_height - 1;
        }

        // right_front
        if ((1 == st.PosID) 
            && (x >= car_half_width) 
            && (x <= car_half_width + check_width)
            && (y >= car_half_height + rotate_center_y)
            && (y <= car_half_height + rotate_center_y + check_width))
        {
            *(p_start[0]++) = x1;
            *(p_start[0]++) = y1;
            *(p_start[1]++) = x2;
            *(p_start[1]++) = y2;
            act_num[0]++;
            act_num[1]++;
        }
        // front_left
        else if ((3 == st.PosID)
            && (x <= -1*car_half_width)
            && (x >= -1*(car_half_width + check_width))
            && (y >= car_half_height + rotate_center_y)
            && (y <= car_half_height + rotate_center_y + check_width))
        {
            *(p_start[2]++) = x1;
            *(p_start[2]++) = y1;
            *(p_start[3]++) = x2;
            *(p_start[3]++) = y2;
            act_num[2]++;
            act_num[3]++;
        }
        // left_rear
        else if ((5 == st.PosID)
            && (x <= -1*car_half_width)
            && (x >= -1*(car_half_width + check_width))
            && (y <= -1*(car_half_height - rotate_center_y))
            && (y >= -1*(car_half_height - rotate_center_y + check_width)))
        {
            *(p_start[4]++) = x1;
            *(p_start[4]++) = y1;
            *(p_start[5]++) = x2;
            *(p_start[5]++) = y2;
            act_num[4]++;
            act_num[5]++;
        }
        // rear_right
        else if ((7 == st.PosID)
            && (x >= car_half_width) 
            && (x <= car_half_width + check_width)
            && (y <= -1*(car_half_height - rotate_center_y))
            && (y >= -1*(car_half_height - rotate_center_y + check_width)))
        {
            *(p_start[6]++) = x1;
            *(p_start[6]++) = y1;
            *(p_start[7]++) = x2;
            *(p_start[7]++) = y2;
            act_num[6]++;
            act_num[7]++;
        }
    }
    fclose(file_r);

    aman_LOG("amanGpu::aman_gpu_load_db_light. act_num [1]=%d, [2]=%d, [3]=%d, [4]=%d\n",
        act_num[0], act_num[2], act_num[4], act_num[6]);
    for (i = 0; i < 8; i++)
    {
        m_light_area_num[i] = act_num[i] * 2;
    }

    return 0;
}


void amanGpu::aman_gpu_light_calc_avg_rgb(int area_id, unsigned char *p_img)
{
    int i;
    int point_cnt = amanGpu::m_light_area_num[area_id] / 2;
#if GLOBAL_RUN_ENV_DESKTOP == 1
    int point_vert = 4;     // rgba
    unsigned int rgb_sum[3] = {0};

    for (i = 0; i < point_cnt; i++)
    {
        int x = amanGpu::m_p_light_area[area_id][2*i];
        int y = amanGpu::m_p_light_area[area_id][2*i+1];
        int det = y * m_texture_width + x;

        rgb_sum[0] += p_img[det*point_vert];
        rgb_sum[1] += p_img[det*point_vert+1];
        rgb_sum[2] += p_img[det*point_vert+2];
    }
    m_light_avg_rgb[area_id][0] = rgb_sum[0] * 1.0 / point_cnt;
    m_light_avg_rgb[area_id][1] = rgb_sum[1] * 1.0 / point_cnt;
    m_light_avg_rgb[area_id][2] = rgb_sum[2] * 1.0 / point_cnt;
#else
    int point_vert = 2;     // yuyv
    unsigned int y_sum = 0;

    for (i = 0; i < point_cnt; i++)
    {
        int x = amanGpu::m_p_light_area[area_id][2*i];
        int y = amanGpu::m_p_light_area[area_id][2*i+1];
        int det = y * m_texture_width + x;

        y_sum += p_img[det*point_vert];
    }
    m_light_avg_y[area_id] = y_sum * 1.0 / point_cnt;
#endif

}


int amanGpu::aman_gpu_light_core(double *p_right, double *p_front, double *p_left, double *p_rear)
{
    int i;
    int thr = 15;
    double s_min = 1.0 - m_color_limit_det;
    double s_max = 1.0 + m_color_limit_det;
    double y[8];     // y,u,v

#if GLOBAL_RUN_ENV_DESKTOP == 1
    // if rgb is lower than threhold, return 1, faild 
    // SDTV with BT.601
    for (i = 0; i < 8; i++)
    {
        y[i] = 0.299 * m_light_avg_rgb[i][0]
            + 0.587 * m_light_avg_rgb[i][1]
            + 0.114 * m_light_avg_rgb[i][2];
        if (y[i] < thr)
        {
            return 1;
        }
    }
#else
        for (i = 0; i < 8; i++)
        {
            y[i] = m_light_avg_y[i];
            if (y[i] < thr)
            {
                return 1;
            }
        }
#endif


    // calc s param
    double a_y = y[0];
    double b_y = y[1];
    double c_y = y[2];
    double d_y = y[3];
    double e_y = y[4];
    double f_y = y[5];
    double g_y = y[6];
    double h_y = y[7];
    
    double K1_y = (c_y*d_y)/(b_y*b_y+c_y*c_y);
    double K2_y = (a_y*b_y)/(b_y*b_y+c_y*c_y);
    double K3_y = (e_y*f_y)/(f_y*f_y+g_y*g_y);
    double K4_y = (g_y*h_y)/(f_y*f_y+g_y*g_y); 

    double S2_y = (c_y*d_y*K2_y + e_y*f_y*K4_y)/(d_y*d_y - c_y*d_y*K1_y + e_y*e_y -e_y*f_y*K3_y);
    double S1_y = K1_y*S2_y + K2_y;
    double S3_y = K3_y*S2_y + K4_y;
    double S4_y = 1.0;

    // limit min and max val
    if (S2_y < s_min) S2_y = s_min;
    else if (S2_y > s_max) S2_y = s_max;

    if (S3_y < s_min) S3_y = s_min;
    else if (S3_y > s_max) S3_y = s_max;

    if (S4_y < s_min) S4_y = s_min;
    else if (S4_y > s_max) S4_y = s_max;
    
    if (S1_y < s_min) S1_y = s_min;
    else if (S1_y > s_max) S1_y = s_max;

    // let darkness cam as reference
    double max = -1.0;
    double s4[4] = {S1_y, S2_y, S3_y, S4_y};
    for (i = 0; i < 4; i++)
    {
        if (s4[i] > max)
        {
            max = s4[i];
        }
    }
    S1_y /= max;
    S2_y /= max;
    S3_y /= max;
    S4_y /= max;

    *p_right = S4_y;
    *p_front = S1_y;
    *p_left  = S2_y;
    *p_rear  = S3_y;

    return 0;
}


/**
* @brief light balance process, gen kr, kg, kb param
* @param p0, right image rgba addr
* @param p0, front image rgba addr
* @param p0, left image rgba addr
* @param p0, rear image rgba addr
*/
int amanGpu::aman_gpu_light_balance(unsigned char *p0, unsigned char *p1,
        unsigned char *p2, unsigned char *p3)
{
    int i;
    // camera hide logic. if have at least one camera hide, light balance is disabled
    // just return 0 to change the color_k value immediately, instead of dynamically change process below
    int is_hide = 0;
    aman_gpu_hide_camera_logic(&is_hide);
    if (is_hide)
    {
        m_light_prc_cnt = 0;    // set proc cnt, when door is close, re calc param
        return 0;
    }

    // if share light balace and this instance need not to do calc, return
    if (m_s_is_share_lb && (0 == m_is_do_lb_process))
    {
        for (i = 0; i < 4; i++)
        {
            m_color_k[i] = amanGpu::m_s_share_lb.m_color_k[i];
        }
        return 0;
    }

    // change color step by step
    //float mag = DEFAULT_FPS * 1.0 / m_fps;
    float mag = 1.0f;
    double step = 0.008 * mag;
    for (int i = 0; i < 4; i++)
    {
        double dst_color_k = m_dst_color_k[i];
        double src_color_k = m_color_k[i];
        double det = dst_color_k - src_color_k;
        double cur_step = (fabs(det) / m_color_limit_det) * (step * 10);
        double tmp_dst_color = 0.0;
        if (cur_step < step)
        {
            cur_step = step;
        }

        if ((det > -1.0*step) && (det < step))
        {
            m_color_k[i] = dst_color_k;
        }
        else if (det >= step)
        {
            tmp_dst_color = src_color_k + cur_step;
            if (tmp_dst_color > dst_color_k)
            {
                tmp_dst_color = dst_color_k;
            }
            m_color_k[i] = tmp_dst_color;
        }
        else
        {
            tmp_dst_color = src_color_k - cur_step;
            if (tmp_dst_color < dst_color_k)
            {
                tmp_dst_color = dst_color_k;
            }
            m_color_k[i] = tmp_dst_color;
        }
    }

    // if light balance share, set share value
    if (m_s_is_share_lb)
    {
        for (i = 0; i < 4; i++)
        {
            amanGpu::m_s_share_lb.m_color_k[i] = m_color_k[i];
        }
    }

    // calc new dst color
    // we have 9 steps to complete the calculation
    // the first 8 steps calculate the average brightness of each area
    // and the last step calculates dst_k by prev avg light
    int div = m_light_prc_cnt % 9;
    m_light_prc_cnt++;
    if (m_light_prc_cnt == 9)
    {
        m_light_prc_cnt = 0;
    }
    // step 0, calc right image's area0 avg light
    if (0 == div)
    {
        aman_gpu_light_calc_avg_rgb(0, p0);
    }
    else if (1 == div)
    {
        aman_gpu_light_calc_avg_rgb(1, p1);
    }
    else if (2 == div)
    {
        aman_gpu_light_calc_avg_rgb(2, p1);
    }
    else if (3 == div)
    {
        aman_gpu_light_calc_avg_rgb(3, p2);
    }
    else if (4 == div)
    {
        aman_gpu_light_calc_avg_rgb(4, p2);
    }
    else if (5 == div)
    {
        aman_gpu_light_calc_avg_rgb(5, p3);
    }
    else if (6 == div)
    {
        aman_gpu_light_calc_avg_rgb(6, p3);
    }
    else if (7 == div)
    {
        aman_gpu_light_calc_avg_rgb(7, p0);
    }
    else if (8 == div) // step 8, calc dst color_k
    {
        double right_val = 1.0;
        double front_val = 1.0;
        double left_val  = 1.0;
        double rear_val  = 1.0;
        if (0 == aman_gpu_light_core(&right_val, &front_val, &left_val, &rear_val))
        {
            m_dst_color_k[0] = right_val;
            m_dst_color_k[1] = front_val;
            m_dst_color_k[2] = left_val;
            m_dst_color_k[3] = rear_val;
        }
    }

    return 0;
}


/**< light balance process function */
int amanGpu::aman_gpu_light_balance_process()
{
    // calc light balance k param
    if (0 == m_is_use_bmp)
    {
#if GLOBAL_RUN_ENV_DESKTOP == 0
        int tex_right = m_camera_uniform2tex[0];
        int tex_front = m_camera_uniform2tex[1];
        int tex_left  = m_camera_uniform2tex[2];
        int tex_rear  = m_camera_uniform2tex[3];
        aman_gpu_light_balance(m_texture_vaddr[tex_right], m_texture_vaddr[tex_front], 
            m_texture_vaddr[tex_left], m_texture_vaddr[tex_rear]);
#endif
    }
    else
    {
        aman_gpu_light_balance(m_texture_vaddr[0], m_texture_vaddr[1],
            m_texture_vaddr[2], m_texture_vaddr[3]);
    }

    return 0;
}


int amanGpu::aman_gpu_load_db_camera(char *p_file, float **p_dst, int *p_point_num)
{
    struct stat statbuff;
    if (stat(p_file, &statbuff) < 0)
    {
        aman_ERR("amanGpu::aman_gpu_load_db_camera. stat file faild\n");
        return 1;
    }
    unsigned long filesize = statbuff.st_size;
    *p_point_num = filesize / sizeof(camera_data_s);

    FILE *file_r = fopen(p_file, "rb");
    *p_dst = (float *)memalign(128, filesize);
    if (NULL == *p_dst)
    {
        aman_ERR("amanGpu::aman_gpu_load_db_camera. alloc mem faild\n");
        return 1;
    }
    fread(*p_dst, filesize, 1, file_r);
    fclose(file_r);

    aman_LOG("amanGpu::aman_gpu_load_db_camera. inst_id=%d, file='%s' success\n", 
        m_inst_id, p_file);

    return 0;
}


int amanGpu::aman_gpu_load_db_fisheye(char *p_file)
{
    struct stat statbuff;
    FILE *file_r = NULL;
    if (stat(p_file, &statbuff) < 0)
    {
        aman_ERR("amanGpu::aman_gpu_load_db_fisheye. stat file faild\n");
        aman_ERR("amanGpu::aman_gpu_load_db_fisheye. use init default fisheye value\n");
        //return 1;
    }
    else
    {
        file_r = fopen(p_file, "rb");
        fread(&m_fisheye_front, sizeof(m_fisheye_front), 1, file_r);
        fread(&m_fisheye_rear, sizeof(m_fisheye_rear), 1, file_r);
        fclose(file_r);
    }

    int re;
    int ini_vaild = 0;
    int lt_x, lt_y, rb_x, rb_y;
    const int camera_offset_x_max = 30; //depend on test result of new camera
    // fisheye front view
    re = ini_get_d2_f_front(&ini_vaild, &lt_x, &lt_y, &rb_x, &rb_y);
    if (re != 0)
    {
		aman_ERR("amanGpu::aman_gpu_load_db_fisheye. read fisheye front config faild\n");
		aman_ERR("amanGpu::aman_gpu_load_db_fisheye. fish front default\n");
		ini_vaild = 0;
		//return 1;
    }
    if (ini_vaild)
    {
        m_fisheye_front_orth[0] = -1.0 + lt_x*1.0 / m_texture_width * 2.0;
        m_fisheye_front_orth[1] = rb_x*1.0 / m_texture_width * 2.0 - 1.0;
        m_fisheye_front_orth[2] = 1.0 - rb_y*1.0 / m_texture_height * 2.0;
        m_fisheye_front_orth[3] = 1.0 - lt_y*1.0 / m_texture_height * 2.0;          
    }
    else
    {
        float offset_x = m_fisheye_front.cx - m_texture_width/2.0;
        aman_LOG("aman_gpu_load_db_fisheye. f: offset(%f)\n", offset_x);
        if(camera_offset_x_max*2 < offset_x)
        {
            offset_x = 0;
        }

        re = ini_get_d2_f_front_auto(&lt_y, &rb_y);
        if (re != 0)
        {
            aman_ERR("amanGpu::aman_gpu_load_db_fisheye. read fish front auto config faild\n");
            aman_ERR("amanGpu::aman_gpu_load_db_fisheyes. fish front auto set default 0, height\n");
            lt_y = 0;
            rb_y = m_texture_height;
            //return 1;
        }

        float scale_fc = 1280.0/656.0;   // TODO: 
        float screen_width = (rb_y-lt_y)*scale_fc;
        lt_x = (m_texture_width-screen_width)/2; //calc by scale
        rb_x = lt_x+screen_width; //calc by scale

        float left = -1.0 + (lt_x+offset_x)*1.0 / m_texture_width * 2.0;
        float right = (rb_x+offset_x)*1.0 / m_texture_width * 2.0 - 1.0;
        float bottom = 1.0 - rb_y*1.0 / m_texture_height * 2.0;
        float top = 1.0 - lt_y*1.0 / m_texture_height * 2.0;

        m_fisheye_front_orth[0] = left;
        m_fisheye_front_orth[1] = right;
        m_fisheye_front_orth[2] = bottom;
        m_fisheye_front_orth[3] = top;
    }

    // fisheye rear view
    re = ini_get_d2_f_rear(&ini_vaild, &lt_x, &lt_y, &rb_x, &rb_y);
    if (re != 0)
    {
		aman_ERR("amanGpu::aman_gpu_load_db_fisheye. read rear config faild\n");
		aman_ERR("amanGpu::aman_gpu_load_db_fisheye. rear set as default\n");
		ini_vaild = 0;
		//return 1;
    }
    if (ini_vaild)
    {
        m_fisheye_rear_orth[0] = -1.0 + lt_x*1.0 / m_texture_width * 2.0;
        m_fisheye_rear_orth[1] = rb_x*1.0 / m_texture_width * 2.0 - 1.0;
        m_fisheye_rear_orth[2] = 1.0 - rb_y*1.0 / m_texture_height * 2.0;
        m_fisheye_rear_orth[3] = 1.0 - lt_y*1.0 / m_texture_height * 2.0;          
    }
    else
    {
        float offset_x = m_fisheye_rear.cx - m_texture_width/2.0;
        aman_LOG("aman_gpu_load_db_fisheye. r: offset(%f)\n", offset_x);
        if(camera_offset_x_max*2 < offset_x)
        {
            offset_x = 0;
        }

        re = ini_get_d2_f_rear_auto(&lt_y, &rb_y);
        if (re != 0)
        {
            aman_ERR("amanGpu::aman_gpu_load_db_fisheye. read fish rear auto config faild\n");
            aman_ERR("amanGpu::aman_gpu_load_db_fisheye. fish fish rear auto set as default\n");
            lt_y = 0;
            rb_y = m_texture_height;
            //return 1;
        }

        float scale_fc = 1280.0/656.0;   // TODO:
        float screen_width = (rb_y-lt_y)*scale_fc;
        lt_x = (m_texture_width-screen_width)/2; //calc by scale
        rb_x = lt_x+screen_width; //calc by scale

        float left = -1.0 + (lt_x-offset_x)*1.0 / m_texture_width * 2.0; //mirrow
        float right = (rb_x-offset_x)*1.0 / m_texture_width * 2.0 - 1.0; //mirrow
        float bottom = 1.0 - rb_y*1.0 / m_texture_height * 2.0;
        float top = 1.0 - lt_y*1.0 / m_texture_height * 2.0;

        m_fisheye_rear_orth[0] = left;
        m_fisheye_rear_orth[1] = right;
        m_fisheye_rear_orth[2] = bottom;
        m_fisheye_rear_orth[3] = top;
    }    

    // fisheye front clip
    re = ini_get_d2_fc_front(&ini_vaild, &lt_x, &lt_y, &rb_x, &rb_y);
    if (re != 0)
    {
		aman_ERR("amanGpu::aman_gpu_load_db_fisheye. read front clip config faild\n");
        aman_ERR("amanGpu::aman_gpu_load_db_fisheye. set front clip config as default\n");
        ini_vaild = 0;
		//return 1;
    }
    if (ini_vaild)
    {
        m_fisheye_clip_front_orth[0] = -1.0 + lt_x*1.0 / m_texture_width * 2.0;
        m_fisheye_clip_front_orth[1] = rb_x*1.0 / m_texture_width * 2.0 - 1.0;
        m_fisheye_clip_front_orth[2] = 1.0 - rb_y*1.0 / m_texture_height * 2.0;
        m_fisheye_clip_front_orth[3] = 1.0 - lt_y*1.0 / m_texture_height * 2.0;  
    }
    else
    {
   		//do not use config, calc automatically
        float offset_x = m_fisheye_front.cx - m_texture_width/2.0;
        aman_LOG("aman_gpu_load_db_fisheye. fc: offset(%f)\n", offset_x);
        if(camera_offset_x_max*2 < offset_x)
        {
            offset_x = 0;
        }

        re = ini_get_d2_fc_front_auto(&lt_y, &rb_y);
        if (re != 0)
        {
            aman_ERR("amanGpu::aman_gpu_load_db_fisheye. read front clip auto config faild\n");
            aman_ERR("amanGpu::aman_gpu_load_db_fisheye. set front clip auto as default\n");
            lt_y = 0;
            rb_y = m_texture_height;
            //return 1;
        }

        float scale_fc = 880.0/656.0;    // TODO:
        float screen_width = (rb_y-lt_y)*scale_fc;
        lt_x = (m_texture_width-screen_width)/2; //calc by scale
        rb_x = lt_x+screen_width; //calc by scale

        float left = -1.0 + (lt_x+offset_x)*1.0 / m_texture_width * 2.0;
        float right = (rb_x+offset_x)*1.0 / m_texture_width * 2.0 - 1.0;
        float bottom = 1.0 - rb_y*1.0 / m_texture_height * 2.0;
        float top = 1.0 - lt_y*1.0 / m_texture_height * 2.0;

        m_fisheye_clip_front_orth[0] = left;
        m_fisheye_clip_front_orth[1] = right;
        m_fisheye_clip_front_orth[2] = bottom;
        m_fisheye_clip_front_orth[3] = top;
    }

    // fish eye rear clip
    re = ini_get_d2_fc_rear(&ini_vaild, &lt_x, &lt_y, &rb_x, &rb_y);
    if (re != 0)
    {
		aman_ERR("amanGpu::aman_gpu_load_db_fisheye. read rear clip config faild\n");
        aman_ERR("amanGpu::aman_gpu_load_db_fisheye. set rear clip config as default\n");
        ini_vaild = 0;
		//return 1;
    }
    if (ini_vaild)
    {
        m_fisheye_clip_rear_orth[0] = -1.0 + lt_x*1.0 / m_texture_width * 2.0;
        m_fisheye_clip_rear_orth[1] = rb_x*1.0 / m_texture_width * 2.0 - 1.0;
        m_fisheye_clip_rear_orth[2] = 1.0 - rb_y*1.0 / m_texture_height * 2.0;
        m_fisheye_clip_rear_orth[3] = 1.0 - lt_y*1.0 / m_texture_height * 2.0;  
    }
    else
    {
        float offset_x = m_fisheye_rear.cx - m_texture_width/2.0;
        aman_LOG("aman_gpu_load_db_fisheye. rc: offset(%f)\n", offset_x);
        if(camera_offset_x_max*2 < offset_x)
        {
            offset_x = 0;
        }

        re = ini_get_d2_fc_rear_auto(&lt_y, &rb_y);
        if (re != 0)
        {
            aman_ERR("amanGpu::aman_gpu_load_db_fisheye. read rear clip auto config faild\n");
            aman_ERR("amanGpu::aman_gpu_load_db_fisheye. set rear clip auto as default\n");
            lt_y = 0;
            rb_y = m_texture_height;
            //return 1;
        }

        float scale_fc = 880.0/656.0;     // TODO:
        float screen_width = (rb_y-lt_y)*scale_fc;
        lt_x = (m_texture_width-screen_width)/2; //calc by scale
        rb_x = lt_x+screen_width; //calc by scale

        float left = -1.0 + (lt_x-offset_x)*1.0 / m_texture_width * 2.0; //mirrow
        float right = (rb_x-offset_x)*1.0 / m_texture_width * 2.0 - 1.0; //mirrow
        float bottom = 1.0 - rb_y*1.0 / m_texture_height * 2.0;
        float top = 1.0 - lt_y*1.0 / m_texture_height * 2.0;
        
        m_fisheye_clip_rear_orth[0] = left;
        m_fisheye_clip_rear_orth[1] = right;
        m_fisheye_clip_rear_orth[2] = bottom;
        m_fisheye_clip_rear_orth[3] = top;

    }

    return 0;
}


void amanGpu::aman_gpu_hide_camera_logic(int *p_is_hide)
{
    // process hide camera. 
    // when hide change, set color mul param to 0.0 to set camera to black
    if (m_is_hide_change)
    {
        m_is_hide_change = 0;
        
        if (m_is_hide_right)
        {
            m_color_k[0] = 0.0f;
        }
        else
        {
            m_color_k[0] = 1.0f;           
        }
        
        if (m_is_hide_front)
        {
            m_color_k[1] = 0.0f;
        }
        else
        {
            m_color_k[1] = 1.0f;
        }
        
        if (m_is_hide_left)
        {
            m_color_k[2] = 0.0f;
        }
        else
        {
            m_color_k[2] = 1.0f;
        }
        
        if (m_is_hide_rear)
        {
            m_color_k[3] = 0.0f;
        }
        else
        {
            m_color_k[3] = 1.0f;
        }
    }
    // in hide state, stop light balance process
    if (m_is_hide_right || m_is_hide_front || m_is_hide_left || m_is_hide_rear)
    {
        *p_is_hide = 1;
    }
    else
    {
        *p_is_hide = 0;
    }
}


/* [csj 20180929-8] */
int amanGpu::aman_gpu_get_view_by_cur_deg(void)
{
	if(IF_IN_RANGE(m_curview_rot_ang, 0, 45))
	{
		return aman_VIEW_3D_LEFT_CENTER;
	}
	else if(IF_IN_RANGE(m_curview_rot_ang, 45, 90))
	{
		return aman_VIEW_3D_LEFT_FRONT;
	}
	else if(IF_IN_RANGE(m_curview_rot_ang, 90, 135))
	{
		return aman_VIEW_3D_FRONT;
	}
	else if(IF_IN_RANGE(m_curview_rot_ang, 135, 180))
	{
		return aman_VIEW_3D_RIGHT_FRONT;
	}
	else if(IF_IN_RANGE(m_curview_rot_ang, -180, -135))
	{
		return aman_VIEW_3D_RIGHT_CENTER;
	}
	else if(IF_IN_RANGE(m_curview_rot_ang, -135, -90))
	{
		return aman_VIEW_3D_RIGHT;
	}
	else if(IF_IN_RANGE(m_curview_rot_ang, -90, -45))
	{
		return aman_VIEW_3D_REAR;
	}
	else if(IF_IN_RANGE(m_curview_rot_ang, -45, 0))
	{
		return aman_VIEW_3D_LEFT;
	}

	aman_ERR("invalid angle(%f)\n", m_curview_rot_ang);
	return aman_VIEW_UNKNOW;
}


