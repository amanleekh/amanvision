#ifndef MAIN_WIDGET_H
#define MAIN_WIDGET_H

#include <QWidget>
#include <QPushButton>
#include <QLabel>
#include <QThread>
#include <QTimer>
#include <QVBoxLayout>
#include <QHBoxLayout>

#include "common_def.h"
#include "clickablelabel.h"
#include "cmd_recver.h"
#include "peilabel.h"
#include "debug_widget.h"

#define MIANWGT_LOG(...)    AVM_LOG(__VA_ARGS__)
#define MAINWGT_ERR(...)    AVM_ERR(__VA_ARGS__)


class SurroundWidget;
class SummWidget;


/**
* auto change view. action enum
*/
typedef enum _auto_chg_view_act_enum_
{
    ACHG_ACTION_GO      = 0x11,
    ACHG_ACTION_STOP    = 0x12,
    ACHG_ACTION_BACK    = 0x13,
    ACHG_ACTION_TURN_L  = 0x21,
    ACHG_ACTION_TURN_N  = 0x22,
    ACHG_ACTION_TURN_R  = 0x23,
    ACHG_ACTION_RADAR   = 0x41,
    ACHG_ACTION_NULL    = 0x80
}auto_chg_view_act_e;


class MainWidget;

/**
 * @brief main UI window
 */
class MainWidget : public QWidget
{
    Q_OBJECT

public:
    MainWidget(int input_width, int input_height);
    void layout();
    void control();

public slots:
    /**< rotate current view. only for auto and 3d views */
    void rotateView(int det_x);
    /**< viewEnum should be AVM_VIEW_E */
    void setView(int viewEnum);
    /**< set radar alert state. bit mask. each radar have 4bit. etc */
    void setRadar(unsigned mask, unsigned int mask2);
    /**< clr radar block by bit mask. --this func is deprecated-- */
    void clrRadar(unsigned mask, unsigned int mask2);
    /**< set assist line by angle deg. eg. 10 for right deg 10. -10 means left deg 10 */
    void setAssistLine(float angle);
    /**< print current paintGL fsp */
    void prtFps();
    /**< set current car speed */
    void setCarSpeed(float speed);
    /**< -1 left, 0 no turn, 1 turn right */
    void setTurnSignal(int turn);
    /**< set car in emergency flashers. 0 off, 1 on */
    void setEmFlahers(int is_on);    
    /**< 0 stop, 1 go, -1 back */
    void setGoBack(int go_back);
    /**< 0 close, 1 open */
    void setDoor(int front_left, int front_right, int rear_left, int rear_right,
        int front_cover, int rear_cover);
    /**< show blank image */
    void shutDown(int is_shutdown);
    /**< set camera power status */
    void setCamPwr(int cam_pwr);
    void setErrVersion(int is_error);
	void wamDisplayStatusChanged(int updatedStatus);
    /*<
     * mute_status:  1: The logic in IHU is for mute
     *               0: The logic in IHU is for not mute
     * is_rescissible : 1 mute status can be changed by WAM
     *                : 0 mute status can not be changed by WAM */
	void setIhuMute(int mute_status, int is_rescissible);

    /**
    * set rearview mirrow fold
    */
	void setMirrowFold(int is_left_fold, int is_right_fold);

private slots:
    //void autoButtonClicked();
    //void autoButtonReleased();
    //void d2ButtonClicked();
    //void d2ButtonReleased();
    //void d3ButtonClicked();
    //void d3ButtonReleased();
    //void changeStyleButtonClicked();
    //void changeStyleButtonReleased();
    void assistButtonClicked();
    void assistButtonReleased();
    void radarButtonClicked();
    void radarButtonReleased();    
    void radioButtonClicked();
    void radioButtonReleased();
    void settimgButtonClicked();
    void settimgButtonReleased();
    //void d2PieClicked(int idx);
    //void d2PieReleased(int idx);
    //void d3PieClicked(int idx);
    //void d3PieReleased(int idx);
    void backButtonClicked();
    void alertButtonClicked();
    void alertButtonReleased();
    void alertBottomMaskClicked(QMouseEvent *event);
    void alertBottomMaskReleased(QMouseEvent *event); 
    void currentViewButtonReleased();
    void reminderTextButtonReleased();
    void reservedClickLabelClicked();
    void reservedClickLabelReleased();
#if 1 //LG UI
	void statusBarButtonReleased();

    void frontSightButtonClicked();
    void frontSightButtonReleased();
	void frontFishSightButtonClicked();
    void frontFishSightButtonReleased();
	
    void rearSightButtonClicked();
    void rearSightButtonReleased();
	void rearFishSightButtonClicked();
    void rearFishSightButtonReleased();
	
    void sideSightButtonClicked();
    void sideSightButtonReleased();

	void d3SightButtonClicked();
    void d3SightButtonReleased();

	void d3DirButtonClicked(int idx);
	void d3DirButtonReleased(int idx);

    void turnLinkButtonClicked();
    void turnLinkButtonReleased();
#endif

private slots:
    void checkVideoVaildProcess();
    void changeRemendText();    /**< delay to show bottom remend text */
	void itemsDisplayTimeout(void);

private:
    int m_input_width;
    int m_input_height;
    // avm view
    SurroundWidget *m_surroundWidget;   /**< avm view widget container */
    // summury view
    SummWidget *m_summWidget;   /**< summ view widget container. fix width=350 */
    // status bar
    QWidget *m_status;  /**< status widget container. fix height=64 */

private:
    //void autoButtonInit();
    //void autoButtonOn();
    //void autoButtonOff();    
    //void d2ButtonInit();
    //void d2ButtonOn();
    //void d2ButtonOff();
    //void d3ButtonInit();
    //void d3ButtonOn();
    //void d3ButtonOff();
    //void changeStyleButtonInit();
    //void changeStyleButtonOn(int style);    /**< style is same as m_style. as 0 for auto, 1 for 2d, 2 for 3d */
    //void changeStyleButtonOff(int style);
    //void changeStyleMenuShowHide(int is_show);
    void assistButtonInit();
    void assistButtonOn();
    void assistButtonPressed();
    void assistButtonOff();
    void radarButtonInit();
    void radarButtonOn();
    void radarButtonPressed();
    void radarButtonOff();    
    void radioButtonInit();
    void radioButtonOn();
    void radioButtonPressed();
    void radioButtonOff();
    /**< 
    * is_extern: is cmd from ihu or not .
    * mute_status:0 not mute, 1 mute. -1 toggle. 
    * is_rescissible. 0 can't change by avm, 1 can change by avm
    * return: 0, change button state success, 1 can't change button state
    */
    int radioButtonExternChg(int is_extern, int mute_status, int is_rescissible);
    void settimgButtonInit();
    void settimgButtonOn();
    void settimgButtonPressed();
    void settimgButtonOff();
    void backButtonInit();
    //void d3PieStyleInit();
    /**< select idx, style change. if is_tmp=1, next time toogle will change style again */
    //void d3pieToggleStyle(int idx, int is_tmp);
    //int d3pieShowHide(int is_show);
    //void d2PieStyleInit();
    //void d2pieToggleStyle(int idx, int is_tmp);
    //int d2pieShowHide(int is_show);
    /**< check if this click is vaild. when a menu is pop up, this click will cancel the menu, not a vaild click */
    int checkClickVaildProc(int is_do_proc);
    /**< check if pie is on, and close it */
    void checkAndClosePie();
#if 1 //LG UI
	void frontSightButtonOn();
	void frontSightButtonPressed();
	void frontSightButtonOff();
	void frontFishSightButtonOn();
	void frontFishSightButtonPressed();
	void frontFishSightButtonOff();

	void rearSightButtonOn();
	void rearSightButtonPressed();
	void rearSightButtonOff();
	void rearFishSightButtonOn();
	void rearFishSightButtonPressed();
	void rearFishSightButtonOff();

	void sideSightButtonOn();
	void sideSightButtonPressed();
	void sideSightButtonOff();

	void d3SightButtonOn();
	void d3SightButtonPressed();
	void d3SightButtonOff();

	void d3DirButtonInit();
    void d3DirButtonNone();
    void d3DirButtonFront();
    void d3DirButtonRear();
    void d3DirButtonLeft();
    void d3DirButtonRight();

    void turnLinkButtonInit();
    void turnLinkButtonOn();
	void turnLinkButtonPressed();
    void turnLinkButtonOff();

    void syncSightImageState(int current_view_enum);
	void hideAllItems();
	void showMainMenu();
#endif

    // debug
private:
    void debugWidgetLayout();
    void debugWidgetShowHide(int is_show);
    void debugStateLogic(int x, int y);
private slots:
    void debugTimeCbFunc();

private:
    DebugWidget *m_p_dbg_wgt;
    QTimer *m_dbg_timer;
    int m_dbg_state;

private:
    // on imx6, we use work thread to receive synced video, and send signal to avm_qpu process
    // on pc, we use a timer, send signal to avm_gpu process
    QThread *m_workerThread;    /**< work thread, wait for synced video */
    QTimer *m_checkGlTimer; /**< desktop env to trigger */
    QTimer *m_checkVideoTimer;  /**< check if video is vaild, if not vaild, show error image */

private:
    CmdRecver *m_cmdRecver;
    QThread *m_cmdRecvThread;   /**< work thread to receive cmd from avm_comu proc*/

private:
    //ClickableLabel *m_chgStyleLabel;
    //ClickableLabel *m_autoLabel;
    //ClickableLabel *m_2DLabel;
    //ClickableLabel *m_3DLabel;
    ClickableLabel *m_settingLabel;
    ClickableLabel *m_assistLabel;
    ClickableLabel *m_radarLabel;
	ClickableLabel *m_turnLinkLabel;
    ClickableLabel *m_radioLabel;
    //QTimer *m_setTimeDelayTimer;
    //PieLabel *m_3DPieLabel;
    //PieLabel *m_2DPieLabel;
    ClickableLabel *m_backLabel;
    
    //ClickableLabel *m_current_view;     /**< banner at top of screen, show current view state, auto or manual style */
    ClickableLabel *m_reminder_text;    /**< text on status bar */
    int m_reminder_text_state;  /**< 0, default text, 1 2d text, 2 3d text, 3 others text */
    QTimer *m_reminder_timer;
    ClickableAdvLabel *m_alert_bottom_mask;    /**< when show alert window, this label fill whole screen with background color */
    ClickableLabel *m_alert_window_body;    /**< alert window body */
    ClickableLabel *m_alert_window_btn;     /**< alert window button */
    ClickableLabel *m_debug_version_info;   /**< show version info */
    ClickableLabel *m_debug_error_info_title;
    ClickableLabel *m_debug_error_info;
	QTimer* m_itemsDisplayTimer; /* main item keeps showing for 5s after view changing */

#if 1 //LG UI
    ClickableLabel* m_sight_front_btn;
	ClickableLabel* m_sight_front_fish_btn;
    ClickableLabel* m_sight_rear_btn;
	ClickableLabel* m_sight_rear_fish_btn;
    ClickableLabel* m_sight_side_btn;
	ClickableLabel* m_sight_3d_btn;
    PieLabel* m_3d_dir_btn;
	
	int m_isFrontSightButtonPopped;
	int m_isRearSightButtonPopped;
	int m_is3DDirButtonPopped;
#endif

    int m_style;        /**< 0 auto, 1 2d, 2 3d */
    int m_isAssistOn;   /**< 0 assist is off, 1 is on */
    int m_isRadarOn;    /**< radar */
	int m_isTurnLinkOn;
    int m_isRadioOn;    /**< 0 radio is off, so mute is on, 1 radio is on, so mute is off */
    int m_isRadioStatFixed;  /**< is radio state fixed. in case ihu force avm mute or not, press button can't change this state */
    int m_isChgStyleOn; /**< is change style menu pop out */
    int m_isSettimgOn;  /**< is setting menu pop out */
    int m_isD2PieOn;
    int m_isD3PieOn;
    int m_check_vaild_bypass_cnt;
    int m_d2pieSelIdx;  /**< d2 pie current select idx, default is -1 */
    int m_d3pieSelIdx;
    int m_is_use_bmp;   /**< if use bmp file as camera input */
    int m_lst_manual_avm_view;  /**< last manual set avm view */
	int m_is_item_hide;

private:
    int buttonStateInit();
    void avmStateInit();
    QString getAutoStr();
    /**< 0 front and rear radar distance >= limit*/
    /**< 1 front radar distance < limit */
    /**< 2 rear radar distance < limit */
    /**< 3 front and rear radar distance < limit*/
    int checkRadarFBDistance(float limit); 
    /**< in some view, widget view will be change */
    void change_view_layout(int view_enum);
    void change_view(int is_manual, int view_enum);
    void change_view_dyn(int is_manual, int view_enum);
    void setDynDebugLabel();
    void setShmCamStatus();
    void alertWindowSH(int alert_type, int is_on);

private:
    void viewChgLogic(auto_chg_view_act_e action);    /**< logic to set and change view state */

private:
    float m_car_speed;  /**< car speed */
    float m_assist_angle;
    int m_turn_signal;  /**< -1 left, 0 no turn, 1 turn right */
    int m_go_back;      /**< 0 stop, 1 go, -1 back */
    int m_front_left_door_open;
    int m_front_right_door_open;
    int m_rear_left_door_open;
    int m_rear_right_door_open;
    int m_left_mirrow_fold;
    int m_right_mirrow_fold;
    unsigned int m_radar_bits[2];  /**< each radar have 2bit, 00 for 0-0.3, 01 for 0.3-1.0, 10 for 1.0-1.5, 11 for 1.5- */

public:
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);

private:
    int m_isMousePressed;
    int m_isMouseMoveSlide;     /**< is mouse slide in mouse move state */
    int m_isMouseClickCanced;   /**< is mouse canecd in click state */
    int m_mouseTotalMove;
    QPoint m_lstPoint;
	QPoint m_pressedPoint;

public:
    static int getShutSownStatus();
    static int getView();
    static int m_isShutDown;
    static MainWidget *m_p_this;
};


#endif // MAIN_WIDGET_H
