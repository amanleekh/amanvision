
#include <QTextStream>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QProcess>
#include <QFileInfo>
#include <QTableWidget>
#include <QHeaderView>
#include <QCryptographicHash>

#include <set>

#include "debug_widget.h"
#include "memlog.h"
#include "ini_config.h"
#include "comu_cmd_define.h"
#include "comu_cmd.h"
#include "sync_video_getter.h"
#include "surround_widget.h"
#include "custom_str.h"
#include "video_capture.h"


static QString s_line;


void ClrStr()
{
    s_line.clear();
}

void GenStr(char *str)
{
    s_line.append(str);
}

void GenStrTime(unsigned int time, char *str)
{
    QString qstr = QString("[") + QString::number(time) + QString("]") + QString(str);
    s_line.append(qstr);
}


QString *GetStr()
{
    return &s_line;
}


DebugWidget::DebugWidget(QWidget *parent)
    : QDialog(parent)
{

}


void DebugWidget::layout()
{
    // left menu
    m_calib_txt_btn = new QPushButton(this);
    m_calib_txt_btn->setText("lst calib txt");
    m_rm_calib_txt_btn = new QPushButton(this);
    m_rm_calib_txt_btn->setText("*del calib txt");
    m_calib_right_img_btn = new QPushButton(this);
    m_calib_right_img_btn->setText("lst calib right img");
    m_calib_front_img_btn = new QPushButton(this);
    m_calib_front_img_btn->setText("lst calib front img");
    m_calib_left_img_btn = new QPushButton(this);
    m_calib_left_img_btn->setText("lst calib left img");
    m_calib_rear_img_btn = new QPushButton(this);
    m_calib_rear_img_btn->setText("lst calib rear img");
    m_dmesg_btn = new QPushButton(this);
    m_dmesg_btn->setText("cur dmesg");
    m_appmsg_btn = new QPushButton(this);
    m_appmsg_btn->setText("cur qt app msg");
    m_comumsg_btn = new QPushButton(this);
    m_comumsg_btn->setText("cur comu msg");
    m_ini_file_btn = new QPushButton(this);
    m_ini_file_btn->setText("show ini file");
    m_cfg_ini_btn = new QPushButton(this);
    m_cfg_ini_btn->setText("change ini file");
    m_calib_param_fas_cfg_btn = new QPushButton(this);
    m_calib_param_fas_cfg_btn->setText("calib param(fas)");
    m_calib = new QPushButton(this);
    m_calib->setText("enter calib menu");
    m_others = new QPushButton(this);
    m_others->setText("others");
    m_exit = new QPushButton(this);
    m_exit->setText("exit"); 
    QGroupBox *left_group_box = new QGroupBox(this);
    left_group_box->setObjectName("left_group");
    left_group_box->setStyleSheet("#left_group{border: 1px solid gray;}");
    QVBoxLayout *left_layout = new QVBoxLayout(this);
    left_layout->setSpacing(2);
    left_layout->setMargin(2);
    //left_layout->setContentsMargins(0,0,0,0);
    left_layout->setAlignment(Qt::AlignTop);
    left_layout->addWidget(m_calib_txt_btn);
    left_layout->addWidget(m_rm_calib_txt_btn);
    left_layout->addWidget(m_calib_right_img_btn);
    left_layout->addWidget(m_calib_front_img_btn);
    left_layout->addWidget(m_calib_left_img_btn);
    left_layout->addWidget(m_calib_rear_img_btn);
    left_layout->addWidget(m_dmesg_btn);
    left_layout->addWidget(m_appmsg_btn);
    left_layout->addWidget(m_comumsg_btn);
    left_layout->addWidget(m_ini_file_btn);
    left_layout->addWidget(m_cfg_ini_btn);
    left_layout->addWidget(m_calib_param_fas_cfg_btn);
    left_layout->addWidget(m_calib);
    left_layout->addWidget(m_others);
    left_layout->addWidget(m_exit);
    left_group_box->setLayout(left_layout);
    left_group_box->setMinimumWidth(182);
    left_group_box->setMaximumWidth(182);

    // right
    m_scrollarea = new QScrollArea(this);

    // layout
    QHBoxLayout *total_layout = new QHBoxLayout(this);
    total_layout->addWidget(left_group_box);
    total_layout->addWidget(m_scrollarea);
    this->setLayout(total_layout);

    connect(m_calib_txt_btn, SIGNAL(released()), this, SLOT(calib_txt_btn_clicked()));
    connect(m_rm_calib_txt_btn, SIGNAL(released()), this, SLOT(rm_calib_txt_btn_clicked()));
    connect(m_calib_right_img_btn, SIGNAL(released()), this, SLOT(calib_right_btn_clicked()));
    connect(m_calib_front_img_btn, SIGNAL(released()), this, SLOT(calib_front_btn_clicked()));
    connect(m_calib_left_img_btn, SIGNAL(released()), this, SLOT(calib_left_btn_clicked()));
    connect(m_calib_rear_img_btn, SIGNAL(released()), this, SLOT(calib_rear_btn_clicked()));
    connect(m_dmesg_btn, SIGNAL(released()), this, SLOT(dmesg_btn_clicked()));
    connect(m_appmsg_btn, SIGNAL(released()), this, SLOT(appmsg_btn_clicked()));
    connect(m_comumsg_btn, SIGNAL(released()), this, SLOT(comumsg_btn_clicked()));
    connect(m_ini_file_btn, SIGNAL(released()), this, SLOT(ini_file_btn_clicked()));
    connect(m_cfg_ini_btn, SIGNAL(released()), this, SLOT(cfg_ini_btn_clicked()));
    connect(m_calib_param_fas_cfg_btn, SIGNAL(released()), this, SLOT(calib_param_fas_cfg_btn_clicked()));
    connect(m_calib, SIGNAL(released()), this, SLOT(calib_btn_clicked()));
    connect(m_others, SIGNAL(released()), this, SLOT(others_btn_clicked()));
    connect(m_exit, SIGNAL(released()), this, SLOT(exit_btn_clicked()));
}


void DebugWidget::calib_txt_btn_clicked()
{

    QLabel *p_text = new QLabel(m_scrollarea);
    QString line;

    // avm_calib_log.txt
    QFile file("avm_calib_log.txt");
    line.append("====avm_calib_log.txt====\n\n");
    if (file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QTextStream stream(&file);
        while (!stream.atEnd())
        {
            line.append(stream.readLine()+"\n");
        }
    }
    else
    {
        line.append("ERROR: read 'avm_calib_log.txt' faild\n");
    }
    file.close();

    // avm_calib_log_comu.txt
    QFile comu_file("avm_calib_log_comu.txt");
    line.append("\n====avm_calib_log_comu.txt====\n\n");
    if (comu_file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QTextStream stream(&comu_file);
        while (!stream.atEnd())
        {
            line.append(stream.readLine()+"\n");
        }
    }
    else
    {
        line.append("ERROR: read 'avm_calib_log_comu.txt' faild\n");
    }    
    comu_file.close();

    // set content
    p_text->setText(line);
    p_text->setObjectName("m_text");
    p_text->setStyleSheet("#m_text{font-size:12px;}");
    m_scrollarea->setWidget(p_text);
}


void DebugWidget::rm_calib_txt_btn_clicked()
{
    remove("avm_calib_log.txt");
    remove("avm_calib_log_comu.txt");
}


void DebugWidget::calib_image_show(int idx)
{
    ImageWidget *p_img_widget = new ImageWidget(m_scrollarea);
    p_img_widget->setMinimumWidth(m_scrollarea->size().width()-36);

    if (1 == idx)
    {
        p_img_widget->AddImage("cap_right.jpg", 0);
        p_img_widget->AddImage("r_draw_roi_image.jpg", 0);
        p_img_widget->AddImage("r_geom_dilate.jpg", 0);
        p_img_widget->AddImage("r_result_mask_contours.jpg", 1);
    }
    else if (2 == idx)
    {
        p_img_widget->AddImage("cap_front.jpg", 0);
        p_img_widget->AddImage("f_draw_roi_image.jpg", 0);
        p_img_widget->AddImage("f_geom_dilate.jpg", 0);
        p_img_widget->AddImage("f_result_mask_contours.jpg", 1);
    }
    else if (3 == idx)
    {
        p_img_widget->AddImage("cap_left.jpg", 0);
        p_img_widget->AddImage("l_draw_roi_image.jpg", 0);
        p_img_widget->AddImage("l_geom_dilate.jpg", 0);
        p_img_widget->AddImage("l_result_mask_contours.jpg", 1);
    }
    else
    {
        p_img_widget->AddImage("cap_rear.jpg", 0);
        p_img_widget->AddImage("b_draw_roi_image.jpg", 0);
        p_img_widget->AddImage("b_geom_dilate.jpg", 0);
        p_img_widget->AddImage("b_result_mask_contours.jpg", 1);
    }
    m_scrollarea->setWidget(p_img_widget);
}



void DebugWidget::calib_right_btn_clicked()
{
    calib_image_show(1);
}

void DebugWidget::calib_front_btn_clicked()
{
    calib_image_show(2);
}

void DebugWidget::calib_left_btn_clicked()
{
    calib_image_show(3);
}

void DebugWidget::calib_rear_btn_clicked()
{
    calib_image_show(4);
}


void DebugWidget::dmesg_btn_clicked()
{
    QLabel *p_text = new QLabel(m_scrollarea);

    QProcess dmesg;
    dmesg.start("dmesg");
    if (!dmesg.waitForStarted())
    {
        p_text->setText("ERROR: start dmesg faild\n");
        m_scrollarea->setWidget(p_text);
        return;
    }
    if (!dmesg.waitForFinished())
    {
        p_text->setText("ERROR: wait dmesg finish faild\n");
        m_scrollarea->setWidget(p_text);
        return;
    }

    QByteArray contents = dmesg.readAll();
    QTextStream stream(&contents);
    QString line;
    while (!stream.atEnd())
    {
        line.append(stream.readLine()+"\n");
    }
    p_text->setText(line);

    p_text->setObjectName("m_text");
    p_text->setStyleSheet("#m_text{font-size:12px;}");
    m_scrollarea->setWidget(p_text);
}


void DebugWidget::appmsg_btn_clicked()
{
    int re;
    int mem_size;
    re = memlog_server_dump_get_mem_size(&mem_size);
    if (re != 0)
    {
        return;
    }
    unsigned char *p_mem = (unsigned char *)malloc(mem_size);
    if (NULL == p_mem)
    {
        return;
    }
    re = memlog_server_dump(p_mem, mem_size);
    if (re != 0)
    {
        free(p_mem);
        return;
    }

    ClrStr();
    //memlog_server_paser(p_mem, mem_size, &GenStr);
    memlog_server_paser_time(p_mem, mem_size, &GenStrTime);
    free(p_mem);
    
    QLabel *p_text = new QLabel(m_scrollarea);
    p_text->setText(*GetStr());

    p_text->setObjectName("m_text");
    p_text->setStyleSheet("#m_text{font-size:12px;}");
    m_scrollarea->setWidget(p_text);    
}


void DebugWidget::comumsg_btn_clicked()
{
    // dump comu log to file
    QLabel *p_text = new QLabel(m_scrollarea);
    int re;
    unsigned char *p_share = NULL;
    ext_share_mem_s *p_share_tmp = NULL;
    int share_size = 0;
    
    re = comucmd_get_ext_share_mem(&p_share, &share_size);
    if ((re != 0) || (NULL == p_share) || (share_size <= 0))
    {
        p_text->setText("ERROR: get share mem faild\n");
        p_text->setObjectName("m_text");
        p_text->setStyleSheet("#m_text{font-size:12px;}");
        m_scrollarea->setWidget(p_text); 
        return;
    }

    p_share_tmp = (ext_share_mem_s *)p_share;
    re = fileprt_dumptofile(p_share_tmp->fileprt_area, (char *)("comu_log.txt"));
    if (re != 0)
    {
        p_text->setText("ERROR: dump file faild\n");
        p_text->setObjectName("m_text");
        p_text->setStyleSheet("#m_text{font-size:12px;}");
        m_scrollarea->setWidget(p_text); 
        return; 
    }

    // read file context
    QFile file("comu_log.txt");
    QString line;
    if (file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QTextStream stream(&file);
        while (!stream.atEnd())
        {
            line.append(stream.readLine()+"\n");
        }
        p_text->setText(line);
    }
    else
    {
        p_text->setText("ERROR: read 'comu_log.txt' faild\n");
    }
    file.close();

    p_text->setObjectName("m_text");
    p_text->setStyleSheet("#m_text{font-size:12px;}");
    m_scrollarea->setWidget(p_text); 
}


void DebugWidget::ini_file_btn_clicked()
{
    QFile file("avm_qt_app_res/config.ini");
    QLabel *p_text = new QLabel(m_scrollarea);
    QString line;
    if (file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QTextStream stream(&file);
        while (!stream.atEnd())
        {
            line.append(stream.readLine()+"\n");
        }
        p_text->setText(line);
    }
    else
    {
        p_text->setText("ERROR: read 'avm_qt_app_res/config.ini' faild\n");
    }
    file.close();

    p_text->setObjectName("m_text");
    p_text->setStyleSheet("#m_text{font-size:12px;}");
    m_scrollarea->setWidget(p_text);
}


void DebugWidget::cfg_ini_btn_clicked()
{
    IniFileWidget *p_ini_widget = new IniFileWidget(m_scrollarea);
    p_ini_widget->setMinimumWidth(m_scrollarea->size().width()-36);
    p_ini_widget->setMaximumWidth(m_scrollarea->size().width()-36);
    p_ini_widget->setMinimumHeight(m_scrollarea->size().height()-80);
    p_ini_widget->setMaximumHeight(m_scrollarea->size().height()-80);
    p_ini_widget->layout();
    m_scrollarea->setWidget(p_ini_widget);
}


void DebugWidget::calib_param_fas_cfg_btn_clicked()
{
    CalibParamWidget *p_calib_param_fas_widget = new CalibParamWidget(m_scrollarea);
    p_calib_param_fas_widget->setMinimumWidth(m_scrollarea->size().width()-36);
    p_calib_param_fas_widget->setMaximumWidth(m_scrollarea->size().width()-36);
    p_calib_param_fas_widget->setMinimumHeight(m_scrollarea->size().height()-80);
    p_calib_param_fas_widget->setMaximumHeight(m_scrollarea->size().height()-80);
    p_calib_param_fas_widget->layout();
    m_scrollarea->setWidget(p_calib_param_fas_widget);
}


void DebugWidget::calib_btn_clicked()
{
    CalibWidget *p_calib_widget = new CalibWidget(m_scrollarea);
    //p_calib_widget->setMinimumWidth(m_scrollarea->size().width()-36);
    p_calib_widget->layout();
    m_scrollarea->setWidget(p_calib_widget);
}


void DebugWidget::others_btn_clicked()
{
    OthersWidget *p_others_widget = new OthersWidget(m_scrollarea);
    p_others_widget->setMinimumWidth(m_scrollarea->size().width()-36);
    p_others_widget->layout();
    m_scrollarea->setWidget(p_others_widget);
}


void DebugWidget::exit_btn_clicked()
{
    this->hide();

}


ImageWidget::ImageWidget(QWidget * parent)
    : QWidget(parent)
{
    m_layout = new QVBoxLayout(this);
    this->setLayout(m_layout);
    
    m_image_idx = 0;
    for (int i = 0; i < IMG_WGT_MAX_IMG; i++)
    {
        m_image[i] = new QLabel(this);
    }
}


void ImageWidget::AddImage(QString name, int is_org)
{
    if (m_image_idx < IMG_WGT_MAX_IMG)
    {
        QPixmap pix_img(name);
        if (pix_img.isNull())
        {
            QString tmp_str = QString("ERROR: '") + name + QString("' is not exist");
            m_image[m_image_idx]->setText(tmp_str);
            m_layout->addWidget(m_image[m_image_idx]);
            m_image_idx++;
        }
        else
        {
            if (0 == is_org)
            {
                pix_img = pix_img.scaledToWidth(this->size().width());
            }
            m_image[m_image_idx]->setPixmap(pix_img);
            m_layout->addWidget(m_image[m_image_idx]);
            m_image_idx++;
        }
    }
}


void ImageWidget::RemoveAllImage()
{
    for (int i = 0; i < m_image_idx; i++)
    {
        m_layout->removeWidget(m_image[m_image_idx]);
    }
}


CalibWidget::CalibWidget(QWidget * parent)
    : QWidget(parent)
{
    calib_src << "avm_calib_res/assist_line_calib_front.db";
    calib_src << "avm_calib_res/assist_line_calib_rear.db";
    calib_src << "avm_calib_res/assist_line_front.db";
    calib_src << "avm_calib_res/assist_line_rear.db";
    calib_src << "avm_calib_res/block_line_front.db";
    calib_src << "avm_calib_res/block_line_rear.db";
    calib_src << "avm_calib_res/fisheye_clip.db";
    calib_src << "avm_calib_res/Table.db";
    calib_src << "avm_calib_res/table_cam_two.db";

    calib_dst << "avm_qt_app_res/data/assist_line_calib_front.db";
    calib_dst << "avm_qt_app_res/data/assist_line_calib_rear.db";
    calib_dst << "avm_qt_app_res/data/assist_line_front.db";
    calib_dst << "avm_qt_app_res/data/assist_line_rear.db";
    calib_dst << "avm_qt_app_res/data/block_line_front.db";
    calib_dst << "avm_qt_app_res/data/block_line_rear.db";
    calib_dst << "avm_qt_app_res/data/fisheye_clip.db";
    calib_dst << "avm_qt_app_res/data/Table.db";
    calib_dst << "avm_qt_app_res/data/table_cam_two.db";  
}


void CalibWidget::layout()
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setSizeConstraint(QLayout::SetMinimumSize);
    layout->setSpacing(15);
    this->setLayout(layout);

    //copy default config
    m_copy_img_to_calib_res = new QPushButton(this);
    m_copy_img_to_calib_res->setText("1-1) copy cur image to calib_res dir");
    layout->addWidget(m_copy_img_to_calib_res);
    m_copy_car_cx_img_to_calib_res = new QPushButton(this);
    m_copy_car_cx_img_to_calib_res->setText("1-2)copy cx image to calib_res dir");
    layout->addWidget(m_copy_car_cx_img_to_calib_res);
    m_copy_car_cc_img_to_calib_res = new QPushButton(this);
    m_copy_car_cc_img_to_calib_res->setText("1-3)copy cc image to calib_res dir");
    layout->addWidget(m_copy_car_cc_img_to_calib_res);
    m_copy_car_cs_img_to_calib_res = new QPushButton(this);
    m_copy_car_cs_img_to_calib_res->setText("1-4)copy cs image to calib_res dir");
    layout->addWidget(m_copy_car_cs_img_to_calib_res);
    m_copy_cfg_cx = new QPushButton(this);
    m_copy_cfg_cx->setText("1-5)copy cx cfg to cur cfg");
    layout->addWidget(m_copy_cfg_cx);
    m_copy_cfg_cc = new QPushButton(this);
    m_copy_cfg_cc->setText("1-6)copy cc cfg to cur cfg");
    layout->addWidget(m_copy_cfg_cc);
    m_copy_cfg_cs = new QPushButton(this);
    m_copy_cfg_cs->setText("1-7)copy cs cfg to cur cfg");
    layout->addWidget(m_copy_cfg_cs);

    //save cam image
    m_calib_save_cam_img = new QPushButton(this);
    m_calib_save_cam_img->setText("2-1) calib:save cam img");
    layout->addWidget(m_calib_save_cam_img);

    //calib
    m_calib_cam_nosave = new QPushButton(this);
    m_calib_cam_nosave->setText("3-1) calib:use cam, not save dbg img");
    layout->addWidget(m_calib_cam_nosave);
    m_calib_cam_save = new QPushButton(this);
    m_calib_cam_save->setText("3-2) calib:use cam, save dbg img");
    layout->addWidget(m_calib_cam_save);
    m_calib_img_nosave = new QPushButton(this);
    m_calib_img_nosave->setText("3-3) calib:use img, not save dbg img");
    layout->addWidget(m_calib_img_nosave);
    m_calib_img_save = new QPushButton(this);
    m_calib_img_save->setText("3-4) calib:use img, save dbg img");
    layout->addWidget(m_calib_img_save);

    m_calib_param_save = new QPushButton(this);
    m_calib_param_save->setText("3-5) calib:use param to calib");
    layout->addWidget(m_calib_param_save);

    //action after calib
    m_rm_lst_alg_tmp_img = new QPushButton(this);
    m_rm_lst_alg_tmp_img->setText("4-1) rm lst alg tmp image");
    layout->addWidget(m_rm_lst_alg_tmp_img);
    m_copy_calib_data = new QPushButton(this);
    m_copy_calib_data->setText("4-2) copy calib data to avm_qt_app data dir");
    layout->addWidget(m_copy_calib_data);
    m_calc_calib_res_hash = new QPushButton(this);
    m_calc_calib_res_hash->setText("4-3) calc calib res hash");
    layout->addWidget(m_calc_calib_res_hash);
    m_calc_data_hash = new QPushButton(this);
    m_calc_data_hash->setText("4-4) calc data hash");
    layout->addWidget(m_calc_data_hash);

    m_label = new QLabel(this);
    m_label->setWordWrap(true);
    m_label->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    layout->addWidget(m_label);


    connect(m_calib_save_cam_img, SIGNAL(released()), this, SLOT(cam_save_cam_img()));
    connect(m_calib_cam_nosave, SIGNAL(released()), this, SLOT(cam_nosave_btn_click()));
    connect(m_calib_cam_save, SIGNAL(released()), this, SLOT(cam_save_btn_click()));
    connect(m_copy_img_to_calib_res, SIGNAL(released()), this, SLOT(copy_img_to_calib_res_btn_click()));
    connect(m_copy_car_cx_img_to_calib_res, SIGNAL(released()), this, SLOT(copy_car_cx_img_to_calib_res_btn_click()));
    connect(m_copy_car_cc_img_to_calib_res, SIGNAL(released()), this, SLOT(copy_car_cc_img_to_calib_res_btn_click()));
    connect(m_copy_car_cs_img_to_calib_res, SIGNAL(released()), this, SLOT(copy_car_cs_img_to_calib_res_btn_click()));
    connect(m_copy_cfg_cx, SIGNAL(released()), this, SLOT(copy_cfg_cx_btn_click()));
    connect(m_copy_cfg_cc, SIGNAL(released()), this, SLOT(copy_cfg_cc_btn_click()));
    connect(m_copy_cfg_cs, SIGNAL(released()), this, SLOT(copy_cfg_cs_btn_click()));
    connect(m_calib_img_nosave, SIGNAL(released()), this, SLOT(img_nosave_btn_click()));
    connect(m_calib_img_save, SIGNAL(released()), this, SLOT(img_save_btn_click()));
    connect(m_calib_param_save, SIGNAL(released()), this, SLOT(param_save_btn_click()));
    connect(m_copy_calib_data, SIGNAL(released()), this, SLOT(copy_calib_data_btn_click()));
    connect(m_rm_lst_alg_tmp_img, SIGNAL(released()), this, SLOT(rm_lst_alg_tmp_img_btn_click()));
    connect(m_calc_calib_res_hash, SIGNAL(released()), this, SLOT(calc_calib_res_hash_btn_click()));
    connect(m_calc_data_hash, SIGNAL(released()), this, SLOT(calc_data_hash_btn_click()));
}


void CalibWidget::cam_save_cam_img()
{
#if GLOBAL_RUN_ENV_DESKTOP == 0
    SyncVideoGetter::saveCamPic();
#else
    m_label->setText("ERROR: pc env can't save cam image\n");
#endif
}


void CalibWidget::cam_nosave_btn_click()
{
    calib(0, 0);
}


void CalibWidget::cam_save_btn_click()
{
    calib(0, 1);
}


void CalibWidget::img_nosave_btn_click()
{
    calib(1, 0);

}


void CalibWidget::img_save_btn_click()
{
    calib(1, 1);
}


void CalibWidget::param_save_btn_click()
{
    QProcess calib;
    QStringList argu;

    argu << QString::number(1) << QString::number(1) << QString::number(4);

    // remove current calib db file fisrst
    for (int i = 0; i < calib_src.size(); i++)
    {
        QFile::remove(calib_src.at(i));
    }

    m_label->setText("");
    calib.start("./avm_calib", argu);
    if (!calib.waitForStarted())
    {
        m_label->setText("ERROR: start calib faild\n");
        return;
    }
    if (!calib.waitForFinished())
    {
        m_label->setText("ERROR: wait calib finish faild\n");
        return;
    }

    QByteArray output = calib.readAllStandardOutput();

    if(output.isEmpty())
    {
        qDebug()<<"[qt]--calib log is empty--";
    }
    else
    {
        qDebug()<<output.data();
    }

    int car_type = ini_get_car_type();

    // check if new calib file exists
    int new_file_exist = 1;
    for (int i = 0; i < calib_src.size(); i++)
    {
        if (false == QFile::exists(calib_src.at(i)))
        {
            new_file_exist = 0;;
        }
    }
    if (1 == new_file_exist)
    {
        m_label->setText(QString("OK: calib finished success, car_type=%1, check lst calib.txt for info").arg(car_type));
    }
    else
    {
        m_label->setText(QString("ERROR: calib finished, car_type=%1, but db is not exist, check lst calib.txt for info").arg(car_type));
    }
}




int CalibWidget::copy_calib_file_to_data_dir()
{
    // file is not exist
    for (int i = 0; i < calib_src.size(); i++)
    {
        if (false == QFile::exists(calib_src.at(i)))
        {
            return 1;
        }
    }

    // remove dst file first, the copy
    for (int i = 0; i < calib_src.size(); i++)
    {
        if (false == QFile::remove(calib_dst.at(i)))
        {
            return 1;
        }
    }
    for (int i = 0; i < calib_src.size(); i++)
    {
        if (false == QFile::copy(calib_src.at(i), calib_dst.at(i)))
        {
            return 1;
        }
    }

    if ((true == QFile::exists("avm_calib_res/cap_right.jpg")) 
        && (true == QFile::exists("avm_calib_res/cap_front.jpg")) 
        && (true == QFile::exists("avm_calib_res/cap_left.jpg")) 
        && (true == QFile::exists("avm_calib_res/cap_rear.jpg")))
    {
        QFile::remove("avm_qt_app_res/data/cap_right.jpg");
        QFile::remove("avm_qt_app_res/data/cap_front.jpg");
        QFile::remove("avm_qt_app_res/data/cap_left.jpg");
        QFile::remove("avm_qt_app_res/data/cap_rear.jpg");

        QFile::copy("avm_calib_res/cap_right.jpg", "avm_qt_app_res/data/cap_right.jpg");
        QFile::copy("avm_calib_res/cap_front.jpg", "avm_qt_app_res/data/cap_front.jpg");
        QFile::copy("avm_calib_res/cap_left.jpg", "avm_qt_app_res/data/cap_left.jpg");
        QFile::copy("avm_calib_res/cap_rear.jpg", "avm_qt_app_res/data/cap_rear.jpg");
    }

    // sync
    QProcess sync;
    sync.start("sync");
    sync.waitForFinished();
    
    sync.start("sync");
    sync.waitForFinished();
    
    return 0;
}


void CalibWidget::calib(int is_use_img, int is_debug)
{
    QProcess calib;
    QStringList argu;

    if (1 == is_use_img)
    {
        if ((false == QFile::exists("avm_calib_res/cap_right.jpg")) 
            || (false == QFile::exists("avm_calib_res/cap_front.jpg")) 
            || (false == QFile::exists("avm_calib_res/cap_left.jpg")) 
            || (false == QFile::exists("avm_calib_res/cap_rear.jpg")))
        {
            m_label->setText("ERR: can't find image in avm_calib_res dir\n");
            return;
        }
        argu << QString::number(is_use_img) << QString::number(is_debug);
    }
    else
    {
        if ((false == QFile::exists("tmp_cap_right.jpg")) 
            || (false == QFile::exists("tmp_cap_front.jpg")) 
            || (false == QFile::exists("tmp_cap_left.jpg")) 
            || (false == QFile::exists("tmp_cap_rear.jpg")))
        {
            m_label->setText("ERR: can't find cam tmp image in cur dir\n");
            return;
        }
        QFile::remove("avm_calib_res/cap_right.jpg");
        QFile::remove("avm_calib_res/cap_front.jpg");
        QFile::remove("avm_calib_res/cap_left.jpg");
        QFile::remove("avm_calib_res/cap_rear.jpg");

        QFile::copy("tmp_cap_right.jpg", "avm_calib_res/cap_right.jpg");
        QFile::copy("tmp_cap_front.jpg", "avm_calib_res/cap_front.jpg");
        QFile::copy("tmp_cap_left.jpg", "avm_calib_res/cap_left.jpg");
        QFile::copy("tmp_cap_rear.jpg", "avm_calib_res/cap_rear.jpg");

        QFile::remove("tmp_cap_right.jpg");
        QFile::remove("tmp_cap_front.jpg");
        QFile::remove("tmp_cap_left.jpg");
        QFile::remove("tmp_cap_rear.jpg");

        // we have save cam pic, so is_use_img = 1
        argu << QString::number(1) << QString::number(is_debug);
    }

    // remove current calib db file fisrst
    for (int i = 0; i < calib_src.size(); i++)
    {
        QFile::remove(calib_src.at(i));
    }
    QFile::remove("avm_calib_log.txt");
    // remove debug image
    QFile::remove("r_draw_roi_image.jpg");
    QFile::remove("r_geom_dilate.jpg");
    QFile::remove("r_result_mask_contours.jpg");
    QFile::remove("f_draw_roi_image.jpg");
    QFile::remove("f_geom_dilate.jpg");
    QFile::remove("f_result_mask_contours.jpg");
    QFile::remove("l_draw_roi_image.jpg");
    QFile::remove("l_geom_dilate.jpg");
    QFile::remove("l_result_mask_contours.jpg");
    QFile::remove("b_draw_roi_image.jpg");
    QFile::remove("b_geom_dilate.jpg");
    QFile::remove("b_result_mask_contours.jpg");
    
    int car_type = ini_get_car_type();
    
    m_label->setText("");
    calib.start("./avm_calib", argu);
    if (!calib.waitForStarted())
    {
        m_label->setText("ERROR: start calib faild\n");
        return;
    }
    if (!calib.waitForFinished())
    {
        m_label->setText("ERROR: wait calib finish faild\n");
        return;
    }

    QByteArray output = calib.readAllStandardOutput();

    if(output.isEmpty())
    {
        qDebug()<<"[qt]--calib log is empty--";
    }
    else
    {
        qDebug()<<output.data();
    }

    // check if new calib file exists
    int new_file_exist = 1;
    for (int i = 0; i < calib_src.size(); i++)
    {
        if (false == QFile::exists(calib_src.at(i)))
        {
            new_file_exist = 0;;
        }
    }
    if (1 == new_file_exist)
    {
        m_label->setText(QString("OK: calib finished success, car_type=%1, check lst calib.txt for info").arg(car_type));
    }
    else
    {
        m_label->setText(QString("ERROR: calib finished, car_type=%1, but db is not exist, check lst calib.txt for info").arg(car_type));
    }
}


void CalibWidget::copy_calib_data_btn_click()
{
    int re = copy_calib_file_to_data_dir();
    if (0 == re)
    {
        m_label->setText("OK: replace calib file success\n");
    }
    else
    {
        m_label->setText("ERROR: replace calib file faild\n");
    }
}


void CalibWidget::rm_lst_alg_tmp_img_btn_click()
{
    QFile::remove("r_draw_roi_image.jpg");
    QFile::remove("f_draw_roi_image.jpg");
    QFile::remove("l_draw_roi_image.jpg");
    QFile::remove("b_draw_roi_image.jpg");

    QFile::remove("r_geom_dilate.jpg");
    QFile::remove("f_geom_dilate.jpg");
    QFile::remove("l_geom_dilate.jpg");
    QFile::remove("b_geom_dilate.jpg");

    QFile::remove("r_result_mask_contours.jpg");
    QFile::remove("f_result_mask_contours.jpg");
    QFile::remove("l_result_mask_contours.jpg");
    QFile::remove("b_result_mask_contours.jpg");

    m_label->setText("OK: rm alg tmp file success\n");
}


void CalibWidget::calc_calib_res_hash_btn_click()
{
    const char *p_file = "avm_calib_res/Table.db";

    if (false == QFile::exists(p_file))
    {
        m_label->setText("ERROR:calib res file file is not exist");
    }
    
    QFile theFile(p_file);
    theFile.open(QIODevice::ReadOnly);
    QByteArray ba = QCryptographicHash::hash(theFile.readAll(), QCryptographicHash::Md5);
    theFile.close();

    m_label->setText("");
    m_label->setText(QString("calib_res_hash:") + QString(ba.toHex().constData()));
}


void CalibWidget::calc_data_hash_btn_click()
{
    const char *p_file = "avm_qt_app_res/data/Table.db";

    if (false == QFile::exists(p_file))
    {
        m_label->setText("ERROR:data file is not exist");
    }
    
    QFile theFile(p_file);
    theFile.open(QIODevice::ReadOnly);
    QByteArray ba = QCryptographicHash::hash(theFile.readAll(), QCryptographicHash::Md5);
    theFile.close();

    m_label->setText("");
    m_label->setText(QString("data_hash:") + QString(ba.toHex().constData()));
}


void CalibWidget::copy_car_img(int type)
{
    type = type;
}


void CalibWidget::copy_cfg(int type)
{
    type = type;
}

void CalibWidget::copy_img_to_calib_res_btn_click()
{
    m_label->setText("");

    if ((true == QFile::exists("cap_right.jpg")) 
        && (true == QFile::exists("cap_front.jpg")) 
        && (true == QFile::exists("cap_left.jpg")) 
        && (true == QFile::exists("cap_rear.jpg")))
    {
        QFile::remove("avm_calib_res/cap_right.jpg");
        QFile::remove("avm_calib_res/cap_front.jpg");
        QFile::remove("avm_calib_res/cap_left.jpg");
        QFile::remove("avm_calib_res/cap_rear.jpg");

        int re1 = QFile::copy("cap_right.jpg", "avm_calib_res/cap_right.jpg");
        int re2 = QFile::copy("cap_front.jpg", "avm_calib_res/cap_front.jpg");
        int re3 = QFile::copy("cap_left.jpg", "avm_calib_res/cap_left.jpg");
        int re4 = QFile::copy("cap_rear.jpg", "avm_calib_res/cap_rear.jpg");
        if (re1 && re2 && re3 && re4)
        {
            m_label->setText("OK: cp cap file to calib res dir success");
        }
        else
        {
            m_label->setText("ERROR: cp cap file to calib res dir faild");
        }
    }
    else
    {
        m_label->setText("ERR: no cap file in current dir\n");
    }
}


void CalibWidget::copy_car_cx_img_to_calib_res_btn_click()
{
    QFile::remove("avm_calib_res/cap_right.jpg");
    QFile::remove("avm_calib_res/cap_front.jpg");
    QFile::remove("avm_calib_res/cap_left.jpg");
    QFile::remove("avm_calib_res/cap_rear.jpg");
    QFile::remove("avm_calib_res/calib_factory.param");
    
    int re1 = QFile::copy("avm_qt_app_res/cx11/cap_right.jpg", "avm_calib_res/cap_right.jpg");
    int re2 = QFile::copy("avm_qt_app_res/cx11/cap_front.jpg", "avm_calib_res/cap_front.jpg");
    int re3 = QFile::copy("avm_qt_app_res/cx11/cap_left.jpg", "avm_calib_res/cap_left.jpg");
    int re4 = QFile::copy("avm_qt_app_res/cx11/cap_rear.jpg", "avm_calib_res/cap_rear.jpg");
    int re5 = QFile::copy("avm_qt_app_res/cx11/calib_factory.param", "avm_calib_res/calib_factory.param");
    if (re1 && re2 && re3 && re4 && re5)
    {
        m_label->setText("OK: cp cx11 cap file and param to calib res dir success");
    }
    else
    {
        m_label->setText("ERROR: cp cx11 cap file and param to calib res dir failed");
    }
}


void CalibWidget::copy_car_cc_img_to_calib_res_btn_click()
{
    QFile::remove("avm_calib_res/cap_right.jpg");
    QFile::remove("avm_calib_res/cap_front.jpg");
    QFile::remove("avm_calib_res/cap_left.jpg");
    QFile::remove("avm_calib_res/cap_rear.jpg");
    QFile::remove("avm_calib_res/calib_factory.param");
    
    int re1 = QFile::copy("avm_qt_app_res/cc11/cap_right.jpg", "avm_calib_res/cap_right.jpg");
    int re2 = QFile::copy("avm_qt_app_res/cc11/cap_front.jpg", "avm_calib_res/cap_front.jpg");
    int re3 = QFile::copy("avm_qt_app_res/cc11/cap_left.jpg", "avm_calib_res/cap_left.jpg");
    int re4 = QFile::copy("avm_qt_app_res/cc11/cap_rear.jpg", "avm_calib_res/cap_rear.jpg");
    int re5 = QFile::copy("avm_qt_app_res/cc11/calib_factory.param", "avm_calib_res/calib_factory.param");
    if (re1 && re2 && re3 && re4 && re5)
    {
        m_label->setText("OK: cp cc11 cap file and param to calib res dir success");
    }
    else
    {
        m_label->setText("ERROR: cp cc11 cap file and param to calib res dir failed");
    }
}


void CalibWidget::copy_car_cs_img_to_calib_res_btn_click()
{
    QFile::remove("avm_calib_res/cap_right.jpg");
    QFile::remove("avm_calib_res/cap_front.jpg");
    QFile::remove("avm_calib_res/cap_left.jpg");
    QFile::remove("avm_calib_res/cap_rear.jpg");
    QFile::remove("avm_calib_res/calib_factory.param");
    
    int re1 = QFile::copy("avm_qt_app_res/cs11/cap_right.jpg", "avm_calib_res/cap_right.jpg");
    int re2 = QFile::copy("avm_qt_app_res/cs11/cap_front.jpg", "avm_calib_res/cap_front.jpg");
    int re3 = QFile::copy("avm_qt_app_res/cs11/cap_left.jpg", "avm_calib_res/cap_left.jpg");
    int re4 = QFile::copy("avm_qt_app_res/cs11/cap_rear.jpg", "avm_calib_res/cap_rear.jpg");
    int re5 = QFile::copy("avm_qt_app_res/cs11/calib_factory.param", "avm_calib_res/calib_factory.param");
    if (re1 && re2 && re3 && re4 && re5)
    {
        m_label->setText("OK: cp cs11 cap file and param to calib res dir success");
    }
    else
    {
        m_label->setText("ERROR: cp cs11 cap file and param to calib res dir failed");
    }
}


void CalibWidget::copy_cfg_cx_btn_click()
{
    QFile::remove("avm_qt_app_res/config.ini");
    int re = QFile::copy("avm_qt_app_res/cx11/config.ini", "avm_qt_app_res/config.ini");
    if (re)
    {
        m_label->setText("OK: cp cx11 car config file to current ini success");
    }
    else
    {
        m_label->setText("ERROR: cp cx11 car config file to current ini faild");
    }
}


void CalibWidget::copy_cfg_cc_btn_click()
{
    QFile::remove("avm_qt_app_res/config.ini");
    int re = QFile::copy("avm_qt_app_res/cc11/config.ini", "avm_qt_app_res/config.ini");
    if (re)
    {
        m_label->setText("OK: cp cc11 car config file to current ini success");
    }
    else
    {
        m_label->setText("ERROR: cp cc11 car config file to current ini faild");
    }
}


void CalibWidget::copy_cfg_cs_btn_click()
{
    QFile::remove("avm_qt_app_res/config.ini");
    int re = QFile::copy("avm_qt_app_res/cs11/config.ini", "avm_qt_app_res/config.ini");
    if (re)
    {
        m_label->setText("OK: cp cs11 car config file to current ini success\n");
    }
    else
    {
        m_label->setText("ERROR: cp cs11 car config file to current ini faild\n");
    }
}



IniFileWidget::IniFileWidget(QWidget * parent)
    : QWidget(parent)
{
    m_row_num = 70;
    m_misc_row_start = 0;
    m_fc_front_row_start = 11;
    m_fc_rear_row_start = 16;
    m_front_auto_start = 21;
    m_rear_auto_start = 25;
    m_calib_right_start = 29;
    m_calib_front_start = 37;
    m_calib_left_start = 45;
    m_calib_rear_start = 53;
    m_calib_mamual_shift_start = 61;
}


void IniFileWidget::insertMiscData()
{
    int row = m_misc_row_start;

    int car_type = ini_get_car_type();
    tb_widget->item(row+0, 0)->setText("car_type");
    tb_widget->item(row+0, 1)->setText(QString::number(car_type));
    tb_widget->item(row+0, 2)->setText(QString::number(car_type));

    int car_subtype = ini_get_car_subtype();
    tb_widget->item(row+1, 0)->setText("car_subtype");
    tb_widget->item(row+1, 1)->setText(QString::number(car_subtype));
    tb_widget->item(row+1, 2)->setText(QString::number(car_subtype));

    int is_use_bmp = ini_get_device_use_bmp_file();
    tb_widget->item(row+2, 0)->setText("is_use_bmp display");
    tb_widget->item(row+2, 1)->setText(QString::number(is_use_bmp));
    tb_widget->item(row+2, 2)->setText(QString::number(is_use_bmp));

    int skip_fas = ini_get_calib_skip_fas();
    tb_widget->item(row+3, 0)->setText("skip_fas");
    tb_widget->item(row+3, 1)->setText(QString::number(skip_fas));
    tb_widget->item(row+3, 2)->setText(QString::number(skip_fas));

    int skip_correct_pripoint = ini_get_calib_skip_cor_pripoint();
    tb_widget->item(row+4, 0)->setText("skip_correct_pripoint");
    tb_widget->item(row+4, 1)->setText(QString::number(skip_correct_pripoint));
    tb_widget->item(row+4, 2)->setText(QString::number(skip_correct_pripoint));

    int is_image_calib = ini_get_calib_use_img();
    tb_widget->item(row+5, 0)->setText("calib use image");
    tb_widget->item(row+5, 1)->setText(QString::number(is_image_calib));
    tb_widget->item(row+5, 2)->setText(QString::number(is_image_calib));

    int is_calib_use_qt_image = ini_get_calib_use_qt_cam_image();
    tb_widget->item(row+6, 0)->setText("calib use qt image");
    tb_widget->item(row+6, 1)->setText(QString::number(is_calib_use_qt_image));
    tb_widget->item(row+6, 2)->setText(QString::number(is_calib_use_qt_image)); 

    float bottom_det = ini_get_bottom_det();
    int int_det = (int)(bottom_det * 100.0);
    tb_widget->item(row+7, 0)->setText("bottom_det(cm)");
    tb_widget->item(row+7, 1)->setText(QString::number(int_det));
    tb_widget->item(row+7, 2)->setText(QString::number(int_det));  

    float car_det = ini_get_car_model_det();
    int int_car_det = (int)(car_det * 100.0);
    tb_widget->item(row+8, 0)->setText("car_det(cm)");
    tb_widget->item(row+8, 1)->setText(QString::number(int_car_det));
    tb_widget->item(row+8, 2)->setText(QString::number(int_car_det));  

    float block_f_det = ini_get_assist_line_2d_front_det();
    int int_block_f_det = (int)(block_f_det * 100.0);
    tb_widget->item(row+9, 0)->setText("block_front(cm)(need recalib)");
    tb_widget->item(row+9, 1)->setText(QString::number(int_block_f_det));
    tb_widget->item(row+9, 2)->setText(QString::number(int_block_f_det));

    float block_r_det = ini_get_assist_line_2d_rear_det();
    int int_block_r_det = (int)(block_r_det * 100.0);
    tb_widget->item(row+10, 0)->setText("block_rear(cm)(need recalib)");
    tb_widget->item(row+10, 1)->setText(QString::number(int_block_r_det));
    tb_widget->item(row+10, 2)->setText(QString::number(int_block_r_det));     
}


void IniFileWidget::insertFcFrontData()
{
    int row = m_fc_front_row_start;
    int vaild, lt_x, lt_y, rb_x, rb_y;
    ini_get_d2_fc_front(&vaild, &lt_x, &lt_y, &rb_x, &rb_y);
    tb_widget->item(row+0, 0)->setText("d2_fc_front_vaild");
    tb_widget->item(row+0, 1)->setText(QString::number(vaild));
    tb_widget->item(row+0, 2)->setText(QString::number(vaild));

    tb_widget->item(row+1, 0)->setText("d2_fc_front_lt_x");
    tb_widget->item(row+1, 1)->setText(QString::number(lt_x));
    tb_widget->item(row+1, 2)->setText(QString::number(lt_x));

    tb_widget->item(row+2, 0)->setText("d2_fc_front_lt_y");
    tb_widget->item(row+2, 1)->setText(QString::number(lt_y));
    tb_widget->item(row+2, 2)->setText(QString::number(lt_y));

    tb_widget->item(row+3, 0)->setText("d2_fc_front_rb_x");
    tb_widget->item(row+3, 1)->setText(QString::number(rb_x));
    tb_widget->item(row+3, 2)->setText(QString::number(rb_x));

    tb_widget->item(row+4, 0)->setText("d2_fc_front_rb_y");
    tb_widget->item(row+4, 1)->setText(QString::number(rb_y));
    tb_widget->item(row+4, 2)->setText(QString::number(rb_y));
}


void IniFileWidget::insertFcRearData()
{
    int row = m_fc_rear_row_start;
    int vaild, lt_x, lt_y, rb_x, rb_y;
    ini_get_d2_fc_rear(&vaild, &lt_x, &lt_y, &rb_x, &rb_y);
    tb_widget->item(row+0, 0)->setText("d2_fc_rear_vaild");
    tb_widget->item(row+0, 1)->setText(QString::number(vaild));
    tb_widget->item(row+0, 2)->setText(QString::number(vaild));

    tb_widget->item(row+1, 0)->setText("d2_fc_rear_lt_x");
    tb_widget->item(row+1, 1)->setText(QString::number(lt_x));
    tb_widget->item(row+1, 2)->setText(QString::number(lt_x));

    tb_widget->item(row+2, 0)->setText("d2_fc_rear_lt_y");
    tb_widget->item(row+2, 1)->setText(QString::number(lt_y));
    tb_widget->item(row+2, 2)->setText(QString::number(lt_y));

    tb_widget->item(row+3, 0)->setText("d2_fc_rear_rb_x");
    tb_widget->item(row+3, 1)->setText(QString::number(rb_x));
    tb_widget->item(row+3, 2)->setText(QString::number(rb_x));

    tb_widget->item(row+4, 0)->setText("d2_fc_rear_rb_y");
    tb_widget->item(row+4, 1)->setText(QString::number(rb_y));
    tb_widget->item(row+4, 2)->setText(QString::number(rb_y));
}

void IniFileWidget::insertFrontAuto()
{
    int row = m_front_auto_start;
    int f_lt_y, f_rb_y, fc_lt_y, fc_rb_y;
    ini_get_d2_fc_front_auto(&fc_lt_y, &fc_rb_y);
    ini_get_d2_f_front_auto(&f_lt_y, &f_rb_y);

    tb_widget->item(row+0, 0)->setText("d2_fc_front_auto_lt_y");
    tb_widget->item(row+0, 1)->setText(QString::number(fc_lt_y));
    tb_widget->item(row+0, 2)->setText(QString::number(fc_lt_y));

    tb_widget->item(row+1, 0)->setText("d2_fc_front_auto_rb_y");
    tb_widget->item(row+1, 1)->setText(QString::number(fc_rb_y));
    tb_widget->item(row+1, 2)->setText(QString::number(fc_rb_y));

    tb_widget->item(row+2, 0)->setText("d2_f_front_auto_lt_y");
    tb_widget->item(row+2, 1)->setText(QString::number(f_lt_y));
    tb_widget->item(row+2, 2)->setText(QString::number(f_lt_y));

    tb_widget->item(row+3, 0)->setText("d2_f_front_auto_rb_y");
    tb_widget->item(row+3, 1)->setText(QString::number(f_rb_y));
    tb_widget->item(row+3, 2)->setText(QString::number(f_rb_y));
}

void IniFileWidget::insertRearAuto()
{
    int row = m_rear_auto_start;
    int f_lt_y, f_rb_y, fc_lt_y, fc_rb_y;
    ini_get_d2_fc_rear_auto(&fc_lt_y, &fc_rb_y);
    ini_get_d2_f_rear_auto(&f_lt_y, &f_rb_y);

    tb_widget->item(row+0, 0)->setText("d2_fc_rear_auto_lt_y");
    tb_widget->item(row+0, 1)->setText(QString::number(fc_lt_y));
    tb_widget->item(row+0, 2)->setText(QString::number(fc_lt_y));

    tb_widget->item(row+1, 0)->setText("d2_fc_rear_auto_rb_y");
    tb_widget->item(row+1, 1)->setText(QString::number(fc_rb_y));
    tb_widget->item(row+1, 2)->setText(QString::number(fc_rb_y));

    tb_widget->item(row+2, 0)->setText("d2_f_rear_auto_lt_y");
    tb_widget->item(row+2, 1)->setText(QString::number(f_lt_y));
    tb_widget->item(row+2, 2)->setText(QString::number(f_lt_y));

    tb_widget->item(row+3, 0)->setText("d2_f_rear_auto_rb_y");
    tb_widget->item(row+3, 1)->setText(QString::number(f_rb_y));
    tb_widget->item(row+3, 2)->setText(QString::number(f_rb_y));
}



void IniFileWidget::insertCalibRight()
{
    int row = m_calib_right_start;
    int row_begin, row_end, col_begin_l, col_end_l, col_begin_m, col_end_m, col_begin_r, col_end_r;
    ini_get_geom_file_param(
        0,
        &row_begin, &row_end, 
        &col_begin_l, &col_end_l, 
        &col_begin_m, &col_end_m, 
        &col_begin_r, &col_end_r);
    tb_widget->item(row+0, 0)->setText("calib_right_row_begin");
    tb_widget->item(row+0, 1)->setText(QString::number(row_begin));
    tb_widget->item(row+0, 2)->setText(QString::number(row_begin));

    tb_widget->item(row+1, 0)->setText("calib_right_row_end");
    tb_widget->item(row+1, 1)->setText(QString::number(row_end));
    tb_widget->item(row+1, 2)->setText(QString::number(row_end));

    tb_widget->item(row+2, 0)->setText("calib_right_col_begin_l");
    tb_widget->item(row+2, 1)->setText(QString::number(col_begin_l));
    tb_widget->item(row+2, 2)->setText(QString::number(col_begin_l));

    tb_widget->item(row+3, 0)->setText("calib_right_col_end_l");
    tb_widget->item(row+3, 1)->setText(QString::number(col_end_l));
    tb_widget->item(row+3, 2)->setText(QString::number(col_end_l));

    tb_widget->item(row+4, 0)->setText("calib_right_col_begin_m");
    tb_widget->item(row+4, 1)->setText(QString::number(col_begin_m));
    tb_widget->item(row+4, 2)->setText(QString::number(col_begin_m));

    tb_widget->item(row+5, 0)->setText("calib_right_col_end_m");
    tb_widget->item(row+5, 1)->setText(QString::number(col_end_m));
    tb_widget->item(row+5, 2)->setText(QString::number(col_end_m));

    tb_widget->item(row+6, 0)->setText("calib_right_col_begin_r");
    tb_widget->item(row+6, 1)->setText(QString::number(col_begin_r));
    tb_widget->item(row+6, 2)->setText(QString::number(col_begin_r));    

    tb_widget->item(row+7, 0)->setText("calib_right_col_end_r");
    tb_widget->item(row+7, 1)->setText(QString::number(col_end_r));
    tb_widget->item(row+7, 2)->setText(QString::number(col_end_r));     
}


void IniFileWidget::insertCalibFront()
{
    int row = m_calib_front_start;
    int row_begin, row_end, col_begin_l, col_end_l, col_begin_m, col_end_m, col_begin_r, col_end_r;
    ini_get_geom_file_param(
        1,
        &row_begin, &row_end, 
        &col_begin_l, &col_end_l, 
        &col_begin_m, &col_end_m, 
        &col_begin_r, &col_end_r);
    tb_widget->item(row+0, 0)->setText("calib_front_row_begin");
    tb_widget->item(row+0, 1)->setText(QString::number(row_begin));
    tb_widget->item(row+0, 2)->setText(QString::number(row_begin));

    tb_widget->item(row+1, 0)->setText("calib_front_row_end");
    tb_widget->item(row+1, 1)->setText(QString::number(row_end));
    tb_widget->item(row+1, 2)->setText(QString::number(row_end));

    tb_widget->item(row+2, 0)->setText("calib_front_col_begin_l");
    tb_widget->item(row+2, 1)->setText(QString::number(col_begin_l));
    tb_widget->item(row+2, 2)->setText(QString::number(col_begin_l));

    tb_widget->item(row+3, 0)->setText("calib_front_col_end_l");
    tb_widget->item(row+3, 1)->setText(QString::number(col_end_l));
    tb_widget->item(row+3, 2)->setText(QString::number(col_end_l));

    tb_widget->item(row+4, 0)->setText("calib_front_col_begin_m");
    tb_widget->item(row+4, 1)->setText(QString::number(col_begin_m));
    tb_widget->item(row+4, 2)->setText(QString::number(col_begin_m));

    tb_widget->item(row+5, 0)->setText("calib_front_col_end_m");
    tb_widget->item(row+5, 1)->setText(QString::number(col_end_m));
    tb_widget->item(row+5, 2)->setText(QString::number(col_end_m));

    tb_widget->item(row+6, 0)->setText("calib_front_col_begin_r");
    tb_widget->item(row+6, 1)->setText(QString::number(col_begin_r));
    tb_widget->item(row+6, 2)->setText(QString::number(col_begin_r));    

    tb_widget->item(row+7, 0)->setText("calib_front_col_end_r");
    tb_widget->item(row+7, 1)->setText(QString::number(col_end_r));
    tb_widget->item(row+7, 2)->setText(QString::number(col_end_r));  
}

void IniFileWidget::insertCalibLeft()
{
    int row = m_calib_left_start;
    int row_begin, row_end, col_begin_l, col_end_l, col_begin_m, col_end_m, col_begin_r, col_end_r;
    ini_get_geom_file_param(
        2,
        &row_begin, &row_end, 
        &col_begin_l, &col_end_l, 
        &col_begin_m, &col_end_m, 
        &col_begin_r, &col_end_r);
    tb_widget->item(row+0, 0)->setText("calib_left_row_begin");
    tb_widget->item(row+0, 1)->setText(QString::number(row_begin));
    tb_widget->item(row+0, 2)->setText(QString::number(row_begin));

    tb_widget->item(row+1, 0)->setText("calib_left_row_end");
    tb_widget->item(row+1, 1)->setText(QString::number(row_end));
    tb_widget->item(row+1, 2)->setText(QString::number(row_end));

    tb_widget->item(row+2, 0)->setText("calib_left_col_begin_l");
    tb_widget->item(row+2, 1)->setText(QString::number(col_begin_l));
    tb_widget->item(row+2, 2)->setText(QString::number(col_begin_l));

    tb_widget->item(row+3, 0)->setText("calib_left_col_end_l");
    tb_widget->item(row+3, 1)->setText(QString::number(col_end_l));
    tb_widget->item(row+3, 2)->setText(QString::number(col_end_l));

    tb_widget->item(row+4, 0)->setText("calib_left_col_begin_m");
    tb_widget->item(row+4, 1)->setText(QString::number(col_begin_m));
    tb_widget->item(row+4, 2)->setText(QString::number(col_begin_m));

    tb_widget->item(row+5, 0)->setText("calib_left_col_end_m");
    tb_widget->item(row+5, 1)->setText(QString::number(col_end_m));
    tb_widget->item(row+5, 2)->setText(QString::number(col_end_m));

    tb_widget->item(row+6, 0)->setText("calib_left_col_begin_r");
    tb_widget->item(row+6, 1)->setText(QString::number(col_begin_r));
    tb_widget->item(row+6, 2)->setText(QString::number(col_begin_r));    

    tb_widget->item(row+7, 0)->setText("calib_left_col_end_r");
    tb_widget->item(row+7, 1)->setText(QString::number(col_end_r));
    tb_widget->item(row+7, 2)->setText(QString::number(col_end_r));  
}

void IniFileWidget::insertCalibRear()
{
    int row = m_calib_rear_start;
    int row_begin, row_end, col_begin_l, col_end_l, col_begin_m, col_end_m, col_begin_r, col_end_r;
    ini_get_geom_file_param(
        3,
        &row_begin, &row_end, 
        &col_begin_l, &col_end_l, 
        &col_begin_m, &col_end_m, 
        &col_begin_r, &col_end_r);
    tb_widget->item(row+0, 0)->setText("calib_rear_row_begin");
    tb_widget->item(row+0, 1)->setText(QString::number(row_begin));
    tb_widget->item(row+0, 2)->setText(QString::number(row_begin));

    tb_widget->item(row+1, 0)->setText("calib_rear_row_end");
    tb_widget->item(row+1, 1)->setText(QString::number(row_end));
    tb_widget->item(row+1, 2)->setText(QString::number(row_end));

    tb_widget->item(row+2, 0)->setText("calib_rear_col_begin_l");
    tb_widget->item(row+2, 1)->setText(QString::number(col_begin_l));
    tb_widget->item(row+2, 2)->setText(QString::number(col_begin_l));

    tb_widget->item(row+3, 0)->setText("calib_rear_col_end_l");
    tb_widget->item(row+3, 1)->setText(QString::number(col_end_l));
    tb_widget->item(row+3, 2)->setText(QString::number(col_end_l));

    tb_widget->item(row+4, 0)->setText("calib_rear_col_begin_m");
    tb_widget->item(row+4, 1)->setText(QString::number(col_begin_m));
    tb_widget->item(row+4, 2)->setText(QString::number(col_begin_m));

    tb_widget->item(row+5, 0)->setText("calib_rear_col_end_m");
    tb_widget->item(row+5, 1)->setText(QString::number(col_end_m));
    tb_widget->item(row+5, 2)->setText(QString::number(col_end_m));

    tb_widget->item(row+6, 0)->setText("calib_rear_col_begin_r");
    tb_widget->item(row+6, 1)->setText(QString::number(col_begin_r));
    tb_widget->item(row+6, 2)->setText(QString::number(col_begin_r));    

    tb_widget->item(row+7, 0)->setText("calib_rear_col_end_r");
    tb_widget->item(row+7, 1)->setText(QString::number(col_end_r));
    tb_widget->item(row+7, 2)->setText(QString::number(col_end_r)); 
}


void IniFileWidget::insertCalibMamualShift()
{
    int i;
    int row = m_calib_mamual_shift_start;
    const char *v_str[] = {
        "calib_right_shift_v",
        "calib_front_shift_v",
        "calib_left_shift_v",
        "calib_rear_shift_v",
    };
    const char *h_str[] = {
        "calib_right_shift_h",
        "calib_front_shift_h",
        "calib_left_shift_h",
        "calib_rear_shift_h",
    };    
    for (i = 0; i < 4; i++)
    {
        int shift_v = ini_get_calib_mamual_shift_v(i);
        tb_widget->item(row+2*i, 0)->setText(v_str[i]);
        tb_widget->item(row+2*i, 1)->setText(QString::number(shift_v));
        tb_widget->item(row+2*i, 2)->setText(QString::number(shift_v));  

        int shift_h = ini_get_calib_mamual_shift_h(i);
        tb_widget->item(row+2*i+1, 0)->setText(h_str[i]);
        tb_widget->item(row+2*i+1, 1)->setText(QString::number(shift_h));
        tb_widget->item(row+2*i+1, 2)->setText(QString::number(shift_h));           
    }

    //for 03+ car_type
    int shift_v_for_subtype = ini_get_shift_v_for_subtype(0);
    tb_widget->item(row+8, 0)->setText("front_shift_v_for_subtype");
    tb_widget->item(row+8, 1)->setText(QString::number(shift_v_for_subtype));
    tb_widget->item(row+8, 2)->setText(QString::number(shift_v_for_subtype));  


}



void IniFileWidget::saveMiscData()
{
    int row = m_misc_row_start;

    int car_subtype = tb_widget->item(row+1, 2)->text().toInt();
    ini_set_car_subtype(car_subtype);
    
    int is_use_bmp = tb_widget->item(row+2, 2)->text().toInt();
    ini_set_device_use_bmp_file(is_use_bmp);

    int skip_fas = tb_widget->item(row+3, 2)->text().toInt();
    ini_set_calib_skip_fas(skip_fas);

    int skip_cor_pripoint = tb_widget->item(row+4, 2)->text().toInt();
    ini_set_calib_skip_cor_pripoint(skip_cor_pripoint);

    int is_calib_use_image = tb_widget->item(row+5, 2)->text().toInt();
    ini_set_calib_use_img(is_calib_use_image);
    
    int is_calib_use_qt_image = tb_widget->item(row+6, 2)->text().toInt();
    ini_set_calib_use_qt_cam_image(is_calib_use_qt_image);

    int int_det = tb_widget->item(row+7, 2)->text().toInt();
    float det = int_det / 100.0;
    ini_set_bottom_det(det);

    int int_car_det = tb_widget->item(row+8, 2)->text().toInt();
    float car_det = int_car_det / 100.0;
    ini_set_car_model_det(car_det);  

    int int_block_f_det = tb_widget->item(row+9, 2)->text().toInt();
    float block_f_det = int_block_f_det / 100.0;
    ini_set_assist_line_2d_front_det(block_f_det);

    int int_block_r_det = tb_widget->item(row+10, 2)->text().toInt();
    float block_r_det = int_block_r_det / 100.0;
    ini_set_assist_line_2d_rear_det(block_r_det);
}

void IniFileWidget::saveFcFrontData()
{
    int row = m_fc_front_row_start;
    int vaild, lt_x, lt_y, rb_x, rb_y;
    vaild = tb_widget->item(row+0, 2)->text().toInt();
    lt_x  = tb_widget->item(row+1, 2)->text().toInt();
    lt_y  = tb_widget->item(row+2, 2)->text().toInt();
    rb_x  = tb_widget->item(row+3, 2)->text().toInt();
    rb_y  = tb_widget->item(row+4, 2)->text().toInt();
    ini_set_d2_fc_front(vaild, lt_x, lt_y, rb_x, rb_y);
}

void IniFileWidget::saveFcRearData()
{
    int row = m_fc_rear_row_start;
    int vaild, lt_x, lt_y, rb_x, rb_y;
    vaild = tb_widget->item(row+0, 2)->text().toInt();
    lt_x  = tb_widget->item(row+1, 2)->text().toInt();
    lt_y  = tb_widget->item(row+2, 2)->text().toInt();
    rb_x  = tb_widget->item(row+3, 2)->text().toInt();
    rb_y  = tb_widget->item(row+4, 2)->text().toInt();
    ini_set_d2_fc_rear(vaild, lt_x, lt_y, rb_x, rb_y);
}

void IniFileWidget::saveFrontAuto()
{
    int row = m_front_auto_start;
    int f_lt_y, f_rb_y, fc_lt_y, fc_rb_y;
    f_lt_y  = tb_widget->item(row+0, 2)->text().toInt();
    f_rb_y  = tb_widget->item(row+1, 2)->text().toInt();
    fc_lt_y = tb_widget->item(row+2, 2)->text().toInt();
    fc_rb_y = tb_widget->item(row+3, 2)->text().toInt();
    ini_set_d2_fc_front_auto(f_lt_y, f_rb_y);
    ini_set_d2_f_front_auto(fc_lt_y, fc_rb_y);
}

void IniFileWidget::saveRearAuto()
{
    int row = m_rear_auto_start;
    int f_lt_y, f_rb_y, fc_lt_y, fc_rb_y;
    f_lt_y  = tb_widget->item(row+0, 2)->text().toInt();
    f_rb_y  = tb_widget->item(row+1, 2)->text().toInt();
    fc_lt_y = tb_widget->item(row+2, 2)->text().toInt();
    fc_rb_y = tb_widget->item(row+3, 2)->text().toInt();
    ini_set_d2_fc_rear_auto(f_lt_y, f_rb_y);
    ini_set_d2_f_rear_auto(fc_lt_y, fc_rb_y);
}



void IniFileWidget::saveCalibRight()
{
    int row = m_calib_right_start;
    int row_begin, row_end, col_begin_l, col_end_l, col_begin_m, col_end_m, col_begin_r, col_end_r;
    row_begin    = tb_widget->item(row+0, 2)->text().toInt();
    row_end      = tb_widget->item(row+1, 2)->text().toInt();
    col_begin_l  = tb_widget->item(row+2, 2)->text().toInt();
    col_end_l    = tb_widget->item(row+3, 2)->text().toInt();
    col_begin_m  = tb_widget->item(row+4, 2)->text().toInt();
    col_end_m    = tb_widget->item(row+5, 2)->text().toInt();    
    col_begin_r  = tb_widget->item(row+6, 2)->text().toInt();   
    col_end_r    = tb_widget->item(row+7, 2)->text().toInt(); 
    ini_set_geom_file_param(0, row_begin, row_end, col_begin_l, col_end_l, col_begin_m, col_end_m, col_begin_r, col_end_r);
}

void IniFileWidget::saveCalibFront()
{
    int row = m_calib_front_start;
    int row_begin, row_end, col_begin_l, col_end_l, col_begin_m, col_end_m, col_begin_r, col_end_r;
    row_begin    = tb_widget->item(row+0, 2)->text().toInt();
    row_end      = tb_widget->item(row+1, 2)->text().toInt();
    col_begin_l  = tb_widget->item(row+2, 2)->text().toInt();
    col_end_l    = tb_widget->item(row+3, 2)->text().toInt();
    col_begin_m  = tb_widget->item(row+4, 2)->text().toInt();
    col_end_m    = tb_widget->item(row+5, 2)->text().toInt();    
    col_begin_r  = tb_widget->item(row+6, 2)->text().toInt();   
    col_end_r    = tb_widget->item(row+7, 2)->text().toInt(); 
    ini_set_geom_file_param(1, row_begin, row_end, col_begin_l, col_end_l, col_begin_m, col_end_m, col_begin_r, col_end_r);
}

void IniFileWidget::saveCalibLeft()
{
    int row = m_calib_left_start;
    int row_begin, row_end, col_begin_l, col_end_l, col_begin_m, col_end_m, col_begin_r, col_end_r;
    row_begin    = tb_widget->item(row+0, 2)->text().toInt();
    row_end      = tb_widget->item(row+1, 2)->text().toInt();
    col_begin_l  = tb_widget->item(row+2, 2)->text().toInt();
    col_end_l    = tb_widget->item(row+3, 2)->text().toInt();
    col_begin_m  = tb_widget->item(row+4, 2)->text().toInt();
    col_end_m    = tb_widget->item(row+5, 2)->text().toInt();    
    col_begin_r  = tb_widget->item(row+6, 2)->text().toInt();   
    col_end_r    = tb_widget->item(row+7, 2)->text().toInt(); 
    ini_set_geom_file_param(2, row_begin, row_end, col_begin_l, col_end_l, col_begin_m, col_end_m, col_begin_r, col_end_r);
}

void IniFileWidget::saveCalibRear()
{
    int row = m_calib_rear_start;
    int row_begin, row_end, col_begin_l, col_end_l, col_begin_m, col_end_m, col_begin_r, col_end_r;
    row_begin    = tb_widget->item(row+0, 2)->text().toInt();
    row_end      = tb_widget->item(row+1, 2)->text().toInt();
    col_begin_l  = tb_widget->item(row+2, 2)->text().toInt();
    col_end_l    = tb_widget->item(row+3, 2)->text().toInt();
    col_begin_m  = tb_widget->item(row+4, 2)->text().toInt();
    col_end_m    = tb_widget->item(row+5, 2)->text().toInt();    
    col_begin_r  = tb_widget->item(row+6, 2)->text().toInt();   
    col_end_r    = tb_widget->item(row+7, 2)->text().toInt(); 
    ini_set_geom_file_param(3, row_begin, row_end, col_begin_l, col_end_l, col_begin_m, col_end_m, col_begin_r, col_end_r);
}

void IniFileWidget::saveCalibMamualShift()
{
    int i;
    int row = m_calib_mamual_shift_start;

    for (i = 0; i < 4; i++)
    {
        int calib_mamual_shift_v = tb_widget->item(row + 2*i, 2)->text().toInt();
        ini_set_calib_mamual_shift_v(i, calib_mamual_shift_v);

        int calib_mamual_shift_h = tb_widget->item(row + 2*i + 1, 2)->text().toInt();
        ini_set_calib_mamual_shift_h(i, calib_mamual_shift_h);        
    }

    int front_shift_v_for_subtype = tb_widget->item(row + 8, 2)->text().toInt();
    ini_set_shift_v_for_subtype(0, front_shift_v_for_subtype);

}


void IniFileWidget::layout()
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    this->setLayout(layout);

    // table
    int row_num = m_row_num;
    tb_widget = new QTableWidget(this);
    //tb_widget->verticalHeader()->setVisible(false);
    tb_widget->setColumnCount(3);
    tb_widget->setRowCount(row_num);
    tb_widget->setHorizontalHeaderLabels(QStringList() << "name" << "cur_value" << "new value");
    tb_widget->setColumnWidth(0, this->width()/3-28);
    tb_widget->setColumnWidth(1, this->width()/3-28);
    tb_widget->setColumnWidth(2, this->width()/3-28);

    std::set<int> gray;
    for (int i = m_fc_front_row_start; i < m_fc_rear_row_start; i++)
    {
        gray.insert(i);
    }
    for (int i = m_calib_right_start; i < m_calib_front_start; i++)
    {
        gray.insert(i);
    }    
    for (int i = m_calib_left_start; i < m_calib_rear_start; i++)
    {
        gray.insert(i);
    }
    for (int i = 0; i < row_num; i++)
    {
        QTableWidgetItem *item = new QTableWidgetItem();
        if (gray.count(i) > 0)
        {
            item->setBackground(QBrush(QColor(Qt::lightGray)));
        }
        item->setTextColor(QColor(Qt::red));
        item->setFlags(0);
        tb_widget->setItem(i, 0, item); 

        QTableWidgetItem *item2 = new QTableWidgetItem();
        if (gray.count(i) > 0)
        {
            item2->setBackground(QBrush(QColor(Qt::lightGray)));
        }
        item2->setTextColor(QColor(Qt::red));
        item2->setFlags(0);
        tb_widget->setItem(i, 1, item2);        
    }
    for(int i = 0; i < row_num; i++)
    {
        // car_type can't be change
        if (i < m_misc_row_start + 1)
        {
            QTableWidgetItem *item = new QTableWidgetItem();
            item->setFlags(0);
            tb_widget->setItem(i, 2, item);
        }
        else
        {
            QTableWidgetItem *item = new QTableWidgetItem();
            item->setFlags(item->flags());
            tb_widget->setItem(i, 2, item);
        }
    }
    insertMiscData();
    insertFcFrontData();
    insertFcRearData();
    insertFrontAuto();
    insertRearAuto();
    insertCalibRight();
    insertCalibFront();
    insertCalibLeft();
    insertCalibRear();
    insertCalibMamualShift();

    // button
    QHBoxLayout *bottom_layout = new QHBoxLayout();
    m_add1 = new QPushButton(this);
    m_add1->setText("+1");
    m_dec1 = new QPushButton(this);
    m_dec1->setText("-1");
    m_add10 = new QPushButton(this);
    m_add10->setText("+10");
    m_dec10 = new QPushButton(this);
    m_dec10->setText("-10");
    m_reset = new QPushButton(this);
    m_reset->setText("reset");   
    m_save = new QPushButton(this);
    m_save->setText("save");      
    bottom_layout->addWidget(m_add1);
    bottom_layout->addWidget(m_dec1);
    bottom_layout->addWidget(m_add10);
    bottom_layout->addWidget(m_dec10);
    bottom_layout->addWidget(m_reset);
    bottom_layout->addWidget(m_save);
    QHBoxLayout *top_layout = new QHBoxLayout();
    top_layout->addWidget(tb_widget);
    layout->addLayout(top_layout);
    layout->addLayout(bottom_layout);

    connect(m_add1, SIGNAL(released()), this, SLOT(add1_btn_click()));
    connect(m_dec1, SIGNAL(released()), this, SLOT(dec1_btn_click()));
    connect(m_add10, SIGNAL(released()), this, SLOT(add10_btn_click()));
    connect(m_dec10, SIGNAL(released()), this, SLOT(dec10_btn_click()));
    connect(m_reset, SIGNAL(released()), this, SLOT(reset_btn_click()));
    connect(m_save, SIGNAL(released()), this, SLOT(save_btn_click()));
}

void IniFileWidget::numCalc(int det)
{
    QTableWidgetItem *item = tb_widget->currentItem();
    if (item != NULL)
    {
        int num = item->text().toInt();
        num += det;
        item->setText(QString::number(num));

        QTableWidgetItem *org_item = tb_widget->item(item->row(), item->column()-1);
        int org_num = org_item->text().toInt();
        if (num != org_num)
        {
            item->setBackground(QBrush(QColor(Qt::red)));
        }
        else
        {
            item->setBackground(QBrush(QColor(Qt::white)));
        }
    }
}


void IniFileWidget::add1_btn_click()
{
    numCalc(1);
}


void IniFileWidget::dec1_btn_click()
{
    numCalc(-1);
}


void IniFileWidget::add10_btn_click()
{
    numCalc(10);
}


void IniFileWidget::dec10_btn_click()
{
    numCalc(-10);
}


void IniFileWidget::reset_btn_click()
{
    for (int i = 0; i < m_row_num; i++)
    {
        QTableWidgetItem *org_item = tb_widget->item(i, 1);
        QTableWidgetItem *item = tb_widget->item(i, 2);
        item->setText(org_item->text());
        item->setBackground(QBrush(QColor(Qt::white)));
    }
}


void IniFileWidget::save_btn_click()
{
    saveMiscData();
    saveFcFrontData();
    saveFcRearData();
    saveFrontAuto();
    saveRearAuto();
    saveCalibRight();
    saveCalibFront();
    saveCalibLeft();
    saveCalibRear();
    saveCalibMamualShift();

    // at last, copy file to car config.ini
    int car_type = ini_get_car_type();
    if (1 == car_type)
    {
        QFile::remove("avm_qt_app_res/cx11/config.ini");
        QFile::copy("avm_qt_app_res/config.ini", "avm_qt_app_res/cx11/config.ini");
    }
    else if (2 == car_type)
    {
        QFile::remove("avm_qt_app_res/cc11/config.ini");
        QFile::copy("avm_qt_app_res/config.ini", "avm_qt_app_res/cc11/config.ini");
    }
    else if (3 == car_type)
    {
        QFile::remove("avm_qt_app_res/cs11/config.ini");
        QFile::copy("avm_qt_app_res/config.ini", "avm_qt_app_res/cs11/config.ini");
    }
}


OthersWidget::OthersWidget(QWidget *parent)
    : QWidget(parent)
{
}


OthersWidget::~OthersWidget()
{
}


void OthersWidget::layout()
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setSizeConstraint(QLayout::SetMinimumSize);
    layout->setSpacing(15);
    this->setLayout(layout);

    m_fps = new QPushButton(this);
    m_fps->setText("show fps");
    m_version = new QPushButton(this);
    m_version->setText("show version");
    m_caminfo = new QPushButton(this);
    m_caminfo->setText("show caminfo");
    m_ps = new QPushButton(this);
    m_ps->setText("ps -aux");
    m_reboot = new QPushButton(this);
    m_reboot->setText("reboot !!!!!");
    m_df = new QPushButton(this);
    m_df->setText("df");
    m_free = new QPushButton(this);
    m_free->setText("free");

    QHBoxLayout *bt_layout_1 = new QHBoxLayout();
    bt_layout_1->addWidget(m_fps);
    bt_layout_1->addWidget(m_version);
    bt_layout_1->addWidget(m_caminfo);
    bt_layout_1->addWidget(m_ps);
    layout->addLayout(bt_layout_1);

    QHBoxLayout *bt_layout_2 = new QHBoxLayout();
    bt_layout_2->addWidget(m_reboot);
    bt_layout_2->addWidget(m_df);
    bt_layout_2->addWidget(m_free);
    layout->addLayout(bt_layout_2);

    m_text = new QLabel(this);
    m_text->setObjectName("m_text_other");
    m_text->setStyleSheet("#m_text_other{font-size:12px;}");
    m_text->setFixedWidth(this->width());
    m_text->setAlignment(Qt::AlignTop);
    layout->addWidget(m_text);

    connect(m_fps, SIGNAL(released()), this, SLOT(fps_btn_clicked()));
    connect(m_caminfo, SIGNAL(released()), this, SLOT(caminfo_btn_clicked()));
    connect(m_version, SIGNAL(released()), this, SLOT(version_btn_clicked()));
    connect(m_ps, SIGNAL(released()), this, SLOT(ps_btn_clicked()));
    connect(m_reboot, SIGNAL(released()), this, SLOT(reboot_btn_clicked()));
    connect(m_df, SIGNAL(released()), this, SLOT(df_btn_clicked()));
    connect(m_free, SIGNAL(released()), this, SLOT(free_btn_clicked()));
}


void OthersWidget::fps_btn_clicked()
{
    int fps = SurroundWidget::getFps();
    m_text->setText(QString("fps is:") + QString::number(fps));
}

void OthersWidget::caminfo_btn_clicked()
{
    int i = 0;
    unsigned int link_status = 0x0;
    unsigned int sync_status = 0x0;
    int cam_arr[4] = {0};   // rear front right left
    QString line;

    line.append(custstr_get_errinfo_title());
    line.append("\n");

    videocap_get_cam_link_status(&link_status);
    videocap_get_cam_sync_status(&sync_status);

    // check max9286 read or write error
    if (link_status & 0xF0000)
    {
        line.append(custstr_get_err_dyn_info(3, 3, 3, 3));
        m_text->setText(line);
        return;
    }

    // check disconnect
    if (link_status != 0x0F)
    {
        for (i = 0; i < 4; i++)
        {
            if (0x00 == (link_status & (0x01 << i)))
            {
                cam_arr[i] = 1;
            }
        }
        line.append(custstr_get_err_dyn_info(cam_arr[1], cam_arr[0], 
            cam_arr[3], cam_arr[2]));
        m_text->setText(line);
        return;  // return, and not check if camera get vsync
    }
    
    // check bad
    if (sync_status != 0x0F)
    {
        for (i = 0; i < 4; i++)
        {
            if (0x00 == (sync_status & (0x01 << i)))
            {
                cam_arr[i] = 2;
            }
        }
    }
    
    line.append(custstr_get_err_dyn_info(cam_arr[1], cam_arr[0], 
        cam_arr[3], cam_arr[2])); 
    m_text->setText(line);
}


void OthersWidget::version_btn_clicked()
{
    // read 'version.txt'
    QFile file("version.txt");
    QString line;
    if (file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QTextStream stream(&file);
        while (!stream.atEnd())
        {
            line.append(stream.readLine()+"\n");
        }
    }
    else
    {
        line.append("version.txt is not exist\n");
    }
    file.close();

    // read 'vpsw_version.txt'
    QFile file2("vpsw_version.txt");
    if (file2.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QTextStream stream(&file2);
        while (!stream.atEnd())
        {
            line.append(stream.readLine()+"\n");
        }
    }
    else
    {
        line.append("vpsw_version.txt is not exist\n");
    }
    file2.close();    

    // 
    m_text->setText(line);
}


void OthersWidget::ps_btn_clicked()
{
    this->common_cmd(QString("ps -aux"));
}


void OthersWidget::reboot_btn_clicked()
{
#if GLOBAL_RUN_ENV_DESKTOP == 1
    m_text->setText("reboot is not support in pc env");
#else
    QProcess reboot;
    reboot.start("reboot");      
#endif
}


void OthersWidget::df_btn_clicked()
{
    this->common_cmd(QString("df"));
}


void OthersWidget::free_btn_clicked()
{
    this->common_cmd(QString("free"));
}


void OthersWidget::common_cmd(QString str)
{
    QProcess cmd;
    cmd.start(str);    
    if (!cmd.waitForStarted())
    {
        m_text->setText("ERROR: start command faild\n");
        return;
    }
    if (!cmd.waitForFinished())
    {
        m_text->setText("ERROR: wait command finish faild\n");
        return;
    }
    QByteArray contents = cmd.readAll();
    QTextStream stream(&contents);
    QString line;
    while (!stream.atEnd())
    {
        line.append(stream.readLine()+"\n");
    }
    m_text->setText(line);  
}


CalibParamWidget::CalibParamWidget(QWidget *parent)
    : QWidget(parent)
{
    m_param_file_path_fas = "avm_calib_res/calib_factory.param";
}


void CalibParamWidget::layout(void)
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setSizeConstraint(QLayout::SetMinimumSize);
    layout->setSpacing(15);
    this->setLayout(layout);

    //calib param
    QHBoxLayout *bt_layout_1 = new QHBoxLayout();
    m_load_from_fas_param = new QPushButton(this);
    m_load_from_fas_param->setText("load from fas param");
    bt_layout_1->addWidget(m_load_from_fas_param);
    m_save_to_fas_param = new QPushButton(this);
    m_save_to_fas_param->setText("save to fas param");
    bt_layout_1->addWidget(m_save_to_fas_param);
    layout->addLayout(bt_layout_1);

    m_tb_widget = new QTableWidget(this);
    m_tb_widget->setColumnCount(3);
    m_tb_widget->setRowCount(24);
    m_tb_widget->setHorizontalHeaderLabels(QStringList() << "name" << "cur_value" << "new value");
    m_tb_widget->setColumnWidth(0, this->width()/3-25);
    m_tb_widget->setColumnWidth(1, this->width()/3-25);
    m_tb_widget->setColumnWidth(2, this->width()/3-25);
    layout->addWidget(m_tb_widget);

    QHBoxLayout *bt_layout_2 = new QHBoxLayout();

    m_btn_0 = new QPushButton(this);
    m_btn_0->setText("0");
    bt_layout_2->addWidget(m_btn_0);
    m_btn_1 = new QPushButton(this);
    m_btn_1->setText("1");
    bt_layout_2->addWidget(m_btn_1);
    m_btn_2 = new QPushButton(this);
    m_btn_2->setText("2");
    bt_layout_2->addWidget(m_btn_2);
    m_btn_3 = new QPushButton(this);
    m_btn_3->setText("3");
    bt_layout_2->addWidget(m_btn_3);
    m_btn_4 = new QPushButton(this);
    m_btn_4->setText("4");
    bt_layout_2->addWidget(m_btn_4);
    m_btn_5 = new QPushButton(this);
    m_btn_5->setText("5");
    bt_layout_2->addWidget(m_btn_5);
    m_btn_6 = new QPushButton(this);
    m_btn_6->setText("6");
    bt_layout_2->addWidget(m_btn_6);
    layout->addLayout(bt_layout_2);

    QHBoxLayout *bt_layout_3 = new QHBoxLayout();
    m_btn_7 = new QPushButton(this);
    m_btn_7->setText("7");
    bt_layout_3->addWidget(m_btn_7);
    m_btn_8 = new QPushButton(this);
    m_btn_8->setText("8");
    bt_layout_3->addWidget(m_btn_8);
    m_btn_9 = new QPushButton(this);
    m_btn_9->setText("9");
    bt_layout_3->addWidget(m_btn_9);
    m_btn_pt = new QPushButton(this);
    m_btn_pt->setText(".");
    bt_layout_3->addWidget(m_btn_pt);
    m_btn_sign = new QPushButton(this);
    m_btn_sign->setText("-");
    bt_layout_3->addWidget(m_btn_sign);
    m_btn_del = new QPushButton(this);
    m_btn_del->setText("Del");
    bt_layout_3->addWidget(m_btn_del);
    layout->addLayout(bt_layout_3);

    m_label = new QLabel(this);
    layout->addWidget(m_label);

    //slots
    connect(m_load_from_fas_param, SIGNAL(released()), this, SLOT(load_from_fas_param_btn_click()));
    connect(m_save_to_fas_param, SIGNAL(released()), this, SLOT(save_to_fas_param_btn_click()));
    connect(m_btn_pt, SIGNAL(released()), this, SLOT(btn_pt_click()));
    connect(m_btn_sign, SIGNAL(released()), this, SLOT(btn_sign_click()));
    connect(m_btn_del, SIGNAL(released()), this, SLOT(btn_del_click()));
    connect(m_btn_0, SIGNAL(released()), this, SLOT(btn_0_click()));
    connect(m_btn_1, SIGNAL(released()), this, SLOT(btn_1_click()));
    connect(m_btn_2, SIGNAL(released()), this, SLOT(btn_2_click()));
    connect(m_btn_3, SIGNAL(released()), this, SLOT(btn_3_click()));
    connect(m_btn_4, SIGNAL(released()), this, SLOT(btn_4_click()));
    connect(m_btn_5, SIGNAL(released()), this, SLOT(btn_5_click()));
    connect(m_btn_6, SIGNAL(released()), this, SLOT(btn_6_click()));
    connect(m_btn_7, SIGNAL(released()), this, SLOT(btn_7_click()));
    connect(m_btn_8, SIGNAL(released()), this, SLOT(btn_8_click()));
    connect(m_btn_9, SIGNAL(released()), this, SLOT(btn_9_click()));

    //init table
    init_table();
}


void CalibParamWidget::init_table(void)
{
    QTableWidgetItem *item = NULL;
    unsigned int total = 0;

    //name col
    for(unsigned int cnt=0; cnt<4; ++cnt)
    {
        item = new QTableWidgetItem();
        item->setBackground(QBrush(QColor(Qt::lightGray)));
        item->setFlags(0);
        item->setTextColor(QColor(Qt::black));
        item->setText(QString("cam[%1]_x").arg(cnt));
        m_tb_widget->setItem(total++, 0, item);

        item = new QTableWidgetItem();
        item->setBackground(QBrush(QColor(Qt::lightGray)));
        item->setFlags(0);
        item->setTextColor(QColor(Qt::black));
        item->setText(QString("cam[%1]_y").arg(cnt));
        m_tb_widget->setItem(total++, 0, item);

        item = new QTableWidgetItem();
        item->setBackground(QBrush(QColor(Qt::lightGray)));
        item->setFlags(0);
        item->setTextColor(QColor(Qt::black));
        item->setText(QString("cam[%1]_z").arg(cnt));
        m_tb_widget->setItem(total++, 0, item);

        item = new QTableWidgetItem();
        item->setBackground(QBrush(QColor(Qt::lightGray)));
        item->setFlags(0);
        item->setTextColor(QColor(Qt::black));
        item->setText(QString("cam[%1]_a").arg(cnt));
        m_tb_widget->setItem(total++, 0, item);

        item = new QTableWidgetItem();
        item->setBackground(QBrush(QColor(Qt::lightGray)));
        item->setFlags(0);
        item->setTextColor(QColor(Qt::black));
        item->setText(QString("cam[%1]_b").arg(cnt));
        m_tb_widget->setItem(total++, 0, item);

        item = new QTableWidgetItem();
        item->setBackground(QBrush(QColor(Qt::lightGray)));
        item->setFlags(0);
        item->setTextColor(QColor(Qt::black));
        item->setText(QString("cam[%1]_c").arg(cnt));
        m_tb_widget->setItem(total++, 0, item);
    }
}

void CalibParamWidget::update_table(void)
{
    QTableWidgetItem *item = NULL;
    unsigned int total = 0;
    double tmp_val = 0.0;

    //cur_value col
    for(unsigned int cnt=0; cnt<4; ++cnt)
    {
        tmp_val = m_rvecs_fas[cnt].at<double>(0,0);
        item = new QTableWidgetItem();
        item->setTextColor(QColor(Qt::black));
        item->setText(QString("%1").arg(tmp_val));
        m_tb_widget->setItem(total++, 1, item);

        tmp_val = m_rvecs_fas[cnt].at<double>(0,1);
        item = new QTableWidgetItem();
        item->setTextColor(QColor(Qt::black));
        item->setText(QString("%1").arg(tmp_val));
        m_tb_widget->setItem(total++, 1, item);

        tmp_val = m_rvecs_fas[cnt].at<double>(0,2);
        item = new QTableWidgetItem();
        item->setTextColor(QColor(Qt::black));
        item->setText(QString("%1").arg(tmp_val));
        m_tb_widget->setItem(total++, 1, item);

        tmp_val = m_mvecs_fas[cnt].at<double>(0,0);
        item = new QTableWidgetItem();
        item->setTextColor(QColor(Qt::black));
        item->setText(QString("%1").arg(tmp_val));
        m_tb_widget->setItem(total++, 1, item);

        tmp_val = m_mvecs_fas[cnt].at<double>(0,1);
        item = new QTableWidgetItem();
        item->setTextColor(QColor(Qt::black));
        item->setText(QString("%1").arg(tmp_val));
        m_tb_widget->setItem(total++, 1, item);

        tmp_val = m_mvecs_fas[cnt].at<double>(0,2);
        item = new QTableWidgetItem();
        item->setTextColor(QColor(Qt::black));
        item->setText(QString("%1").arg(tmp_val));
        m_tb_widget->setItem(total++, 1, item);
    }
}


void CalibParamWidget::save_table(void)
{
    unsigned int total = 0;
    QTableWidgetItem* pItem = NULL;

    for(unsigned int cnt=0; cnt<4; ++cnt)
    {
        pItem = m_tb_widget->item(total++, 2);
        if( (NULL != pItem)&&(!pItem->text().isEmpty()) )
        {
            m_rvecs_fas[cnt].at<double>(0,0) = pItem->text().toDouble();
        }

        pItem = m_tb_widget->item(total++, 2);
        if( (NULL != pItem)&&(!pItem->text().isEmpty()) )
        {
            m_rvecs_fas[cnt].at<double>(0,1) = pItem->text().toDouble();
        }

        pItem = m_tb_widget->item(total++, 2);
        if( (NULL != pItem)&&(!pItem->text().isEmpty()) )
        {
            m_rvecs_fas[cnt].at<double>(0,2) = pItem->text().toDouble();
        }

        pItem = m_tb_widget->item(total++, 2);
        if( (NULL != pItem)&&(!pItem->text().isEmpty()) )
        {
            m_mvecs_fas[cnt].at<double>(0,0) = pItem->text().toDouble();
        }

        pItem = m_tb_widget->item(total++, 2);
        if( (NULL != pItem)&&(!pItem->text().isEmpty()) )
        {
            m_mvecs_fas[cnt].at<double>(0,1) = pItem->text().toDouble();
        }

        pItem = m_tb_widget->item(total++, 2);
        if( (NULL != pItem)&&(!pItem->text().isEmpty()) )
        {
            m_mvecs_fas[cnt].at<double>(0,2) = pItem->text().toDouble();
        }
    }
}


void CalibParamWidget::read_mat_binary(QFile& inputFile, cv::Mat& mat)
{
    int rows, cols, type;
    inputFile.read((char*)(&rows), sizeof(int));
    if (rows==0)
    {
        return;
    }
    inputFile.read((char*)(&cols), sizeof(int));
    inputFile.read((char*)(&type), sizeof(int));

    mat.release();
    mat.create(rows, cols, type);
    inputFile.read((char*)(mat.data), mat.elemSize() * mat.total());
}


void CalibParamWidget::write_mat_binary(QFile& inputFile, cv::Mat& mat)
{
    if (mat.empty())
    {
        int s = 0;
        inputFile.write((const char*)(&s), sizeof(int));
    }
    else
    {
        int type = mat.type();
        inputFile.write((const char*)(&mat.rows), sizeof(int));
        inputFile.write((const char*)(&mat.cols), sizeof(int));
        inputFile.write((const char*)(&type), sizeof(int));
        inputFile.write((const char*)(mat.data), mat.elemSize() * mat.total());
    }
}


void CalibParamWidget::add_str_to_cur_item(QString str)
{
    QString tmp("");
    QTableWidgetItem *item = m_tb_widget->currentItem();
    if (item != NULL)
    {
       tmp = item->text() + str;
       item->setText(tmp);
    }
}


void CalibParamWidget::load_from_fas_param_btn_click()
{
    if (false == QFile::exists(m_param_file_path_fas))
    {
        m_label->setText(QString("failed: file not existed"));
        return;
    }

    QFile theFile(m_param_file_path_fas);
    theFile.open(QIODevice::ReadOnly);
    theFile.read((char*)&m_file_head_fas, sizeof(m_file_head_fas));

    for(int cnt=0; cnt<4; ++cnt)
    {
        theFile.read((char*)&m_calib_roi_fas[cnt], sizeof(CalibRoi));
    }
    for(int cnt=0; cnt<4; ++cnt)
    {
        read_mat_binary(theFile, m_rvecs_fas[cnt]);
    }
    for(int cnt=0; cnt<4; ++cnt)
    {
        read_mat_binary(theFile, m_mvecs_fas[cnt]);
    }

    theFile.close();

    update_table();

    m_label->setText(QString("Load success"));
}


void CalibParamWidget::save_to_fas_param_btn_click()
{
    if ( (0==m_rvecs_fas[0].rows)||(0==m_mvecs_fas[0].rows) )
    {
        m_label->setText(QString("failed: data is empty"));
        return;
    }

    save_table();

    QFile theFile(m_param_file_path_fas);
    theFile.open(QIODevice::WriteOnly);
    theFile.write((char*)&m_file_head_fas, sizeof(m_file_head_fas));

    for(int cnt=0; cnt<4; ++cnt)
    {
        theFile.write((char*)&m_calib_roi_fas[cnt], sizeof(CalibRoi));
    }
    for(int cnt=0; cnt<4; ++cnt)
    {
        write_mat_binary(theFile, m_rvecs_fas[cnt]);
    }
    for(int cnt=0; cnt<4; ++cnt)
    {
        write_mat_binary(theFile, m_mvecs_fas[cnt]);
    }

    theFile.close();

    update_table();

    m_label->setText(QString("Save success, please recalib"));
}


void CalibParamWidget::btn_pt_click(void)
{
    add_str_to_cur_item(QString("."));
}

void CalibParamWidget::btn_sign_click(void)
{
    add_str_to_cur_item(QString("-"));
}

void CalibParamWidget::btn_del_click(void)
{
    QTableWidgetItem *item = m_tb_widget->currentItem();
    if (item != NULL)
    {
       item->setText("");
    }
}

void CalibParamWidget::btn_0_click(void)
{
    add_str_to_cur_item(QString("0"));
}

void CalibParamWidget::btn_1_click(void)
{
    add_str_to_cur_item(QString("1"));
}

void CalibParamWidget::btn_2_click(void)
{
    add_str_to_cur_item(QString("2"));
}

void CalibParamWidget::btn_3_click(void)
{
    add_str_to_cur_item(QString("3"));
}

void CalibParamWidget::btn_4_click(void)
{
    add_str_to_cur_item(QString("4"));
}

void CalibParamWidget::btn_5_click(void)
{
    add_str_to_cur_item(QString("5"));
}

void CalibParamWidget::btn_6_click(void)
{
    add_str_to_cur_item(QString("6"));
}

void CalibParamWidget::btn_7_click(void)
{
    add_str_to_cur_item(QString("7"));
}

void CalibParamWidget::btn_8_click(void)
{
    add_str_to_cur_item(QString("8"));
}

void CalibParamWidget::btn_9_click(void)
{
    add_str_to_cur_item(QString("9"));
}



