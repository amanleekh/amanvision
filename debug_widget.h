#ifndef DEBUG_WIDGET_H
#define DEBUG_WIDGET_H

#include <QWidget>
#include <QDialog>
#include <QPushButton>
#include <QScrollArea>
#include <QLabel>
#include <QVBoxLayout>
#include <QTableWidget>
#include <QStringList>
#include <QFile>

#include "common_def.h"
#include "opencv2/core/core.hpp"


#define IMG_WGT_MAX_IMG     (7)


/**
* param file struct, copy from avm_calib
*/
typedef struct _param_file_struct_
{
    int major;
    int minor;
    int file_size;
    int cam_idx;    //<* 0 front, 1 rear, 2 left, 3 right, -1 reserved
    int calib_ok;   //<* is calib success
    int reserved[3];
}param_file_s;


//copy from avm_calib
typedef struct
{
    double camera[4];
    double distort[4];
    int calib_row_begin;
    int calib_row_end;
    int calib_col_begin_l;
    int calib_col_end_l;
    int calib_col_begin_m;
    int calib_col_end_m;
    int calib_col_begin_r;
    int calib_col_end_r;
} CalibRoi;


class ImageWidget : public QWidget
{
public:
    ImageWidget(QWidget *parent);
    void AddImage(QString name, int is_org);
    void RemoveAllImage();

private:
    QVBoxLayout *m_layout;
    int m_image_idx;
    QLabel *m_image[IMG_WGT_MAX_IMG];
};


class CalibWidget : public QWidget
{
    Q_OBJECT
    
public:
    CalibWidget(QWidget *parent);
    void layout();

private:
    QPushButton *m_calib_save_cam_img;
    QPushButton *m_calib_cam_nosave;
    QPushButton *m_calib_cam_save;
    QPushButton *m_copy_img_to_calib_res;
    QPushButton *m_copy_car_cx_img_to_calib_res;
    QPushButton *m_copy_car_cc_img_to_calib_res;
    QPushButton *m_copy_car_cs_img_to_calib_res;
    QPushButton *m_copy_cfg_cx;
    QPushButton *m_copy_cfg_cc;
    QPushButton *m_copy_cfg_cs;
    QPushButton *m_calib_img_nosave;
    QPushButton *m_calib_img_save;
    QPushButton *m_calib_param_save;
    QPushButton *m_copy_calib_data;
    QPushButton *m_rm_lst_alg_tmp_img;
    QPushButton *m_calc_calib_res_hash;
    QPushButton *m_calc_data_hash;
    QLabel *m_label;
    QStringList calib_src;
    QStringList calib_dst;

private:
    void calib(int is_use_img, int is_debug);
    void copy_car_img(int type);
    void copy_cfg(int type);
    int copy_calib_file_to_data_dir();

private slots:
    void cam_save_cam_img();
    void cam_nosave_btn_click();
    void cam_save_btn_click();
    void copy_img_to_calib_res_btn_click();
    void copy_car_cx_img_to_calib_res_btn_click();
    void copy_car_cc_img_to_calib_res_btn_click();
    void copy_car_cs_img_to_calib_res_btn_click();
    void copy_cfg_cx_btn_click();
    void copy_cfg_cc_btn_click();
    void copy_cfg_cs_btn_click();
    void img_nosave_btn_click();
    void img_save_btn_click();
    void param_save_btn_click();
    void copy_calib_data_btn_click();
    void rm_lst_alg_tmp_img_btn_click();
    void calc_calib_res_hash_btn_click();
    void calc_data_hash_btn_click();
};


class IniFileWidget : public QWidget
{
    Q_OBJECT
    
public:
    IniFileWidget(QWidget *parent);
    void layout();

private:
    void insertMiscData();
    void insertFcFrontData();
    void insertFcRearData();
    void insertFrontAuto();
    void insertRearAuto();
    void insertCalibRight();
    void insertCalibFront();
    void insertCalibLeft();
    void insertCalibRear();
    void insertCalibMamualShift();
    void saveMiscData();
    void saveFcFrontData();
    void saveFcRearData();
    void saveFrontAuto();
    void saveRearAuto();
    void saveCalibRight();
    void saveCalibFront();
    void saveCalibLeft();
    void saveCalibRear();
    void saveCalibMamualShift();

private:
    int m_row_num;
    int m_misc_row_start;
    int m_fc_front_row_start;
    int m_fc_rear_row_start;
    int m_front_auto_start;
    int m_rear_auto_start;
    int m_calib_right_start;
    int m_calib_front_start;
    int m_calib_left_start;
    int m_calib_rear_start;
    int m_calib_mamual_shift_start;
    QTableWidget *tb_widget;
    QPushButton *m_add1;
    QPushButton *m_dec1;
    QPushButton *m_add10;
    QPushButton *m_dec10;
    QPushButton *m_reset;
    QPushButton *m_save; 

private:
    void numCalc(int det);

private slots:
    void add1_btn_click();
    void dec1_btn_click();
    void add10_btn_click();
    void dec10_btn_click();
    void reset_btn_click();
    void save_btn_click();    
};


class CalibParamWidget : public QWidget
{
    Q_OBJECT

public:
    CalibParamWidget(QWidget *parent);
    void layout(void);

private:
    void init_table(void);
    void update_table(void);
    void save_table(void);
    void read_mat_binary(QFile& inputFile, cv::Mat& mat);
    void write_mat_binary(QFile& inputFile, cv::Mat& mat);
    void add_str_to_cur_item(QString str);

private slots:
    void load_from_fas_param_btn_click(void);
    void save_to_fas_param_btn_click(void);

private slots:
    void btn_pt_click(void);
    void btn_sign_click(void);
    void btn_del_click(void);
    void btn_0_click(void);
    void btn_1_click(void);
    void btn_2_click(void);
    void btn_3_click(void);
    void btn_4_click(void);
    void btn_5_click(void);
    void btn_6_click(void);
    void btn_7_click(void);
    void btn_8_click(void);
    void btn_9_click(void);

private:
    QString m_param_file_path_fas;
    param_file_s m_file_head_fas;
    CalibRoi m_calib_roi_fas[4];
    cv::Mat m_rvecs_fas[4];
    cv::Mat m_mvecs_fas[4];

private:
    QPushButton *m_load_from_fas_param;
    QPushButton *m_save_to_fas_param;
    QTableWidget* m_tb_widget;
    QLabel* m_label;

private:
    QPushButton* m_btn_0;
    QPushButton* m_btn_1;
    QPushButton* m_btn_2;
    QPushButton* m_btn_3;
    QPushButton* m_btn_4;
    QPushButton* m_btn_5;
    QPushButton* m_btn_6;
    QPushButton* m_btn_7;
    QPushButton* m_btn_8;
    QPushButton* m_btn_9;
    QPushButton* m_btn_pt;
    QPushButton* m_btn_sign;
    QPushButton* m_btn_del;
};


class OthersWidget : public QWidget
{
    Q_OBJECT

public:
    OthersWidget(QWidget *parent);
    ~OthersWidget();
    void layout();

private:
    QPushButton *m_fps;
    QPushButton *m_maxreg;
    QPushButton *m_version;
    QPushButton *m_ps;
    QPushButton *m_reboot;
    QPushButton *m_df;
    QPushButton *m_free;
    QPushButton *m_caminfo;
    QLabel *m_text;

private slots:
    void fps_btn_clicked();
    void caminfo_btn_clicked();
    void version_btn_clicked();
    void ps_btn_clicked();
    void reboot_btn_clicked();
    void df_btn_clicked();
    void free_btn_clicked();
    void common_cmd(QString str);
};


class DebugWidget : public QDialog
{
    Q_OBJECT

public:
    DebugWidget(QWidget *parent);
    void layout();

private:
    QPushButton *m_calib_txt_btn;
    QPushButton *m_rm_calib_txt_btn;
    QPushButton *m_calib_right_img_btn;
    QPushButton *m_calib_front_img_btn;
    QPushButton *m_calib_left_img_btn;
    QPushButton *m_calib_rear_img_btn;
    QPushButton *m_dmesg_btn;
    QPushButton *m_appmsg_btn;
    QPushButton *m_comumsg_btn;
    QPushButton *m_ini_file_btn;
    QPushButton *m_cfg_ini_btn;
    QPushButton *m_calib_param_fas_cfg_btn;
    QPushButton *m_calib;
    QPushButton *m_others;
    QPushButton *m_reset_all;
    QPushButton *m_exit;
    QScrollArea *m_scrollarea;

private:
    void calib_image_show(int idx);

private slots:
    void calib_txt_btn_clicked();
    void rm_calib_txt_btn_clicked();
    void calib_right_btn_clicked();
    void calib_front_btn_clicked();
    void calib_left_btn_clicked();
    void calib_rear_btn_clicked();
    void dmesg_btn_clicked();
    void appmsg_btn_clicked();
    void comumsg_btn_clicked();
    void ini_file_btn_clicked();
    void cfg_ini_btn_clicked();
    void calib_param_fas_cfg_btn_clicked();
    void calib_btn_clicked();
    void others_btn_clicked();
    void exit_btn_clicked();
};



#endif // DEBUG_WIDGET_H
