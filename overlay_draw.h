#ifndef OVERLAY_DRAW_H
#define OVERLAY_DRAW_H

#include "GLES2/gl2.h"
#include "modelloader.h"

#include <glm/mat4x4.hpp>


// must same as avm_qt_app's define
// -------
#define ASSIST_LINE_DEG_CNT     (91)    // [-45, 45] deg
#define ASSIST_LINE_MIN_DEG     (-45)
#define ASSIST_LINE_MAX_DEG     (45)
#define ASSIST_LINE_LENGTH      (4)


/**
* struct for bootom data
*/
typedef struct _bottom_data_param_struct_
{
    float w;    /**< box w*/
    float h;
    float z;
    float y_det;
    float r;
    int r_slice;    /**< ridius slice */
    int depth;      /**< circle count, max is 4 */
    float rgb[3];
    float alpth_depth[4];

    float *p_vert;
    float *p_color;
    int point_num;
}bottom_data_param_s;


/**
* have two data file, for front wheel and rear wheel
*/
typedef struct _assist_line_db_header_struct_
{
    int file_size;
    int pair_num;
    float line_width;

    // line ahead for wheel(front wheel or rear wheel)
    unsigned int left_line_arr_det[ASSIST_LINE_DEG_CNT];
    int left_line_num[ASSIST_LINE_DEG_CNT];     // pair count

    unsigned int right_line_arr_det[ASSIST_LINE_DEG_CNT];
    int right_line_num[ASSIST_LINE_DEG_CNT];    // pair count

    // line back for wheel(front wheel or rear wheel)
    unsigned int left_back_line_arr_det[ASSIST_LINE_DEG_CNT];
    int left_back_line_num[ASSIST_LINE_DEG_CNT];    // pair count

    unsigned int right_back_line_arr_det[ASSIST_LINE_DEG_CNT];
    int right_back_line_num[ASSIST_LINE_DEG_CNT];   // pair count  

    // color
    unsigned int color_arr_det;
    int color_num;

    // meter 
    float meter_width;  /**< meter block width, height is equal line_width */
    float meter2line_det;   /**< gap between line and meter block */
    int meter_num;  /**< how many meter block */
    int each_meter_block_pt_num;
    unsigned int left_meter_arr_det[ASSIST_LINE_DEG_CNT];
    int left_meter_num[ASSIST_LINE_DEG_CNT];    // point vert count

    unsigned int right_meter_arr_det[ASSIST_LINE_DEG_CNT];
    int right_meter_num[ASSIST_LINE_DEG_CNT];   // point vert count
}assist_line_db_header_s;


typedef struct _block_line_struct_
{
    int file_size;
    int pair_num;
    float color[4];
    
    int point_arr_det;
    int act_pair_num;
}block_line_s;

// -------


typedef struct _idv_obj_struct_
{
    char name[256];     /**< single object's name */
    float *p_vert_arr;  /**< array to hold vert point */
    int vert_num;   /**< how many vert point in p_vert_arr */
}idv_obj_s;


typedef struct _obj_info_struct_
{
    const char *p_obj_file_name;    /**< obj file name */

    int total_point_num; /**< total have how many point(vector) */
    int point_dim;  /**< a vert point have how many dim */
    float min_x;
    float max_x;
    float min_y;
    float max_y;
    float min_z;
    float max_z;
    int idv_obj_num;    /**< how many object in this obj file */
    idv_obj_s *p_idv_obj_arr;   /**< struct to hold all object in this obj file */
}obj_info_s;


class OverlayDraw
{
public:
    OverlayDraw();

    int create_program();

    int init_bottom_point_num(bottom_data_param_s *p_param);
    int gen_bottom_data(bottom_data_param_s *p_param);
    int pre_load_bottom_data();
    int gen_bottom_vbo();
    int bottom_render_process();
    int pre_load_radar_data();
    int gen_radar_vbo();
    void set_radar_mvp(glm::mat4 *p_m, glm::mat4 *p_v, glm::mat4 *p_p);
    void set_radar_rotate(float angle);
    int radar_render_process();
    // two 32bit mask for 16 radar
    // each radar use 4bit
    // eg, mask = 0x01, mask2=0x00, means radar0's level is 1
    // eg, mask = 0x12, mask2=0x000, means radar0's level is 2, radar1's level is 1
    void radar_set_alert(unsigned int mask, unsigned int mask2, int *p_chg);
    // this func is deprected
    void radar_clr_alert(unsigned int mask, unsigned int mask2, int *p_chg);
    void radar_show(int is_show);

    int pre_gen_assist_line_data();
    int gen_assist_line_vbo(); 
    void set_assist_line_mvp(glm::mat4 *p_m, glm::mat4 *p_v, glm::mat4 *p_p);
    int assist_line_process(int space);  // 0 for 3d stitch space, 1 for front, 2 for rear camera
    void set_assist_line_rotate(float angle);
    void set_assist_line_angle(float ang);
    void assist_line_show(int is_show);
    void assist_line_set_direction(int is_front);
    void set_assist_line_force_update();
    void overlay_render_start();
    void overlay_render_end();

    int pre_load_block_line_data();
    int gen_block_line_vbo();
    int block_line_process(int space); // 0 for 3d stitch space, 1 for front, 2 for rear camera
    int block_line_show(int is_show);

private:
    unsigned int get_time_ms();
    int load_obj_file(const char *p_obj_file_name, int point_dim, obj_info_s *p_info);  
    int trans_obj(obj_info_s *p_info, float scale_val, float dst_w, float dst_h, float det, float z_val);
    int assist_line_3d_process();
    int assist_line_3d_process_front_left();
    int assist_line_3d_process_front_right();
    int assist_line_3d_process_rear_left();
    int assist_line_3d_process_rear_right();
    int assist_line_2d_process_front();
    int assist_line_2d_process_rear();
    int assist_line_2d_process(int is_front);
    int pre_gen_assist_line_3d_data();
    int pre_gen_assist_line_rear_wheel_3d_data();
    int pre_gen_assist_line_front_cam_data();
    int pre_gen_assist_line_rear_cam_data();
    int pre_load_block_line_front_data();
    int pre_load_block_line_rear_data();

private:
    void *m_shader_handler;         /**< shader handler */
    GLint m_loc_vertex_position;    /**< attr and uniform location */
    GLint m_loc_color;
    GLint m_loc_mvp;

private:
    GLuint m_bottom_vert_buffer;
    GLuint m_bottom_color_buffer;
    GLuint m_radar_vert_buffer;
    GLuint m_radar_color_buffer;
    GLuint m_left_assist_line_vert_buffer;  /**< front wheel's left assist line */
    GLuint m_left_meter_buffer;
    GLuint m_right_assist_line_vert_buffer; /**< front wheel's right assist line */ 
    GLuint m_right_meter_buffer;
    GLuint m_rear_wheel_left_assist_line_vert_buffer;   /**< rear wheel's left assist line */
    GLuint m_rear_wheel_right_assist_line_vert_buffer;  /**< rear wheel's right assist line */
    GLuint m_assist_line_color_buffer;
    //GLuint m_assist_line_color_buffer;
    GLuint m_block_line_front_vert_buffer;
    GLuint m_block_line_front_color_buffer;
    GLuint m_block_line_rear_vert_buffer;
    GLuint m_block_line_rear_color_buffer;

private:
    glm::mat4 m_radar_m;
    glm::mat4 m_radar_v;
    glm::mat4 m_radar_p;
    glm::mat4 m_radar_mvp;
    glm::mat4 m_assist_line_m;
    glm::mat4 m_assist_line_v;
    glm::mat4 m_assist_line_p;
    glm::mat4 m_assist_line_mvp;

private:
    const char *m_p_vshader_str;
    const char *m_p_fshader_str;

    //obj_info_s m_p_bottom_loader;
    float *m_p_bottom_vert_arr;
    float *m_p_bottom_color_arr;
    int m_bottom_point_num;
    int m_bottom_load;

    obj_info_s m_p_radar_loader;
    float *m_p_radar_color_arr;
    unsigned int m_radar_load;

    unsigned int m_radar_state[2];
    int m_radar_change;     /**< radar change flag in 3d space */
    int m_radar_2d_front_change;    /**< radar change flag in 2d fonrt */
    int m_radar_2d_rear_change;     /**< radar change flag in 2d rear */
    float m_radar_alpha;
    int m_is_radar_show;

    //obj_info_s m_p_assist_line_loader;
    //float *m_p_assist_line_color_arr;
    //unsigned int m_assist_line_load;

private:
    int m_assist_line_pair_cnt;         /**< point pair cnt */

    // stitch 3d coord space
    // - front wheel's ahead line
    float *m_left_assist_line_vert_arr[ASSIST_LINE_DEG_CNT];
    int m_left_assist_line_num[ASSIST_LINE_DEG_CNT];
    float *m_right_assist_line_vert_arr[ASSIST_LINE_DEG_CNT];
    int m_right_assist_line_num[ASSIST_LINE_DEG_CNT];
    // - front wheel's meter block
    float *m_left_meter_vert_arr[ASSIST_LINE_DEG_CNT];
    int m_left_meter_num[ASSIST_LINE_DEG_CNT];
    float *m_right_meter_vert_arr[ASSIST_LINE_DEG_CNT];
    int m_right_meter_num[ASSIST_LINE_DEG_CNT];    
    // - front wheel's back line
    float *m_left_back_assist_line_vert_arr[ASSIST_LINE_DEG_CNT];
    int m_left_back_assist_line_num[ASSIST_LINE_DEG_CNT];
    float *m_right_back_assist_line_vert_arr[ASSIST_LINE_DEG_CNT];
    int m_right_back_assist_line_num[ASSIST_LINE_DEG_CNT];
    // - rear wheel's ahead line
    float *m_rear_wheel_left_assist_line_vert_arr[ASSIST_LINE_DEG_CNT];
    int m_rear_wheel_left_assist_line_num[ASSIST_LINE_DEG_CNT];
    float *m_rear_wheel_right_assist_line_vert_arr[ASSIST_LINE_DEG_CNT];
    int m_rear_wheel_right_assist_line_num[ASSIST_LINE_DEG_CNT];
    // - rear wheel's back line
    float *m_rear_wheel_left_back_assist_line_vert_arr[ASSIST_LINE_DEG_CNT];
    int m_rear_wheel_left_back_assist_line_num[ASSIST_LINE_DEG_CNT];
    float *m_rear_wheel_right_back_assist_line_vert_arr[ASSIST_LINE_DEG_CNT];
    int m_rear_wheel_right_back_assist_line_num[ASSIST_LINE_DEG_CNT];
    // -rear wheel's back line's meter block
    float *m_rear_wheel_left_back_meter_vert_arr[ASSIST_LINE_DEG_CNT];
    int m_rear_wheel_left_back_meter_num[ASSIST_LINE_DEG_CNT];
    float *m_rear_wheel_right_back_meter_vert_arr[ASSIST_LINE_DEG_CNT];
    int m_rear_wheel_right_back_meter_num[ASSIST_LINE_DEG_CNT];    

    // signal camera calib coord space
    // - front camera
    float *m_front_cam_left_assist_line_vert_arr[ASSIST_LINE_DEG_CNT];
    int m_front_cam_left_assist_line_num[ASSIST_LINE_DEG_CNT];
    float *m_front_cam_right_assist_line_vert_arr[ASSIST_LINE_DEG_CNT];
    int m_front_cam_right_assist_line_num[ASSIST_LINE_DEG_CNT];
    // - front camera meter block
    float *m_front_cam_left_meter_vert_arr[ASSIST_LINE_DEG_CNT];
    int m_front_cam_left_meter_num[ASSIST_LINE_DEG_CNT];
    float *m_front_cam_right_meter_vert_arr[ASSIST_LINE_DEG_CNT];
    int m_front_cam_right_meter_num[ASSIST_LINE_DEG_CNT];    
    // - rear camera
    float *m_rear_cam_left_assist_line_vert_arr[ASSIST_LINE_DEG_CNT];
    int m_rear_cam_left_assist_line_num[ASSIST_LINE_DEG_CNT];
    float *m_rear_cam_right_assist_line_vert_arr[ASSIST_LINE_DEG_CNT];
    int m_rear_cam_right_assist_line_num[ASSIST_LINE_DEG_CNT];    
    // - rear camera meter block
    float *m_rear_cam_left_meter_vert_arr[ASSIST_LINE_DEG_CNT];
    int m_rear_cam_left_meter_num[ASSIST_LINE_DEG_CNT];
    float *m_rear_cam_right_meter_vert_arr[ASSIST_LINE_DEG_CNT];
    int m_rear_cam_right_meter_num[ASSIST_LINE_DEG_CNT];  

    // color array
    float *m_p_assist_line_color_arr;

    // block line
    int m_block_line_pair_cnt;
    float *m_block_line_front_vert_arr;
    int m_block_line_front_num;
    float *m_block_line_rear_vert_arr;
    int m_block_line_rear_num;
    float *m_block_line_front_color_arr;
    float *m_block_line_rear_color_arr;
    
    unsigned int m_assist_line_load;
    int m_current_deg;
    int m_assist_dir;   /**< 0 is front, 1 back*/
    int m_assist_change;
    int m_is_assist_show;

private:
    int m_block_line_load;
    int m_is_block_line_show;
};


#endif // OVERLAY_DRAW_H
