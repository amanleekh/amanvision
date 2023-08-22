#ifndef CUSTOM_STR_H
#define CUSTOM_STR_H

#include <QString>

int custstr_set_country(int is_chinese);

QString custstr_get_view_str(int view_enum);

QString custstr_get_auto();

QString custstr_get_manual();

QString custstr_get_assist_str();

QString custstr_get_radar_str();

QString custstr_get_radio_str();

QString custstr_get_speed_alert_str();

QString custstr_get_cam_err_alert_str();

QString custstr_get_version_alert_str();

QString custstr_get_init_alert_str(int err_num);

QString custstr_get_alert_ok_str();

QString custstr_get_3dpie_str(int idx);

QString custstr_get_2dpie_str(int idx);

QString custstr_get_careful_str();

QString custstr_get_remd_manual_3d_str();

QString custstr_get_remd_manual_2d_str();

QString custstr_get_remd_mute_str();

QString custstr_get_errinfo_title();

// value
// 0 ok
// 1 disconnect
// 2 bad
// 3 unknow
QString custstr_get_err_dyn_info(int front, int rear, int left, int right);


#endif // CUSTOM_STR_H
