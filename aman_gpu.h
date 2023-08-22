

#ifndef _aman_GPU_H_
#define _aman_GPU_H_

#include <pthread.h>
#include "common_def.h"
#include "GLES2/gl2.h"
#include <glm/mat4x4.hpp>

#define amanGPU_LOG(...)     aman_LOG(__VA_ARGS__)
#define amanGPU_ERR(...)     aman_ERR(__VA_ARGS__)

#include <QImage>
#include <QMutex>
#if GLOBAL_RUN_ENV_DESKTOP == 0
#include "EGL/egl.h"
#else
// in desktop app, we use qimage to load image file
#include <QMatrix4x4>
#endif


/**< open perspective angle */
#define GPU_PERSPECTIVE_ANGLE   (45.0f)


/**
 * @brief The aman_VIEW_E enum
 * descrip all view port which aman support
 */
typedef enum _aman_VIEW_ENUM_
{
    aman_VIEW_AUTO = 0,          /**< bird view */
    aman_VIEW_3D_FRONT = 1,
    aman_VIEW_3D_REAR = 2,
    aman_VIEW_3D_LEFT = 3,
    aman_VIEW_3D_RIGHT = 4,
    aman_VIEW_2D_FRONT = 5,
    aman_VIEW_2D_REAR = 6,
    aman_VIEW_CAMERA_FRONT = 7,
    aman_VIEW_CAMERA_REAR = 8,
    aman_VIEW_CAMERA_LEFTRIGHT = 9,  /**< left camera and right camera at screen left and right */

    aman_VIEW_3D_LEFT_FRONT = 10,
    aman_VIEW_3D_LEFT_CENTER = 11,
    aman_VIEW_3D_RIGHT_FRONT = 12,
    aman_VIEW_3D_RIGHT_CENTER = 13,

    aman_VIEW_CAMERA_FISH_FRONT = 14,    /*< origin fish eye view, clip less */
    aman_VIEW_CAMERA_FISH_REAR = 15,
    aman_VIEW_CAMERA_LEFTRIGHT_CPY = 16,     /**< same as aman_VIEW_CAMERA_LEFTRIGHT */
    aman_VIEW_CAMERA_FISH_CLIP_FRONT = 17,   /**< fisheye clip, fit left view width */
    aman_VIEW_CAMERA_FISH_CLIP_REAR = 18,

    aman_VIEW_3D_FREE = 19,      /**< in 3d free 360 degree rotate */
    
    aman_VIEW_LAST = aman_VIEW_3D_FREE,
    aman_VIEW_UNKNOW = 99
}aman_VIEW_E;


/**
* fisheye clip param
*/
typedef struct _fisheye_clip_param_struct_
{
    int xl;     /**< x left, eg 0 */
    int xr;     /**< xr to left, eg, 1280*/
    int yt;     /**< y top coord to top, eg, 0 */
    int yb;     /**< y bottom coord to top, eg, 720 */

    int act_xl;
    int act_xr;
    int cx;     /**< center x and y */
    int cy;
}fisheye_clip_param_s;


/**
* opengl context share, vbo res
*/
typedef struct _amangpu_share_vbo_struct_
{
    GLuint m_aman_table_vbo;
    GLuint m_aman_idx_vbo;
    GLuint m_aman_single_front_camera_vbo;
    GLuint m_aman_single_rear_camera_vbo;
    GLuint m_aman_two_camera_vbo;
    GLuint m_aman_fisheye_front_vbo;
    GLuint m_aman_fisheye_rear_vbo;
    GLuint m_aman_fisheye_clip_front_vbo;
    GLuint m_aman_fisheye_clip_rear_vbo;
    int m_aman_mem_st_num;
    int m_front_cam_point_num;  /**< for fisheye correction view */
    int m_rear_cam_point_num;   /**< for fisheye correction view */
    int m_two_cam_point_num;    
}amangpu_share_vbo_s;


/**
* opengl context share, texture res
*/
typedef struct _amangpu_share_texure_struct_
{
    GLuint m_texture_id[4]; /**< texture id array, save glGenTextures result(4 camera image texture) */
    unsigned char *m_texture_vaddr[4];
    unsigned int m_texture_paddr[4];
    unsigned int m_tex_uid_l;
    unsigned int m_tex_uid_h;
    int m_tex_updated;  /**< is opengl texture updated */
    int m_video_fc[4];  /**< if pc env use video, video frame cnt */
}amangpu_share_texure_s;


/**
* 3d stitch view, light balance share
*/
typedef struct _amangpu_share_ltbalance_struct_
{
    float m_color_k[4];    /**< four image's rgb param. */
    float m_fps;
}amangpu_share_ltbalance_s;


/**< get chn image's paadr and vaadr, p_ts is image v4l2 capture ms */
typedef int (get_chn_image_addr_cbfuc)(int chnidx, unsigned int *p_paddr, unsigned char **p_vaddr, unsigned int *p_ts);
/**< release all image */
typedef int (rel_chn_image_addr_cbfuc)();


/**
 * @brief The amanGpu class
 */
class amanGpu
{
public:
    /**
    * @param car_type, car type, 1 for lk01, 2 for 02, 3 for 03, 4 for dcy11
    * @param proj_width, opengl projection marrix width, usually the width of parent's widget
    * @param proj_height
    * @param tex_width, input texture image width, usually camera image width
    * @param tex_height
    * @param p_get_func, get tex image func and release func
    * @param is_share_context_vbo, is param widget instances is share with each other, in this case, vbo & texture only create and init once
    * @param is_share_context_texture, texture share
    */
    amanGpu(int car_type, int proj_width, int proj_height,
        int proj_wideangle_width, int proj_wideangle_height,
        int tex_width, int tex_height,
        get_chn_image_addr_cbfuc *p_get_func,
        rel_chn_image_addr_cbfuc *p_release_func,
        int is_share_context_vbo, int is_share_context_texture);
    ~amanGpu();

    /** 
    * @brief load .data file, init opengl env, etc 
    * @param is_use_fbo, if 1, use opengl fbo texture as the output of the gl program, and then output to the FB
    *        if 1, gl output->fbo texture->linux fb, this is usefull when you want to get gl output for post process(eg, alg use stitch image as input)
    *        if 0, gl output->linux fb
    */
    int amangpu_init(void);

    /** set eye z position in bird view, height=x meter(have default value even not set) */
    int aman_gpu_postion_set_auto_height(float height)
    {
        m_rotate_auto_top_height = height;
        return 0;
    }

    /**< get auto-view eye pos z height */
    float aman_gpu_postion_get_auto_height()
    {
        return m_rotate_auto_top_height;
    }

    /** 
    * @brief set opengl projection width&height, uaually parent widget wh change
    * @param proj_w, new projection width, opengl ouput width will change in next gl draw
    * @param proj_h
    */
    void aman_gpu_update_wh(int proj_w, int proj_h)
    {
        m_projWidth = proj_w;
        m_projHeight = proj_h;
    }

    /**
     * @brief aman_gpu_set_view, set module output view
     *        ouput view will change immediately, if you want to change the view dynamically, you can use func 'aman_gpu_set_view_dyn_trans'
     * @param view_enum, value in aman_VIEW_E
     * @param is_force, if force==0 and current_view==view_enum, this func will do nothing. otherwise will do setting view process
     */
    int aman_gpu_set_view(int view_enum, int is_force = 0);

    /**
    * @brief dyn trans from cur view to dst view
    *        only if both views are 3D, have dynamic animation, otherwise equal to 'aman_gpu_set_view'
    */
    int aman_gpu_set_view_dyn_trans(int view_enum);

    /**< get current view_enum, aman_VIEW_E */
    int aman_gpu_get_view()
    {
        return m_current_view;
    }

    /**
     * @brief whether it is currently in dynamic transformation
     *        eg. form 3d left to 3d right. if in dyn trans, action will take time 
     */
    int aman_gpu_is_in_dyn_view()
    {
        return m_is_in_view_trans;
    }

    /**
    * @brief set 3d rot center x and y, (meter)
    *        colth center wihch for calib is not the center of opengl rotate center
    */
    void aman_gpu_set_rotate_center_coord(float center_x, float center_y)
    {
        m_rotate_center_x = center_x;
        m_rotate_center_y = center_y;
    }

    /**
    * @brief Horizontal rotation 3d view
    * @param is_angle, use param view_angle for rotate, x_det is invivld
    * @param x_det, 
    * @param view_angle, dst view angle
    */
    int aman_gpu_set_rotate(int is_angle, int x_det, float view_angle);

    /**
    * @brief rotation 3d view, both horizontal and vertical directions can be rotated
    * @param x_det
    */
    int aman_gpu_omni_directional_rotate(int x_det, int y_det);

    /**
    * @brief Horizontal rotation 3d view
    *        the difference between this function and 'aman_gpu_set_rotate' is that this func use det rotation angle
    * @param det_angle, 
    */
    int aman_gpu_set_rotate_det_angle(float det_angle);

    /**
    * @brief get current 3d view's rotate angle(hor rotate)
    */
    float aman_gpu_get_rotate_angle()
    {
        return m_curview_rot_ang;
    }

    /**
    * @brief get opengl project matrix and view matrix
    */
    void aman_gpu_get_v_p(glm::mat4 *p_v, glm::mat4 *p_p);

    int aman_gpu_check_in_3d_view(int view_enum);

    int aman_gpu_check_in_pure_3d_view(int view_enum);

    /**
    * @brief hide stitch camera, can be used when door is open
    * @param 0 not hide, 1 hide. -1 unchange
    */
    void aman_gpu_hide_stitch(int is_hide_right, int is_hide_front,
        int is_hide_left, int is_hide_rear);

    /**
     * @brief aman_gpu_process, process don't change width and height
     * @param ext_param, 0 for main proc, calc light balance independently, 1 will use static value for light balance
     * @return
     */
    int aman_gpu_process();

    /**
    * @brief set widget output fps, used for frequency of the light balance
    */
    void aman_gpu_set_fps(float fps)
    {
        m_s_share_lb.m_fps = fps;
    }

    int aman_gpu_get_view_by_cur_deg(void);

private:
    /** load .data file to mem and do init */
    int aman_gpu_init_aman_table_data();  

    /**< create opengl program, and get attr location */
    int aman_gpu_env_init();

    /**< use mem buff data to gen opengl vbo */
    int aman_gpu_load_data_vbo();
    
    /** @brief opengl postion value init, view_enum dependent value, etc. */
    int aman_gpu_postion_init();

    /**< get time ms */
    unsigned int get_time_ms();

    /**< u64++ */
    void uint64_inc(unsigned int *p_h, unsigned int *p_l);

    /**< u64 comp */
    int uint64_cmp(unsigned int *p1_h, unsigned int *p1_l, unsigned int *p2_h, unsigned int *p2_l);

    /**< u64 set */
    void uint64_set(unsigned int *p1_h, unsigned int *p1_l, unsigned int *p2_h, unsigned int *p2_l);

    /**< lock */
    void share_lock();

    /**< unlock */
    void share_unlock();

    /**< update texute image address */
    int aman_gpu_update_dev_img_addr();
    int aman_gpu_update_pcenv_img_addr(int is_bmp_seq);
    int aman_gpu_update_img_addr();

    /**< update viv texture based on texture image address */
    int aman_gpu_update_viv_gltex();

    /**< update image tex */
    int aman_gpu_update_bmp_gltex();

    /**< generate opengl texture */
    int aman_gpu_process_pre_texture();  

    /**< @brief aman_gpu_texture_init, create texture */
    int aman_gpu_texture_init();

     /**< @brief opengl vbo init */
    int aman_gpu_vbo_init();    

    /**< load llight balance data */
    int aman_gpu_light_balance_init();

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
    int load_db_table(char *p_file, float **p_buffer_arr, 
        unsigned int **p_idx_arr, int *p_st_arr_num);

    /**< 3d view stitch opengl es process */
    int aman_gpu_stitch_process();

    /**
    * @brief front camera view, include fisheye correction, fisheye wide angle, fisheye normal view
    * @param type, 0 for fisheye correction view, 1 for fisheye wide angle without coord, 2 for fisheye without coord
    */
    int aman_gpu_normal_front_process(int type);

    
    int aman_gpu_normal_rear_process(int type);
    int aman_gpu_normal_two_process();
    int aman_gpu_view_dyn_process();
    float aman_gpu_get_view_deg(int view_enum);    
    int aman_gpu_load_db_light(char *p_file);   /**< read db data and set light array data */
    void aman_gpu_light_calc_avg_rgb(int area_id, unsigned char *p_img);
    int aman_gpu_light_core(double *p_right, double *p_front, double *p_left, double *p_rear);
    int aman_gpu_light_balance(unsigned char *p0, unsigned char *p1,
        unsigned char *p2, unsigned char *p3);    /**< light balance process, gen kr, kg, kb param */
    int aman_gpu_light_balance_process();    /**< light balance process function */
	int aman_gpu_load_db_camera(char *p_file, float **p_dst, int *p_point_num);	/**< load single or two camera calib data */
    void aman_gpu_hide_camera_logic(int *p_is_hide);
    int aman_gpu_load_db_fisheye(char *p_file);

    // stitch loc
private:
    void *m_shader_handler;   /**< handler of shader_load_str return val */
    GLint m_loc_vertices;     /**< location of vertices */
    GLint m_loc_texcoord;     /**< location of texcoord */
    GLint m_loc_texcoord_ex;  /**< location of ex texcoord, see aman_gpu_table.h for detail */
    GLint m_loc_choose_tex_id;   /**< [0, 7]. choose which texture (0-3) be use or mix */
    GLint m_loc_alpha_ex;    /**< second texture's alpha value */

    GLint m_loc_transform_mat;      /**< loc transform matrix */
    GLint m_loc_sampler;        /**< texture sampler */
    GLint m_loc_sampler1;
    GLint m_loc_sampler2;
    GLint m_loc_sampler3;
    GLint m_loc_colork;

    // single or two camera loc
private:
	void *m_cam_shader_handler;
	GLint m_cam_loc_vertices;
	GLint m_cam_loc_texcoord;
	GLint m_cam_loc_sampler;
	GLint m_cam_loc_sampler1;
	GLint m_cam_loc_choose_tex_id;

	GLint m_cam_loc_transform_mat;

	// view or rotate or mvp matrix
private:
    glm::mat4 m_gl_proj;    /**< gl proj, view, model matrix */
    glm::mat4 m_gl_view;
    glm::mat4 m_gl_model;
    glm::mat4 m_mvp;        /**< glm mvp result, can be used for glUniformMatrix4fv() */
    int m_projWidth;      /**< opengl proj matrix width and height. aspect */
    int m_projHeight;
    int m_proj_wideangle_width;
    int m_proj_wideangle_height;
    int m_current_view;
    int m_is_use_bmp;  /**< if use bmp file as camera image */
    int m_car_type;

    float m_rotate_center_x;    /**< stitch coord is create by cloth center, is not same as car center */
    float m_rotate_center_y;
    float m_rotate_top_r;       /**< (3d view)view matrix top radius */
    float m_rotate_bottom_r;    /**< (3d view)view matrix bottom radius */
    float m_rotate_max_z_angle; /**< (3d view) max z rotate angle, used in aman_gpu_omni_directional_rotate() func */
    float m_rotate_top_height;     /**< (3d view)view matrix top z value */
    float m_rotate_bottom_height;  /**< (3d view)view matrix bottom z value */
    float m_rotate_auto_top_r;  /**< (auto view)view matrix top radius */
    float m_rotate_auto_bottom_r;
    float m_rotate_auto_top_height;
    float m_rotate_auto_bottom_height;
    float m_2d_front_det;       /**< (2d front view, y det)*/
    float m_2d_rear_det;
    float m_2d_height;          /**< (2d view, height)*/
    float m_curview_rot_ang;    /**< angle rotate */
    float m_2d_f_clip_rate;     /**< 2d camera view, clip image bottom part */
    float m_2d_r_clip_rate;
	float m_curview_rot_ang_z;  /**< current rotate z angle */
	float m_rotate_r;

	// dyn trans
private:
    int m_is_in_view_trans;     /**< if current in dyn view trans state */
    int m_dyn_trans_dst_view;   /**< dst dyn view state */
    float m_dyn_trans_dst_deg;  /**< dst view deg */
    int m_dyn_trans_frame_cnt;

	// texture
private:
    int m_texture_width;
    int m_texture_height;
    GLuint m_texture_id[4]; /**< texture id array, save gentex result */
    unsigned char *m_texture_vaddr[4];
    unsigned int m_texture_paddr[4];    /**< texture iamge's physic address */
    GLenum m_num_2_texture_enum[4];    /**< array trans num to enum textureID, ie, 'num'->GL_TEXTURE'num' */
    int m_camera_uniform2tex[4];    /**< uniform[%d] is which tex */
    int m_camera_uniform2tex_bmp[4];
    int m_camera_tex2uniform[4];    /**< tex[%d] to uniform[%d] */
    int m_camera_tex2uniform_bmp[4];

    unsigned int m_tex_uid_l;   /**< tex_uid, inc every opengl process */
    unsigned int m_tex_uid_h;
    get_chn_image_addr_cbfuc *m_p_get_imgfunc;  /**< get tex image cb func */
    rel_chn_image_addr_cbfuc *m_p_rel_imgfunc;  /**< release tex image cb func */
    QImage m_img1;
    QImage m_img2;
    QImage m_img3;
    QImage m_img4;

	// stitch file name and mem
private:
    char *m_p_db_file_name;     /**< aman table file name string */
    char *m_p_vshader_name;
    char *m_p_fshader_name;
    float *m_p_aman_mem;         /**< table mem. fill with aman_buffer_data_s */
    unsigned int *m_p_aman_idx_mem;  /**< table idx mem*/
    int m_aman_mem_st_num;       /**< mem have how many struct(aman_buffer_data_s) */

	// single or two camea file name
private:
	char *m_p_cam_vshader_name;
	char *m_p_cam_fshader_name;
	char *m_p_front_db_file_name;
	char *m_p_rear_db_file_name;
	char *m_p_two_db_file_name;
	char *m_p_fisheye_file_name;

	// vbo
private:
    GLuint m_aman_table_vbo;     /**< vbo */
    GLuint m_aman_idx_vbo;
    GLuint m_aman_single_front_camera_vbo;
    GLuint m_aman_single_rear_camera_vbo;
    GLuint m_aman_two_camera_vbo;
    GLuint m_aman_fisheye_front_vbo;
    GLuint m_aman_fisheye_rear_vbo;
    GLuint m_aman_fisheye_clip_front_vbo;
    GLuint m_aman_fisheye_clip_rear_vbo;

	// single or two camera mem
private:
    float *m_p_front_cam_mem;
    float *m_p_rear_cam_mem;
    float *m_p_two_cam_mem;
    int m_front_cam_point_num;
    int m_rear_cam_point_num;
    int m_two_cam_point_num;

	// light balance
private:
    int *m_p_light_area[8];     /**< trans area's coord(x, y) array. we have 8 area. each camea have two */
                                /**< eg. area0 is m_p_light_area[0], point to data is area0's 'point[0].xy, point[1].xy, ... */
    int m_light_area_num[8];    /**< how many point in this area. x,y is 2 point. so (x,y) num is /2 */  
    unsigned int m_light_prc_cnt;
    double m_light_avg_rgb[8][3];   /**< area's avg rgb. eg m_light_avg_rgb[0] is for area0. m_light_avg_rgb[0][0] is r, m_light_avg_rgb[0][1] is g, etc */
    double m_light_avg_y[8];
    float m_color_k[4];    /**< four image's rgb param. */
    float m_dst_color_k[4];  /**< dst color param */
    float m_color_limit_det;
    int m_is_do_lb_process; /**< is this instance do light balance calc */

    // hide center camera image, in case door open
    int m_is_hide_change;
    int m_is_hide_right;
    int m_is_hide_front;
    int m_is_hide_left;
    int m_is_hide_rear;

    // fisheye clip param
private:
    fisheye_clip_param_s m_fisheye_front;
    fisheye_clip_param_s m_fisheye_rear;
    float m_fisheye_front_orth[4]; /**< eg, glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f) param*/
    float m_fisheye_rear_orth[4]; 
    float m_fisheye_clip_front_orth[4]; /**< eg, glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f) param*/
    float m_fisheye_clip_rear_orth[4];

    /**< etc */
    int m_inst_id;

    /**< instance share resource */
    static QMutex m_s_mutex;
    static int m_s_inst_id;     /**< instance id, 0- */
    static int m_s_is_share_lb;
    static int m_s_is_share_lb_load_sts;    /**< 0 not load, 1 pre load success, -1 pre load faild */
    static int m_s_is_share_vbo;
    static int m_s_is_share_vbo_load_sts;
    static int m_s_is_share_tex;
    static int m_s_is_share_tex_load_sts;
    static amangpu_share_vbo_s m_s_hare_vbo;
    static amangpu_share_texure_s m_s_share_tex;
    static amangpu_share_ltbalance_s m_s_share_lb;
};


#endif

