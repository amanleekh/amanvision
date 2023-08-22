
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>

#include "common_def.h"
#include "cmd_recver.h"
#include "comu_cmd.h"
#include "comu_cmd_define.h"
#include "video_capture.h"
#include "main_widget.h"
#include "sync_video_getter.h"


static ext_share_mem_s *g_p_shm = NULL;

static comucmd_reg_s g_reg_arr[] = {
    {COMUCMD_ID_ROTATE_VIEW, "rotate_view", &CmdRecver::cmdFuncRotateView},
    {COMUCMD_ID_CHG_VIEW, "change_view", &CmdRecver::cmdFuncChgView},   
    {COMUCMD_ID_TOUCH, "touch", &CmdRecver::cmdFuncTouch},   
    {COMUCMD_ID_ERROR_IMG, "error_image", &CmdRecver::cmdFuncErrorImage},
    {COMUCMD_ID_RADAR, "set&CmdRecver::clr radar", &CmdRecver::cmdFuncRadar},
    {COMUCMD_ID_ASSIST_ANG, "set assist line angle", &CmdRecver::cmdFuncAssistLine},
    {COMUCMD_ID_FPS, "get fps", &CmdRecver::cmdFuncFps},
    {COMUCMD_ID_TEST, "test", &CmdRecver::cmdFuncTest},  
    {COMUCMD_ID_CAR_SPEED, "set car speed", &CmdRecver::cmdFuncCarSpeed},      
    {COMUCMD_ID_TURN_SIGNAL, "set turn signal", &CmdRecver::cmdFuncTurnSignal},
    {COMUCMD_ID_GOBACK, "set go back", &CmdRecver::cmdFuncGoBack},
    {COMUCMD_ID_DOOR, "set door", &CmdRecver::cmdFuncDoor},
    {COMUCMD_ID_GETAUTO, "get auto", &CmdRecver::cmdFuncGetAuto},
    {COMUCMD_ID_GETRADIO, "get radio", &CmdRecver::cmdFuncGetRadio},   
    {COMUCMD_ID_SHUTDOWN, "shut down", &CmdRecver::cmdFuncShutDown}, 
    {COMUCMD_ID_GETSHUTDOWN, "get shut down", &CmdRecver::cmdFuncGetShutDown},
    {COMUCMD_ID_TEST_CAMPWR, "set camera power", &CmdRecver::cmdFuncSetCamPwr},   
    {COMUCMD_ID_ERR_VER, "set error version", &CmdRecver::cmdFuncSetErrVersion},
    {COMUCMD_ID_GETVIEW, "get view", &CmdRecver::cmdFuncGetView},
    {COMUCMD_ID_SIM_SLOT_SIG, "simulate slot", &CmdRecver::cmdFuncSimSlotSig},
    {COMUCMD_ID_WAM_DISPALY, "display status", &CmdRecver::cmdSetWamDisplayStatus},
    {COMUCMD_ID_IHU_MUTE_ATT, "ihu mute", &CmdRecver::cmdFuncSetIhuMute},
    {COMUCMD_ID_EM_FLASHER, "emergency flashers status", &CmdRecver::cmdFuncEmFlasher},
    {COMUCMD_ID_MIRROW_FOLD, "rearview mirrow fold", &CmdRecver::cmdFuncSetMirrowFold},
    {COMUCMD_ID_SETSVIMG, "set save cam image flag", &CmdRecver::cmdFuncSetSvCamFlag},
    {COMUCMD_ID_CKSVIMG, "check save cam image flag", &CmdRecver::cmdFuncGetSvCamFlag},
};


CmdRecver *CmdRecver::m_pThis = NULL;



static void cmdrecv_get_process_name(const pid_t pid, char *name)
{
    char procfile[256];
    sprintf(procfile, "/proc/%d/cmdline", pid);
    FILE *f = fopen(procfile, "r");
    if (f)
    {
        size_t size;
        size = fread(name, sizeof(char), sizeof(procfile), f);
        if (size > 0)
        {
            if ('\n' == name[size - 1])
            {
                name[size - 1] = '\0';
            }
        }
        fclose(f);
    }
}


CmdRecver::CmdRecver()
{
    CmdRecver::m_pThis = this;
}


void CmdRecver::cmdRecvProcess()
{
    // register all cmd
    // TODO: it's possible to send cmd before register all cmd
    registerAllCmd();

    // send signal to avm_comu proc.
    // send signal to avm_comu process, not qtcreater
    char parent_name[256];
    pid_t parent_pid = getppid();
    cmdrecv_get_process_name(parent_pid, parent_name);
    if (strstr(parent_name, "avm_comu") != NULL)
    {
        kill(getppid(), SIGUSR2);   // use kill function to send signal
    }   

    while (1)
    {
        comucmd_slave_proc_cmd();
    }
}


void CmdRecver::registerAllCmd()
{
    comucmd_reg_s reg;
    
    for (unsigned int i = 0; i < sizeof(g_reg_arr) / sizeof(comucmd_reg_s); i++)
    {
        reg.cmd_id = g_reg_arr[i].cmd_id;
        reg.p_desc_str = g_reg_arr[i].p_desc_str;
        reg.p_cmd_func = g_reg_arr[i].p_cmd_func;
        comucmd_slave_register_cmd(&reg);
    }  
}


int CmdRecver::cmdFuncRotateView(unsigned int cmd_id, 
    void *p_snd_data, int snd_data_size,
    void *p_ret_data, int *p_ret_data_size)
{
    cmd_id = cmd_id;
    snd_data_size = snd_data_size;
    p_ret_data = p_ret_data;
    p_ret_data_size = p_ret_data_size;

    cmd_rotate_view_s *p = (cmd_rotate_view_s *)(p_snd_data);
    emit m_pThis->rotateView(p->det_x);
    
    return 0;
}


int CmdRecver::cmdFuncChgView(unsigned int cmd_id, 
    void *p_snd_data, int snd_data_size,
    void *p_ret_data, int *p_ret_data_size)
{
    cmd_id = cmd_id;
    snd_data_size = snd_data_size;
    p_ret_data = p_ret_data;
    p_ret_data_size = p_ret_data_size;

    cmd_change_view_s *p = (cmd_change_view_s *)p_snd_data;
    emit m_pThis->changeView(p->view_enum);

    return 0;
}


int CmdRecver::cmdFuncTouch(unsigned int cmd_id, 
    void *p_snd_data, int snd_data_size,
    void *p_ret_data, int *p_ret_data_size)
{
    cmd_id = cmd_id;
    p_snd_data = p_snd_data;
    snd_data_size = snd_data_size;
    p_ret_data = p_ret_data;
    p_ret_data_size = p_ret_data_size;

    return 0;
}


int CmdRecver::cmdFuncErrorImage(unsigned int cmd_id,
    void *p_snd_data, int snd_data_size,
    void *p_ret_data, int *p_ret_data_size)
{
    cmd_id = cmd_id;
    snd_data_size = snd_data_size;
    p_ret_data = p_ret_data;
    p_ret_data_size = p_ret_data_size;

    cmd_error_image_s *p = (cmd_error_image_s *)p_snd_data;

    if (p->show_error_image)
    {
        videocap_set_video_ok(0);
    }
    else
    {
        videocap_set_video_ok(1);
    }

    return 0;
}


int CmdRecver::cmdFuncRadar(unsigned int cmd_id,
    void *p_snd_data, int snd_data_size,
    void *p_ret_data, int *p_ret_data_size)
{
    cmd_id = cmd_id;
    snd_data_size = snd_data_size;
    p_ret_data = p_ret_data;
    p_ret_data_size = p_ret_data_size;

    cmd_radar_s *p = (cmd_radar_s *)p_snd_data;

    if (p->is_on)
    {
        emit m_pThis->setRadar(p->mask[0], p->mask[1]);
    }
    else
    {
        emit m_pThis->clrRadar(p->mask[0], p->mask[1]);
    }

    return 0;
}


int CmdRecver::cmdFuncAssistLine(unsigned int cmd_id,
    void *p_snd_data, int snd_data_size,
    void *p_ret_data, int *p_ret_data_size)
{
    cmd_id = cmd_id;
    snd_data_size = snd_data_size;
    p_ret_data = p_ret_data;
    p_ret_data_size = p_ret_data_size;

    cmd_assist_line_s *p = (cmd_assist_line_s *)p_snd_data;
    float act_angle = -1.0f * p->angle;     // CAN signal, neg val is right, pos is left
    emit m_pThis->setAssistAngle(act_angle);

    return 0;
}


int CmdRecver::cmdFuncFps(unsigned int cmd_id,
    void *p_snd_data, int snd_data_size,
    void *p_ret_data, int *p_ret_data_size)
{
    cmd_id = cmd_id;
    p_snd_data = p_snd_data;
    snd_data_size = snd_data_size;
    p_ret_data = p_ret_data;
    p_ret_data_size = p_ret_data_size;

    //cmd_fps_s *p = (cmd_fps_s *)p_snd_data;
    emit m_pThis->getFps();

    return 0;
}


int CmdRecver::cmdFuncTest(unsigned int cmd_id, 
    void *p_snd_data, int snd_data_size,
    void *p_ret_data, int *p_ret_data_size)
{
    cmd_id = cmd_id;
    p_snd_data = p_snd_data;
    snd_data_size = snd_data_size;
    p_ret_data = p_ret_data;
    p_ret_data_size = p_ret_data_size;

    return 0;
}


int CmdRecver::cmdFuncCarSpeed(unsigned int cmd_id, 
    void *p_snd_data, int snd_data_size,
    void *p_ret_data, int *p_ret_data_size)
{
    cmd_id = cmd_id;
    snd_data_size = snd_data_size;
    p_ret_data = p_ret_data;
    p_ret_data_size = p_ret_data_size;

    cmd_car_speed_s *p = (cmd_car_speed_s *)p_snd_data;
    float speed = p->speed * 1.0f;
    emit m_pThis->setSpeed(speed);

    return 0;
}


int CmdRecver::cmdFuncTurnSignal(unsigned int cmd_id, 
    void *p_snd_data, int snd_data_size,
    void *p_ret_data, int *p_ret_data_size)
{
    cmd_id = cmd_id;
    snd_data_size = snd_data_size;
    p_ret_data = p_ret_data;
    p_ret_data_size = p_ret_data_size;

    cmd_turn_signal_s *p = (cmd_turn_signal_s *)p_snd_data;
    int turn = p->turn;
    emit m_pThis->setTurnSignal(turn);

    return 0;
}


int CmdRecver::cmdFuncEmFlasher(unsigned int cmd_id, 
    void *p_snd_data, int snd_data_size,
    void *p_ret_data, int *p_ret_data_size)
{
    cmd_id = cmd_id;
    snd_data_size = snd_data_size;
    p_ret_data = p_ret_data;
    p_ret_data_size = p_ret_data_size;

    int *p_state = (int *)p_snd_data;
    emit m_pThis->setEmFlahers(*p_state);

    return 0;
}



int CmdRecver::cmdFuncGoBack(unsigned int cmd_id, 
    void *p_snd_data, int snd_data_size,
    void *p_ret_data, int *p_ret_data_size)
{
    cmd_id = cmd_id;
    snd_data_size = snd_data_size;
    p_ret_data = p_ret_data;
    p_ret_data_size = p_ret_data_size;

    cmd_go_back_s *p = (cmd_go_back_s *)p_snd_data;
    int go_back = p->go_back;
    emit m_pThis->setGoBack(go_back);

    return 0;
}


int CmdRecver::cmdFuncDoor(unsigned int cmd_id, 
    void *p_snd_data, int snd_data_size,
    void *p_ret_data, int *p_ret_data_size)
{    
    cmd_id = cmd_id;
    snd_data_size = snd_data_size;
    p_ret_data = p_ret_data;
    p_ret_data_size = p_ret_data_size;


    cmd_door_s *p = (cmd_door_s *)p_snd_data;
    int front_left = p->is_front_left_door_open;
    int front_right = p->is_front_right_door_open;
    int rear_left = p->is_rear_left_door_open;
    int rear_right = p->is_rear_right_door_open;
    int front_cover = p->is_front_cover_open;
    int rear_cover = p->is_rear_cover_open;
    emit m_pThis->setDoor(front_left, front_right, rear_left, rear_right,
        front_cover, rear_cover);

    return 0;
}


int CmdRecver::cmdFuncGetAuto(unsigned int cmd_id, 
    void *p_snd_data, int snd_data_size,
    void *p_ret_data, int *p_ret_data_size)
{
    cmd_id = cmd_id;
    p_snd_data = p_snd_data;
    snd_data_size = snd_data_size;
    p_ret_data = p_ret_data;
    p_ret_data_size = p_ret_data_size;

    return 0;
}


int CmdRecver::cmdFuncGetRadio(unsigned int cmd_id, 
    void *p_snd_data, int snd_data_size,
    void *p_ret_data, int *p_ret_data_size)
{
    cmd_id = cmd_id;
    p_snd_data = p_snd_data;
    snd_data_size = snd_data_size;
    p_ret_data = p_ret_data;
    p_ret_data_size = p_ret_data_size;

    return 0;
}


int CmdRecver::cmdFuncShutDown(unsigned int cmd_id, 
    void *p_snd_data, int snd_data_size,
    void *p_ret_data, int *p_ret_data_size)
{
    cmd_id = cmd_id;
    //p_snd_data = p_snd_data;
    snd_data_size = snd_data_size;
    p_ret_data = p_ret_data;
    p_ret_data_size = p_ret_data_size;

    cmd_shutdown_s *p = (cmd_shutdown_s *)p_snd_data;

    emit m_pThis->shutDown(p->is_shut_down);

    return 0;
}


int CmdRecver::cmdFuncGetShutDown(unsigned int cmd_id, 
    void *p_snd_data, int snd_data_size,
    void *p_ret_data, int *p_ret_data_size)
{
    cmd_id = cmd_id;
    p_snd_data = p_snd_data;
    snd_data_size = snd_data_size;
    //p_ret_data = p_ret_data;
    //p_ret_data_size = p_ret_data_size;

    int *p_status = (int *)p_ret_data;
    *p_status = MainWidget::getShutSownStatus();
    *p_ret_data_size = sizeof(int);

    return 0;
}


int CmdRecver::cmdFuncSetCamPwr(unsigned int cmd_id,
    void *p_snd_data, int snd_data_size,
    void *p_ret_data, int *p_ret_data_size)
{
    cmd_id = cmd_id;
    //p_snd_data = p_snd_data;
    snd_data_size = snd_data_size;
    p_ret_data = p_ret_data;
    p_ret_data_size = p_ret_data_size;

    int *cam_pwr = (int *)p_snd_data;

    emit m_pThis->setCamPwr(*cam_pwr);

    return 0;
}


int CmdRecver::cmdFuncSetErrVersion(unsigned int cmd_id,
    void *p_snd_data, int snd_data_size,
    void *p_ret_data, int *p_ret_data_size)
{
    cmd_id = cmd_id;
    //p_snd_data = p_snd_data;
    snd_data_size = snd_data_size;
    p_ret_data = p_ret_data;
    p_ret_data_size = p_ret_data_size;

    int *p_err_version = (int *)p_snd_data;

    emit m_pThis->setErrVersion(*p_err_version);

    return 0;
}


int CmdRecver::cmdFuncGetView(unsigned int cmd_id,
    void *p_snd_data, int snd_data_size,
    void *p_ret_data, int *p_ret_data_size)
{
    cmd_id = cmd_id;
    p_snd_data = p_snd_data;
    snd_data_size = snd_data_size;
    //p_ret_data = p_ret_data;
    //p_ret_data_size = p_ret_data_size;

    int *p_status = (int *)p_ret_data;
    *p_status = MainWidget::getView();
    *p_ret_data_size = sizeof(int);

    return 0;    
}

int CmdRecver::cmdFuncSimSlotSig(unsigned int cmd_id, 
    void *p_snd_data, int snd_data_size,
    void *p_ret_data, int *p_ret_data_size)
{
    cmd_id = cmd_id;
    snd_data_size = snd_data_size;
    p_ret_data = p_ret_data;
    p_ret_data_size = p_ret_data_size;

    CMD_SLOT_SIG simSigIdx = (CMD_SLOT_SIG)((int*)p_snd_data)[0];

    switch(simSigIdx)
	{
		case CMD_SLOT_SIG_2D_BUTTON_RELEASED:
		{
			emit m_pThis->release2DButton();
		}
		break;

		case CMD_SLOT_SIG_2D_PIE_RELEASED:
		{
			emit m_pThis->release2DPie(((int*)p_snd_data)[1]);
		}
		break;

		case CMD_SLOT_SIG_3D_BUTTON_RELEASED:
		{
			emit m_pThis->release3DButton();
		}
		break;

		case CMD_SLOT_SIG_3D_PIE_RELEASED:
		{
			emit m_pThis->release3DPie(((int*)p_snd_data)[1]);
		}
		break;

		case CMD_SLOT_SIG_ASSIST_RELEASED:
		{
			emit m_pThis->releaseAssist();
		}
		break;

		case CMD_SLOT_SIG_RADAR_RELEASED:
		{
			emit m_pThis->releaseRadar();
		}
		break;

		case CMD_SLOT_SIG_RADIO_RELEASED:
		{
			emit m_pThis->releaseRadio();
		}
		break;

		default:
		    break;
	}

    return 0;
}

int CmdRecver::cmdSetWamDisplayStatus(unsigned int cmd_id,
    void *p_snd_data, int snd_data_size,
    void *p_ret_data, int *p_ret_data_size)
{
    cmd_id = cmd_id;
    snd_data_size = snd_data_size;
    p_ret_data = p_ret_data;
    p_ret_data_size = p_ret_data_size;

    cmd_wam_dispaly_s *p = (cmd_wam_dispaly_s *)p_snd_data;

    emit m_pThis->setWamDisplayStatus(p->is_wam_diaplay);

    return 0;
}


int CmdRecver::cmdFuncSetIhuMute(unsigned int cmd_id, 
        void *p_snd_data, int snd_data_size,
        void *p_ret_data, int *p_ret_data_size)
{
    cmd_id = cmd_id;
    snd_data_size = snd_data_size;
    p_ret_data = p_ret_data;
    p_ret_data_size = p_ret_data_size;

    cmd_ihu_mute_s *p = (cmd_ihu_mute_s *)p_snd_data;

    emit m_pThis->setIhuMute(p->mute_status, p->is_rescissible);

    return 0;    
}


int CmdRecver::cmdFuncSetMirrowFold(unsigned int cmd_id, 
        void *p_snd_data, int snd_data_size,
        void *p_ret_data, int *p_ret_data_size)
{
    cmd_id = cmd_id;
    snd_data_size = snd_data_size;
    p_ret_data = p_ret_data;
    p_ret_data_size = p_ret_data_size;

    rearview_mirrow_fold_s *p = (rearview_mirrow_fold_s *)p_snd_data;

    emit m_pThis->setMirrowFold(p->is_left_fold, p->is_right_fold);

    return 0;    
}


int CmdRecver::cmdFuncSetSvCamFlag(unsigned int cmd_id, 
    void *p_snd_data, int snd_data_size,
    void *p_ret_data, int *p_ret_data_size)
{
    cmd_id = cmd_id;
    p_snd_data = p_snd_data;
    snd_data_size = snd_data_size;
    p_ret_data = p_ret_data;
    p_ret_data_size = p_ret_data_size;

#if GLOBAL_RUN_ENV_DESKTOP == 0
    SyncVideoGetter::saveCamPic();
#endif    

    return 0;
}

int CmdRecver::cmdFuncGetSvCamFlag(unsigned int cmd_id, 
    void *p_snd_data, int snd_data_size,
    void *p_ret_data, int *p_ret_data_size)
{
    cmd_id = cmd_id;
    p_snd_data = p_snd_data;
    snd_data_size = snd_data_size;
    //p_ret_data = p_ret_data;
    //p_ret_data_size = p_ret_data_size;

#if GLOBAL_RUN_ENV_DESKTOP == 0
    int *p_flag = (int *)p_ret_data;
    *p_flag = SyncVideoGetter::getSaveCamPicFlag();
    *p_ret_data_size = sizeof(int);
#else
    p_ret_data = p_ret_data;
    p_ret_data_size = p_ret_data_size;
#endif       

    return 0;
}



static int shm_get_address()
{
    if (NULL == g_p_shm)
    {
        unsigned char *p_mem = NULL;
        int memsize = 0;
        comucmd_get_ext_share_mem(&p_mem, &memsize);
        if (p_mem != NULL)
        {
            g_p_shm = (ext_share_mem_s *)p_mem;
            return 0;
        }
        else
        {
            return 1;
        }
    }

    return 0;
}


void shm_set_is_manual(int is_manual)
{
    if (0 == shm_get_address())
    {
        g_p_shm->is_manual = is_manual;
    }

}


int shm_set_is_manual()
{
    if (0 == shm_get_address())
    {
        return g_p_shm->is_manual;
    }
    else
    {
        return 0;
    }
}

void shm_set_is_assist_off(int is_off)
{
    if (0 == shm_get_address())
    {
        g_p_shm->is_assist_line_off = is_off;
    }
}

int shm_get_is_assist_off()
{
    if (0 == shm_get_address())
    {
        return g_p_shm->is_assist_line_off;
    }
    else
    {
        return 0;
    }
}


void shm_set_is_radar_off(int is_off)
{
    if (0 == shm_get_address())
    {
        g_p_shm->is_radar_off = is_off;
    }
}

int shm_get_is_radar_off()
{
    if (0 == shm_get_address())
    {
        return g_p_shm->is_radar_off;
    }
    else
    {
        return 0;
    }
}

void shm_set_is_radio_off(int is_off)
{
    if (0 == shm_get_address())
    {
        g_p_shm->is_radio_off = is_off;
    }
}


int shm_get_is_radio_off()
{
    if (0 == shm_get_address())
    {
        return g_p_shm->is_radio_off;
    }
    else
    {
        return 0;
    }
}


void shm_set_is_closed(int is_close)
{
    if (0 == shm_get_address())
    {
        g_p_shm->is_close = is_close;
    }    
}

int shm_get_is_closed()
{
    if (0 == shm_get_address())
    {
        return g_p_shm->is_close;
    }
    else
    {
        return 0;
    }    
}


void shm_set_err_screen(int is_err_screen, int type)
{
    if (0 == shm_get_address())
    {
        g_p_shm->is_err_screen[0] = is_err_screen;
        g_p_shm->is_err_screen[1] = type;
    }      
}



void shm_set_cam_status(int is_error, 
    int err_right, int err_front, int err_left, int err_rear)
{
    if (0 == shm_get_address())
    {
        g_p_shm->is_cam_error[0] = is_error;
        g_p_shm->is_cam_error[1] = err_right;
        g_p_shm->is_cam_error[2] = err_front;
        g_p_shm->is_cam_error[3] = err_left;
        g_p_shm->is_cam_error[4] = err_rear;
    }    
}



