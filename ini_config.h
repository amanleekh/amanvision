

#ifndef _INI_CONFIG_H_
#define _INI_CONFIG_H_

#ifdef __cplusplus
extern "C" {
#endif


int ini_check_file(int is_check_all_opt);

// car model
int ini_get_car_model_name(char *p_buffrt, int buffer_len);
float ini_get_car_model_scale();
float ini_get_car_model_dst_w();
float ini_get_car_model_dst_h();
float ini_get_car_model_dst_z();
float ini_get_car_model_det();
int ini_set_car_model_det(float det);
int ini_get_car_model_fl_wheel_name(char *p_buffrt, int buffer_len);
int ini_get_car_model_fr_wheel_name(char *p_buffrt, int buffer_len);
int ini_get_car_model_bl_wheel_name(char *p_buffrt, int buffer_len);
int ini_get_car_model_br_wheel_name(char *p_buffrt, int buffer_len);
int ini_get_car_model_fl_door_name(char *p_buffrt, int buffer_len);
int ini_get_car_model_fr_door_name(char *p_buffrt, int buffer_len);
int ini_get_car_model_bl_door_name(char *p_buffrt, int buffer_len);
int ini_get_car_model_br_door_name(char *p_buffrt, int buffer_len);

int ini_get_car_model_front_win_name(char *p_buffrt, int buffer_len);
int ini_get_car_model_fl_win_name(char *p_buffrt, int buffer_len);
int ini_get_car_model_fr_win_name(char *p_buffrt, int buffer_len);
int ini_get_car_model_bl_win_name(char *p_buffrt, int buffer_len);
int ini_get_car_model_br_win_name(char *p_buffrt, int buffer_len);
int ini_get_car_model_rear_win_name(char *p_buffrt, int buffer_len);

int ini_get_car_model_fl_light_name(char *p_buffrt, int buffer_len);
int ini_get_car_model_fr_light_name(char *p_buffrt, int buffer_len);
int ini_get_car_model_bl_light_name(char *p_buffrt, int buffer_len);
int ini_get_car_model_br_light_name(char *p_buffrt, int buffer_len);

float ini_get_car_model_window_alpha();
float ini_get_car_model_blink_r();
float ini_get_car_model_blink_g();
float ini_get_car_model_blink_b();
float ini_get_material_specular_r();
float ini_get_material_specular_g();
float ini_get_material_specular_b();



// d2_view. fish_clip
int ini_get_d2_vrate(float *p_front_vrate, float *p_rear_vrate);
int ini_get_d2_fc_front(int *p_front_vaild, int *p_lt_x, int *p_lt_y, int *p_rb_x, int *p_rb_y);
int ini_set_d2_fc_front(int front_vaild, int lt_x, int lt_y, int rb_x, int rb_y);
int ini_get_d2_fc_front_auto(int *p_lt_y, int *p_rb_y);
int ini_set_d2_fc_front_auto(int lt_y, int rb_y);
int ini_get_d2_fc_rear(int *p_rear_vaild, int *p_lt_x, int *p_lt_y, int *p_rb_x, int *p_rb_y);
int ini_set_d2_fc_rear(int rear_vaild, int lt_x, int lt_y, int rb_x, int rb_y);
int ini_get_d2_fc_rear_auto(int *p_lt_y, int *p_rb_y);
int ini_set_d2_fc_rear_auto(int lt_y, int rb_y);
// d2_view. fish
int ini_get_d2_f_front(int *p_front_vaild, int *p_lt_x, int *p_lt_y, int *p_rb_x, int *p_rb_y);
int ini_set_d2_f_front(int front_vaild, int lt_x, int lt_y, int rb_x, int rb_y);
int ini_get_d2_f_front_auto(int *p_lt_y, int *p_rb_y);
int ini_set_d2_f_front_auto(int lt_y, int rb_y);
int ini_get_d2_f_rear(int *p_rear_vaild, int *p_lt_x, int *p_lt_y, int *p_rb_x, int *p_rb_y);
int ini_set_d2_f_rear(int rear_vaild, int lt_x, int lt_y, int rb_x, int rb_y);
int ini_get_d2_f_rear_auto(int *p_lt_y, int *p_rb_y);
int ini_set_d2_f_rear_auto(int lt_y, int rb_y);


// bottom
int ini_get_bottom_name(char *p_buffrt, int buffer_len);
float ini_get_bottom_w();
float ini_get_bottom_h();
float ini_get_bottom_det();
int ini_set_bottom_det(float det);

// radar
int ini_get_radar_name(char *p_buffrt, int buffer_len);
float ini_get_radar_w();
float ini_get_radar_h();
float ini_get_radar_det();

// assist_line
int ini_get_assist_line_name(char *p_buffrt, int buffer_len);
int ini_get_assist_line_rear_wheel_name(char *p_buffrt, int buffer_len);
int ini_get_assist_line_front_cam_name(char *p_buffrt, int buffer_len);
int ini_get_assist_line_rear_cam_name(char *p_buffrt, int buffer_len);
int ini_get_block_line_front_name(char *p_buffrt, int buffer_len);
int ini_get_block_line_rear_name(char *p_buffrt, int buffer_len);
float ini_get_assist_line_det();
float ini_get_assist_line_2d_front_det();
int ini_set_assist_line_2d_front_det(float det);
float ini_get_assist_line_2d_rear_det();
int ini_set_assist_line_2d_rear_det(float det);


//misc
float ini_get_rotate_center_x();
float ini_get_rotate_center_y();
int ini_get_device_use_bmp_file();
void ini_set_device_use_bmp_file(int is_use_bmp);


// car info
float ini_get_car_l_length();
float ini_get_car_tt_length();
float ini_get_car_tb_length();
float ini_get_car_w_length();
int ini_get_car_type();
int ini_get_car_subtype();
void ini_set_car_subtype(int car_subtype);


// camera seq
int ini_get_cam_right();
int ini_get_cam_front();
int ini_get_cam_left();
int ini_get_cam_rear();

// log
int ini_get_log_to_console();
int ini_get_log_to_shm();

//camera vaild
int ini_get_idx0();
int ini_get_idx1();
int ini_get_idx2();
int ini_get_idx3();

// geom param
int ini_get_geom_file_param(
    int idx,
    int *p_row_begin, int *p_row_end, 
    int *p_col_begin_l, int *p_col_end_l, 
    int *p_col_begin_m, int *p_col_end_m,
    int *p_col_begin_r, int *p_col_end_r);
int ini_set_geom_file_param(
    int idx,
    int row_begin, int row_end, 
    int col_begin_l, int col_end_l,
    int col_begin_m, int col_end_m,
    int col_begin_r, int col_end_r);

//calib param
int ini_get_calib_skip_fas(void);
void ini_set_calib_skip_fas(int skip_fas);
int ini_get_calib_mamual_shift_v(int index);
int ini_get_calib_mamual_shift_h(int index);
void ini_set_calib_mamual_shift_v(int index, int shift_v);
void ini_set_calib_mamual_shift_h(int index, int shift_h);
int ini_get_shift_v_for_subtype(int index);
void ini_set_shift_v_for_subtype(int index, int shift_v);

int ini_get_calib_skip_cor_pripoint(void);
void ini_set_calib_skip_cor_pripoint(int skip_cor_pripoint);


int ini_get_calib_use_img(void);
void ini_set_calib_use_img(int is_calib_use_image);

int ini_get_calib_use_qt_cam_image(void);
void ini_set_calib_use_qt_cam_image(int is_calib_use_qt_image);

// myicv geom param
int ini_get_myicv_geom_file_param(
    int idx,
    int *p_row_begin, int *p_row_end,
    int *p_col_begin_l, int *p_col_end_l,
    int *p_col_begin_r, int *p_col_end_r);

int ini_set_myicv_geom_file_param(
    int idx,
    int row_begin, int row_end,
    int col_begin_l, int col_end_l,
    int col_begin_r, int col_end_r);


// myicv calib param
int ini_get_calib_myicv_vaild(void);
void ini_set_calib_myicv_vaild(int calib_myicv_vaild);

int ini_get_calib_myicv_d(void);
void ini_set_calib_myicv_d(int calib_myicv_d);

int ini_get_calib_myicv_max_approx(void);
void ini_set_calib_myicv_max_approx(int calib_myicv_max_approx);


#ifdef __cplusplus
}
#endif

#endif

