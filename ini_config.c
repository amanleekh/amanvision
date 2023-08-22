
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common_def.h"
#include "ini_config.h"
#include "minIni.h"


int ini_check_file(int is_check_all_opt)
{
    if (-1 == access( "avm_qt_app_res/config.ini", F_OK))
    {
        AVM_ERR("ini_check_file. ini file is not exist\n");
        return 1;
    }
    if (0 == is_check_all_opt)
    {
        return 0;
    }

    return 0;
}


int ini_get_car_model_name(char *p_buffrt, int buffer_len)
{
    ini_gets("CAR_MODEL", "car_model_name", "dummy",
        p_buffrt, buffer_len, "avm_qt_app_res/config.ini");
    if (0 == memcmp(p_buffrt, "dummy", sizeof("dummy")))
    {
        return 1;
    }

    return 0;
}


float ini_get_car_model_scale()
{
    return ini_getf("CAR_MODEL", "car_model_scale", -1.0, "avm_qt_app_res/config.ini");
}


float ini_get_car_model_dst_w()
{
    return ini_getf("CAR_MODEL", "car_model_dst_w", -1.0, "avm_qt_app_res/config.ini");
}


float ini_get_car_model_dst_h()
{
    return ini_getf("CAR_MODEL", "car_model_dst_h", -1.0, "avm_qt_app_res/config.ini");
}


float ini_get_car_model_dst_z()
{
    return ini_getf("CAR_MODEL", "car_model_dst_z", -1.0, "avm_qt_app_res/config.ini");
}


float ini_get_car_model_det()
{
    return ini_getf("CAR_MODEL", "car_model_det", -1.0, "avm_qt_app_res/config.ini");
}


int ini_set_car_model_det(float det)
{
    return ini_putf("CAR_MODEL", "car_model_det", det, "avm_qt_app_res/config.ini");
}


float ini_get_car_model_window_alpha()
{
    return ini_getf("CAR_MODEL", "car_model_window_alpha", 0.5, "avm_qt_app_res/config.ini");
}

float ini_get_car_model_blink_r()
{
    return ini_getf("CAR_MODEL", "car_model_blink_r", 1.0, "avm_qt_app_res/config.ini");
}

float ini_get_car_model_blink_g()
{
    return ini_getf("CAR_MODEL", "car_model_blink_g", 0.49, "avm_qt_app_res/config.ini");
}

float ini_get_car_model_blink_b()
{
    return ini_getf("CAR_MODEL", "car_model_blink_b", 0.01, "avm_qt_app_res/config.ini");
}


float ini_get_material_specular_r()
{
    return ini_getf("CAR_MODEL", "car_model_material_specular_r", 0.2, "avm_qt_app_res/config.ini");
}

float ini_get_material_specular_g()
{
    return ini_getf("CAR_MODEL", "car_model_material_specular_g", 0.2, "avm_qt_app_res/config.ini");
}

float ini_get_material_specular_b()
{
    return ini_getf("CAR_MODEL", "car_model_material_specular_b", 0.2, "avm_qt_app_res/config.ini");
}


int ini_get_car_model_fl_wheel_name(char *p_buffrt, int buffer_len)
{
    ini_gets("CAR_MODEL", "car_model_wheel_fl", "dummy",
        p_buffrt, buffer_len, "avm_qt_app_res/config.ini");
    if (0 == memcmp(p_buffrt, "dummy", sizeof("dummy")))
    {
        return 1;
    }

    return 0;    
}


int ini_get_car_model_fr_wheel_name(char *p_buffrt, int buffer_len)
{
    ini_gets("CAR_MODEL", "car_model_wheel_fr", "dummy",
        p_buffrt, buffer_len, "avm_qt_app_res/config.ini");
    if (0 == memcmp(p_buffrt, "dummy", sizeof("dummy")))
    {
        return 1;
    }

    return 0;    
}

int ini_get_car_model_bl_wheel_name(char *p_buffrt, int buffer_len)
{
    ini_gets("CAR_MODEL", "car_model_wheel_bl", "dummy",
        p_buffrt, buffer_len, "avm_qt_app_res/config.ini");
    if (0 == memcmp(p_buffrt, "dummy", sizeof("dummy")))
    {
        return 1;
    }

    return 0;    
}


int ini_get_car_model_br_wheel_name(char *p_buffrt, int buffer_len)
{
    ini_gets("CAR_MODEL", "car_model_wheel_br", "dummy",
        p_buffrt, buffer_len, "avm_qt_app_res/config.ini");
    if (0 == memcmp(p_buffrt, "dummy", sizeof("dummy")))
    {
        return 1;
    }

    return 0;    
}


int ini_get_car_model_fl_door_name(char *p_buffrt, int buffer_len)
{
    ini_gets("CAR_MODEL", "car_model_door_fl", "dummy",
        p_buffrt, buffer_len, "avm_qt_app_res/config.ini");
    if (0 == memcmp(p_buffrt, "dummy", sizeof("dummy")))
    {
        return 1;
    }

    return 0;    
}


int ini_get_car_model_fr_door_name(char *p_buffrt, int buffer_len)
{
    ini_gets("CAR_MODEL", "car_model_door_fr", "dummy",
        p_buffrt, buffer_len, "avm_qt_app_res/config.ini");
    if (0 == memcmp(p_buffrt, "dummy", sizeof("dummy")))
    {
        return 1;
    }

    return 0;    
}

int ini_get_car_model_bl_door_name(char *p_buffrt, int buffer_len)
{
    ini_gets("CAR_MODEL", "car_model_door_bl", "dummy",
        p_buffrt, buffer_len, "avm_qt_app_res/config.ini");
    if (0 == memcmp(p_buffrt, "dummy", sizeof("dummy")))
    {
        return 1;
    }

    return 0;    
}


int ini_get_car_model_br_door_name(char *p_buffrt, int buffer_len)
{
    ini_gets("CAR_MODEL", "car_model_door_br", "dummy",
        p_buffrt, buffer_len, "avm_qt_app_res/config.ini");
    if (0 == memcmp(p_buffrt, "dummy", sizeof("dummy")))
    {
        return 1;
    }

    return 0;    
}

int ini_get_car_model_front_win_name(char *p_buffrt, int buffer_len)
{
    ini_gets("CAR_MODEL", "car_model_win_front", "dummy",
        p_buffrt, buffer_len, "avm_qt_app_res/config.ini");
    if (0 == memcmp(p_buffrt, "dummy", sizeof("dummy")))
    {
        return 1;
    }

    return 0;
}

int ini_get_car_model_fl_win_name(char *p_buffrt, int buffer_len)
{
    ini_gets("CAR_MODEL", "car_model_win_fl", "dummy",
        p_buffrt, buffer_len, "avm_qt_app_res/config.ini");
    if (0 == memcmp(p_buffrt, "dummy", sizeof("dummy")))
    {
        return 1;
    }

    return 0;
}

int ini_get_car_model_fr_win_name(char *p_buffrt, int buffer_len)
{
    ini_gets("CAR_MODEL", "car_model_win_fr", "dummy",
        p_buffrt, buffer_len, "avm_qt_app_res/config.ini");
    if (0 == memcmp(p_buffrt, "dummy", sizeof("dummy")))
    {
        return 1;
    }

    return 0;
}

int ini_get_car_model_bl_win_name(char *p_buffrt, int buffer_len)
{
    ini_gets("CAR_MODEL", "car_model_win_bl", "dummy",
        p_buffrt, buffer_len, "avm_qt_app_res/config.ini");
    if (0 == memcmp(p_buffrt, "dummy", sizeof("dummy")))
    {
        return 1;
    }

    return 0;
}

int ini_get_car_model_br_win_name(char *p_buffrt, int buffer_len)
{
    ini_gets("CAR_MODEL", "car_model_win_br", "dummy",
        p_buffrt, buffer_len, "avm_qt_app_res/config.ini");
    if (0 == memcmp(p_buffrt, "dummy", sizeof("dummy")))
    {
        return 1;
    }

    return 0;
}

int ini_get_car_model_rear_win_name(char *p_buffrt, int buffer_len)
{
    ini_gets("CAR_MODEL", "car_model_win_rear", "dummy",
        p_buffrt, buffer_len, "avm_qt_app_res/config.ini");
    if (0 == memcmp(p_buffrt, "dummy", sizeof("dummy")))
    {
        return 1;
    }

    return 0;
}


int ini_get_car_model_fl_light_name(char *p_buffrt, int buffer_len)
{
    ini_gets("CAR_MODEL", "car_model_light_fl", "dummy",
        p_buffrt, buffer_len, "avm_qt_app_res/config.ini");
    if (0 == memcmp(p_buffrt, "dummy", sizeof("dummy")))
    {
        return 1;
    }

    return 0;    
}


int ini_get_car_model_fr_light_name(char *p_buffrt, int buffer_len)
{
    ini_gets("CAR_MODEL", "car_model_light_fr", "dummy",
        p_buffrt, buffer_len, "avm_qt_app_res/config.ini");
    if (0 == memcmp(p_buffrt, "dummy", sizeof("dummy")))
    {
        return 1;
    }

    return 0;    
}

int ini_get_car_model_bl_light_name(char *p_buffrt, int buffer_len)
{
    ini_gets("CAR_MODEL", "car_model_light_bl", "dummy",
        p_buffrt, buffer_len, "avm_qt_app_res/config.ini");
    if (0 == memcmp(p_buffrt, "dummy", sizeof("dummy")))
    {
        return 1;
    }

    return 0;    
}


int ini_get_car_model_br_light_name(char *p_buffrt, int buffer_len)
{
    ini_gets("CAR_MODEL", "car_model_light_br", "dummy",
        p_buffrt, buffer_len, "avm_qt_app_res/config.ini");
    if (0 == memcmp(p_buffrt, "dummy", sizeof("dummy")))
    {
        return 1;
    }

    return 0;    
}


// d2_view
int ini_get_d2_vrate(float *p_front_vrate, float *p_rear_vrate)
{
    *p_front_vrate = ini_getf("D2_VIEW", "d2_view_f_vrate", -1.0, "avm_qt_app_res/config.ini");
    *p_rear_vrate = ini_getf("D2_VIEW", "d2_view_r_vrate", -1.0, "avm_qt_app_res/config.ini");
    if (*p_front_vrate <= 0.0f)
    {
        return 1;
    }
    if (*p_rear_vrate <= 0.0f)
    {
        return 1;
    }

    return 0;
}


int ini_get_d2_fc_front(int *p_front_vaild, int *p_lt_x, int *p_lt_y, int *p_rb_x, int *p_rb_y)
{
    *p_front_vaild = ini_getl("D2_VIEW", "d2_fc_front_vaild", -1, "avm_qt_app_res/config.ini");
    if (-1 == *p_front_vaild)
    {
        return 1;
    }
    *p_lt_x = ini_getl("D2_VIEW", "d2_fc_front_lt_x", -1, "avm_qt_app_res/config.ini");
    *p_lt_y = ini_getl("D2_VIEW", "d2_fc_front_lt_y", -1, "avm_qt_app_res/config.ini");
    *p_rb_x = ini_getl("D2_VIEW", "d2_fc_front_rb_x", -1, "avm_qt_app_res/config.ini");
    *p_rb_y = ini_getl("D2_VIEW", "d2_fc_front_rb_y", -1, "avm_qt_app_res/config.ini");
    if ((*p_lt_x < 0) || (*p_lt_y < 0) || (*p_rb_x < 0) || (*p_rb_y < 0))
    {
        return 1;
    }
    return 0;
}

int ini_set_d2_fc_front(int front_vaild, int lt_x, int lt_y, int rb_x, int rb_y)
{
    ini_putl("D2_VIEW", "d2_fc_front_vaild", front_vaild, "avm_qt_app_res/config.ini");
    ini_putl("D2_VIEW", "d2_fc_front_lt_x", lt_x, "avm_qt_app_res/config.ini");
    ini_putl("D2_VIEW", "d2_fc_front_lt_y", lt_y, "avm_qt_app_res/config.ini");
    ini_putl("D2_VIEW", "d2_fc_front_rb_x", rb_x, "avm_qt_app_res/config.ini");
    ini_putl("D2_VIEW", "d2_fc_front_rb_y", rb_y, "avm_qt_app_res/config.ini");

    return 0;
}

int ini_get_d2_fc_front_auto(int *p_lt_y, int *p_rb_y)
{
    *p_lt_y = ini_getl("D2_VIEW", "d2_fc_front_auto_lt_y", -1, "avm_qt_app_res/config.ini");
    *p_rb_y = ini_getl("D2_VIEW", "d2_fc_front_auto_rb_y", -1, "avm_qt_app_res/config.ini");
    if ((*p_lt_y < 0) || (*p_rb_y < 0))
    {
        return 1;
    }
    return 0;
}

int ini_set_d2_fc_front_auto(int lt_y, int rb_y)
{
    ini_putl("D2_VIEW", "d2_fc_front_auto_lt_y", lt_y, "avm_qt_app_res/config.ini");
    ini_putl("D2_VIEW", "d2_fc_front_auto_rb_y", rb_y, "avm_qt_app_res/config.ini");

    return 0;
}


int ini_get_d2_fc_rear(int *p_rear_vaild, int *p_lt_x, int *p_lt_y, int *p_rb_x, int *p_rb_y)
{
    *p_rear_vaild = ini_getl("D2_VIEW", "d2_fc_rear_vaild", -1, "avm_qt_app_res/config.ini");
    if (-1 == *p_rear_vaild)
    {
        return 1;
    }
    *p_lt_x = ini_getl("D2_VIEW", "d2_fc_rear_lt_x", -1, "avm_qt_app_res/config.ini");
    *p_lt_y = ini_getl("D2_VIEW", "d2_fc_rear_lt_y", -1, "avm_qt_app_res/config.ini");
    *p_rb_x = ini_getl("D2_VIEW", "d2_fc_rear_rb_x", -1, "avm_qt_app_res/config.ini");
    *p_rb_y = ini_getl("D2_VIEW", "d2_fc_rear_rb_y", -1, "avm_qt_app_res/config.ini");
    if ((*p_lt_x < 0) || (*p_lt_y < 0) || (*p_rb_x < 0) || (*p_rb_y < 0))
    {
        return 1;
    }
    return 0;
}

int ini_set_d2_fc_rear(int rear_vaild, int lt_x, int lt_y, int rb_x, int rb_y)
{
    ini_putl("D2_VIEW", "d2_fc_rear_vaild", rear_vaild, "avm_qt_app_res/config.ini");
    ini_putl("D2_VIEW", "d2_fc_rear_lt_x", lt_x, "avm_qt_app_res/config.ini");
    ini_putl("D2_VIEW", "d2_fc_rear_lt_y", lt_y, "avm_qt_app_res/config.ini");
    ini_putl("D2_VIEW", "d2_fc_rear_rb_x", rb_x, "avm_qt_app_res/config.ini");
    ini_putl("D2_VIEW", "d2_fc_rear_rb_y", rb_y, "avm_qt_app_res/config.ini");

    return 0;
}

int ini_get_d2_fc_rear_auto(int *p_lt_y, int *p_rb_y)
{
    *p_lt_y = ini_getl("D2_VIEW", "d2_fc_rear_auto_lt_y", -1, "avm_qt_app_res/config.ini");
    *p_rb_y = ini_getl("D2_VIEW", "d2_fc_rear_auto_rb_y", -1, "avm_qt_app_res/config.ini");
    if ((*p_lt_y < 0) || (*p_rb_y < 0))
    {
        return 1;
    }
    return 0;
}

int ini_set_d2_fc_rear_auto(int lt_y,int rb_y)
{
    ini_putl("D2_VIEW", "d2_fc_rear_auto_lt_y", lt_y, "avm_qt_app_res/config.ini");
    ini_putl("D2_VIEW", "d2_fc_rear_auto_rb_y", rb_y, "avm_qt_app_res/config.ini");

    return 0;
}

// front and rear fish
int ini_get_d2_f_front(int *p_front_vaild, int *p_lt_x, int *p_lt_y, int *p_rb_x, int *p_rb_y)
{
    *p_front_vaild = ini_getl("D2_VIEW", "d2_f_front_vaild", -1, "avm_qt_app_res/config.ini");
    if (-1 == *p_front_vaild)
    {
        return 1;
    }
    *p_lt_x = ini_getl("D2_VIEW", "d2_f_front_lt_x", -1, "avm_qt_app_res/config.ini");
    *p_lt_y = ini_getl("D2_VIEW", "d2_f_front_lt_y", -1, "avm_qt_app_res/config.ini");
    *p_rb_x = ini_getl("D2_VIEW", "d2_f_front_rb_x", -1, "avm_qt_app_res/config.ini");
    *p_rb_y = ini_getl("D2_VIEW", "d2_f_front_rb_y", -1, "avm_qt_app_res/config.ini");
    if ((*p_lt_x < 0) || (*p_lt_y < 0) || (*p_rb_x < 0) || (*p_rb_y < 0))
    {
        return 1;
    }
    return 0;
}

int ini_set_d2_f_front(int front_vaild, int lt_x, int lt_y, int rb_x, int rb_y)
{
    ini_putl("D2_VIEW", "d2_f_front_vaild", front_vaild, "avm_qt_app_res/config.ini");
    ini_putl("D2_VIEW", "d2_f_front_lt_x", lt_x, "avm_qt_app_res/config.ini");
    ini_putl("D2_VIEW", "d2_f_front_lt_y", lt_y, "avm_qt_app_res/config.ini");
    ini_putl("D2_VIEW", "d2_f_front_rb_x", rb_x, "avm_qt_app_res/config.ini");
    ini_putl("D2_VIEW", "d2_f_front_rb_y", rb_y, "avm_qt_app_res/config.ini");

    return 0;
}

int ini_get_d2_f_front_auto(int *p_lt_y, int *p_rb_y)
{
    *p_lt_y = ini_getl("D2_VIEW", "d2_f_front_auto_lt_y", -1, "avm_qt_app_res/config.ini");
    *p_rb_y = ini_getl("D2_VIEW", "d2_f_front_auto_rb_y", -1, "avm_qt_app_res/config.ini");
    if ((*p_lt_y < 0) || (*p_rb_y < 0))
    {
        return 1;
    }
    return 0;
}

int ini_set_d2_f_front_auto(int lt_y, int rb_y)
{
    ini_putl("D2_VIEW", "d2_f_front_auto_lt_y", lt_y, "avm_qt_app_res/config.ini");
    ini_putl("D2_VIEW", "d2_f_front_auto_rb_y", rb_y, "avm_qt_app_res/config.ini");

    return 0;
}

int ini_get_d2_f_rear(int *p_rear_vaild, int *p_lt_x, int *p_lt_y, int *p_rb_x, int *p_rb_y)
{
    *p_rear_vaild = ini_getl("D2_VIEW", "d2_f_rear_vaild", -1, "avm_qt_app_res/config.ini");
    if (-1 == *p_rear_vaild)
    {
        return 1;
    }
    *p_lt_x = ini_getl("D2_VIEW", "d2_f_rear_lt_x", -1, "avm_qt_app_res/config.ini");
    *p_lt_y = ini_getl("D2_VIEW", "d2_f_rear_lt_y", -1, "avm_qt_app_res/config.ini");
    *p_rb_x = ini_getl("D2_VIEW", "d2_f_rear_rb_x", -1, "avm_qt_app_res/config.ini");
    *p_rb_y = ini_getl("D2_VIEW", "d2_f_rear_rb_y", -1, "avm_qt_app_res/config.ini");
    if ((*p_lt_x < 0) || (*p_lt_y < 0) || (*p_rb_x < 0) || (*p_rb_y < 0))
    {
        return 1;
    }
    return 0;
}

int ini_set_d2_f_rear(int rear_vaild, int lt_x, int lt_y, int rb_x, int rb_y)
{
    ini_putl("D2_VIEW", "d2_f_rear_vaild", rear_vaild, "avm_qt_app_res/config.ini");
    ini_putl("D2_VIEW", "d2_f_rear_lt_x", lt_x, "avm_qt_app_res/config.ini");
    ini_putl("D2_VIEW", "d2_f_rear_lt_y", lt_y, "avm_qt_app_res/config.ini");
    ini_putl("D2_VIEW", "d2_f_rear_rb_x", rb_x, "avm_qt_app_res/config.ini");
    ini_putl("D2_VIEW", "d2_f_rear_rb_y", rb_y, "avm_qt_app_res/config.ini");

    return 0;
}

int ini_get_d2_f_rear_auto(int *p_lt_y, int *p_rb_y)
{
    *p_lt_y = ini_getl("D2_VIEW", "d2_f_rear_auto_lt_y", -1, "avm_qt_app_res/config.ini");
    *p_rb_y = ini_getl("D2_VIEW", "d2_f_rear_auto_rb_y", -1, "avm_qt_app_res/config.ini");
    if ((*p_lt_y < 0) || (*p_rb_y < 0))
    {
        return 1;
    }
    return 0;
}

int ini_set_d2_f_rear_auto(int lt_y, int rb_y)
{
    ini_putl("D2_VIEW", "d2_f_rear_auto_lt_y", lt_y, "avm_qt_app_res/config.ini");
    ini_putl("D2_VIEW", "d2_f_rear_auto_rb_y", rb_y, "avm_qt_app_res/config.ini");

    return 0;
}


// bottom
int ini_get_bottom_name(char *p_buffrt, int buffer_len)
{
    ini_gets("BOTTOM", "bottom_name", "dummy",
        p_buffrt, buffer_len, "avm_qt_app_res/config.ini");
    if (0 == memcmp(p_buffrt, "dummy", sizeof("dummy")))
    {
        return 1;
    }

    return 0;
}


float ini_get_bottom_w()
{
    return ini_getf("BOTTOM", "bottom_w", 0.0, "avm_qt_app_res/config.ini");
}


float ini_get_bottom_h()
{
    return ini_getf("BOTTOM", "bottom_h", 0.0, "avm_qt_app_res/config.ini");
}


float ini_get_bottom_det()
{
    return ini_getf("BOTTOM", "bottom_det", 0.0, "avm_qt_app_res/config.ini");
}

int ini_set_bottom_det(float det)
{
    return ini_putf("BOTTOM", "bottom_det", det, "avm_qt_app_res/config.ini");
}


// radar
int ini_get_radar_name(char *p_buffrt, int buffer_len)
{
    ini_gets("RADAR", "radar_name", "dummy",
        p_buffrt, buffer_len, "avm_qt_app_res/config.ini");
    if (0 == memcmp(p_buffrt, "dummy", sizeof("dummy")))
    {
        return 1;
    }

    return 0;
}


float ini_get_radar_w()
{
    return ini_getf("RADAR", "radar_w", 0.0, "avm_qt_app_res/config.ini");
}


float ini_get_radar_h()
{
    return ini_getf("RADAR", "radar_h", 0.0, "avm_qt_app_res/config.ini");
}


float ini_get_radar_det()
{
    return ini_getf("RADAR", "radar_det", 0.0, "avm_qt_app_res/config.ini");
}


//misc
float ini_get_rotate_center_x()
{
    return ini_getf("MISC_CONF", "rotate_center_x", 999.0f, "avm_qt_app_res/config.ini");
}

float ini_get_rotate_center_y()
{
    return ini_getf("MISC_CONF", "rotate_center_y", 999.0f, "avm_qt_app_res/config.ini");
}

int ini_get_device_use_bmp_file()
{
    int is_use_bmp = ini_getl("MISC_CONF", "device_use_bmp_file", -1, "avm_qt_app_res/config.ini");
    if (-1 == is_use_bmp)
    {
        return 0;
    }
    return is_use_bmp;
}

void ini_set_device_use_bmp_file(int is_use_bmp)
{
    ini_putl("MISC_CONF", "device_use_bmp_file", is_use_bmp, "avm_qt_app_res/config.ini");
}

// car info
float ini_get_car_l_length()
{
    return ini_getf("CAR_INFO", "car_l_length", -1.0f, "avm_qt_app_res/config.ini");
}


float ini_get_car_tt_length()
{
    return ini_getf("CAR_INFO", "car_tt_length", -1.0f, "avm_qt_app_res/config.ini");
}

float ini_get_car_tb_length()
{
    return ini_getf("CAR_INFO", "car_tb_length", -1.0f, "avm_qt_app_res/config.ini");
}


float ini_get_car_w_length()
{
    return ini_getf("CAR_INFO", "car_w_length", -1.0f, "avm_qt_app_res/config.ini");
}


int ini_get_car_type()
{
    return ini_getl("CAR_INFO", "car_type", -1, "avm_qt_app_res/config.ini");
}

int ini_get_car_subtype()
{
    return ini_getl("CAR_INFO", "car_subtype", -1, "avm_qt_app_res/config.ini");
}

void ini_set_car_subtype(int car_subtype)
{
    ini_putl("CAR_INFO", "car_subtype", car_subtype, "avm_qt_app_res/config.ini");
}

int ini_get_assist_line_name(char *p_buffrt, int buffer_len)
{
    ini_gets("ASSIST_LINE", "assist_line_name", "dummy",
        p_buffrt, buffer_len, "avm_qt_app_res/config.ini");
    if (0 == memcmp(p_buffrt, "dummy", sizeof("dummy")))
    {
        return 1;
    }

    return 0;
}


int ini_get_block_line_front_name(char *p_buffrt, int buffer_len)
{
    ini_gets("ASSIST_LINE", "assist_block_line_front_name", "dummy",
        p_buffrt, buffer_len, "avm_qt_app_res/config.ini");
    if (0 == memcmp(p_buffrt, "dummy", sizeof("dummy")))
    {
        return 1;
    }

    return 0;
}


int ini_get_block_line_rear_name(char *p_buffrt, int buffer_len)
{
    ini_gets("ASSIST_LINE", "assist_block_line_rear_name", "dummy",
        p_buffrt, buffer_len, "avm_qt_app_res/config.ini");
    if (0 == memcmp(p_buffrt, "dummy", sizeof("dummy")))
    {
        return 1;
    }

    return 0;
}


int ini_get_assist_line_rear_wheel_name(char *p_buffrt, int buffer_len)
{
    ini_gets("ASSIST_LINE", "assist_line_rear_wheel_name", "dummy",
        p_buffrt, buffer_len, "avm_qt_app_res/config.ini");
    if (0 == memcmp(p_buffrt, "dummy", sizeof("dummy")))
    {
        return 1;
    }

    return 0;
}


int ini_get_assist_line_front_cam_name(char *p_buffrt, int buffer_len)
{
    ini_gets("ASSIST_LINE", "assist_line_front_cam_name", "dummy",
        p_buffrt, buffer_len, "avm_qt_app_res/config.ini");
    if (0 == memcmp(p_buffrt, "dummy", sizeof("dummy")))
    {
        return 1;
    }

    return 0;
}


int ini_get_assist_line_rear_cam_name(char *p_buffrt, int buffer_len)
{
    ini_gets("ASSIST_LINE", "assist_line_rear_cam_name", "dummy",
        p_buffrt, buffer_len, "avm_qt_app_res/config.ini");
    if (0 == memcmp(p_buffrt, "dummy", sizeof("dummy")))
    {
        return 1;
    }

    return 0;
}


float ini_get_assist_line_det()
{
    return ini_getf("ASSIST_LINE", "assist_line_det", -999.0f, "avm_qt_app_res/config.ini");
}


float ini_get_assist_line_2d_front_det()
{
    return ini_getf("ASSIST_LINE", "assist_line_2d_front_det", -999.0f, "avm_qt_app_res/config.ini");
}


int ini_set_assist_line_2d_front_det(float det)
{
    return ini_putf("ASSIST_LINE", "assist_line_2d_front_det", det, "avm_qt_app_res/config.ini");
}


float ini_get_assist_line_2d_rear_det()
{
    return ini_getf("ASSIST_LINE", "assist_line_2d_rear_det", -999.0f, "avm_qt_app_res/config.ini");
}


int ini_set_assist_line_2d_rear_det(float det)
{
    return ini_putf("ASSIST_LINE", "assist_line_2d_rear_det", det, "avm_qt_app_res/config.ini");
}


int ini_get_cam_right()
{
    return ini_getl("CAMERA_SEQ", "cam_right", -1, "avm_qt_app_res/config.ini");
}


int ini_get_cam_front()
{
    return ini_getl("CAMERA_SEQ", "cam_front", -1, "avm_qt_app_res/config.ini");
}


int ini_get_cam_left()
{
    return ini_getl("CAMERA_SEQ", "cam_left", -1, "avm_qt_app_res/config.ini");
}

int ini_get_cam_rear()
{
    return ini_getl("CAMERA_SEQ", "cam_rear", -1, "avm_qt_app_res/config.ini");
}

int ini_get_log_to_console()
{
    return ini_getl("LOG", "log_to_console", -1, "avm_qt_app_res/config.ini");
}

int ini_get_log_to_shm()
{
    return ini_getl("LOG", "log_to_shm", -1, "avm_qt_app_res/config.ini");
}


int ini_get_idx0()
{
    return ini_getl("CAMERA_VAILD", "cam_idx0", -1, "avm_qt_app_res/config.ini");
}


int ini_get_idx1()
{
    return ini_getl("CAMERA_VAILD", "cam_idx1", -1, "avm_qt_app_res/config.ini");
}

int ini_get_idx2()
{
    return ini_getl("CAMERA_VAILD", "cam_idx2", -1, "avm_qt_app_res/config.ini");
}

int ini_get_idx3()
{
    return ini_getl("CAMERA_VAILD", "cam_idx3", -1, "avm_qt_app_res/config.ini");
}


int ini_get_geom_file_param(
    int idx,
    int *p_row_begin, int *p_row_end, 
    int *p_col_begin_l, int *p_col_end_l, 
    int *p_col_begin_m, int *p_col_end_m, 
    int *p_col_begin_r, int *p_col_end_r)
{
    if (0 == idx)
    {
        *p_row_begin = ini_getl("CALIB_PARAM", "calib_right_row_begin", -1, "avm_qt_app_res/config.ini");
        *p_row_end = ini_getl("CALIB_PARAM", "calib_right_row_end", -1, "avm_qt_app_res/config.ini");
        *p_col_begin_l = ini_getl("CALIB_PARAM", "calib_right_col_begin_l", -1, "avm_qt_app_res/config.ini");
        *p_col_end_l = ini_getl("CALIB_PARAM", "calib_right_col_end_l", -1, "avm_qt_app_res/config.ini");   
        *p_col_begin_m = ini_getl("CALIB_PARAM", "calib_right_col_begin_m", -1, "avm_qt_app_res/config.ini");
        *p_col_end_m = ini_getl("CALIB_PARAM", "calib_right_col_end_m", -1, "avm_qt_app_res/config.ini");           
        *p_col_begin_r = ini_getl("CALIB_PARAM", "calib_right_col_begin_r", -1, "avm_qt_app_res/config.ini");
        *p_col_end_r = ini_getl("CALIB_PARAM", "calib_right_col_end_r", -1, "avm_qt_app_res/config.ini"); 
        if ((*p_row_begin < 0) || (*p_row_end < 0)
            || (*p_col_begin_l < 0) || (*p_col_end_l < 0)
            || (*p_col_begin_r < 0) || (*p_col_end_r < 0))
        {
            return 1;
        }
    }
    else if (1 == idx)
    {
        *p_row_begin = ini_getl("CALIB_PARAM", "calib_front_row_begin", -1, "avm_qt_app_res/config.ini");
        *p_row_end = ini_getl("CALIB_PARAM", "calib_front_row_end", -1, "avm_qt_app_res/config.ini");
        *p_col_begin_l = ini_getl("CALIB_PARAM", "calib_front_col_begin_l", -1, "avm_qt_app_res/config.ini");
        *p_col_end_l = ini_getl("CALIB_PARAM", "calib_front_col_end_l", -1, "avm_qt_app_res/config.ini");     
        *p_col_begin_m = ini_getl("CALIB_PARAM", "calib_front_col_begin_m", -1, "avm_qt_app_res/config.ini");
        *p_col_end_m = ini_getl("CALIB_PARAM", "calib_front_col_end_m", -1, "avm_qt_app_res/config.ini");     
        *p_col_begin_r = ini_getl("CALIB_PARAM", "calib_front_col_begin_r", -1, "avm_qt_app_res/config.ini");
        *p_col_end_r = ini_getl("CALIB_PARAM", "calib_front_col_end_r", -1, "avm_qt_app_res/config.ini");
        if ((*p_row_begin < 0) || (*p_row_end < 0)
            || (*p_col_begin_l < 0) || (*p_col_end_l < 0)
            || (*p_col_begin_r < 0) || (*p_col_end_r < 0))
        {
            return 1;
        }
    }
    else if (2 == idx)
    {
        *p_row_begin = ini_getl("CALIB_PARAM", "calib_left_row_begin", -1, "avm_qt_app_res/config.ini");
        *p_row_end = ini_getl("CALIB_PARAM", "calib_left_row_end", -1, "avm_qt_app_res/config.ini");
        *p_col_begin_l = ini_getl("CALIB_PARAM", "calib_left_col_begin_l", -1, "avm_qt_app_res/config.ini");
        *p_col_end_l = ini_getl("CALIB_PARAM", "calib_left_col_end_l", -1, "avm_qt_app_res/config.ini");     
        *p_col_begin_m = ini_getl("CALIB_PARAM", "calib_left_col_begin_m", -1, "avm_qt_app_res/config.ini");
        *p_col_end_m = ini_getl("CALIB_PARAM", "calib_left_col_end_m", -1, "avm_qt_app_res/config.ini");     
        *p_col_begin_r = ini_getl("CALIB_PARAM", "calib_left_col_begin_r", -1, "avm_qt_app_res/config.ini");
        *p_col_end_r = ini_getl("CALIB_PARAM", "calib_left_col_end_r", -1, "avm_qt_app_res/config.ini");    
        if ((*p_row_begin < 0) || (*p_row_end < 0)
            || (*p_col_begin_l < 0) || (*p_col_end_l < 0)
            || (*p_col_begin_r < 0) || (*p_col_end_r < 0))
        {
            return 1;
        }
    }
    else if (3 == idx)
    {
        *p_row_begin = ini_getl("CALIB_PARAM", "calib_rear_row_begin", -1, "avm_qt_app_res/config.ini");
        *p_row_end = ini_getl("CALIB_PARAM", "calib_rear_row_end", -1, "avm_qt_app_res/config.ini");
        *p_col_begin_l = ini_getl("CALIB_PARAM", "calib_rear_col_begin_l", -1, "avm_qt_app_res/config.ini");
        *p_col_end_l = ini_getl("CALIB_PARAM", "calib_rear_col_end_l", -1, "avm_qt_app_res/config.ini");     
        *p_col_begin_m = ini_getl("CALIB_PARAM", "calib_rear_col_begin_m", -1, "avm_qt_app_res/config.ini");
        *p_col_end_m = ini_getl("CALIB_PARAM", "calib_rear_col_end_m", -1, "avm_qt_app_res/config.ini");     
        *p_col_begin_r = ini_getl("CALIB_PARAM", "calib_rear_col_begin_r", -1, "avm_qt_app_res/config.ini");
        *p_col_end_r = ini_getl("CALIB_PARAM", "calib_rear_col_end_r", -1, "avm_qt_app_res/config.ini");
        if ((*p_row_begin < 0) || (*p_row_end < 0)
            || (*p_col_begin_l < 0) || (*p_col_end_l < 0)
            || (*p_col_begin_r < 0) || (*p_col_end_r < 0))
        {
            return 1;
        }
    }

    return 0;
}


int ini_set_geom_file_param(
    int idx,
    int row_begin, int row_end, 
    int col_begin_l, int col_end_l, 
    int col_begin_m, int col_end_m, 
    int col_begin_r, int col_end_r)
{
    if (0 == idx)
    {
        ini_putl("CALIB_PARAM", "calib_right_row_begin", row_begin, "avm_qt_app_res/config.ini");
        ini_putl("CALIB_PARAM", "calib_right_row_end", row_end, "avm_qt_app_res/config.ini");
        ini_putl("CALIB_PARAM", "calib_right_col_begin_l", col_begin_l, "avm_qt_app_res/config.ini");
        ini_putl("CALIB_PARAM", "calib_right_col_end_l", col_end_l, "avm_qt_app_res/config.ini");     
        ini_putl("CALIB_PARAM", "calib_right_col_begin_m", col_begin_m, "avm_qt_app_res/config.ini");
        ini_putl("CALIB_PARAM", "calib_right_col_end_m", col_end_m, "avm_qt_app_res/config.ini");     
        ini_putl("CALIB_PARAM", "calib_right_col_begin_r", col_begin_r, "avm_qt_app_res/config.ini");
        ini_putl("CALIB_PARAM", "calib_right_col_end_r", col_end_r, "avm_qt_app_res/config.ini"); 
    }
    else if (1 == idx)
    {
        ini_putl("CALIB_PARAM", "calib_front_row_begin", row_begin, "avm_qt_app_res/config.ini");
        ini_putl("CALIB_PARAM", "calib_front_row_end", row_end, "avm_qt_app_res/config.ini");
        ini_putl("CALIB_PARAM", "calib_front_col_begin_l", col_begin_l, "avm_qt_app_res/config.ini");
        ini_putl("CALIB_PARAM", "calib_front_col_end_l", col_end_l, "avm_qt_app_res/config.ini");     
        ini_putl("CALIB_PARAM", "calib_front_col_begin_m", col_begin_m, "avm_qt_app_res/config.ini");
        ini_putl("CALIB_PARAM", "calib_front_col_end_m", col_end_m, "avm_qt_app_res/config.ini");     
        ini_putl("CALIB_PARAM", "calib_front_col_begin_r", col_begin_r, "avm_qt_app_res/config.ini");
        ini_putl("CALIB_PARAM", "calib_front_col_end_r", col_end_r, "avm_qt_app_res/config.ini");    
    }
    else if (2 == idx)
    {
        ini_putl("CALIB_PARAM", "calib_left_row_begin", row_begin, "avm_qt_app_res/config.ini");
        ini_putl("CALIB_PARAM", "calib_left_row_end", row_end, "avm_qt_app_res/config.ini");
        ini_putl("CALIB_PARAM", "calib_left_col_begin_l", col_begin_l, "avm_qt_app_res/config.ini");
        ini_putl("CALIB_PARAM", "calib_left_col_end_l", col_end_l, "avm_qt_app_res/config.ini");     
        ini_putl("CALIB_PARAM", "calib_left_col_begin_m", col_begin_m, "avm_qt_app_res/config.ini");
        ini_putl("CALIB_PARAM", "calib_left_col_end_m", col_end_m, "avm_qt_app_res/config.ini");     
        ini_putl("CALIB_PARAM", "calib_left_col_begin_r", col_begin_r, "avm_qt_app_res/config.ini");
        ini_putl("CALIB_PARAM", "calib_left_col_end_r", col_end_r, "avm_qt_app_res/config.ini");    
    }
    else if (3 == idx)
    {
        ini_putl("CALIB_PARAM", "calib_rear_row_begin", row_begin, "avm_qt_app_res/config.ini");
        ini_putl("CALIB_PARAM", "calib_rear_row_end", row_end, "avm_qt_app_res/config.ini");
        ini_putl("CALIB_PARAM", "calib_rear_col_begin_l", col_begin_l, "avm_qt_app_res/config.ini");
        ini_putl("CALIB_PARAM", "calib_rear_col_end_l", col_end_l, "avm_qt_app_res/config.ini");     
        ini_putl("CALIB_PARAM", "calib_rear_col_begin_m", col_begin_m, "avm_qt_app_res/config.ini");
        ini_putl("CALIB_PARAM", "calib_rear_col_end_m", col_end_m, "avm_qt_app_res/config.ini");     
        ini_putl("CALIB_PARAM", "calib_rear_col_begin_r", col_begin_r, "avm_qt_app_res/config.ini");
        ini_putl("CALIB_PARAM", "calib_rear_col_end_r", col_end_r, "avm_qt_app_res/config.ini");
    }

    return 0;
}

int ini_get_calib_skip_fas(void)
{
    return ini_getl("CALIB_PARAM", "calib_skip_fas", 0, "avm_qt_app_res/config.ini");
}

void ini_set_calib_skip_fas(int skip_fas)
{
    ini_putl("CALIB_PARAM", "calib_skip_fas", skip_fas, "avm_qt_app_res/config.ini");
}

int ini_get_calib_skip_cor_pripoint(void)
{
    return ini_getl("CALIB_PARAM", "calib_skip_correct_pripoint", 0, "avm_qt_app_res/config.ini");
}

void ini_set_calib_skip_cor_pripoint(int skip_cor_pripoint)
{
    ini_putl("CALIB_PARAM", "calib_skip_correct_pripoint", skip_cor_pripoint, "avm_qt_app_res/config.ini");
}


int ini_get_shift_v_for_subtype(int index)
{
    char *v_str[] = {
        "front_shift_v_for_subtype"
    };

    if (index != 0)
    {
        return 0;
    }
    return ini_getl("CALIB_PARAM", v_str[index], 0, "avm_qt_app_res/config.ini");
}


int ini_get_calib_mamual_shift_v(int index)
{
    char *v_str[] = {
        "calib_right_manual_shift_v",
        "calib_front_manual_shift_v",
        "calib_left_manual_shift_v",
        "calib_rear_manual_shift_v"
    };

    if ((index < 0) || (index > 3))
    {
        return 0;
    }
    return ini_getl("CALIB_PARAM", v_str[index], 0, "avm_qt_app_res/config.ini");
}

int ini_get_calib_mamual_shift_h(int index)
{
    char *h_str[] = {
        "calib_right_manual_shift_h",
        "calib_front_manual_shift_h",
        "calib_left_manual_shift_h",
        "calib_rear_manual_shift_h"
    };

    if ((index < 0) || (index > 3))
    {
        return 0;
    }
	return ini_getl("CALIB_PARAM", h_str[index], 0, "avm_qt_app_res/config.ini");
}

void ini_set_shift_v_for_subtype(int index, int shift_v)
{
    char *v_str[] = {
        "front_shift_v_for_subtype"
    };

    if ((index < 0) || (index > 3))
    {
        return;
    }
    ini_putl("CALIB_PARAM", v_str[index], shift_v, "avm_qt_app_res/config.ini");
}


void ini_set_calib_mamual_shift_v(int index, int shift_v)
{
    char *v_str[] = {
        "calib_right_manual_shift_v",
        "calib_front_manual_shift_v",
        "calib_left_manual_shift_v",
        "calib_rear_manual_shift_v"
    };

    if ((index < 0) || (index > 3))
    {
        return;
    }
	ini_putl("CALIB_PARAM", v_str[index], shift_v, "avm_qt_app_res/config.ini");
}


void ini_set_calib_mamual_shift_h(int index, int shift_h)
{
    char *h_str[] = {
        "calib_right_manual_shift_h",
        "calib_front_manual_shift_h",
        "calib_left_manual_shift_h",
        "calib_rear_manual_shift_h"
    };

    if ((index < 0) || (index > 3))
    {
        return;
    }
	ini_putl("CALIB_PARAM", h_str[index], shift_h, "avm_qt_app_res/config.ini");
}


int ini_get_calib_use_img(void)
{
	return ini_getl("CALIB_PARAM", "calib_use_image", -1, "avm_qt_app_res/config.ini");
}

void ini_set_calib_use_img(int is_calib_use_image)
{
	ini_putl("CALIB_PARAM", "calib_use_image", is_calib_use_image, "avm_qt_app_res/config.ini");
}


int ini_get_calib_use_qt_cam_image(void)
{
	return ini_getl("CALIB_PARAM", "calib_use_qt_cam_img", -1, "avm_qt_app_res/config.ini");
}

void ini_set_calib_use_qt_cam_image(int is_calib_use_qt_image)
{
	ini_putl("CALIB_PARAM", "calib_use_qt_cam_img", is_calib_use_qt_image, "avm_qt_app_res/config.ini");
}

int ini_get_calib_myicv_vaild(void)
{
    return ini_getl("CALIB_PARAM", "calib_myicv_vaild", 0, "avm_qt_app_res/config.ini");
}

void ini_set_calib_myicv_vaild(int calib_myicv_vaild)
{
    ini_putl("CALIB_PARAM", "calib_myicv_vaild", calib_myicv_vaild, "avm_qt_app_res/config.ini");
}

int ini_get_calib_myicv_d(void)
{
    return ini_getl("CALIB_PARAM", "calib_myicv_d", 30, "avm_qt_app_res/config.ini");
}

void ini_set_calib_myicv_d(int calib_myicv_d)
{
    ini_putl("CALIB_PARAM", "calib_myicv_d", calib_myicv_d, "avm_qt_app_res/config.ini");
}

int ini_get_calib_myicv_max_approx(void)
{
    return ini_getl("CALIB_PARAM", "calib_myicv_max_approx", 20, "avm_qt_app_res/config.ini");
}

void ini_set_calib_myicv_max_approx(int calib_myicv_max_approx)
{
    ini_putl("CALIB_PARAM", "calib_myicv_max_approx", calib_myicv_max_approx, "avm_qt_app_res/config.ini");
}

int ini_get_myicv_geom_file_param(
    int idx,
    int *p_row_begin, int *p_row_end, 
    int *p_col_begin_l, int *p_col_end_l, 
    int *p_col_begin_r, int *p_col_end_r)
{
    if (0 == idx)
    {
        *p_row_begin = ini_getl("CALIB_PARAM", "calib_myicv_right_row_begin", -1, "avm_qt_app_res/config.ini");
        *p_row_end = ini_getl("CALIB_PARAM", "calib_myicv_right_row_end", -1, "avm_qt_app_res/config.ini");
        *p_col_begin_l = ini_getl("CALIB_PARAM", "calib_myicv_right_col_begin_l", -1, "avm_qt_app_res/config.ini");
        *p_col_end_l = ini_getl("CALIB_PARAM", "calib_myicv_right_col_end_l", -1, "avm_qt_app_res/config.ini");
        *p_col_begin_r = ini_getl("CALIB_PARAM", "calib_myicv_right_col_begin_r", -1, "avm_qt_app_res/config.ini");
        *p_col_end_r = ini_getl("CALIB_PARAM", "calib_myicv_right_col_end_r", -1, "avm_qt_app_res/config.ini");
        if ((*p_row_begin < 0) || (*p_row_end < 0)
            || (*p_col_begin_l < 0) || (*p_col_end_l < 0)
            || (*p_col_begin_r < 0) || (*p_col_end_r < 0))
        {
            return 1;
        }
    }
    else if (1 == idx)
    {
        *p_row_begin = ini_getl("CALIB_PARAM", "calib_myicv_front_row_begin", -1, "avm_qt_app_res/config.ini");
        *p_row_end = ini_getl("CALIB_PARAM", "calib_myicv_front_row_end", -1, "avm_qt_app_res/config.ini");
        *p_col_begin_l = ini_getl("CALIB_PARAM", "calib_myicv_front_col_begin_l", -1, "avm_qt_app_res/config.ini");
        *p_col_end_l = ini_getl("CALIB_PARAM", "calib_myicv_front_col_end_l", -1, "avm_qt_app_res/config.ini");
        *p_col_begin_r = ini_getl("CALIB_PARAM", "calib_myicv_front_col_begin_r", -1, "avm_qt_app_res/config.ini");
        *p_col_end_r = ini_getl("CALIB_PARAM", "calib_myicv_front_col_end_r", -1, "avm_qt_app_res/config.ini");
        if ((*p_row_begin < 0) || (*p_row_end < 0)
            || (*p_col_begin_l < 0) || (*p_col_end_l < 0)
            || (*p_col_begin_r < 0) || (*p_col_end_r < 0))
        {
            return 1;
        }
    }
    else if (2 == idx)
    {
        *p_row_begin = ini_getl("CALIB_PARAM", "calib_myicv_left_row_begin", -1, "avm_qt_app_res/config.ini");
        *p_row_end = ini_getl("CALIB_PARAM", "calib_myicv_left_row_end", -1, "avm_qt_app_res/config.ini");
        *p_col_begin_l = ini_getl("CALIB_PARAM", "calib_myicv_left_col_begin_l", -1, "avm_qt_app_res/config.ini");
        *p_col_end_l = ini_getl("CALIB_PARAM", "calib_myicv_left_col_end_l", -1, "avm_qt_app_res/config.ini");
        *p_col_begin_r = ini_getl("CALIB_PARAM", "calib_myicv_left_col_begin_r", -1, "avm_qt_app_res/config.ini");
        *p_col_end_r = ini_getl("CALIB_PARAM", "calib_myicv_left_col_end_r", -1, "avm_qt_app_res/config.ini");
        if ((*p_row_begin < 0) || (*p_row_end < 0)
            || (*p_col_begin_l < 0) || (*p_col_end_l < 0)
            || (*p_col_begin_r < 0) || (*p_col_end_r < 0))
        {
            return 1;
        }
    }
    else if (3 == idx)
    {
        *p_row_begin = ini_getl("CALIB_PARAM", "calib_myicv_rear_row_begin", -1, "avm_qt_app_res/config.ini");
        *p_row_end = ini_getl("CALIB_PARAM", "calib_myicv_rear_row_end", -1, "avm_qt_app_res/config.ini");
        *p_col_begin_l = ini_getl("CALIB_PARAM", "calib_myicv_rear_col_begin_l", -1, "avm_qt_app_res/config.ini");
        *p_col_end_l = ini_getl("CALIB_PARAM", "calib_myicv_rear_col_end_l", -1, "avm_qt_app_res/config.ini");
        *p_col_begin_r = ini_getl("CALIB_PARAM", "calib_myicv_rear_col_begin_r", -1, "avm_qt_app_res/config.ini");
        *p_col_end_r = ini_getl("CALIB_PARAM", "calib_myicv_rear_col_end_r", -1, "avm_qt_app_res/config.ini");
        if ((*p_row_begin < 0) || (*p_row_end < 0)
            || (*p_col_begin_l < 0) || (*p_col_end_l < 0)
            || (*p_col_begin_r < 0) || (*p_col_end_r < 0))
        {
            return 1;
        }
    }

    return 0;
}


int ini_set_myicv_geom_file_param(
    int idx,
    int row_begin, int row_end, 
    int col_begin_l, int col_end_l, 
    int col_begin_r, int col_end_r)
{
    if (0 == idx)
    {
        ini_putl("CALIB_PARAM", "calib_myicv_right_row_begin", row_begin, "avm_qt_app_res/config.ini");
        ini_putl("CALIB_PARAM", "calib_myicv_right_row_end", row_end, "avm_qt_app_res/config.ini");
        ini_putl("CALIB_PARAM", "calib_myicv_right_col_begin_l", col_begin_l, "avm_qt_app_res/config.ini");
        ini_putl("CALIB_PARAM", "calib_myicv_right_col_end_l", col_end_l, "avm_qt_app_res/config.ini");
        ini_putl("CALIB_PARAM", "calib_myicv_right_col_begin_r", col_begin_r, "avm_qt_app_res/config.ini");
        ini_putl("CALIB_PARAM", "calib_myicv_right_col_end_r", col_end_r, "avm_qt_app_res/config.ini");
    }
    else if (1 == idx)
    {
        ini_putl("CALIB_PARAM", "calib_myicv_front_row_begin", row_begin, "avm_qt_app_res/config.ini");
        ini_putl("CALIB_PARAM", "calib_myicv_front_row_end", row_end, "avm_qt_app_res/config.ini");
        ini_putl("CALIB_PARAM", "calib_myicv_front_col_begin_l", col_begin_l, "avm_qt_app_res/config.ini");
        ini_putl("CALIB_PARAM", "calib_myicv_front_col_end_l", col_end_l, "avm_qt_app_res/config.ini");
        ini_putl("CALIB_PARAM", "calib_myicv_front_col_begin_r", col_begin_r, "avm_qt_app_res/config.ini");
        ini_putl("CALIB_PARAM", "calib_myicv_front_col_end_r", col_end_r, "avm_qt_app_res/config.ini");
    }
    else if (2 == idx)
    {
        ini_putl("CALIB_PARAM", "calib_myicv_left_row_begin", row_begin, "avm_qt_app_res/config.ini");
        ini_putl("CALIB_PARAM", "calib_myicv_left_row_end", row_end, "avm_qt_app_res/config.ini");
        ini_putl("CALIB_PARAM", "calib_myicv_left_col_begin_l", col_begin_l, "avm_qt_app_res/config.ini");
        ini_putl("CALIB_PARAM", "calib_myicv_left_col_end_l", col_end_l, "avm_qt_app_res/config.ini");
        ini_putl("CALIB_PARAM", "calib_myicv_left_col_begin_r", col_begin_r, "avm_qt_app_res/config.ini");
        ini_putl("CALIB_PARAM", "calib_myicv_left_col_end_r", col_end_r, "avm_qt_app_res/config.ini");
    }
    else if (3 == idx)
    {
        ini_putl("CALIB_PARAM", "calib_myicv_rear_row_begin", row_begin, "avm_qt_app_res/config.ini");
        ini_putl("CALIB_PARAM", "calib_myicv_rear_row_end", row_end, "avm_qt_app_res/config.ini");
        ini_putl("CALIB_PARAM", "calib_myicv_rear_col_begin_l", col_begin_l, "avm_qt_app_res/config.ini");
        ini_putl("CALIB_PARAM", "calib_myicv_rear_col_end_l", col_end_l, "avm_qt_app_res/config.ini");
        ini_putl("CALIB_PARAM", "calib_myicv_rear_col_begin_r", col_begin_r, "avm_qt_app_res/config.ini");
        ini_putl("CALIB_PARAM", "calib_myicv_rear_col_end_r", col_end_r, "avm_qt_app_res/config.ini");
    }

    return 0;
}



