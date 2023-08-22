
#ifndef _COMUCMD_DEFINE_H_
#define _COMUCMD_DEFINE_H_

#ifdef __cplusplus
extern "C" {
#endif


#define SOC_VER_MAJOR       0xFF
#define SOC_VER_MINOR       0xFF
#define SOC_VER_REVISION    0xFF


#define COMUCMD_ID_ROTATE_VIEW  (100)   // rotate view angle (for test)
#define COMUCMD_ID_CHG_VIEW     (101)   // change current view (for test)
#define COMUCMD_ID_TOUCH        (102)   // pass the touch screen struct to qt app
#define COMUCMD_ID_ERROR_IMG    (103)   // switch black image
#define COMUCMD_ID_RADAR        (104)   // radar
#define COMUCMD_ID_ASSIST_ANG   (105)   // assist line angle
#define COMUCMD_ID_FPS          (106)   // print fps
#define COMUCMD_ID_TEST         (110)   // test
#define COMUCMD_ID_CAR_SPEED	(111)	// set car speed
#define COMUCMD_ID_TURN_SIGNAL	(112)	// set car turn signal
#define COMUCMD_ID_GOBACK		(113)	// set car go ahead or go back
#define COMUCMD_ID_DOOR			(114)	// set door state
#define COMUCMD_ID_GETAUTO		(115)	// if qt_app in auto state
#define COMUCMD_ID_GETRADIO		(116)	// if qt_app audio is on or off
#define COMUCMD_ID_SHUTDOWN     (117)   // just show black screen, but not close stitch and cam pwr. shutdown command is send from comu to qt_app, to let it show black screen
#define COMUCMD_ID_GETSHUTDOWN  (118)   // get shutdown status. check if qt_app have received shutdown cmd, and show black screen successd
#define COMUCMD_ID_TEST_CAMPWR  (119)   // test cmd to set camera power to 1 or 0
#define COMUCMD_ID_ERR_VER      (120)   // mcu version and comu version is not match
#define COMUCMD_ID_GETVIEW      (121)   // get current view enum
#define COMUCMD_ID_GETUART      (122)   // get uart status
#define COMUCMD_ID_SIM_SLOT_SIG	(123)	// simulate slot signal
#define COMUCMD_ID_WAM_DISPALY  (124)   // the status of wam dispaly
#define COMUCMD_ID_IHU_MUTE_ATT (125)   // the attribute of mute status in IHU
#define COMUCMD_ID_EM_FLASHER   (126)   // set emergency flashers state
#define COMUCMD_ID_MIRROW_FOLD  (127)   // set rearview mirrow fold state
#define COMUCMD_ID_SETSVIMG     (129)   // set save cam image flag
#define COMUCMD_ID_CKSVIMG      (130)   // check if image is saved


typedef enum _cmd_slot_signal_
{
	CMD_SLOT_SIG_MIN = 0,
	CMD_SLOT_SIG_2D_BUTTON_RELEASED = CMD_SLOT_SIG_MIN,
	CMD_SLOT_SIG_2D_PIE_RELEASED,
	CMD_SLOT_SIG_3D_BUTTON_RELEASED,
	CMD_SLOT_SIG_3D_PIE_RELEASED,
	CMD_SLOT_SIG_ASSIST_RELEASED,
	CMD_SLOT_SIG_RADAR_RELEASED,
	CMD_SLOT_SIG_RADIO_RELEASED,
	CMD_SLOT_SIG_MAX
}CMD_SLOT_SIG;

/**
* cmd_rotate_view_s, input struct for COMUCMD_ID_ROTATE_VIEW cmd
*/
typedef struct _cmd_rotate_view_struct_
{
    int det_x;  /**< rotate x det */
}cmd_rotate_view_s;


/**
* cmd_change_view_s, input struct for COMUCMD_ID_CHG_VIEW
*/
typedef struct _cmd_change_view_struct_
{
    unsigned int view_enum;     /**< view enum */
}cmd_change_view_s;


/**
* cmd_touch_s, input struct for COMUCMD_ID_TOUCH
*/
typedef struct _cmd_touch_struct_
{
    int reserved;
}cmd_touch_s;


/**
* cmd_touch_s, input struct for COMUCMD_ID_TOUCH
*/
typedef struct _cmd_error_image_struct_
{
    int show_error_image;
}cmd_error_image_s;


typedef struct _cmd_radar_struct_
{
    int is_on;  // this val is dprecated
    unsigned int mask[2];
}cmd_radar_s;


typedef struct _cmd_assist_line_struct_
{
    float angle;
}cmd_assist_line_s;


/**
* cmd_fps_s, input struct for COMUCMD_ID_FPS
*/
typedef struct _cmd_fps_struct_
{
    int reserved;
}cmd_fps_s;


/**
* cmd_test_s, input struct for COMUCMD_ID_TEST
*/
typedef struct _cmd_test_struct_
{
    int reserved;
}cmd_test_s;


/**
* cmd_car_speed_s, input struct for COMUCMD_ID_CAR_SPEED
*/
typedef struct _cmd_car_speed_struct_
{
    unsigned int speed;
}cmd_car_speed_s;


/**
* cmd_turn_signal_s, input struct for COMUCMD_ID_TURN_SIGNAL
*/
typedef struct _cmd_turn_signal_struct_
{
    int turn;	// -1 left, 0 no turn, 1 turn right
}cmd_turn_signal_s;


/**
* cmd_go_back_s, input struct for COMUCMD_ID_GOBACK
*/
typedef struct _cmd_go_back_struct_
{
    int go_back;	// 0 stop, 1 go, -1 back
}cmd_go_back_s;


/**
* cmd_door_s, input struct for COMUCMD_ID_DOOR
*/
typedef struct _cmd_door_struct_
{
    int is_front_left_door_open;
	int is_front_right_door_open;
    int is_rear_left_door_open;
	int is_rear_right_door_open;	

	int is_front_cover_open;
	int is_rear_cover_open;	
}cmd_door_s;


/**
* cmd_shutdown_s, 1 for shut down, 0 for restart
*/
typedef struct _cmd_shutdown_struct_
{
    int is_shut_down;
}cmd_shutdown_s;


/**
* cmd_wam_dispaly_s, 1 for showing, 0 for not showing
*/
typedef struct _cmd_wam_dispaly_struct_
{
    int is_wam_diaplay;
}cmd_wam_dispaly_s;


/*
 * mute_status:  1: The logic in IHU is for mute
 *               0: The logic in IHU is for not mute
 *
 * is_rescissible : 1 mute status can be changed by WAM
 *                : 0 mute status can not be changed by WAM
 */
typedef struct _cmd_ihu_mute_struct_
{
    int mute_status;
    int is_rescissible;
}cmd_ihu_mute_s;

/**
* emergency_flasher_st, 1 for flash, 0 for not flash
*/
typedef struct _cmd_emergency_flasher_struct_
{
    int emergency_flasher;
}emergency_flasher_st;


//COMUCMD_ID_MIRROW_FOLD
typedef struct _rearview_mirrow_fold_struct_
{
    int is_left_fold;
    int is_right_fold;
}rearview_mirrow_fold_s;


/**
* ext share mem, this struct mem is alloced by qt
* total shm mem size = 
*  + sizeof(comucmd_s)
*  + COMUCMD_SND_MEM_SIZE(64K)
*  + COMUCMD_RET_MEM_SIZE(1K)  
*  + COMUCMD_EXT_SHARE_MEM(6K) = sizeof(comucmd_s) + 71K
* 
* memmap list below:
*
* comucmd_s
* COMUCMD_SND_MEM_SIZE  (64K)
* COMUCMD_RET_MEM_SIZE  (1K)
* COMUCMD_EXT_SHARE_MEM (6K)
*
*/
typedef struct _ext_share_mem_struct_
{
    int is_manual;          /**< avm_qt_app -> avm_comu */
    int is_assist_line_off; /**< avm_qt_app -> avm_comu */
    int is_radar_off;       /**< avm_qt_app -> avm_comu */
    int is_radio_off;       /**< avm_qt_app -> avm_comu */
    int is_close;           /**< avm_qt_app -> avm_comu, qt press back-botton or alert screen's ok-button */
    int is_cam_error[5];    /**< avm_qt_app -> avm_comu. [0] error, [1][2][3][4] right, front, left, rear */
    int is_err_screen[2];   /**< avm_qt_app -> avm_comu. [0]current screen is error screen, [1] error screen course */

    unsigned int car_speed;         /**< avm_comu -> avm_qt_app */
    unsigned int turn_signal;       /**< avm_comu -> avm_qt_app */
    unsigned int go_back_signal;    /**< avm_comu -> avm_qt_app */
    unsigned int vcap_hb_code[4];   /**< avm_qt_app -> avm_comu */ /* qt vcap thread status code */
    int factory_calib_ok;           /**< avm_qt_app -> avm_comu, qt read factory_calib.param file, check if factory calib success */
    int qt_status;                  /**< avm_qt_app -> avm_comu */ /* qt process status code, >0 OK, <=0 NG, 0x01 surround widget ok, 0x02 summwiget ok, 0x03 both ok */
    int this_struct_size;           /**< avm_qt_app -> avm_comu. this 'ext_share_mem_s' struct size */
    unsigned int reserved[14];
    
    
    // to store comu file printf info
    unsigned char fileprt_area[5*1024];
}ext_share_mem_s;


#ifdef __cplusplus
}
#endif

#endif

