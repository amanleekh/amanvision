
#include <QPixmap>
#include <QString>

#include <stdio.h>

#include "main_widget.h"
#include "surround_widget.h"
#include "summ_widget.h"
#include "video_capture.h"
#include "sync_video_getter.h"
#include "clickablelabel.h"
#include "cmd_recver.h"
#include "version.h"
#include "comu_cmd.h"
#include "custom_str.h"
#include "peilabel.h"
#include "ini_config.h"
#include "comu_cmd_define.h"

// font size=20px
#define FONT_SIZE_20        "font-size:20px;"
// setting button on/off background color
#define SETTING_ON_BC       "background-color:rgba(0, 0, 0, 255);"
#define SETTING_OFF_BC      "background-color:rgba(0, 0, 0, 255);"
// change style button on/off background color
#define CHNAGESTYLE_ON_BC   "background-color:rgba(0, 0, 0, 255);"
#define CHNAGESTYLE_OFF_BC  "background-color:rgba(0, 0, 0, 255);"

// pie keeps showing for x seconds after view changing
#define POPUP_DISAPPEAR_DELAY		(5*1000)


extern int get_main_init_faild();

int MainWidget::m_isShutDown = 0;
MainWidget *MainWidget::m_p_this = NULL;


MainWidget::MainWidget(int input_width, int input_height)
{
    m_surroundWidget = NULL;
    m_summWidget = NULL;
    m_status = NULL;
    m_workerThread = NULL;
    m_checkGlTimer = NULL;

    //m_chgStyleLabel = NULL;
    //m_autoLabel = NULL;
    //m_2DLabel = NULL;
    //m_3DLabel = NULL;
    m_settingLabel = NULL;
    m_assistLabel = NULL;
    m_radioLabel = NULL;
	m_turnLinkLabel = NULL;
    //m_3DPieLabel = NULL;
    //m_2DPieLabel = NULL;
    m_backLabel = NULL;
    //m_current_view = NULL;
    //m_alert_bottom_mask = NULL;
    m_alert_window_body = NULL;
    m_alert_window_btn = NULL;
    m_debug_version_info = NULL;
    m_debug_error_info_title = NULL;
    m_debug_error_info = NULL;
    m_reminder_text_state = 0;
    m_reminder_timer = NULL;

    m_assist_angle = 0.0f;
    m_style = 0;
    m_isAssistOn = 0;
    m_isRadarOn = 0;
    m_isRadioOn = 0;
	m_isTurnLinkOn = 0;
    m_isRadioStatFixed = 0;
    m_isSettimgOn = 0;
    m_isChgStyleOn = 0;
    m_isD2PieOn = 0;
    m_isD3PieOn = 0;
    m_d2pieSelIdx = -1;
    m_d3pieSelIdx = -1;
    m_is_use_bmp = 0;
#if GLOBAL_RUN_ENV_DESKTOP == 1
    m_is_use_bmp = 1;
#else
    m_is_use_bmp = ini_get_device_use_bmp_file();
#endif

    m_car_speed = 0.0f;
    m_assist_angle = 0.0f;
    m_turn_signal = 0;
    m_go_back = 0;
    m_front_left_door_open = 0;
    m_front_right_door_open = 0;
    m_rear_left_door_open = 0;
    m_rear_right_door_open = 0;    
    m_left_mirrow_fold = 0;
    m_right_mirrow_fold = 0;
    m_radar_bits[0]= 0xFFFFFFFF;    // FF means all radar are hided
    m_radar_bits[1]= 0xFFFFFFFF;

    m_isMousePressed = 0;
    m_isMouseMoveSlide = 0;
    m_isMouseClickCanced = 0;
    m_mouseTotalMove = 0;
    
    m_check_vaild_bypass_cnt = 0;
    MainWidget::m_p_this = this;

#if 1 //LG UI
    m_isFrontSightButtonPopped = 0;
	m_isRearSightButtonPopped = 0;
	m_is3DDirButtonPopped = 0;
	m_is_item_hide = 0;
#endif

    m_input_width = input_width;
    m_input_height = input_height;
}


void MainWidget::layout()
{
#define LAYOUT_SURR_WIDTH           (880)
#define LAYOUT_SURR_AUTO_WIDTH      (656)       // surr widget width in auto view
#define LAYOUT_SURR_WIDE_WIDTH      (1280)      // wide front and rear view, widget width
#define LAYOUT_SUMM_WIDTH           (399)
#define LAYOUT_GAP_LINE_WIDTH       (1)
#define LAYOUT_TOTAL_WIDTH          (LAYOUT_SURR_WIDTH+LAYOUT_GAP_LINE_WIDTH+LAYOUT_SUMM_WIDTH)
#define LAYOUT_TOP_HEIGHT           (656)
#define LAYOUT_T_STAT_HEIGHT        (48)        // top status bar height
#define LAYOUT_B_STAT_HEIGHT        (64)        // bottom status bar height
#define LAYOUT_TOTAL_HEIGHT         (LAYOUT_TOP_HEIGHT+LAYOUT_B_STAT_HEIGHT)
#define LAYOUT_LAB_WIDTH            (80)        // button width
#define LAYOUT_CHANGESTYLE_WIDTH    (56)
#define LAYOUT_STYLE_WIDTH          (107)
#define LAYOUT_STYLE_HEIGHT         (64)
#define LAYOUT_SETTING_WIDTH        (56)
#define LAYOUT_SETTING_HEIGHT       (64)
#define LAYOUT_SETTING_ELE_WIDTH    (107)
#define LAYOUT_SLAB_WIDTH           (200)
#define LAYOUT_REMDER_WIDTH         (LAYOUT_SURR_WIDTH-LAYOUT_LAB_WIDTH*2)
#define LAYOUT_CUR_VIEW_W   		(401)
#define LAYOUT_CUR_VIEW_H   		(48)

    this->setObjectName("main_widget");
    this->setStyleSheet("#main_widget{background-color:rgb(0,0,0)}");

    m_surroundWidget = new SurroundWidget(this, LAYOUT_SURR_WIDE_WIDTH, 
        LAYOUT_SURR_WIDTH, LAYOUT_TOP_HEIGHT, m_input_width, m_input_height);
    m_surroundWidget->setGeometry(0, 0,
        LAYOUT_SURR_WIDTH, LAYOUT_TOP_HEIGHT);

    m_summWidget = new SummWidget(this, LAYOUT_SUMM_WIDTH, 
        LAYOUT_SUMM_WIDTH, LAYOUT_TOP_HEIGHT, m_input_width, m_input_height);
    m_summWidget->setGeometry(LAYOUT_SURR_WIDTH+LAYOUT_GAP_LINE_WIDTH, 0,
        LAYOUT_SUMM_WIDTH, LAYOUT_TOP_HEIGHT);

    m_status = new ClickableLabel(this);
    m_status->setGeometry(0, LAYOUT_TOP_HEIGHT,
        LAYOUT_TOTAL_WIDTH, LAYOUT_B_STAT_HEIGHT);
    m_status->setStyleSheet("background-color:rgb(0,0,0)");

    m_reminder_text = new ClickableLabel(this);
    m_reminder_text->setGeometry(
        (LAYOUT_TOTAL_WIDTH-LAYOUT_REMDER_WIDTH)/2, LAYOUT_TOP_HEIGHT,
        LAYOUT_REMDER_WIDTH, LAYOUT_B_STAT_HEIGHT);  
    m_reminder_text->setStyleSheet(
        "color:rgb(255,255,255);"
        "qproperty-alignment: AlignCenter;"
        FONT_SIZE_20);    
    m_reminder_text->setText(custstr_get_careful_str());

//LG UI
#define ITEM_START_POS_X  		(17)
#define ITEM_START_POS_Y  		(29)
#define ITEM_SPACE_H  			(18)
#define ITEM_SPACE_V  			(9)
#define ITEM_WIDTH  			(178)
#define ITEM_HEIGHT 			(64)
#define SETTING_ITEM_WIDTH 		(64)
#define SETTING_ITEM_HEIGHT 	(64)
#define MUTE_ITEM_POS_X			(935)
#define MUTE_ITEM_POS_Y			(29)
#define MUTE_ITEM_WIDTH			(80)
#define MUTE_ITEM_HEIGHT		(60)
#define D3_DIR_BTN_WIDTH		(138)
#define D3_DIR_BTN_HEIGHT		(129)

    m_sight_rear_btn = new ClickableLabel(this);
    m_sight_rear_btn->setGeometry(ITEM_START_POS_X,
		ITEM_START_POS_Y, ITEM_WIDTH, ITEM_HEIGHT);
    rearSightButtonOff();

	m_sight_rear_fish_btn = new ClickableLabel(this);
    m_sight_rear_fish_btn->setGeometry(ITEM_START_POS_X,
		ITEM_START_POS_Y+ITEM_HEIGHT+ITEM_SPACE_V,
		ITEM_WIDTH, ITEM_HEIGHT);
    rearFishSightButtonOff();
    m_sight_rear_fish_btn->hide();

	m_sight_front_btn = new ClickableLabel(this);
    m_sight_front_btn->setGeometry(ITEM_START_POS_X+ITEM_WIDTH+ITEM_SPACE_H,
		ITEM_START_POS_Y, ITEM_WIDTH, ITEM_HEIGHT);
    frontSightButtonOff();
	
	m_sight_front_fish_btn = new ClickableLabel(this);
	m_sight_front_fish_btn->setGeometry(ITEM_START_POS_X+ITEM_WIDTH+ITEM_SPACE_H,
		ITEM_START_POS_Y+ITEM_HEIGHT+ITEM_SPACE_V,
		ITEM_WIDTH, ITEM_HEIGHT);
    frontFishSightButtonOff();
    m_sight_front_fish_btn->hide();

    m_sight_side_btn = new ClickableLabel(this);
    m_sight_side_btn->setGeometry(ITEM_START_POS_X+(ITEM_WIDTH+ITEM_SPACE_H)*2,
		ITEM_START_POS_Y, ITEM_WIDTH, ITEM_HEIGHT);
    sideSightButtonOff();

	m_sight_3d_btn = new ClickableLabel(this);
	m_sight_3d_btn->setGeometry(ITEM_START_POS_X+(ITEM_WIDTH+ITEM_SPACE_H)*3,
		ITEM_START_POS_Y, ITEM_WIDTH, ITEM_HEIGHT);
	d3SightButtonOff();

    m_3d_dir_btn = new PieLabel(this);
	m_3d_dir_btn->setGeometry(
		ITEM_START_POS_X+(ITEM_WIDTH+ITEM_SPACE_H)*3+(ITEM_WIDTH-D3_DIR_BTN_WIDTH)/2,
		ITEM_START_POS_Y+ITEM_HEIGHT+ITEM_SPACE_V,
		D3_DIR_BTN_WIDTH, D3_DIR_BTN_HEIGHT);
	d3DirButtonInit();
	m_3d_dir_btn->hide();
    d3DirButtonNone();

	m_settingLabel = new ClickableLabel(this);
    m_settingLabel->setGeometry(
        ITEM_START_POS_X+(ITEM_WIDTH+ITEM_SPACE_H)*4,
        ITEM_START_POS_Y, SETTING_ITEM_WIDTH, SETTING_ITEM_HEIGHT);
    settimgButtonInit();

	m_radarLabel = new ClickableLabel(this);
    m_radarLabel->setGeometry(
        ITEM_START_POS_X+(ITEM_WIDTH+ITEM_SPACE_H)*4,
        ITEM_START_POS_Y+SETTING_ITEM_HEIGHT+ITEM_SPACE_V,
        SETTING_ITEM_WIDTH, SETTING_ITEM_HEIGHT);
    radarButtonInit();

	m_assistLabel = new ClickableLabel(this);
    m_assistLabel->setGeometry(
        ITEM_START_POS_X+(ITEM_WIDTH+ITEM_SPACE_H)*4,
        ITEM_START_POS_Y+(SETTING_ITEM_HEIGHT+ITEM_SPACE_V)*2,
        SETTING_ITEM_WIDTH, SETTING_ITEM_HEIGHT);
    assistButtonInit();

	m_turnLinkLabel = new ClickableLabel(this);
	m_turnLinkLabel->setGeometry(
		ITEM_START_POS_X+(ITEM_WIDTH+ITEM_SPACE_H)*4,
        ITEM_START_POS_Y+(SETTING_ITEM_HEIGHT+ITEM_SPACE_V)*3,
        SETTING_ITEM_WIDTH, SETTING_ITEM_HEIGHT);
	turnLinkButtonInit();

    m_radioLabel = new ClickableLabel(this);
    m_radioLabel->setGeometry(
        MUTE_ITEM_POS_X, MUTE_ITEM_POS_Y,
        MUTE_ITEM_WIDTH, MUTE_ITEM_HEIGHT);
    radioButtonInit();

#define LAYOUT_FONT_SIZE            (14)
#define LAYOUT_FONT_LENGTH          (LAYOUT_SURR_WIDTH)
#define LAYOUT_LINE_MARGIN          (2)
#define LAYOUT_ALERT_BODY_WIDTH     (640)
#define LAYOUT_ALERT_BODY_HEIGHT    (260)
#define LAYOUT_ALERT_BODY_TEXT_PAD  (80)
#define LAYOUT_ALERT_BTN_WIDTH      (640)
#define LAYOUT_ALERT_BTN_HEIGHT     (48)
#define LAYOUT_ALERT_BTN_ADDH       (50)

    // when show alert window, this mask image cover stitch video
    m_alert_bottom_mask = new ClickableAdvLabel(this);
    m_alert_bottom_mask->hide();
    m_alert_bottom_mask->setGeometry(0, 0, LAYOUT_SURR_WIDTH+LAYOUT_SUMM_WIDTH,
        LAYOUT_TOTAL_HEIGHT);
    m_alert_bottom_mask->setStyleSheet("background-color:rgba(0,0,0,0)");
	
    m_alert_window_body = new ClickableLabel(this);
    m_alert_window_body->hide();
    m_alert_window_body->setGeometry(
        (LAYOUT_TOTAL_WIDTH-LAYOUT_ALERT_BODY_WIDTH)/2,
        (LAYOUT_TOTAL_HEIGHT-LAYOUT_ALERT_BODY_HEIGHT)/2,
        LAYOUT_ALERT_BODY_WIDTH, LAYOUT_ALERT_BODY_HEIGHT);
    m_alert_window_body->setStyleSheet(
        "qproperty-alignment: AlignTop;"
        "padding-top:52px;"
        "padding-left:32px;"
        "background-color:rgba(27,27,27,255);"
        "color:rgb(204,204,204);"
        FONT_SIZE_20);

    m_alert_window_btn = new ClickableLabel(this);
    m_alert_window_btn->hide();
    m_alert_window_btn->setGeometry(
        (LAYOUT_TOTAL_WIDTH-LAYOUT_ALERT_BTN_WIDTH)/2,
        LAYOUT_TOTAL_HEIGHT - ((LAYOUT_TOTAL_HEIGHT-LAYOUT_ALERT_BODY_HEIGHT)/2 + LAYOUT_ALERT_BTN_HEIGHT),
        LAYOUT_ALERT_BTN_WIDTH, LAYOUT_ALERT_BTN_HEIGHT);
    m_alert_window_btn->setText(custstr_get_alert_ok_str());
    m_alert_window_btn->setStyleSheet(
        "qproperty-alignment: AlignCenter;"
        "color:rgb(184,184,184);"
        FONT_SIZE_20
        "background-color:rgb(35,35,35);");


    m_debug_error_info_title = new ClickableLabel(this);
    m_debug_error_info_title->hide();
    m_debug_error_info_title->setGeometry(0, LAYOUT_TOTAL_HEIGHT-LAYOUT_FONT_SIZE*3-LAYOUT_LINE_MARGIN*3,
        LAYOUT_FONT_LENGTH, LAYOUT_FONT_SIZE);
    m_debug_error_info_title->setStyleSheet("QLabel{color:white;font-size:14px;margin:0;padding:0;}");
    m_debug_error_info_title->setText(custstr_get_errinfo_title());
    
    m_debug_error_info = new ClickableLabel(this);
    m_debug_error_info->hide();
    m_debug_error_info->setGeometry(0, LAYOUT_TOTAL_HEIGHT-LAYOUT_FONT_SIZE*2-LAYOUT_LINE_MARGIN*2,
        LAYOUT_FONT_LENGTH, LAYOUT_FONT_SIZE);
    m_debug_error_info->setStyleSheet("QLabel{color:white;font-size:14px;margin:0;padding:0;}");
    setDynDebugLabel();


    m_debug_version_info = new ClickableLabel(this);
    m_debug_version_info->setGeometry(0, LAYOUT_TOTAL_HEIGHT-LAYOUT_FONT_SIZE-LAYOUT_LINE_MARGIN,
        LAYOUT_FONT_LENGTH, LAYOUT_FONT_SIZE);
    m_debug_version_info->setStyleSheet("border:none;color:white;font-size:14px;margin:0;padding:0;");
    char *verCStr, *verCStr1, *verCStr2, *verCStr3;
	char* vpVerCstr;
    char *dateCStr;
    ver_get_version(&verCStr, &verCStr1, &verCStr2, &verCStr3);
    ver_get_date(&dateCStr);
	ver_get_vp_ver(&vpVerCstr);
    QString qstr = QString("version: ") 
        + QString(verCStr) + QString(",")
        + QString(verCStr1) + QString(",")
        + QString(verCStr2) + QString(",")
        + QString(verCStr3)
        + QString(" / VP ver: ") + QString(vpVerCstr)
        + QString(" / ") + QString(dateCStr);
    m_debug_version_info->hide();
    m_debug_version_info->setText(qstr);

    //LG UI
    connect(m_sight_front_btn, SIGNAL(clicked()), this, SLOT(frontSightButtonClicked()));
    connect(m_sight_front_btn, SIGNAL(released()), this, SLOT(frontSightButtonReleased()));
	connect(m_sight_front_fish_btn, SIGNAL(clicked()), this, SLOT(frontFishSightButtonClicked()));
    connect(m_sight_front_fish_btn, SIGNAL(released()), this, SLOT(frontFishSightButtonReleased()));
    connect(m_sight_rear_btn, SIGNAL(clicked()), this, SLOT(rearSightButtonClicked()));
    connect(m_sight_rear_btn, SIGNAL(released()), this, SLOT(rearSightButtonReleased()));
	connect(m_sight_rear_fish_btn, SIGNAL(clicked()), this, SLOT(rearFishSightButtonClicked()));
    connect(m_sight_rear_fish_btn, SIGNAL(released()), this, SLOT(rearFishSightButtonReleased()));
    connect(m_sight_side_btn, SIGNAL(clicked()), this, SLOT(sideSightButtonClicked()));
    connect(m_sight_side_btn, SIGNAL(released()), this, SLOT(sideSightButtonReleased()));
	connect(m_sight_3d_btn, SIGNAL(clicked()), this, SLOT(d3SightButtonClicked()));
    connect(m_sight_3d_btn, SIGNAL(released()), this, SLOT(d3SightButtonReleased()));
    connect(m_3d_dir_btn, SIGNAL(clicked(int)), this, SLOT(d3DirButtonClicked(int)));
    connect(m_3d_dir_btn, SIGNAL(released(int)), this, SLOT(d3DirButtonReleased(int)));
	connect(m_turnLinkLabel, SIGNAL(clicked()), this, SLOT(turnLinkButtonClicked()));
    connect(m_turnLinkLabel, SIGNAL(released()), this, SLOT(turnLinkButtonReleased()));

    //connect(m_2DLabel, SIGNAL(clicked()), this, SLOT(d2ButtonClicked()));
    //connect(m_2DLabel, SIGNAL(released()), this, SLOT(d2ButtonReleased()));
    //connect(m_3DLabel, SIGNAL(clicked()), this, SLOT(d3ButtonClicked()));
    //connect(m_3DLabel, SIGNAL(released()), this, SLOT(d3ButtonReleased()));
    //connect(m_autoLabel, SIGNAL(clicked()), this, SLOT(autoButtonClicked()));
    //connect(m_autoLabel, SIGNAL(released()), this, SLOT(autoButtonReleased()));
    //connect(m_chgStyleLabel, SIGNAL(clicked()), this, SLOT(changeStyleButtonClicked()));
    //connect(m_chgStyleLabel, SIGNAL(released()), this, SLOT(changeStyleButtonReleased()));
    connect(m_assistLabel, SIGNAL(clicked()), this, SLOT(assistButtonClicked()));
    connect(m_assistLabel, SIGNAL(released()), this, SLOT(assistButtonReleased()));
    connect(m_radarLabel, SIGNAL(clicked()), this, SLOT(radarButtonClicked()));
    connect(m_radarLabel, SIGNAL(released()), this, SLOT(radarButtonReleased()));
    connect(m_radioLabel, SIGNAL(clicked()), this, SLOT(radioButtonClicked()));
    connect(m_radioLabel, SIGNAL(released()), this, SLOT(radioButtonReleased()));
    connect(m_settingLabel, SIGNAL(clicked()), this, SLOT(settimgButtonClicked()));
    connect(m_settingLabel, SIGNAL(released()), this, SLOT(settimgButtonReleased()));
    //connect(m_2DPieLabel, SIGNAL(clicked(int)), this, SLOT(d2PieClicked(int)));
    //connect(m_2DPieLabel, SIGNAL(released(int)), this, SLOT(d2PieReleased(int)));
    //connect(m_3DPieLabel, SIGNAL(clicked(int)), this, SLOT(d3PieClicked(int)));
    //connect(m_3DPieLabel, SIGNAL(released(int)), this, SLOT(d3PieReleased(int)));
    //connect(m_backLabel, SIGNAL(clicked()), this, SLOT(backButtonClicked()));
    
    connect(m_alert_window_btn, SIGNAL(clicked()), this, SLOT(alertButtonClicked()));
    connect(m_alert_window_btn, SIGNAL(released()), this, SLOT(alertButtonReleased()));
    //connect(m_current_view, SIGNAL(released()), this, SLOT(currentViewButtonReleased()));
    connect(m_reminder_text, SIGNAL(released()), this, SLOT(reminderTextButtonReleased()));
	connect(m_status, SIGNAL(released()), this, SLOT(statusBarButtonReleased()));
    connect(m_alert_bottom_mask, SIGNAL(clicked(QMouseEvent *)), this, SLOT(alertBottomMaskClicked(QMouseEvent *)));
    connect(m_alert_bottom_mask, SIGNAL(released(QMouseEvent *)), this, SLOT(alertBottomMaskReleased(QMouseEvent *)));
    connect(m_alert_window_body, SIGNAL(clicked()), this, SLOT(reservedClickLabelClicked()));
    connect(m_alert_window_body, SIGNAL(released()), this, SLOT(reservedClickLabelReleased())); 
    connect(m_debug_version_info, SIGNAL(clicked()), this, SLOT(reservedClickLabelClicked()));
    connect(m_debug_version_info, SIGNAL(released()), this, SLOT(reservedClickLabelReleased())); 
    connect(m_debug_error_info, SIGNAL(clicked()), this, SLOT(reservedClickLabelClicked()));
    connect(m_debug_error_info, SIGNAL(released()), this, SLOT(reservedClickLabelReleased()));    




    debugWidgetLayout();
}

#if 1 //LG UI
void MainWidget::frontSightButtonClicked()
{
	if (0 == checkClickVaildProc(1))
    {
        return;
    }
	
	frontSightButtonPressed();
}


void MainWidget::frontSightButtonReleased()
{
	if (0 == checkClickVaildProc(1))
    {
        return;
    }

	frontSightButtonOn();
	
    if(1 == m_isFrontSightButtonPopped)
    {
        m_isFrontSightButtonPopped = 0;

        m_sight_front_fish_btn->hide();
    }
    else
    {
        m_isFrontSightButtonPopped = 1;

        m_sight_front_fish_btn->show();
    }

	int cur_view = getView();
	if( (cur_view != AVM_VIEW_CAMERA_FISH_CLIP_FRONT)
		&&(cur_view != AVM_VIEW_CAMERA_FISH_FRONT) )
	{
		change_view(1, AVM_VIEW_CAMERA_FISH_CLIP_FRONT);
    	syncSightImageState(AVM_VIEW_CAMERA_FISH_CLIP_FRONT);
		m_style = 1;
	}

	m_itemsDisplayTimer->start(POPUP_DISAPPEAR_DELAY);
}

void MainWidget::frontFishSightButtonClicked()
{
	if (0 == checkClickVaildProc(1))
    {
        return;
    }
	
	frontFishSightButtonPressed();
}
void MainWidget::frontFishSightButtonReleased()
{
	if (0 == checkClickVaildProc(1))
    {
        return;
    }

	int cur_view = getView();
	if(cur_view == AVM_VIEW_CAMERA_FISH_FRONT)
	{
		change_view(1, AVM_VIEW_CAMERA_FISH_CLIP_FRONT);
		syncSightImageState(AVM_VIEW_CAMERA_FISH_CLIP_FRONT);
		m_sight_front_fish_btn->show();
	}
	else
	{
		change_view(1, AVM_VIEW_CAMERA_FISH_FRONT);
		syncSightImageState(AVM_VIEW_CAMERA_FISH_FRONT);
	}
	m_style = 1;

	m_itemsDisplayTimer->start(POPUP_DISAPPEAR_DELAY);
}

void MainWidget::rearSightButtonClicked()
{
	if (0 == checkClickVaildProc(1))
    {
        return;
    }
	
	rearSightButtonPressed();
}


void MainWidget::rearSightButtonReleased()
{
	if (0 == checkClickVaildProc(1))
    {
        return;
    }

	rearSightButtonOn();
	
    if(1 == m_isRearSightButtonPopped)
    {
        m_isRearSightButtonPopped = 0;

        m_sight_rear_fish_btn->hide();
    }
    else
    {
        m_isRearSightButtonPopped = 1;

        m_sight_rear_fish_btn->show();
    }

	int cur_view = getView();
	if( (cur_view != AVM_VIEW_CAMERA_FISH_CLIP_REAR)
		&&(cur_view != AVM_VIEW_CAMERA_FISH_REAR) )
	{
		change_view(1, AVM_VIEW_CAMERA_FISH_CLIP_REAR);
		syncSightImageState(AVM_VIEW_CAMERA_FISH_CLIP_REAR);
		m_style = 1;
	}

	m_itemsDisplayTimer->start(POPUP_DISAPPEAR_DELAY);
}

void MainWidget::rearFishSightButtonClicked()
{
	if (0 == checkClickVaildProc(1))
    {
        return;
    }
	
	rearFishSightButtonPressed();
}
void MainWidget::rearFishSightButtonReleased()
{
	if (0 == checkClickVaildProc(1))
    {
        return;
    }

	int cur_view = getView();
	if(cur_view == AVM_VIEW_CAMERA_FISH_REAR)
	{
		change_view(1, AVM_VIEW_CAMERA_FISH_CLIP_REAR);
		syncSightImageState(AVM_VIEW_CAMERA_FISH_CLIP_REAR);
		m_sight_rear_fish_btn->show();
	}
	else
	{
		change_view(1, AVM_VIEW_CAMERA_FISH_REAR);
		syncSightImageState(AVM_VIEW_CAMERA_FISH_REAR);
	}
	m_style = 1;

	m_itemsDisplayTimer->start(POPUP_DISAPPEAR_DELAY);
}

void MainWidget::sideSightButtonClicked()
{
	if (0 == checkClickVaildProc(1))
    {
        return;
    }
	
	sideSightButtonPressed();
}
void MainWidget::sideSightButtonReleased()
{
	if (0 == checkClickVaildProc(1))
    {
        return;
    }

	sideSightButtonOn();

	change_view(1, AVM_VIEW_CAMERA_LEFTRIGHT_CPY);
	syncSightImageState(AVM_VIEW_CAMERA_LEFTRIGHT_CPY);
	m_style = 1;

	m_itemsDisplayTimer->start(POPUP_DISAPPEAR_DELAY);
}

void MainWidget::d3SightButtonClicked()
{
	if (0 == checkClickVaildProc(1))
    {
        return;
    }
	
	d3SightButtonPressed();
}
void MainWidget::d3SightButtonReleased()
{
	if (0 == checkClickVaildProc(1))
    {
        return;
    }

	d3SightButtonOn();

	int cur_view = getView();
	if(1 == m_surroundWidget->checkInPure3dView(cur_view))
	{
		if(0 == m_is3DDirButtonPopped)
		{
			m_is3DDirButtonPopped = 1;

			// avoid display errors caused by external changes in avm view
            int pie_idx = m_3d_dir_btn->FindIndexByUid(cur_view);
            if (pie_idx < 0)
            {
                m_3d_dir_btn->TriggerIdxImage(0);
            }
            else
            {
                m_3d_dir_btn->TriggerIdxImage(pie_idx);
            }
			m_3d_dir_btn->show();
		}
		else
		{
			m_is3DDirButtonPopped = 0;

			m_3d_dir_btn->hide();
		}
	}
	else
	{
		change_view(1, AVM_VIEW_3D_REAR);
		syncSightImageState(AVM_VIEW_3D_REAR);
		m_style = 2;
	}

	m_itemsDisplayTimer->start(POPUP_DISAPPEAR_DELAY);
}

void MainWidget::d3DirButtonClicked(int idx)
{
	if (0 == checkClickVaildProc(1))
    {
        return;
    }
	
    idx = idx;
}

void MainWidget::d3DirButtonReleased(int idx)
{
	if ((idx < 0) || (idx > m_3d_dir_btn->GetIndexLimit()))
    {
        return;
    }
    
    int new_view = m_3d_dir_btn->FindUidByIndex(idx);
    if (new_view >= 0)
    {
        // in dyn change, when dyn move is on and dst view is not arrive,
        // current view probably = new view and exit this dyn change
        int cur_view = m_surroundWidget->getView();
        //if (cur_view == new_view)
        //{
        //    return; 
        //}
        int is_cur_view_3d = m_surroundWidget->checkInPure3dView(cur_view);
        int is_new_view_3d = m_surroundWidget->checkInPure3dView(new_view);
        if (is_cur_view_3d && is_new_view_3d)
        {
            change_view_dyn(1, new_view);
        }
        else
        {
            change_view(1, new_view);
			syncSightImageState(new_view);
        }

		m_itemsDisplayTimer->start(POPUP_DISAPPEAR_DELAY);
    }
}



void MainWidget::frontSightButtonOn()
{
	m_sight_front_btn->setStyleSheet(
        "color:rgb(255,255,255);"
        FONT_SIZE_20
        "padding-left:10px;"
        "background-color:rgba(0, 0, 0, 128);"
        "background-image:url(avm_qt_app_res/ui/new/Front-s-btn.png);");
}
void MainWidget::frontSightButtonPressed()
{
	int cur_view = getView();
	if( (cur_view == AVM_VIEW_CAMERA_FISH_FRONT)
		||(cur_view == AVM_VIEW_CAMERA_FISH_CLIP_FRONT) )
	{
		//now already highlight
		m_sight_front_btn->setStyleSheet(
        "color:rgb(255,255,255);"
        FONT_SIZE_20
        "padding-left:10px;"
        "background-color:rgba(0, 0, 0, 128);"
        "background-image:url(avm_qt_app_res/ui/new/Front-sp-btn.png);");
	}
	else
	{
		m_sight_front_btn->setStyleSheet(
        "color:rgb(255,255,255);"
        FONT_SIZE_20
        "padding-left:10px;"
        "background-color:rgba(0, 0, 0, 128);"
        "background-image:url(avm_qt_app_res/ui/new/Front-np-btn.png);");
	}
}
void MainWidget::frontSightButtonOff()
{
	m_sight_front_btn->setStyleSheet(
        "color:rgb(255,255,255);"
        FONT_SIZE_20
        "padding-left:10px;"
        "background-color:rgba(0, 0, 0, 128);"
        "background-image:url(avm_qt_app_res/ui/new/Front-n-btn.png);");
}

void MainWidget::frontFishSightButtonOn()
{
	m_sight_front_fish_btn->setStyleSheet(
        "color:rgb(255,255,255);"
        FONT_SIZE_20
        "padding-left:10px;"
        "background-color:rgba(0, 0, 0, 128);"
        "background-image:url(avm_qt_app_res/ui/new/pac_front_junction_s.png);");
}
void MainWidget::frontFishSightButtonPressed()
{
	int cur_view = getView();
	if(cur_view == AVM_VIEW_CAMERA_FISH_FRONT)
	{
		m_sight_front_fish_btn->setStyleSheet(
	        "color:rgb(255,255,255);"
	        FONT_SIZE_20
	        "padding-left:10px;"
	        "background-color:rgba(0, 0, 0, 128);"
	        "background-image:url(avm_qt_app_res/ui/new/pac_front_junction_sp.png);");
	}
	else
	{
		m_sight_front_fish_btn->setStyleSheet(
	        "color:rgb(255,255,255);"
	        FONT_SIZE_20
	        "padding-left:10px;"
	        "background-color:rgba(0, 0, 0, 128);"
	        "background-image:url(avm_qt_app_res/ui/new/pac_front_junction_np.png);");
	}
}
void MainWidget::frontFishSightButtonOff()
{
	m_sight_front_fish_btn->setStyleSheet(
        "color:rgb(255,255,255);"
        FONT_SIZE_20
        "padding-left:10px;"
        "background-color:rgba(0, 0, 0, 128);"
        "background-image:url(avm_qt_app_res/ui/new/pac_front_junction_n.png);");
}

void MainWidget::rearSightButtonOn()
{
	m_sight_rear_btn->setStyleSheet(
        "color:rgb(255,255,255);"
        FONT_SIZE_20
        "padding-left:10px;"
        "background-color:rgba(0, 0, 0, 128);"
        "background-image:url(avm_qt_app_res/ui/new/Rear-s-btn.png);");
}
void MainWidget::rearSightButtonPressed()
{
	int cur_view = getView();
	if( (cur_view == AVM_VIEW_CAMERA_FISH_REAR)
		||(cur_view == AVM_VIEW_CAMERA_FISH_CLIP_REAR) )
	{
		m_sight_rear_btn->setStyleSheet(
	        "color:rgb(255,255,255);"
	        FONT_SIZE_20
	        "padding-left:10px;"
	        "background-color:rgba(0, 0, 0, 128);"
	        "background-image:url(avm_qt_app_res/ui/new/Rear-sp-btn.png);");
	}
	else
	{
		m_sight_rear_btn->setStyleSheet(
	        "color:rgb(255,255,255);"
	        FONT_SIZE_20
	        "padding-left:10px;"
	        "background-color:rgba(0, 0, 0, 128);"
	        "background-image:url(avm_qt_app_res/ui/new/Rear-np-btn.png);");
	}
}
void MainWidget::rearSightButtonOff()
{
	m_sight_rear_btn->setStyleSheet(
        "color:rgb(255,255,255);"
        FONT_SIZE_20
        "padding-left:10px;"
        "background-color:rgba(0, 0, 0, 128);"
        "background-image:url(avm_qt_app_res/ui/new/Rear-n-btn.png);");
}

void MainWidget::rearFishSightButtonOn()
{
	m_sight_rear_fish_btn->setStyleSheet(
        "color:rgb(255,255,255);"
        FONT_SIZE_20
        "padding-left:10px;"
        "background-color:rgba(0, 0, 0, 128);"
        "background-image:url(avm_qt_app_res/ui/new/pac_rear_junction_s.png);");
}
void MainWidget::rearFishSightButtonPressed()
{
	int cur_view = getView();
	if(cur_view == AVM_VIEW_CAMERA_FISH_REAR)
	{
		m_sight_rear_fish_btn->setStyleSheet(
	        "color:rgb(255,255,255);"
	        FONT_SIZE_20
	        "padding-left:10px;"
	        "background-color:rgba(0, 0, 0, 128);"
	        "background-image:url(avm_qt_app_res/ui/new/pac_rear_junction_sp.png);");
	}
	else
	{
		m_sight_rear_fish_btn->setStyleSheet(
	        "color:rgb(255,255,255);"
	        FONT_SIZE_20
	        "padding-left:10px;"
	        "background-color:rgba(0, 0, 0, 128);"
	        "background-image:url(avm_qt_app_res/ui/new/pac_rear_junction_np.png);");
	}
}
void MainWidget::rearFishSightButtonOff()
{
	m_sight_rear_fish_btn->setStyleSheet(
        "color:rgb(255,255,255);"
        FONT_SIZE_20
        "padding-left:10px;"
        "background-color:rgba(0, 0, 0, 128);"
        "background-image:url(avm_qt_app_res/ui/new/pac_rear_junction_n.png);");
}

void MainWidget::sideSightButtonOn()
{
	m_sight_side_btn->setStyleSheet(
        "color:rgb(255,255,255);"
        FONT_SIZE_20
        "padding-left:10px;"
        "background-color:rgba(0, 0, 0, 128);"
        "background-image:url(avm_qt_app_res/ui/new/Side-s-btn.png);");
}
void MainWidget::sideSightButtonPressed()
{
	int cur_view = getView();
	if( (cur_view == AVM_VIEW_CAMERA_LEFTRIGHT)
		||(cur_view == AVM_VIEW_CAMERA_LEFTRIGHT_CPY) )
	{
		m_sight_side_btn->setStyleSheet(
	        "color:rgb(255,255,255);"
	        FONT_SIZE_20
	        "padding-left:10px;"
	        "background-color:rgba(0, 0, 0, 128);"
	        "background-image:url(avm_qt_app_res/ui/new/Side-sp-btn.png);");
	}
	else
	{
		m_sight_side_btn->setStyleSheet(
	        "color:rgb(255,255,255);"
	        FONT_SIZE_20
	        "padding-left:10px;"
	        "background-color:rgba(0, 0, 0, 128);"
	        "background-image:url(avm_qt_app_res/ui/new/Side-np-btn.png);");
	}
}
void MainWidget::sideSightButtonOff()
{
	m_sight_side_btn->setStyleSheet(
        "color:rgb(255,255,255);"
        FONT_SIZE_20
        "padding-left:10px;"
        "background-color:rgba(0, 0, 0, 128);"
        "background-image:url(avm_qt_app_res/ui/new/Side-n-btn.png);");
}

void MainWidget::d3SightButtonOn()
{
	m_sight_3d_btn->setStyleSheet(
        "color:rgb(255,255,255);"
        FONT_SIZE_20
        "padding-left:10px;"
        "background-color:rgba(0, 0, 0, 128);"
        "background-image:url(avm_qt_app_res/ui/new/d3-s-btn.png);");
}
void MainWidget::d3SightButtonPressed()
{
	int cur_view = getView();
	if(1 == m_surroundWidget->checkInPure3dView(cur_view))
	{
		m_sight_3d_btn->setStyleSheet(
	        "color:rgb(255,255,255);"
	        FONT_SIZE_20
	        "padding-left:10px;"
	        "background-color:rgba(0, 0, 0, 128);"
	        "background-image:url(avm_qt_app_res/ui/new/d3-sp-btn.png);");
	}
	else
	{
		m_sight_3d_btn->setStyleSheet(
	        "color:rgb(255,255,255);"
	        FONT_SIZE_20
	        "padding-left:10px;"
	        "background-color:rgba(0, 0, 0, 128);"
	        "background-image:url(avm_qt_app_res/ui/new/d3-np-btn.png);");
	}
}
void MainWidget::d3SightButtonOff()
{
	m_sight_3d_btn->setStyleSheet(
        "color:rgb(255,255,255);"
        FONT_SIZE_20
        "padding-left:10px;"
        "background-color:rgba(0, 0, 0, 128);"
        "background-image:url(avm_qt_app_res/ui/new/d3-n-btn.png);");
}

#define D3_DIR_BTN_CIRCLE_EX    (138)
#define D3_DIR_BTN_CIRCLE_IN    (62) 
void MainWidget::d3DirButtonInit()
{
	m_3d_dir_btn->Init(D3_DIR_BTN_CIRCLE_IN/2, D3_DIR_BTN_CIRCLE_EX/2, 4, QString(""));

	const char *str[7] = {
        "avm_qt_app_res/ui/new/d3_dir_btn_n.png",
        "",
        "",
        "avm_qt_app_res/ui/new/d3_dir_btn_right.png",
        "avm_qt_app_res/ui/new/d3_dir_btn_front.png",
        "avm_qt_app_res/ui/new/d3_dir_btn_left.png",
        "avm_qt_app_res/ui/new/d3_dir_btn_rear.png",
    };
    // uid is avm_view enum value
    int uid[7] = {
        AVM_VIEW_UNKNOW,
        -1,
        -1,
        AVM_VIEW_3D_RIGHT_CENTER,
        AVM_VIEW_3D_FRONT,
        AVM_VIEW_3D_LEFT_CENTER,
        AVM_VIEW_3D_REAR,
    };
    for (int i = 0; i < 7; i++)
    {
        m_3d_dir_btn->AddImage(i, uid[i], QString(str[i]));
    }
}
void MainWidget::d3DirButtonNone()
{
	m_3d_dir_btn->setStyleSheet(
        "color:rgb(255,255,255);"
        FONT_SIZE_20
        "padding-left:10px;"
        "background-color:rgba(0, 0, 0, 128);"
        "background-image:url(avm_qt_app_res/ui/new/d3_dir_btn_n.png);");
}
void MainWidget::d3DirButtonFront()
{
	m_3d_dir_btn->setStyleSheet(
        "color:rgb(255,255,255);"
        FONT_SIZE_20
        "padding-left:10px;"
        "background-color:rgba(0, 0, 0, 128);"
        "background-image:url(avm_qt_app_res/ui/new/d3_dir_btn_front.png);");
}
void MainWidget::d3DirButtonRear()
{
	m_3d_dir_btn->setStyleSheet(
        "color:rgb(255,255,255);"
        FONT_SIZE_20
        "padding-left:10px;"
        "background-color:rgba(0, 0, 0, 128);"
        "background-image:url(avm_qt_app_res/ui/new/d3_dir_btn_rear.png);");
}
void MainWidget::d3DirButtonLeft()
{
	m_3d_dir_btn->setStyleSheet(
        "color:rgb(255,255,255);"
        FONT_SIZE_20
        "padding-left:10px;"
        "background-color:rgba(0, 0, 0, 128);"
        "background-image:url(avm_qt_app_res/ui/new/d3_dir_btn_left.png);");
}
void MainWidget::d3DirButtonRight()
{
	m_3d_dir_btn->setStyleSheet(
        "color:rgb(255,255,255);"
        FONT_SIZE_20
        "padding-left:10px;"
        "background-color:rgba(0, 0, 0, 128);"
        "background-image:url(avm_qt_app_res/ui/new/d3_dir_btn_right.png);");
}
#endif


void MainWidget::debugWidgetLayout()
{
#define DW_LYT_X_DET    (50)
#define DW_LYT_Y_DET    (50)
#define DW_BTN_SIDE_LEN	(80)
#define DW_LYT_WIDTH    (LAYOUT_SURR_WIDE_WIDTH-DW_LYT_X_DET*2)
#define DW_LYT_HEIGHT   (LAYOUT_TOP_HEIGHT-DW_LYT_Y_DET*2)

    m_p_dbg_wgt = new DebugWidget(this);
    m_p_dbg_wgt->setGeometry(DW_LYT_X_DET, DW_LYT_Y_DET, DW_LYT_WIDTH, DW_LYT_HEIGHT);
    m_p_dbg_wgt->hide();
    m_p_dbg_wgt->setStyleSheet("font-size:16px;");
    //m_p_dbg_wgt->setSizeGripEnabled(true);
    m_p_dbg_wgt->layout();

    m_dbg_state = 0;
    m_dbg_timer = new QTimer(this);
    m_dbg_timer->setSingleShot(true);
    connect(m_dbg_timer, SIGNAL(timeout()), this, SLOT(debugTimeCbFunc()));    
}


void MainWidget::debugWidgetShowHide(int is_show)
{
    if (is_show)
    {
        m_p_dbg_wgt->show();
    }
    else
    {
        m_p_dbg_wgt->hide();
    }
}


void MainWidget::debugTimeCbFunc()
{
    m_dbg_state = 0;
}


void MainWidget::debugStateLogic(int pos_x, int pos_y)
{
    if (0 == m_dbg_state)
    {
        if ((pos_x > 0) && (pos_x < DW_BTN_SIDE_LEN) 
            && (pos_y > LAYOUT_TOP_HEIGHT-DW_BTN_SIDE_LEN) && (pos_y < LAYOUT_TOP_HEIGHT))
        {
            m_dbg_state = 1;
            m_dbg_timer->start(2*1000);
        }
        else
        {
            m_dbg_state = 0;
        }
    }
    else if (1 == m_dbg_state)
    {
        if ((pos_x > LAYOUT_TOTAL_WIDTH-DW_BTN_SIDE_LEN) && (pos_x < LAYOUT_TOTAL_WIDTH) 
            && (pos_y > LAYOUT_TOP_HEIGHT-DW_BTN_SIDE_LEN) && (pos_y < LAYOUT_TOP_HEIGHT))
        {
            m_dbg_state = 2;
            m_dbg_timer->start(2*1000);
        }
        else
        {
            m_dbg_state = 0;
        }
    }
    else if (2 == m_dbg_state)
    {
        if ((pos_x > 0) && (pos_x < DW_BTN_SIDE_LEN) 
            && (pos_y > LAYOUT_TOP_HEIGHT-DW_BTN_SIDE_LEN) && (pos_y < LAYOUT_TOP_HEIGHT))
        {
            m_dbg_state = 3;
            m_dbg_timer->start(2*1000);
        }
        else
        {
            m_dbg_state = 0;
        }        
    } 
    else if (3 == m_dbg_state)
    {
        if ((pos_x > LAYOUT_TOTAL_WIDTH-DW_BTN_SIDE_LEN) && (pos_x < LAYOUT_TOTAL_WIDTH) 
            && (pos_y > LAYOUT_TOP_HEIGHT-DW_BTN_SIDE_LEN) && (pos_y < LAYOUT_TOP_HEIGHT))
        {
            m_dbg_state = 4;
            m_dbg_timer->start(2*1000);
        } 
        else
        {
            m_dbg_state = 0;
        }
    }  
    else if (4 == m_dbg_state)
    {
        if ((pos_x > 0) && (pos_x < DW_BTN_SIDE_LEN) 
            && (pos_y > LAYOUT_TOP_HEIGHT-DW_BTN_SIDE_LEN) && (pos_y < LAYOUT_TOP_HEIGHT))
        {
            m_dbg_state = 5;
            m_dbg_timer->start(2*1000);
        }
        else
        {
            m_dbg_state = 0;
        }
    } 
    else if (5 == m_dbg_state)
    {
        if ((pos_x > LAYOUT_TOTAL_WIDTH-DW_BTN_SIDE_LEN) && (pos_x < LAYOUT_TOTAL_WIDTH) 
            && (pos_y > LAYOUT_TOP_HEIGHT-DW_BTN_SIDE_LEN) && (pos_y < LAYOUT_TOP_HEIGHT))
        {
            m_dbg_state = 0;
            debugWidgetShowHide(1);
        }  
        else
        {
            m_dbg_state = 0;
        }
    }     
}


int MainWidget::buttonStateInit()
{
    // first set val to default, then trigger button to set ui and state to dst

    // set default manual view
    m_style = -1;

    // default assist line is on
    m_isAssistOn = 0;
    assistButtonReleased();

    // default radio is on, so mute is off
    m_isRadioOn = 0;
    radioButtonReleased();

    // default radar is on
    m_isRadarOn = 0;
    radarButtonReleased();

    return 0;
}


void MainWidget::avmStateInit()
{
    if (m_surroundWidget != NULL)
    {
        // set view
        viewChgLogic(ACHG_ACTION_NULL);
        m_style = 1; //default 2d front
		change_view(1, AVM_VIEW_CAMERA_FISH_CLIP_FRONT); //default 2d front
		syncSightImageState(AVM_VIEW_CAMERA_FISH_CLIP_FRONT);

		m_itemsDisplayTimer->start(POPUP_DISAPPEAR_DELAY);
		
        //
        m_surroundWidget->setRadar(m_radar_bits[0], m_radar_bits[1]);
        m_surroundWidget->setAssistLine(m_assist_angle);
    }
    if (m_summWidget != NULL)
    {
        m_summWidget->setRadar(m_radar_bits[0], m_radar_bits[1]);
        m_summWidget->setAssistLine(m_assist_angle);
    }
}

/**
* control func. config signal and slots
*/
void MainWidget::control()
{
    int re;

    // create reminder timer, to change reminder text when click 2d/3d button
    m_reminder_timer = new QTimer(this);
    m_reminder_timer->setSingleShot(true);
    connect(m_reminder_timer, SIGNAL(timeout()), this, SLOT(changeRemendText()));

	m_itemsDisplayTimer = new QTimer(this);
	m_itemsDisplayTimer->setSingleShot(true);
	connect(m_itemsDisplayTimer, SIGNAL(timeout()), this, SLOT(itemsDisplayTimeout()));

    // get comu_cmd module's ext mem address, and set button's init state
    re = buttonStateInit();
    if (re != 0)
    {
        AVM_ERR("MainWidget::control. buttonStateInit faild\n");
        return;
    }
    // avm state init
    avmStateInit();

    // create a work thread to receive cmd from avm_comu proc
    m_cmdRecver = new CmdRecver();
    connect(m_cmdRecver, SIGNAL(rotateView(int)), this, SLOT(rotateView(int)));
    connect(m_cmdRecver, SIGNAL(changeView(int)), this, SLOT(setView(int)));
    connect(m_cmdRecver, SIGNAL(setRadar(unsigned int, unsigned int)), this, SLOT(setRadar(unsigned int, unsigned int)));
    connect(m_cmdRecver, SIGNAL(clrRadar(unsigned int, unsigned int)), this, SLOT(clrRadar(unsigned int, unsigned int)));
    connect(m_cmdRecver, SIGNAL(setAssistAngle(float)), this, SLOT(setAssistLine(float)));
    connect(m_cmdRecver, SIGNAL(getFps()), this, SLOT(prtFps()));
    connect(m_cmdRecver, SIGNAL(setSpeed(float)), this, SLOT(setCarSpeed(float)));
    connect(m_cmdRecver, SIGNAL(setTurnSignal(int)), this, SLOT(setTurnSignal(int)));
    connect(m_cmdRecver, SIGNAL(setGoBack(int)), this, SLOT(setGoBack(int)));
    connect(m_cmdRecver, SIGNAL(setDoor(int, int, int, int, int, int)), 
        this, SLOT(setDoor(int, int, int, int, int, int)));
    connect(m_cmdRecver, SIGNAL(shutDown(int)), this, SLOT(shutDown(int)));
    connect(m_cmdRecver, SIGNAL(setCamPwr(int)), this, SLOT(setCamPwr(int)));
    connect(m_cmdRecver, SIGNAL(setErrVersion(int)), this, SLOT(setErrVersion(int)));    
    connect(m_cmdRecver, SIGNAL(setEmFlahers(int)), this, SLOT(setEmFlahers(int)));
    connect(m_cmdRecver, SIGNAL(setIhuMute(int, int)), this, SLOT(setIhuMute(int, int)));
    connect(m_cmdRecver, SIGNAL(setMirrowFold(int, int)), this, SLOT(setMirrowFold(int, int)));

	//for unit test
	//connect(m_cmdRecver, SIGNAL(release2DButton()), this, SLOT(d2ButtonReleased()));
	//connect(m_cmdRecver, SIGNAL(release2DPie(int)), this, SLOT(d2PieReleased(int)));
	//connect(m_cmdRecver, SIGNAL(release3DButton()), this, SLOT(d3ButtonReleased()));
	//connect(m_cmdRecver, SIGNAL(release3DPie(int)), this, SLOT(d3PieReleased(int)));
	connect(m_cmdRecver, SIGNAL(releaseAssist()), this, SLOT(assistButtonReleased()));
	connect(m_cmdRecver, SIGNAL(releaseRadar()), this, SLOT(radarButtonReleased()));
	connect(m_cmdRecver, SIGNAL(releaseRadio()), this, SLOT(radioButtonReleased()));
	connect(m_cmdRecver, SIGNAL(setWamDisplayStatus(int)), this, SLOT(wamDisplayStatusChanged(int)));
    
    m_cmdRecvThread = new QThread;
    m_cmdRecvThread->setObjectName("cmd_recv");
    m_cmdRecver->moveToThread(m_cmdRecvThread);
    connect(m_cmdRecvThread,  SIGNAL(started()), m_cmdRecver, SLOT(cmdRecvProcess()));
    m_cmdRecvThread->start();

#if GLOBAL_RUN_ENV_DESKTOP == 0
    if (0 == m_is_use_bmp)
    {
        // create a worker thread, send signal to videsyncgetter class to start video sync get process
        // worker thread start to wiat sync video when we run m_workerThread->start()
        m_workerThread = new QThread;
        m_workerThread->setObjectName("video_worker");
        SyncVideoGetter *syncVideoGetter = new SyncVideoGetter(1);
        syncVideoGetter->moveToThread(m_workerThread);

        connect(m_workerThread, SIGNAL(started()), syncVideoGetter, SLOT(fetchSyncVideoProcess()));

        // when worker thread get an synced frame
        if (m_surroundWidget != NULL)
        {
            connect(syncVideoGetter, SIGNAL(finishGetOneFrame()), m_surroundWidget, SLOT(stitchUpdate()), Qt::DirectConnection);
        }
        if (m_summWidget != NULL)
        {
            connect(syncVideoGetter, SIGNAL(finishGetOneFrame()), m_summWidget, SLOT(stitchUpdate()), Qt::DirectConnection);
        }

        // start receive syncvideo
        m_workerThread->start();
    }
    else
    {
        // in desktop model, we start a timer to trigger stitchupdate
        // trigger intv is 40ms
        m_checkGlTimer = new QTimer;
        if (m_surroundWidget != NULL)
        {
            connect(m_checkGlTimer, SIGNAL(timeout()), m_surroundWidget, SLOT(stitchUpdateTime()), Qt::DirectConnection);
        }
        if (m_summWidget != NULL)
        {
            connect(m_checkGlTimer, SIGNAL(timeout()), m_summWidget, SLOT(stitchUpdateTime()), Qt::DirectConnection);
        }
        m_checkGlTimer->start(40); 
    }
#else
    // in desktop model, we start a timer to trigger stitchupdate
    // trigger intv is 40ms
    m_checkGlTimer = new QTimer;
    if (m_surroundWidget != NULL)
    {
        connect(m_checkGlTimer, SIGNAL(timeout()), m_surroundWidget, SLOT(stitchUpdateTime()), Qt::DirectConnection);
    }
    if (m_summWidget != NULL)
    {
        connect(m_checkGlTimer, SIGNAL(timeout()), m_summWidget, SLOT(stitchUpdateTime()), Qt::DirectConnection);
    }
    m_checkGlTimer->start(40);
#endif

    // check if init faild
    if (get_main_init_faild())
    {
        alertWindowSH(0, 1);
    }
    // start a timer to check video is vaild, if video is not vaild, show error image
    m_checkVideoTimer = new QTimer;
    connect(m_checkVideoTimer, SIGNAL(timeout()), this, SLOT(checkVideoVaildProcess()));
    m_checkVideoTimer->start(1000);
}


void MainWidget::setDynDebugLabel()
{

    int i = 0;
    unsigned int link_status = 0x0;
    unsigned int sync_status = 0x0;
    int cam_arr[4] = {0};   // rear front right left
    QString line;
     
    videocap_get_cam_link_status(&link_status);
    videocap_get_cam_sync_status(&sync_status);
    
    // check max9286 read or write error
    if (link_status & 0xF0000)
    {
        line.append(custstr_get_err_dyn_info(3, 3, 3, 3));
        m_debug_error_info->setText(line);
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
        m_debug_error_info->setText(line);
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
    m_debug_error_info->setText(line);
    
}


void MainWidget::setShmCamStatus()
{
    static int s_init_flag = 0;
    static int s_trans_arr[4] = {0};

    if (0 == s_init_flag)
    {
        s_init_flag = 1;
        s_trans_arr[0] = ini_get_cam_right();
        s_trans_arr[1] = ini_get_cam_front();
        s_trans_arr[2] = ini_get_cam_left();
        s_trans_arr[3] = ini_get_cam_rear(); 
    }

    int i = 0;
    int is_error = 0;
    int is_error_arr[4] = {0};
    unsigned int link_status = 0x0;
    unsigned int sync_status = 0x0;

    videocap_get_cam_link_status(&link_status);
    videocap_get_cam_sync_status(&sync_status);

    // check disconnect
    if (link_status != 0x0F)
    {
        for (i = 0; i < 4; i++)
        {
            if (0x00 == (link_status & (0x01 << i)))
            {
                is_error_arr[i] = 0x1;
            }
        }

        is_error = 1;
        shm_set_cam_status(is_error, 
            is_error_arr[s_trans_arr[0]], 
            is_error_arr[s_trans_arr[1]], 
            is_error_arr[s_trans_arr[2]],
            is_error_arr[s_trans_arr[3]]);
        return;
    }
    
    // check bad
    if (sync_status != 0x0F)
    {
        for (i = 0; i < 4; i++)
        {
            if (0x00 == (sync_status & (0x01 << i)))
            {
                is_error_arr[i] |= (0x1 << 4);
            }
        }
        is_error = 1;
    }

    shm_set_cam_status(is_error, 
        is_error_arr[s_trans_arr[0]], 
        is_error_arr[s_trans_arr[1]], 
        is_error_arr[s_trans_arr[2]],
        is_error_arr[s_trans_arr[3]]);
}


/**< rotate current view. only for auto and 3d views */
void MainWidget::rotateView(int det_x)
{
    m_surroundWidget->rotateView(det_x);
}

/**< viewEnum should be AVM_VIEW_E */
void MainWidget::setView(int viewEnum)
{
    change_view(1, viewEnum);
}


void MainWidget::setRadar(unsigned mask, unsigned int mask2)
{
    m_radar_bits[0] = mask;
    m_radar_bits[1] = mask2;
    
    m_surroundWidget->setRadar(mask, mask2);
    m_summWidget->setRadar(mask, mask2);

#if 0
    int old_view, new_view;
    old_view = m_surroundWidget->getView();
    MainWidget::viewChgLogic(ACHG_ACTION_RADAR);
    new_view = m_surroundWidget->getView();
    if (old_view != new_view)
    {
        checkAndClosePie();
    }
#endif
}

/**< clr radar block by bit mask. --this func is deprecated-- */
void MainWidget::clrRadar(unsigned mask, unsigned int mask2)
{
    mask = mask;
    mask2 = mask2;
}

/**< set assist line by angle deg. eg. 10 for right deg 10. -10 means left deg 10 */
void MainWidget::setAssistLine(float angle)
{
    m_surroundWidget->setAssistLine(angle);
    m_summWidget->setAssistLine(angle);
}

/**< print current paintGL fsp */
void MainWidget::prtFps()
{
    m_surroundWidget->prtFps();
}

/**< set current car speed */
void MainWidget::setCarSpeed(float speed)
{
    m_car_speed = speed;
    
    int speed_int = (int)speed;
    if (m_go_back >= 0)
    {
        m_surroundWidget->setSpeed(speed_int);
        m_summWidget->setSpeed(speed_int);
    }
    else
    {
        m_surroundWidget->setSpeed(-1*speed_int);
        m_summWidget->setSpeed(-1*speed_int);
    }
}


/**< -1 left, 0 no turn, 1 turn right */
void MainWidget::setTurnSignal(int turn)
{
    m_turn_signal = turn;
    
    int old_view, new_view;
    old_view = m_surroundWidget->getView();
    if (-1 == turn)
    {
        viewChgLogic(ACHG_ACTION_TURN_L);
    }
    else if (0 == turn)
    {
        viewChgLogic(ACHG_ACTION_TURN_N);
    }
    else
    {
        viewChgLogic(ACHG_ACTION_TURN_R);
    }
    new_view = m_surroundWidget->getView();
    if (old_view != new_view)
    {
        checkAndClosePie();
    }

    m_surroundWidget->setTurnSignal(turn);
    m_summWidget->setTurnSignal(turn);   
}


void MainWidget::setEmFlahers(int is_on)
{
    m_surroundWidget->setEmFlahers(is_on);
    m_summWidget->setEmFlahers(is_on);
}


/**< 0 stop, 1 go, -1 back */
void MainWidget::setGoBack(int go_back)
{
    m_go_back = go_back;
    
    if (go_back >= 0)
    {
        m_surroundWidget->setCarDirection(1);
        m_summWidget->setCarDirection(1);
    }
    else
    {
        m_surroundWidget->setCarDirection(0);
        m_summWidget->setCarDirection(0);
    }

    int old_view, new_view;
    setCarSpeed(m_car_speed);
    old_view = m_surroundWidget->getView();
    if (go_back > 0)
    {
        viewChgLogic(ACHG_ACTION_GO);
    }
    else if (0 == go_back)
    {
        viewChgLogic(ACHG_ACTION_STOP);
    }
    else
    {
        viewChgLogic(ACHG_ACTION_BACK);
    }
    new_view = m_surroundWidget->getView();
    if (old_view != new_view)
    {
        checkAndClosePie();
    }
}

/**< 0 close, 1 open */
void MainWidget::setDoor(int front_left, int front_right, int rear_left, int rear_right,
    int front_cover, int rear_cover)
{
    int hide_right = 0;
    int hide_front = 0;
    int hide_left = 0;
    int hide_rear = 0;

    m_front_left_door_open = front_left;
    m_front_right_door_open = front_right;
    m_rear_left_door_open = rear_left;
    m_rear_right_door_open = rear_right;

    // 
    if (m_front_left_door_open || m_rear_left_door_open || m_left_mirrow_fold)
    {
        hide_left = 1;
    }
    if (m_front_right_door_open || m_rear_right_door_open || m_right_mirrow_fold)
    {
        hide_right = 1;
    }
    if (rear_cover)
    {
        hide_rear = 1;
    }
    front_cover = front_cover;

    // door  mirrow     
    // 1     1       -> hide carton
    // 1     0       -> hide carton
    // 0     1       -> hide
    // 0     0       -> not hide
    if (m_surroundWidget != NULL)
    {
        m_surroundWidget->hideCamera(hide_right, hide_front, hide_left, hide_rear);
        m_surroundWidget->setDoor(front_left, front_right, rear_left, rear_right);
    }
    if (m_summWidget != NULL)
    {
        m_summWidget->hideCamera(hide_right, hide_front, hide_left, hide_rear);
        m_summWidget->setDoor(front_left, front_right, rear_left, rear_right);
    }    
}


void MainWidget::shutDown(int is_shutdown)
{
    if (is_shutdown == MainWidget::m_isShutDown)
    {
        return;
    }
    
    if (is_shutdown)
    {
        alertWindowSH(2, 1);
        MainWidget::m_isShutDown = 1;
    }
    else
    {
        alertWindowSH(2, 0);
        MainWidget::m_isShutDown = 0; 
    }
}


void MainWidget::setCamPwr(int cam_pwr)
{
    static int s_lst_cam_pwr = -1;

    // while camera power is switch on, set a flag for checkVaildProcess
    // not show error image immediately, have a delay
    if ((0 == s_lst_cam_pwr) && (1 == cam_pwr))
    {
        m_check_vaild_bypass_cnt = 10;
        AVM_LOG("MainWidget::setCamPwr. set bypass=%d\n", m_check_vaild_bypass_cnt);
    }
    
    if (cam_pwr)
    {
        shutDown(0);
        s_lst_cam_pwr = 1;
    }
    else
    {
        shutDown(1);
        s_lst_cam_pwr = 0;
    }

#if GLOBAL_RUN_ENV_DESKTOP == 0
    videocap_pause_process(cam_pwr);
#else
    AVM_ERR("MainWidget::setCamPwr. cam_pwr=%d, it's pc env!\n", cam_pwr);
#endif
}


void MainWidget::setErrVersion(int is_error)
{
    if (is_error)
    {
        alertWindowSH(3, 1);
    }
    else
    {
        alertWindowSH(3, 0);
    }    
}


void MainWidget::wamDisplayStatusChanged(int updatedStatus)
{
	// cmd_wam_dispaly_s, 1 for showing, 0 for not showing
	if(0 == updatedStatus)
	{
		checkClickVaildProc(1);
	}
}


void MainWidget::setIhuMute(int mute_status, int is_rescissible)
{
    AVM_LOG("MainWidget::setIhuMute. mute_status=%d, is_rescissible=%d\n",
        mute_status, is_rescissible);
    radioButtonExternChg(1, mute_status, is_rescissible);
}


void MainWidget::setMirrowFold(int is_left_fold, int is_right_fold)
{
    AVM_LOG("MainWidget::setMirrowFold. is_left_fold=%d, is_right_fold=%d\n",
        is_left_fold, is_right_fold);

    int hide_right = 0;
    int hide_left = 0;

    m_left_mirrow_fold = is_left_fold;
    m_right_mirrow_fold = is_right_fold;
    
    if (m_left_mirrow_fold || m_front_left_door_open || m_rear_left_door_open)
    {
        hide_left = 1;
    }
    if (m_right_mirrow_fold || m_front_right_door_open || m_rear_right_door_open)
    {
        hide_right = 1;
    }

    if (m_surroundWidget != NULL)
    {
        m_surroundWidget->hideCamera(hide_right, -1, hide_left, -1);
    }
    if (m_summWidget != NULL)
    {
        m_summWidget->hideCamera(hide_right, -1, hide_left, -1);
        m_summWidget->setMirrowFold(is_left_fold, is_right_fold);
    }
}


int MainWidget::getShutSownStatus()
{
    return MainWidget::m_isShutDown;
}


int MainWidget::getView()
{
    if (m_p_this != NULL)
    {
        return m_p_this->m_surroundWidget->getView();
    }
    else
    {
        return -1;
    }
}


// 0 init faild (this err is set before widget init.)
// 1 camera error
// 2 speed is exceed max speed
// 3 mcu and app version not match
void MainWidget::alertWindowSH(int alert_type, int is_on)
{
    static int s_err_init = 0;
    static int s_err_cam = 0;       
    static int s_err_speed = 0; 
    static int s_err_version = 0;

    if ((0 == alert_type) && (is_on != s_err_init))
    {
        s_err_init = is_on;
    }
    else if ((1 == alert_type) && (is_on != s_err_cam))
    {
        s_err_cam = is_on;
    }
    else if ((2 == alert_type) && (is_on != s_err_speed))
    {
        s_err_speed = is_on;
    }
    else if ((3 == alert_type) && (is_on != s_err_version))
    {
        s_err_version = is_on;
    }
    else
    {
        return;
    }

    if (s_err_init)
    {
        m_alert_bottom_mask->show();
        m_debug_error_info_title->hide();
        m_debug_error_info->hide();
        m_debug_version_info->show();
        m_alert_window_body->show();
        m_alert_window_body->setText(custstr_get_init_alert_str(get_main_init_faild()));
        m_alert_window_btn->show();
        shm_set_err_screen(1, 0); 
    }
    else if (s_err_version)
    {
        m_alert_bottom_mask->show();
        m_debug_error_info_title->hide();
        m_debug_error_info->hide();
        m_debug_version_info->show();
        m_alert_window_body->show();
        m_alert_window_body->setText(custstr_get_version_alert_str());
        m_alert_window_btn->show(); 
        shm_set_err_screen(1, 3);
    }
    else if (s_err_cam)
    {
        m_alert_bottom_mask->show();
        m_debug_error_info_title->show();
        m_debug_error_info->show();
        m_debug_version_info->show();
        m_alert_window_body->show();
        m_alert_window_body->setText(custstr_get_cam_err_alert_str());
        m_alert_window_btn->show(); 
        shm_set_err_screen(1, 1);
    }
    else if (s_err_speed)
    {
        m_alert_bottom_mask->show();
        m_debug_error_info_title->hide();
        m_debug_error_info->hide();
        m_debug_version_info->show();
        m_alert_window_body->show();
        m_alert_window_body->setText(custstr_get_speed_alert_str());
        m_alert_window_btn->show();
        shm_set_err_screen(1, 2);
    }
    else
    {
        m_alert_bottom_mask->hide();
        m_debug_error_info_title->hide();
        m_debug_error_info->hide();
        m_debug_version_info->hide();
        m_alert_window_body->hide();
        m_alert_window_btn->hide(); 
        shm_set_err_screen(0, 0);
    }   
}

void MainWidget::checkVideoVaildProcess()
{
    int isOk = 0;
    videocap_get_video_ok(&isOk, NULL, NULL);

    // if stitch is shutdown, just return without checking video status change
    if (MainWidget::m_isShutDown)
    {
        return;
    }
    // delay check video status change if have bypass cnt
    if (m_check_vaild_bypass_cnt > 0)
    {
        m_check_vaild_bypass_cnt--;
        return;
    }

    // if use bmp file as camera input, do not check camera vaild
    if (0 == m_is_use_bmp)
    {
        if (0 == isOk)
        {
            alertWindowSH(1, 1);
            setDynDebugLabel();
            setShmCamStatus();
        }
        else
        {
            alertWindowSH(1, 0);
            setShmCamStatus();
        }
    }
    else
    {
        alertWindowSH(1, 0);
    }
}


int MainWidget::checkClickVaildProc(int is_do_proc)
{
    int vaild = 1;
	return 1; //no use for LG UI

#if 0 //LG UI
    if (m_isChgStyleOn)
    {
        if (is_do_proc)
        {
            changeStyleButtonOff(m_style);
            m_isChgStyleOn = 0;
            changeStyleMenuShowHide(0);
        }
        vaild = 0;
    }
#endif
    if (m_isSettimgOn)
    {
        if (is_do_proc)
        {
            m_isSettimgOn = 0;

            settimgButtonOff();
            
            m_assistLabel->hide();
            m_radarLabel->hide();
            //m_radioLabel->hide();
            m_turnLinkLabel->hide();
        }
        vaild = 0;
    }

#if 0 //LG UI
    if (m_isD2PieOn)
    {
        if (is_do_proc)
        {
            m_isD2PieOn = 0;
            m_2DPieLabel->ShowHide(0);
			m_pie_timer->stop();
        }
        vaild = 0;
    }
    if (m_isD3PieOn)
    {
        if (is_do_proc)
        {
            m_isD3PieOn = 0;
            m_3DPieLabel->ShowHide(0);
			m_pie_timer->stop();
        }
        vaild = 0;
    }    
#endif
    return vaild;
}


void MainWidget::checkAndClosePie()
{
#if 0 //LG UI
    if (m_isD2PieOn)
    {
        m_isD2PieOn = 0;
        m_2DPieLabel->ShowHide(0);
    }
    if (m_isD3PieOn)
    {
        m_isD3PieOn = 0;
        m_3DPieLabel->ShowHide(0);
    }
#endif
}


void MainWidget::changeRemendText()
{
    if (m_reminder_text_state != 0)
    {
        m_reminder_text_state = 0;
        m_reminder_text->setText(custstr_get_careful_str());
    }
}


#if 0 //LG UI
void MainWidget::autoButtonInit()
{
    if (NULL == m_autoLabel)
    {
        return;
    }
    //m_autoLabel->setText("AUTO");
    m_autoLabel->setStyleSheet(
        "color:rgb(255,255,255);"
        FONT_SIZE_20
        "background-color:rgba(0, 0, 0, 128);"
        "qproperty-alignment: AlignCenter;"
        "background-image:url(avm_qt_app_res/ui/BTN_AUTO unselected.png);");
}


void MainWidget::autoButtonOn()
{
    if (NULL == m_autoLabel)
    {
        return;
    }
    m_autoLabel->setStyleSheet(
        "color:rgb(240,240,240);"
        FONT_SIZE_20
        "background-color:rgba(0, 0, 0, 160);"
        "qproperty-alignment: AlignCenter;"
        "background-image:url(avm_qt_app_res/ui/BTN_AUTO selected.png);");
}


void MainWidget::autoButtonOff()
{
    if (NULL == m_autoLabel)
    {
        return;
    }
    m_autoLabel->setStyleSheet(
        "color:rgb(255,255,255);"
        FONT_SIZE_20
        "background-color:rgba(0, 0, 0, 128);"
        "qproperty-alignment: AlignCenter;"
        "background-image:url(avm_qt_app_res/ui/BTN_AUTO unselected.png);");
}


void MainWidget::autoButtonClicked()
{
    if (0 == m_style)
    {
        //autoButtonOff();
    }
    else
    {
        autoButtonOn();
    }
}


void MainWidget::autoButtonReleased()
{
    int old_style = m_style;
    m_style = 0;
    
    // change ui
    d2ButtonOff();
    d3ButtonOff();
    autoButtonOn();
    changeStyleButtonOff(m_style);
    //m_chgStyleLabel->setText("AUTO");

    // hide
    changeStyleMenuShowHide(0);
    m_isChgStyleOn = 0;

    // do action
    if (old_style != 0)
    {
        shm_set_is_manual(0);
        viewChgLogic(ACHG_ACTION_NULL);
    }
}


void MainWidget::d2ButtonInit()
{
    //m_2DLabel->setText("2D");
    m_2DLabel->setStyleSheet(
        "color:rgb(255,255,255);"
        FONT_SIZE_20
        "background-color:rgba(0, 0, 0, 128);"
        "qproperty-alignment: AlignCenter;"
        "background-image:url(avm_qt_app_res/ui/BTN_2D_unselected.png);");
}


void MainWidget::d2ButtonOn()
{
    m_2DLabel->setStyleSheet(
        "color:rgb(240,240,240);"
        FONT_SIZE_20
        "background-color:rgba(0, 0, 0, 160);"
        "qproperty-alignment: AlignCenter;"
        "background-image:url(avm_qt_app_res/ui/BTN_2D_selected.png);");
}


void MainWidget::d2ButtonOff()
{
    m_2DLabel->setStyleSheet(
        "color:rgb(255,255,255);"
        FONT_SIZE_20
        "background-color:rgba(0, 0, 0, 128);"
        "qproperty-alignment: AlignCenter;"
        "background-image:url(avm_qt_app_res/ui/BTN_2D_unselected.png);");
}


void MainWidget::d2ButtonClicked()
{
    if (1 == m_style)
    {
        //d2ButtonOff();
    }
    else
    {
        d2ButtonOn();
    }
}


void MainWidget::d2ButtonReleased()
{
    int old_style = m_style;
    m_style = 1;
    
    // set ui
    autoButtonOff();
    d3ButtonOff();
    d2ButtonOn();
    changeStyleButtonOff(m_style);
    //m_chgStyleLabel->setText("2D");

    changeStyleMenuShowHide(0);
    m_reminder_text->setText(custstr_get_remd_manual_2d_str());
    m_reminder_text_state = 1;
    m_reminder_timer->start(5*1000);
    m_isChgStyleOn = 0;

    // action
    if (old_style != 1)
    {
        shm_set_is_manual(1);
        change_view(1, AVM_VIEW_CAMERA_FISH_CLIP_FRONT);
    }
}


void MainWidget::d3ButtonInit()
{
    //m_3DLabel->setText("3D");
    m_3DLabel->setStyleSheet(
        "color:rgb(255,255,255);"
        FONT_SIZE_20
        "background-color:rgba(0, 0, 0, 128);"
        "qproperty-alignment: AlignCenter;"
        "background-image:url(avm_qt_app_res/ui/BTN_3D_unselected.png);");
}


void MainWidget::d3ButtonOn()
{
    m_3DLabel->setStyleSheet(
        "color:rgb(240,240,240);"
        FONT_SIZE_20
        "background-color:rgba(0, 0, 0, 160);"
        "qproperty-alignment: AlignCenter;"
        "background-image:url(avm_qt_app_res/ui/BTN_3D_selected.png);");
}


void MainWidget::d3ButtonOff()
{
    m_3DLabel->setStyleSheet(
        "color:rgb(255,255,255);"
        FONT_SIZE_20
        "background-color:rgba(0, 0, 0, 128);"
        "qproperty-alignment: AlignCenter;"
        "background-image:url(avm_qt_app_res/ui/BTN_3D_unselected.png);");
}


void MainWidget::d3ButtonClicked()
{
    if (2 == m_style)
    {
        //d3ButtonOff();
    }
    else
    {
        d3ButtonOn();
    }
}


void MainWidget::d3ButtonReleased()
{
    int old_style = m_style;
    m_style = 2;
    
    // set ui
    autoButtonOff();
    d3ButtonOn();
    d2ButtonOff();
    changeStyleButtonOff(m_style);
    //m_chgStyleLabel->setText("3D");

    changeStyleMenuShowHide(0);
    m_reminder_text->setText(custstr_get_remd_manual_3d_str());
    m_reminder_text_state = 2;
    m_reminder_timer->start(5*1000);
    m_isChgStyleOn = 0;

    // action, 0 auto, 1 2d, 2 3d
    if (old_style != 2)
    {
        shm_set_is_manual(1);
        change_view(1, AVM_VIEW_3D_REAR);
    }
}


void MainWidget::changeStyleButtonInit()
{
    autoButtonOn();
    d2ButtonOff();
    d3ButtonOff();
    changeStyleButtonOff(m_style);
    //m_chgStyleLabel->setText("AUTO");

    m_isChgStyleOn = 0;
    changeStyleMenuShowHide(0);
}


void MainWidget::changeStyleButtonOn(int style)
{
    if (0 == style)
    {
        m_chgStyleLabel->setStyleSheet(
            "color:rgb(240,240,240);"
            FONT_SIZE_20
            CHNAGESTYLE_ON_BC
            "qproperty-alignment: AlignCenter;"
            "background-image:url(avm_qt_app_res/ui/BTN_status_auto selected.png);");
    }
    else if (1 == style)
    {
        m_chgStyleLabel->setStyleSheet(
            "color:rgb(240,240,240);"
            FONT_SIZE_20
            CHNAGESTYLE_ON_BC
            "qproperty-alignment: AlignCenter;"
            "background-image:url(avm_qt_app_res/ui/BTN_status_2D selected.png);");        
    }
    else if (2 == style)
    {
        m_chgStyleLabel->setStyleSheet(
            "color:rgb(240,240,240);"
            FONT_SIZE_20
            CHNAGESTYLE_ON_BC
            "qproperty-alignment: AlignCenter;"
            "background-image:url(avm_qt_app_res/ui/BTN_status_3D selected.png);");        
    }
}


void MainWidget::changeStyleButtonOff(int style)
{
    if (0 == style)
    {
        m_chgStyleLabel->setStyleSheet(
            "color:rgb(255,255,255);"
            FONT_SIZE_20
            CHNAGESTYLE_OFF_BC
            "qproperty-alignment: AlignCenter;"
            "background-image:url(avm_qt_app_res/ui/BTN_status_auto unselected.png);"
            "background-position:center center;");
    }
    else if (1 == style)
    {
        m_chgStyleLabel->setStyleSheet(
            "color:rgb(255,255,255);"
            FONT_SIZE_20
            CHNAGESTYLE_OFF_BC
            "qproperty-alignment: AlignCenter;"
            "background-image:url(avm_qt_app_res/ui/BTN_status_2D unselected.png);"
            "background-position:center center;");        
    }
    else if (2 == style)
    {
        m_chgStyleLabel->setStyleSheet(
            "color:rgb(255,255,255);"
            FONT_SIZE_20
            CHNAGESTYLE_OFF_BC
            "qproperty-alignment: AlignCenter;"
            "background-image:url(avm_qt_app_res/ui/BTN_status_3D unselected.png);"
            "background-position:center center;");    
    }
}


void MainWidget::changeStyleMenuShowHide(int is_show)
{
    if (is_show)
    {     
        m_2DLabel->show();
        m_3DLabel->show();
        //m_autoLabel->show();
    }
    else
    {
        m_2DLabel->hide();
        m_3DLabel->hide();
        //m_autoLabel->hide();       
    }
}

void MainWidget::changeStyleButtonClicked()
{
    if (0 == checkClickVaildProc(0))
    {
        return;
    }
    if (1 == m_style)
    {
        changeStyleButtonOff(2);
    }
    else if (2 == m_style)
    {
        changeStyleButtonOff(1);
    }
}


void MainWidget::changeStyleButtonReleased()
{
    // check if at least one menu exist, close them first, and this click this not vaild
    if (0 == checkClickVaildProc(1))
    {
        return;
    }

    // 2d->3d, m_style from 1->2
    if (1 == m_style)
    {
		int old_style = m_style;
	    m_style = 2;

	    m_reminder_text->setText(custstr_get_remd_manual_3d_str());
	    m_reminder_text_state = 2;
	    m_reminder_timer->start(5*1000);

	    // action, 0 auto, 1 2d, 2 3d
	    if (old_style != 2)
	    {
	        shm_set_is_manual(1);
	        change_view(1, AVM_VIEW_3D_REAR);
	    }
    }
    // 3d->2d, m_style from 2->1
    else if (2 == m_style)
    {
		int old_style = m_style;
	    m_style = 1;

	    m_reminder_text->setText(custstr_get_remd_manual_2d_str());
	    m_reminder_text_state = 1;
	    m_reminder_timer->start(5*1000);

	    // action
	    if (old_style != 1)
	    {
	        shm_set_is_manual(1);
	        change_view(1, AVM_VIEW_CAMERA_FISH_CLIP_FRONT);
	    }
    }
}
#endif


void MainWidget::assistButtonInit()
{
    assistButtonOff();
	m_assistLabel->hide();
}
void MainWidget::assistButtonOn()
{
    m_assistLabel->setStyleSheet(
        "color:rgb(255,255,255);"
        FONT_SIZE_20
        "padding-left:10px;"
        "background-image:url(avm_qt_app_res/ui/new/pac_steering_path_s.png);"
        "background-color:rgba(0, 0, 0, 128);");
}
void MainWidget::assistButtonPressed()
{
	if(1 == m_isAssistOn)
	{
		m_assistLabel->setStyleSheet(
	        "color:rgb(255,255,255);"
	        FONT_SIZE_20
	        "padding-left:10px;"
	        "background-image:url(avm_qt_app_res/ui/new/pac_steering_path_sp.png);"
	        "background-color:rgba(0, 0, 0, 128);");
	}
	else
	{
		m_assistLabel->setStyleSheet(
	        "color:rgb(255,255,255);"
	        FONT_SIZE_20
	        "padding-left:10px;"
	        "background-image:url(avm_qt_app_res/ui/new/pac_steering_path_np.png);"
	        "background-color:rgba(0, 0, 0, 128);");
	}
}
void MainWidget::assistButtonOff()
{
    m_assistLabel->setStyleSheet(
        "color:rgb(255,255,255);"
        FONT_SIZE_20
        "padding-left:10px;"
        "background-image:url(avm_qt_app_res/ui/new/pac_steering_path_n.png);"
        "background-color:rgba(0, 0, 0, 128);");
}
void MainWidget::assistButtonClicked()
{
	assistButtonPressed();
}
void MainWidget::assistButtonReleased()
{
    if (m_isAssistOn)
    {
        m_isAssistOn = 0;
        assistButtonOff();

        // action
        shm_set_is_assist_off(1);

        if (m_surroundWidget != NULL)
        {
            m_surroundWidget->showAssistLine(0);
        }
        if (m_summWidget != NULL)
        {
            m_summWidget->showAssistLine(0);
        }
    }
    else
    {
        m_isAssistOn = 1;
        assistButtonOn();

        // action
        shm_set_is_assist_off(0);

        if (m_surroundWidget != NULL)
        {
            m_surroundWidget->showAssistLine(1);
        }
        if (m_summWidget != NULL)
        {
            m_summWidget->showAssistLine(1);
        }
    }

    m_itemsDisplayTimer->start(POPUP_DISAPPEAR_DELAY);
}



void MainWidget::radarButtonInit()
{
    radarButtonOff();
	m_radarLabel->hide();
}
void MainWidget::radarButtonOn()
{
    m_radarLabel->setStyleSheet(
        "color:rgb(255,255,255);"
        FONT_SIZE_20
        "padding-left:10px;"
        "background-image:url(avm_qt_app_res/ui/new/pac_distance_info_s.png);"
        "background-color:rgba(0, 0, 0, 128);");
}
void MainWidget::radarButtonPressed()
{
	if(1 == m_isRadarOn)
	{
		m_radarLabel->setStyleSheet(
	        "color:rgb(255,255,255);"
	        FONT_SIZE_20
	        "padding-left:10px;"
	        "background-image:url(avm_qt_app_res/ui/new/pac_distance_info_sp.png);"
	        "background-color:rgba(0, 0, 0, 128);");
	}
    else
	{
		m_radarLabel->setStyleSheet(
	        "color:rgb(255,255,255);"
	        FONT_SIZE_20
	        "padding-left:10px;"
	        "background-image:url(avm_qt_app_res/ui/new/pac_distance_info_np.png);"
	        "background-color:rgba(0, 0, 0, 128);");
	}
}
void MainWidget::radarButtonOff()
{
    m_radarLabel->setStyleSheet(
        "color:rgb(255,255,255);"
        FONT_SIZE_20
        "padding-left:10px;"
        "background-image:url(avm_qt_app_res/ui/new/pac_distance_info_n.png);"
        "background-color:rgba(0, 0, 0, 128);");
}
void MainWidget::radarButtonClicked()
{
	radarButtonPressed();
}
void MainWidget::radarButtonReleased()
{
    if (m_isRadarOn)
    {
        m_isRadarOn = 0;
        radarButtonOff();

        // action
        shm_set_is_radar_off(1);

        if (m_surroundWidget != NULL)
        {
            m_surroundWidget->showRadar(0);
            m_surroundWidget->showBlockLine(0);
        }
        if (m_summWidget != NULL)
        {
            m_summWidget->showRadar(0);
        }
    }
    else
    {
        m_isRadarOn = 1;
        radarButtonOn();

        // action
        shm_set_is_radar_off(0);

        if (m_surroundWidget != NULL)
        {
            m_surroundWidget->showRadar(1);
            m_surroundWidget->showBlockLine(1);
        }
        if (m_summWidget != NULL)
        {
            m_summWidget->showRadar(1);
        }
    }

	m_itemsDisplayTimer->start(POPUP_DISAPPEAR_DELAY);
}



void MainWidget::turnLinkButtonInit()
{
    turnLinkButtonOn();
	m_isTurnLinkOn = 1;
	m_turnLinkLabel->hide();
}
void MainWidget::turnLinkButtonOn()
{
    m_turnLinkLabel->setStyleSheet(
        "color:rgb(255,255,255);"
        FONT_SIZE_20
        "padding-left:10px;"
        "background-image:url(avm_qt_app_res/ui/new/pac_turn_link_s.png);"
        "background-color:rgba(0, 0, 0, 128);");
}
void MainWidget::turnLinkButtonPressed()
{
	if(1 == m_isTurnLinkOn)
	{
	    m_turnLinkLabel->setStyleSheet(
	        "color:rgb(255,255,255);"
	        FONT_SIZE_20
	        "padding-left:10px;"
	        "background-image:url(avm_qt_app_res/ui/new/pac_turn_link_sp.png);"
	        "background-color:rgba(0, 0, 0, 128);");
	}
	else
	{
		m_turnLinkLabel->setStyleSheet(
	        "color:rgb(255,255,255);"
	        FONT_SIZE_20
	        "padding-left:10px;"
	        "background-image:url(avm_qt_app_res/ui/new/pac_turn_link_np.png);"
	        "background-color:rgba(0, 0, 0, 128);");
	}
}
void MainWidget::turnLinkButtonOff()
{
    m_turnLinkLabel->setStyleSheet(
        "color:rgb(255,255,255);"
        FONT_SIZE_20
        "padding-left:10px;"
        "background-image:url(avm_qt_app_res/ui/new/pac_turn_link_n.png);"
        "background-color:rgba(0, 0, 0, 128);");
}
void MainWidget::turnLinkButtonClicked()
{
	turnLinkButtonPressed();
}
void MainWidget::turnLinkButtonReleased()
{
	if(1 == m_isTurnLinkOn)
	{
		m_isTurnLinkOn = 0;

		turnLinkButtonOff();
	}
	else
	{
		m_isTurnLinkOn = 1;

		turnLinkButtonOn();
	}

	m_itemsDisplayTimer->start(POPUP_DISAPPEAR_DELAY);
}




void MainWidget::radioButtonInit()
{
    radioButtonOn();
}
void MainWidget::radioButtonOn()
{
    m_radioLabel->setStyleSheet(
        FONT_SIZE_20
        "padding-left:10px;"
        "background-image:url(avm_qt_app_res/ui/new/Volume-n-btn.png);");
}
void MainWidget::radioButtonOff()
{
    m_radioLabel->setStyleSheet(
        FONT_SIZE_20
        "padding-left:10px;"
        "background-image:url(avm_qt_app_res/ui/new/Volume-mute-n-btn.png);");
}
void MainWidget::radioButtonClicked()
{
}
void MainWidget::radioButtonReleased()
{
    if (1 == radioButtonExternChg(0, -1, 1))
    {
        m_reminder_text->setText(custstr_get_remd_mute_str());
	    m_reminder_text_state = 3;
	    m_reminder_timer->start(5*1000);
    }

	m_itemsDisplayTimer->start(POPUP_DISAPPEAR_DELAY);
}
/**< 
* @func radioButtonExternChg
* @param is_extern: is cmd from ihu or not .
* @param mute_status:0 not mute, 1 mute. -1 toggle. 
* @param is_rescissible. 0 can't change by avm, 1 can change by avm
* @return 0 state can change, 1 state can't change(state fix and change by avm itself)
*/
int MainWidget::radioButtonExternChg(int is_extern, int mute_status, int is_rescissible)
{
    if (is_extern)
    {
        // set if value is fixed 
        if (0 == is_rescissible)
        {
            m_isRadioStatFixed = 1;
        }
        else
        {
            m_isRadioStatFixed = 0;
        }
    }
    if ((0 == m_isRadioStatFixed) || (is_extern))
    {
        int curr_mute_state;
        if (m_isRadioOn)
        {
            curr_mute_state = 0;
        }
        else
        {
            curr_mute_state = 1;
        }
        if ((-1 == mute_status) || (mute_status != curr_mute_state))
        {
            if (m_isRadioOn)
            {
                m_isRadioOn = 0;
                radioButtonOff();
                
                shm_set_is_radio_off(1);
            }
            else
            {
                m_isRadioOn = 1;
                radioButtonOn();

                shm_set_is_radio_off(0);
            }
        }

        return 0;
    }
    else
    {
        return 1;
    }
}



void MainWidget::settimgButtonInit()
{      
    settimgButtonOff();
    m_isSettimgOn = 0;
}
void MainWidget::settimgButtonOn()
{
    m_settingLabel->setStyleSheet(
        "color:rgb(255,255,255);"
        FONT_SIZE_20
        "padding-left:10px;"
        "background-color:rgba(0, 0, 0, 128);"
        "background-image:url(avm_qt_app_res/ui/new/pac_more_s.png);");
}
void MainWidget::settimgButtonPressed()
{
	if(1 == m_isSettimgOn)
	{
	    m_settingLabel->setStyleSheet(
	        "color:rgb(255,255,255);"
	        FONT_SIZE_20
	        "padding-left:10px;"
	        "background-color:rgba(0, 0, 0, 128);"
	        "background-image:url(avm_qt_app_res/ui/new/pac_more_sp.png);");
	}
	else
	{
		m_settingLabel->setStyleSheet(
	        "color:rgb(255,255,255);"
	        FONT_SIZE_20
	        "padding-left:10px;"
	        "background-color:rgba(0, 0, 0, 128);"
	        "background-image:url(avm_qt_app_res/ui/new/pac_more_np.png);");
	}
}
void MainWidget::settimgButtonOff()
{
    m_settingLabel->setStyleSheet(
        "color:rgb(255,255,255);"
        FONT_SIZE_20
        "padding-left:10px;"
        "background-color:rgba(0, 0, 0, 128);"
        "background-image:url(avm_qt_app_res/ui/new/pac_more_n.png);");
}
void MainWidget::settimgButtonClicked()
{
    settimgButtonPressed();
}
void MainWidget::settimgButtonReleased()
{
    if (0 == checkClickVaildProc(1))
    {
        return;
    }
	
    if (m_isSettimgOn)
    {
        m_isSettimgOn = 0;

        settimgButtonOff();

        m_assistLabel->hide();
        m_radarLabel->hide();
        //m_radioLabel->hide(); //LG UI
        m_turnLinkLabel->hide();
    }
    else
    {
        m_isSettimgOn = 1;

        settimgButtonOn();

        m_assistLabel->show();
        m_radarLabel->show();
		//m_radioLabel->show(); //LG UI
        m_turnLinkLabel->show();
    }

	m_itemsDisplayTimer->start(POPUP_DISAPPEAR_DELAY);
}



void MainWidget::backButtonInit()
{
    m_backLabel->setStyleSheet(
        "background-image:url(avm_qt_app_res/ui/back_button_off.png);"
        "background-repeat:none;"
        "background-position:center right;");
}
void MainWidget::backButtonClicked()
{
    AVM_LOG("MainWidget::backButtonClicked\n");
    shm_set_is_closed(1);
}



void MainWidget::alertButtonClicked()
{
    m_alert_window_btn->setText(custstr_get_alert_ok_str());
    m_alert_window_btn->setStyleSheet(
        "qproperty-alignment: AlignCenter;"
        "color:rgb(128,128,128);"
        FONT_SIZE_20
        "background-color:rgb(35,35,35);");

}

void MainWidget::alertButtonReleased()
{
    m_alert_window_btn->setText(custstr_get_alert_ok_str());
    m_alert_window_btn->setStyleSheet(
        "qproperty-alignment: AlignCenter;"
        "color:rgb(184,184,184);"
        FONT_SIZE_20
        "background-color:rgb(35,35,35);");

    shm_set_is_closed(1);
}


void MainWidget::alertBottomMaskClicked(QMouseEvent *event)
{
    event = event;
}


void MainWidget::alertBottomMaskReleased(QMouseEvent *event)
{
    int pos_x = event->pos().x();
    int pos_y = event->pos().y();
    debugStateLogic(pos_x, pos_y);    
}


void MainWidget::currentViewButtonReleased()
{
    checkClickVaildProc(1);
}


void MainWidget::reminderTextButtonReleased()
{
    checkClickVaildProc(1);
}


void MainWidget::reservedClickLabelClicked()
{
}

void MainWidget::reservedClickLabelReleased()
{
}


void MainWidget::statusBarButtonReleased()
{
	//nothing
}


void MainWidget::mousePressEvent(QMouseEvent *event)
{
    // first close all other menu, if exist
    if (0 == checkClickVaildProc(1))
    {
        m_isMouseClickCanced = 1;
        m_isMousePressed = 0;
        return;
    }
    else
    {
        m_isMouseClickCanced = 0;
    }
    
    if (event->button() == Qt::LeftButton)
    {
        m_isMousePressed = 1;
        m_mouseTotalMove = 0;
        m_lstPoint = event->pos();
		m_pressedPoint = m_lstPoint;
    }
}


void MainWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (0 == m_isMousePressed)
    {
        return;
    }

#if 1
    int det_x = event->pos().x() - m_lstPoint.x();
    int det_y = event->pos().y() - m_lstPoint.y();
    
    // filter too big det value
    if ((abs(det_x) >= LAYOUT_SURR_WIDTH)
        || (abs(det_y) >= LAYOUT_TOP_HEIGHT))
    {
        return;
    }
    // filter min move, set as just press action
    int min_limit = 4;
    if ((abs(det_x) + abs(det_y)) < min_limit)
    {
        return;
    }

	if(1 == m_style)
	{
		m_isMouseMoveSlide = 1;
	}
	else if ((2 == m_style) && (AVM_VIEW_AUTO != m_surroundWidget->getView()))
	{
#if 1
		hideAllItems();

        // calc det_y
        //if (m_lstPoint.y() < LAYOUT_TOP_HEIGHT / 2)
        //{
        //   det_x = -1 * det_x;
        //}
        //det_y = -1 * det_y;

        // 
        float a_x = m_lstPoint.x() - LAYOUT_SURR_WIDTH/2.0;
        float a_y = m_lstPoint.y() - LAYOUT_TOP_HEIGHT/2.0;
        float dot_mul = a_x * det_x + a_y * det_y;
        float a_len = sqrt(a_x * a_x + a_y * a_y);
        float b_len = sqrt(det_x*det_x + det_y*det_y);
        float cosang = dot_mul / (a_len * b_len);
        float det_ridisu = fabs(b_len * sin(acos(cosang)));
        float is_right = a_y * det_x - a_x * det_y;
        if (is_right < 0.0f)
        {
            det_ridisu = -1.0 * det_ridisu;
        }
        
        m_surroundWidget->rotateView_omni(det_ridisu, -1*det_y); /* [csj 20181015-4] */
#else
		float a_x = m_lstPoint.x() - LAYOUT_SURR_WIDTH/2.0;
	    float a_y = m_lstPoint.y() - LAYOUT_TOP_HEIGHT/2.0;
        float dot_mul = a_x * det_x + a_y * det_y;
        float a_len = sqrt(a_x * a_x + a_y * a_y);
        float b_len = sqrt(det_x*det_x + det_y*det_y);
        float cosang = dot_mul / (a_len * b_len);
        float det_ridisu = fabs(b_len * sin(acos(cosang)));
        float is_right = a_y * det_x - a_x * det_y;
        if (is_right < 0.0f)
        {
            det_ridisu = -1.0 * det_ridisu;
        }
        m_surroundWidget->rotateView(det_ridisu);
#endif
#if 0 //LG UI
		int new_view = m_surroundWidget->get_view_by_cur_deg();
		QString str2 = custstr_get_view_str(new_view);
    	m_current_view->setText(str2);
#endif
	}
	
	m_lstPoint = event->pos();
#endif

#if 0
    int min_limit = 2;
    int min_limit_slow_move = 4;
    int det_x = event->pos().x() - m_lstPoint.x();
    int det_y = event->pos().y() - m_lstPoint.y();
    if (m_lstPoint.x() > LAYOUT_SURR_WIDTH / 2)
    {
        det_y = -1 * det_y;
    }
    if (m_lstPoint.y() < LAYOUT_TOP_HEIGHT / 2)
    {
        det_x = -1 * det_x;
    }
    int det = det_x + det_y;    // this value is similar to left and right move length
    while (1)
    {
        if ((abs(det) < min_limit))
        {
            m_mouseTotalMove += det;
            // multi slow move lead a left and right move len > limit
            if (abs(m_mouseTotalMove) > min_limit_slow_move)
            {
                break;
            }
            // multi slove move, total move is less than limit, just return and wait next press
            else
            {
                m_isMouseMoveSlide = 0;
                return;
            }
        }
        else
        {
            break;
        }
    }
    m_isMouseMoveSlide = 1;
    int det_add = det;

    if (1 == m_style)
    {
    }
    else if (2 == m_style)
    {
        int i;
        int cur_view = m_surroundWidget->getView();
        static int trans_arr[][3] = {
            {AVM_VIEW_3D_FRONT, AVM_VIEW_3D_RIGHT_FRONT, AVM_VIEW_3D_LEFT_FRONT},
            {AVM_VIEW_3D_LEFT, AVM_VIEW_3D_LEFT_CENTER, AVM_VIEW_3D_REAR},
            {AVM_VIEW_3D_REAR, AVM_VIEW_3D_LEFT, AVM_VIEW_3D_RIGHT},
            {AVM_VIEW_3D_RIGHT, AVM_VIEW_3D_REAR, AVM_VIEW_3D_RIGHT_CENTER},
            {AVM_VIEW_3D_LEFT_FRONT, AVM_VIEW_3D_FRONT, AVM_VIEW_3D_LEFT_CENTER},
            {AVM_VIEW_3D_LEFT_CENTER, AVM_VIEW_3D_LEFT_FRONT, AVM_VIEW_3D_LEFT},
            {AVM_VIEW_3D_RIGHT_FRONT, AVM_VIEW_3D_RIGHT_CENTER, AVM_VIEW_3D_FRONT},
            {AVM_VIEW_3D_RIGHT_CENTER, AVM_VIEW_3D_RIGHT, AVM_VIEW_3D_RIGHT_FRONT}
        };
        int arr_cnt = sizeof(trans_arr) / sizeof(int) / 3;
        for (i = 0; i < arr_cnt; i++)
        {
            if (trans_arr[i][0] == cur_view)
            {
                if (det_add > 0)
                {
                    change_view_dyn(1, trans_arr[i][1]);
                }
                else
                {
                    change_view_dyn(1, trans_arr[i][2]);
                }
                break;
            }
        }
        if (i == arr_cnt)
        {
            //AVM_ERR("SurroundWidget::mouseMoveEvent. current view=%d is nor support\n",
            //    cur_view);
        }
    }
    m_isMousePressed = 0;
#endif
}


void MainWidget::hideAllItems()
{
	if(0 == m_is_item_hide)
	{
		m_is_item_hide = 1;

        //sync state before hide
        int cur_view = m_surroundWidget->getView();
        syncSightImageState(cur_view);

		//hide all
		m_sight_front_btn->hide();
		m_sight_front_fish_btn->hide();
		m_sight_rear_btn->hide();
		m_sight_rear_fish_btn->hide();
		m_sight_side_btn->hide();
		m_sight_3d_btn->hide();
		m_3d_dir_btn->hide();
		m_settingLabel->hide();
		m_radarLabel->hide();
		m_assistLabel->hide();
		m_turnLinkLabel->hide();

		m_isFrontSightButtonPopped = 0;
		m_isRearSightButtonPopped = 0;
		m_is3DDirButtonPopped = 0;
		m_isSettimgOn = 0;

		settimgButtonOff();
	}
}
void MainWidget::showMainMenu()
{
	if(1 == m_is_item_hide)
	{
		m_is_item_hide = 0;

		//show
		m_sight_front_btn->show();
		m_sight_rear_btn->show();
		m_sight_side_btn->show();
		m_sight_3d_btn->show();
		m_settingLabel->show();
	}
}



void MainWidget::mouseReleaseEvent(QMouseEvent *event)
{
    // debug
    int pos_x = event->pos().x();
    int pos_y = event->pos().y();
    debugStateLogic(pos_x, pos_y);

    //
    if (event->button() == Qt::LeftButton)
    {
        m_isMousePressed = 0;
    }

    if ((1 == m_isMouseClickCanced) || (0 == m_style))
    {
        //m_isMouseMoveSlide = 0;
        m_isMouseClickCanced = 0;
        return;
    }

    // check mouse move from press point
    int pie_show_limit = 4;
    int press_mv_x = pos_x - m_pressedPoint.x();
    int press_mv_y = pos_y - m_pressedPoint.y();
    int press_mv = abs(press_mv_x) + abs(press_mv_y);
    if (press_mv > pie_show_limit)
    {
		if(1 == m_isMouseMoveSlide)
		{
			m_isMouseMoveSlide = 0;
			if(1 == m_is_item_hide)
			{
				showMainMenu();
				m_itemsDisplayTimer->start(POPUP_DISAPPEAR_DELAY);
                return;
			}
			
			if (1 == m_style)
		    {
				int i;
		        int cur_view = m_surroundWidget->getView();
		        static int trans_arr[][3] = {
		            {AVM_VIEW_CAMERA_FISH_CLIP_FRONT, AVM_VIEW_CAMERA_LEFTRIGHT_CPY, AVM_VIEW_CAMERA_FISH_CLIP_REAR},
		            {AVM_VIEW_CAMERA_FISH_FRONT, AVM_VIEW_CAMERA_LEFTRIGHT_CPY, AVM_VIEW_CAMERA_FISH_CLIP_REAR},
		            {AVM_VIEW_CAMERA_FISH_CLIP_REAR, AVM_VIEW_CAMERA_FISH_CLIP_FRONT, AVM_VIEW_CAMERA_LEFTRIGHT_CPY},
		            {AVM_VIEW_CAMERA_FISH_REAR, AVM_VIEW_CAMERA_FISH_CLIP_FRONT, AVM_VIEW_CAMERA_LEFTRIGHT_CPY},
		            {AVM_VIEW_CAMERA_LEFTRIGHT_CPY, AVM_VIEW_CAMERA_FISH_CLIP_REAR, AVM_VIEW_CAMERA_FISH_CLIP_FRONT},
		            {AVM_VIEW_CAMERA_LEFTRIGHT, AVM_VIEW_CAMERA_FISH_CLIP_REAR, AVM_VIEW_CAMERA_FISH_CLIP_FRONT},
		        };
		        int arr_cnt = sizeof(trans_arr) / sizeof(int) / 3;
		        for (i = 0; i < arr_cnt; i++)
		        {
		            if (trans_arr[i][0] == cur_view)
		            {
		            	int new_view = 0;
		                if (press_mv_x > 0)
		                {
		                	new_view = trans_arr[i][1];
		                }
		                else
		                {
		                	new_view = trans_arr[i][2];
		                }

						if(new_view != AVM_VIEW_UNKNOW)
						{
							change_view(1, new_view);
							syncSightImageState(new_view);
							m_itemsDisplayTimer->start(POPUP_DISAPPEAR_DELAY);
						}
		                break;
		            }
		        }
		    }
		}
    }
	else
	{
		//clicked
		int top = 0;    //LAYOUT_TOP_HEIGHT/2-LAYOUT_PIE_WH/2;
	    int left = 0;   //LAYOUT_SURR_WIDTH/2-LAYOUT_PIE_WH/2;
	    int bottom = LAYOUT_TOP_HEIGHT;  //LAYOUT_TOP_HEIGHT/2+LAYOUT_PIE_WH/2;
	    int right = m_surroundWidget->size().width();
	    int x = m_pressedPoint.x();
	    int y = m_pressedPoint.y();
	    if ((x > left) && (x < right) && (y > top) && (y < bottom))
	    {
	    	//only accept surround widget mouse event
			if(0 == m_is_item_hide)
			{
				hideAllItems();
			}
			else
			{
				showMainMenu();
				m_itemsDisplayTimer->start(POPUP_DISAPPEAR_DELAY);
			}
	    }
		
	}
}


#if 0 //LG UI
void MainWidget::d2PieStyleInit()
{
#if 0
    QString style = QString(
        "color:rgb(255,255,255);"
        FONT_SIZE_20);
    const int position[6][2] = {
        {120-2, 135-1},         // center
        {230-2, 135-1},         // left-right
        {155-2, 33},            // front
        {32-2,  78},            // fisheye-front
        {32-2,  200-1},         // rear
        {155-2, 236}            // fisheye-rear
    };

    for (int i = 0; i < 5+1; i++)
    {
        QLabel *p_label = m_2DPieLabel->GetLabel(i);
        p_label->setText(custstr_get_2dpie_str(i));

        QSize size = p_label->size();
        p_label->setGeometry(position[i][0], position[i][1],
            size.width(), size.height());
        p_label->setStyleSheet(style);
    }
#endif    
    const char *str[7] = {
        "avm_qt_app_res/ui/d2_none.png",
        "avm_qt_app_res/ui/d2_fisheye_front.png",
        "avm_qt_app_res/ui/d2_fisheye_rear.png",
        "avm_qt_app_res/ui/d2_right.png",
        "avm_qt_app_res/ui/d2_front.png",
        "avm_qt_app_res/ui/d2_left.png",
        "avm_qt_app_res/ui/d2_rear.png",
    };
    int uid[7] = {
        AVM_VIEW_UNKNOW,
        AVM_VIEW_CAMERA_FISH_FRONT,
        AVM_VIEW_CAMERA_FISH_REAR,
        AVM_VIEW_CAMERA_LEFTRIGHT,
        AVM_VIEW_CAMERA_FISH_CLIP_FRONT,
        AVM_VIEW_CAMERA_LEFTRIGHT_CPY,    // hack
        AVM_VIEW_CAMERA_FISH_CLIP_REAR
    };
    for (int i = 0; i < 7; i++)
    {
        m_2DPieLabel->AddImage(i, uid[i], QString(str[i]));
    }    
}


void MainWidget::d2pieToggleStyle(int idx, int is_tmp)
{
    // 0 is null string, so it's not vaild
    if ((idx <= 0) || (idx > m_2DPieLabel->GetIndexLimit()))
    {
        return;
    }
    
    QLabel *p_label;    
    p_label = m_2DPieLabel->GetLabel(m_d2pieSelIdx);
    if (p_label != NULL)
    {
        p_label->setStyleSheet(
            "color:rgb(255,255,255);"
            FONT_SIZE_20);
    }
    p_label = m_2DPieLabel->GetLabel(idx);
    if (p_label != NULL)
    {
        p_label->setStyleSheet(
            "color:rgb(200,200,200);"
            FONT_SIZE_20);
    }
    if (0 == is_tmp)
    {
        m_d2pieSelIdx = idx;
    }
}


void MainWidget::d2PieClicked(int idx)
{
    idx = idx;
    //d2pieToggleStyle(idx, 1);
}


void MainWidget::d2PieReleased(int idx)
{
    // if select a certen idx, change it style to select style
    if ((idx < 0) || (idx > m_2DPieLabel->GetIndexLimit()))
    {
        d2pieShowHide(0);
        return;
    }
    //d2pieToggleStyle(idx, 0);
	
    int new_view = m_2DPieLabel->FindUidByIndex(idx);
    if (new_view >= 0)
    {
        change_view(1, new_view);
    }

    m_pie_timer->start(PIE_DISAPPEAR_DELAY);
}


int MainWidget::d2pieShowHide(int is_show)
{
    if (is_show)
    {
        int cur_view = m_surroundWidget->getView();
        int pie_idx = m_2DPieLabel->FindIndexByUid(cur_view);
        if (pie_idx < 0)
        {
            m_2DPieLabel->TriggerIdxImage(0);
        }
        else
        {
            m_2DPieLabel->TriggerIdxImage(pie_idx);
        }

        int surround_width = m_surroundWidget->size().width();
        m_2DPieLabel->setGeometry(
            surround_width/2-LAYOUT_PIE_WH/2,
            LAYOUT_TOP_HEIGHT/2-LAYOUT_PIE_WH/2,
            LAYOUT_PIE_WH, LAYOUT_PIE_WH);
		
        m_2DPieLabel->ShowHide(1);
        m_isD2PieOn = 1;
    }
    else
    {
        m_isD2PieOn = 0; 
        m_2DPieLabel->ShowHide(0);       
    }

    return 0;
}


void MainWidget::d3PieStyleInit()
{
#if 0
    QString style = QString(
        "color:rgb(255,255,255);"
        FONT_SIZE_20);
    const int position[9][2] = {
        {120-2, 135-1},         // 3d-top
        {245-2, 135-1},         // right
        {209-2, 60-1 },         // right-front
        {133-2, 23-1 },         // front
        {57-2,  60-1 },         // left-front
        {20-2,  135-1},         // left
        {57-2,  210-1},         // left-rear
        {133-2, 245-1},         // rear
        {209-2, 210-1}          // right-rear
    };

    for (int i = 0; i < 8+1; i++)
    {
        QLabel *p_label = m_3DPieLabel->GetLabel(i);
        p_label->setText(custstr_get_3dpie_str(i));

        QSize size = p_label->size();
        p_label->setGeometry(position[i][0], position[i][1],
            size.width(), size.height());
        p_label->setStyleSheet(style);
    }
#endif
    const char *str[11] = {
        "avm_qt_app_res/ui/d3_none.png",
        "", //avm_qt_app_res/ui/d3_top.png
        "",
        "avm_qt_app_res/ui/d3_right.png",
        "avm_qt_app_res/ui/d3_front_right.png",
        "avm_qt_app_res/ui/d3_front.png",
        "avm_qt_app_res/ui/d3_front_left.png",
        "avm_qt_app_res/ui/d3_left.png",
        "avm_qt_app_res/ui/d3_rear_left.png",
        "avm_qt_app_res/ui/d3_rear.png",
        "avm_qt_app_res/ui/d3_rear_right.png",
    };
    // uid is avm_view enum value
    int uid[11] = {
        AVM_VIEW_UNKNOW,
        -1,
        -1,
        AVM_VIEW_3D_RIGHT_CENTER,
        AVM_VIEW_3D_RIGHT_FRONT,
        AVM_VIEW_3D_FRONT,
        AVM_VIEW_3D_LEFT_FRONT,
        AVM_VIEW_3D_LEFT_CENTER,
        AVM_VIEW_3D_LEFT,
        AVM_VIEW_3D_REAR,
        AVM_VIEW_3D_RIGHT
    };
    for (int i = 0; i < 11; i++)
    {
        m_3DPieLabel->AddImage(i, uid[i], QString(str[i]));
    }
}


void MainWidget::d3pieToggleStyle(int idx, int is_tmp)
{
    QLabel *p_label;
    p_label = m_3DPieLabel->GetLabel(m_d3pieSelIdx);
    if (p_label != NULL)
    {
        p_label->setStyleSheet(
            "color:rgb(255,255,255);"
            FONT_SIZE_20);
    }
    
    p_label = m_3DPieLabel->GetLabel(idx);
    if (p_label != NULL)
    {
        p_label->setStyleSheet(
            "color:rgb(200,200,200);"
            FONT_SIZE_20);
    }
    if (0 == is_tmp)
    {
        m_d3pieSelIdx = idx;
    }
}


void MainWidget::d3PieClicked(int idx)
{
    idx = idx;
    //d3pieToggleStyle(idx, 1);
}


void MainWidget::d3PieReleased(int idx)
{
    // if click is out of index, just return
    // else set certen idx style to select style
    if ((idx < 0) || (idx > m_3DPieLabel->GetIndexLimit()))
    {
        d3pieShowHide(0);
        return;
    }
    //d3pieToggleStyle(idx, 0);
    
    int new_view = m_3DPieLabel->FindUidByIndex(idx);
    if (new_view >= 0)
    {
        int cur_view = m_surroundWidget->getView();
        if (cur_view == new_view)
        {
            m_isD3PieOn = 0;
            m_3DPieLabel->ShowHide(0);
            return; 
        }
        int is_cur_view_3d = m_surroundWidget->checkInPure3dView(cur_view);
        int is_new_view_3d = m_surroundWidget->checkInPure3dView(new_view);
        if (is_cur_view_3d && is_new_view_3d)
        {
            change_view_dyn(1, new_view);
        }
        else
        {
            change_view(1, new_view);
        }
    }

    m_pie_timer->start(PIE_DISAPPEAR_DELAY);
}


int MainWidget::d3pieShowHide(int is_show)
{
    if (is_show)
    {
        int cur_view = m_surroundWidget->getView();
        int pie_idx = m_3DPieLabel->FindIndexByUid(cur_view);

        if (pie_idx < 0)
        {
            m_3DPieLabel->TriggerIdxImage(0);
        }
        else
        {
            m_3DPieLabel->TriggerIdxImage(pie_idx);
        }    
        m_3DPieLabel->ShowHide(1);
        m_isD3PieOn = 1;
    }
    else
    {
        m_isD3PieOn = 0; 
        m_3DPieLabel->ShowHide(0);          
    }

    return 0;
}
#endif


QString MainWidget::getAutoStr()
{
    // 0 for auto, 1 2d, 2 3d
    if (0 == m_style)
    {
        return custstr_get_auto();
    }
    else
    {
        return custstr_get_manual();
    }
}


void MainWidget::change_view_layout(int view_enum)
{
    QSize widget_size = m_surroundWidget->size();
    // widget is (1280*656) in wide front and wide rear view
    if ((AVM_VIEW_CAMERA_FISH_FRONT == view_enum)
        || (AVM_VIEW_CAMERA_FISH_REAR == view_enum))
    {
        if ((widget_size.width() != LAYOUT_SURR_WIDE_WIDTH)
            || (widget_size.height() != LAYOUT_TOP_HEIGHT))
        {
            m_surroundWidget->setGeometry(0, 0,
                LAYOUT_SURR_WIDE_WIDTH, LAYOUT_TOP_HEIGHT);
            m_surroundWidget->updateWidgetWH(LAYOUT_SURR_WIDE_WIDTH, LAYOUT_TOP_HEIGHT);
        }
        m_summWidget->hide();
#if 0 //LG UI
		m_current_view->setGeometry(
            (LAYOUT_SURR_WIDE_WIDTH - LAYOUT_CUR_VIEW_W)/2, 0,
            LAYOUT_CUR_VIEW_W, LAYOUT_CUR_VIEW_H);
#endif
        m_surroundWidget->setView(view_enum);  
    }
    // widget is (950*656) in otner view
    else
    {
        if ((widget_size.width() != LAYOUT_SURR_WIDTH)
            || (widget_size.height() != LAYOUT_TOP_HEIGHT))
        {
            m_surroundWidget->setGeometry(0, 0,
                LAYOUT_SURR_WIDTH, LAYOUT_TOP_HEIGHT);
            m_surroundWidget->updateWidgetWH(LAYOUT_SURR_WIDTH, LAYOUT_TOP_HEIGHT);
        }
        m_summWidget->show();
#if 0 //LG UI
		m_current_view->setGeometry(
            (LAYOUT_SURR_WIDTH - LAYOUT_CUR_VIEW_W)/2, 0,
            LAYOUT_CUR_VIEW_W, LAYOUT_CUR_VIEW_H);
#endif
        m_surroundWidget->setView(view_enum);
    }    
}


void MainWidget::syncSightImageState(int current_view_enum)
{
	//close all
	frontSightButtonOff();
	frontFishSightButtonOff();
	rearSightButtonOff();
	rearFishSightButtonOff();
	sideSightButtonOff();
	d3SightButtonOff();

	//hide child popup
	m_sight_front_fish_btn->hide();
	m_isFrontSightButtonPopped = 0;
	
	m_sight_rear_fish_btn->hide();
	m_isRearSightButtonPopped = 0;
	
	m_3d_dir_btn->hide();
	m_is3DDirButtonPopped = 0;

	//highlight the image of current view
	switch(current_view_enum)
	{
		//2d
        case AVM_VIEW_CAMERA_FISH_FRONT:
		{
			frontFishSightButtonOn();
			m_sight_front_fish_btn->show();
			m_isFrontSightButtonPopped = 1;
		}
        case AVM_VIEW_CAMERA_FISH_CLIP_FRONT:
		{
			frontSightButtonOn();
			break;
		}
        case AVM_VIEW_CAMERA_FISH_REAR:
		{
			rearFishSightButtonOn();
			m_sight_rear_fish_btn->show();
			m_isRearSightButtonPopped = 1;
		}
        case AVM_VIEW_CAMERA_FISH_CLIP_REAR:
		{
			rearSightButtonOn();
			break;
		}
        case AVM_VIEW_CAMERA_LEFTRIGHT:
		case AVM_VIEW_CAMERA_LEFTRIGHT_CPY:
		{
			sideSightButtonOn();
			break;
		}

		//3d
		case AVM_VIEW_3D_FRONT:
		{
			d3DirButtonFront();
			d3SightButtonOn();
			break;
		}
        case AVM_VIEW_3D_REAR:
		{
			d3DirButtonRear();
			d3SightButtonOn();
			break;
		}
        case AVM_VIEW_3D_LEFT_CENTER:
		{
			d3DirButtonLeft();
			d3SightButtonOn();
			break;
		}
        case AVM_VIEW_3D_RIGHT_CENTER:
		{
			d3DirButtonRight();
			d3SightButtonOn();
			break;
		}
        case AVM_VIEW_3D_FREE:
        {
            d3SightButtonOn();
            break;
        }

		default:
			AVM_ERR("Err cur view:%d\r\n", current_view_enum);
	}

    if(1 == m_isRadarOn)
    {
        radarButtonOn();
    }
    else
    {
        radarButtonOff();
    }

    if(1 == m_isAssistOn)
    {
        assistButtonOn();
    }
    else
    {
        assistButtonOff();
    }

    if(1 == m_isTurnLinkOn)
    {
        turnLinkButtonOn();
    }
    else
    {
        turnLinkButtonOff();
    }
}


void MainWidget::change_view(int is_manual, int view_enum)
{
    // if view_enum is the same, return
    int current_view = m_surroundWidget->getView();
    if (current_view == view_enum)
    {
        return;
    }

	if(1 == m_surroundWidget->checkInPure3dView(view_enum))
	{
		m_style = 2;
	}
	else
	{
		m_style = 1;
	}

    if (m_surroundWidget != NULL)
    {
        change_view_layout(view_enum);
    }

    if (is_manual)
    {
        m_lst_manual_avm_view = view_enum;
    }

#if 0 //LG UI
    QString str2 = custstr_get_view_str(view_enum);
    if (m_current_view != NULL)
    {
        m_current_view->setText(str2);
        if (m_surroundWidget->checkIn3dView(view_enum))
        {
            m_current_view->hide();
        }
        else
        {
            m_current_view->show();
        }
    }
#endif
}


void MainWidget::change_view_dyn(int is_manual, int view_enum)
{
    if (m_surroundWidget != NULL)
    {
        m_surroundWidget->setViewDyn(view_enum);
    }
    if (is_manual)
    {
        m_lst_manual_avm_view = view_enum;
    }
#if 0 //LG UI
    QString str2 = custstr_get_view_str(view_enum);
    m_current_view->setText(str2);
#endif
}


/**< 0 front and rear radar distance >= limit*/
/**< 1 front radar distance < limit */
/**< 2 rear radar distance < limit */
/**< 3 front and rear radar distance < limit*/
int MainWidget::checkRadarFBDistance(float limit)
{
    int front_left_val      = (m_radar_bits[0] >> (4*(1-1))) & 0x0F;
    int front_left_val_ext  = (m_radar_bits[0] >> (4*(2-1))) & 0x0F;
    int rear_left_val       = (m_radar_bits[0] >> (4*(6-1))) & 0x0F;
    int rear_left_val_ext   = (m_radar_bits[0] >> (4*(5-1))) & 0x0F;

    int front_right_val     = (m_radar_bits[1] >> (4*(5-1))) & 0x0F;
    int front_right_val_ext = (m_radar_bits[1] >> (4*(5-1))) & 0x0F;
    int rear_right_val      = (m_radar_bits[1] >> (4*(1-1))) & 0x0F;
    int rear_right_val_ext  = (m_radar_bits[1] >> (4*(2-1))) & 0x0F;

    // 0x00 0.0-0.35m
    // 0x01 0.35-0.6m
    // 0x02 0.6-0.9m
    // 0x02 0.9-1.2m
    // ....
    int div = (int)((limit + 0.15f) / 0.3f);
    int front_less = 0;
    int rear_less = 0;
    if ((front_left_val <= div) || (front_right_val <= div)
        || (front_left_val_ext <= div) || (front_right_val_ext <= div))
    {
        front_less = 1;
    }
    if ((rear_left_val <= div) || (rear_right_val <= div)
        || (rear_left_val_ext <= div) || (rear_right_val_ext <= div))
    {
        rear_less = 1;
    }

    // all >= limit
    if ((0 == front_less) && (0 == rear_less))
    {
        return 0;
    }
    // front < limit
    else if (1 == front_less)
    {
        return 1;
    }
    // rear < limit
    else if (1 == rear_less)
    {   
        return 2;
    }
    // all < limit
    else
    {   
        return 3;
    }
}


void MainWidget::viewChgLogic(auto_chg_view_act_e action)
{
    if (ACHG_ACTION_BACK == action)
    {
    	showMainMenu();
		m_itemsDisplayTimer->start(POPUP_DISAPPEAR_DELAY);
		
        change_view(0, AVM_VIEW_CAMERA_FISH_CLIP_REAR);
		syncSightImageState(AVM_VIEW_CAMERA_FISH_CLIP_REAR);
        return; 
    }
    else if (ACHG_ACTION_STOP == action)
    {
    	showMainMenu();
		m_itemsDisplayTimer->start(POPUP_DISAPPEAR_DELAY);

        if ((-1 == m_turn_signal) && (m_go_back >= 0))
        {
            int current_view = m_surroundWidget->getView();
            int is_cur_view_3d = m_surroundWidget->checkInPure3dView(current_view);
            if (is_cur_view_3d)
            {
                change_view_dyn(0, AVM_VIEW_3D_LEFT);
            }
            else
            {
                change_view(0, AVM_VIEW_3D_LEFT);
				syncSightImageState(AVM_VIEW_3D_LEFT);
            }
            return; 
        }
        else if ((1 == m_turn_signal) && (m_go_back >= 0))
        {
            int current_view = m_surroundWidget->getView();
            int is_cur_view_3d = m_surroundWidget->checkInPure3dView(current_view);
            if (is_cur_view_3d)
            {
                change_view_dyn(0, AVM_VIEW_3D_RIGHT);
            }
            else
            {
                change_view(0, AVM_VIEW_3D_RIGHT);
				syncSightImageState(AVM_VIEW_3D_RIGHT);
            }
            return;
        }
        else
        {
            //nothing   
        }
    }
	else if (ACHG_ACTION_GO == action)
	{
		showMainMenu();
		m_itemsDisplayTimer->start(POPUP_DISAPPEAR_DELAY);

        if ((-1 == m_turn_signal) && (m_go_back >= 0))
        {
            int current_view = m_surroundWidget->getView();
            int is_cur_view_3d = m_surroundWidget->checkInPure3dView(current_view);
            if (is_cur_view_3d)
            {
                change_view_dyn(0, AVM_VIEW_3D_LEFT);
            }
            else
            {
                change_view(0, AVM_VIEW_3D_LEFT);
				syncSightImageState(AVM_VIEW_3D_LEFT);
            }
            return; 
        }
        else if ((1 == m_turn_signal) && (m_go_back >= 0))
        {
            int current_view = m_surroundWidget->getView();
            int is_cur_view_3d = m_surroundWidget->checkInPure3dView(current_view);
            if (is_cur_view_3d)
            {
                change_view_dyn(0, AVM_VIEW_3D_RIGHT);
            }
            else
            {
                change_view(0, AVM_VIEW_3D_RIGHT);
				syncSightImageState(AVM_VIEW_3D_RIGHT);
            }
            return;
        }
        else
        {
        	//if last manual view is 2d, then 2d front
        	//if last manual view is 3d, then 3d front
        	int is_old_view_3d = m_surroundWidget->checkInPure3dView(m_lst_manual_avm_view);
			if(1 == is_old_view_3d)
			{
				//before is 3d
                int current_view = m_surroundWidget->getView();
				int is_cur_view_3d = m_surroundWidget->checkInPure3dView(current_view);
				if(1 == is_cur_view_3d)
				{
					change_view_dyn(1, AVM_VIEW_3D_REAR);
				}
				else
				{
					change_view(1, AVM_VIEW_3D_REAR);
					syncSightImageState(AVM_VIEW_3D_REAR);
				}
			}
			else
			{
				//before is 2d
				change_view(1, AVM_VIEW_CAMERA_FISH_CLIP_FRONT);
				syncSightImageState(AVM_VIEW_CAMERA_FISH_CLIP_FRONT);
			}
        }
	}
    else if (ACHG_ACTION_TURN_L == action)
    {
    	if(1 == m_isTurnLinkOn)
		{
			if (m_go_back >= 0)
	        {
	        	showMainMenu();
				m_itemsDisplayTimer->start(POPUP_DISAPPEAR_DELAY);
				
	            int current_view = m_surroundWidget->getView();
	            int is_cur_view_3d = m_surroundWidget->checkInPure3dView(current_view);
	            if (is_cur_view_3d)
	            {
	                change_view_dyn(0, AVM_VIEW_3D_LEFT);
	            }
	            else
	            {
	                change_view(0, AVM_VIEW_3D_LEFT);
					syncSightImageState(AVM_VIEW_3D_LEFT);
	            }
	            return;   
	        }
	        else
	        {
	            return;
	        }
		}
    }
    else if (ACHG_ACTION_TURN_N == action)
    {
    	showMainMenu();
		m_itemsDisplayTimer->start(POPUP_DISAPPEAR_DELAY);
		
        if (m_go_back < 0)
        {
            change_view(0, AVM_VIEW_CAMERA_FISH_CLIP_REAR);
			syncSightImageState(AVM_VIEW_CAMERA_FISH_CLIP_REAR);
            return;
        }
        else
        {
            int current_view = m_surroundWidget->getView();
            if (m_lst_manual_avm_view != current_view)
            {
                int is_cur_view_3d = m_surroundWidget->checkInPure3dView(current_view);
                int is_new_view_3d = m_surroundWidget->checkInPure3dView(m_lst_manual_avm_view);
                if (is_cur_view_3d && is_new_view_3d)
                {
                    change_view_dyn(0, m_lst_manual_avm_view);
                }
                else
                {
                    change_view(0, m_lst_manual_avm_view);
					syncSightImageState(m_lst_manual_avm_view);
                }
            }
        }
    }
    else if (ACHG_ACTION_TURN_R == action)
    {
    	if(1 == m_isTurnLinkOn)
		{
			if (m_go_back >= 0)
	        {
	        	showMainMenu();
				m_itemsDisplayTimer->start(POPUP_DISAPPEAR_DELAY);
	
	            int current_view = m_surroundWidget->getView();
	            int is_cur_view_3d = m_surroundWidget->checkInPure3dView(current_view);
	            if (is_cur_view_3d)
	            {
	                change_view_dyn(0, AVM_VIEW_3D_RIGHT);
	            }
	            else
	            {
	                change_view(0, AVM_VIEW_3D_RIGHT);
					syncSightImageState(AVM_VIEW_3D_RIGHT);
	            }
	            return;   
	        }
	        else
	        {
	            return;
	        } 
		}
    }
    else
    {  
    }
}

void MainWidget::itemsDisplayTimeout(void)
{
	hideAllItems();
}


