#ifndef CMD_RECVER_H
#define CMD_RECVER_H

#include <QObject>
#include <QWidget>

#include "comu_cmd.h"
#include "comu_cmd_define.h"

/**
 * @brief The CmdRecver class
 */
class CmdRecver : public QObject
{
    Q_OBJECT

public:
    CmdRecver();

public slots:
    // cmd process thread func, get cmd from avm_comu proc
    // this process function is start by a worker thread
    void cmdRecvProcess();

private:
    // register all comu cmd
    void registerAllCmd();

signals:
    // signals bind to surround_widget, when cmd want rotate view, send signal to surround_widget
    void rotateView(int det_x);
    void changeView(int view_enum);
    void setRadar(unsigned int mask, unsigned int mask2);
    void clrRadar(unsigned int mask, unsigned int mask2);
    void setAssistAngle(float angle);   /**< >0 right, <0 left */
    void getFps();
    void test();
    void setSpeed(float speed);     /**< neg or pos */
    void setTurnSignal(int turn);   /**< -1 left, 0 no turn, 1 turn right */
    void setEmFlahers(int is_on);   /**< 0 off, 1 on */
    void setGoBack(int go_back);     /**< 0 stop, 1 go, -1 back */
    void setDoor(int front_left, int front_right, int rear_left, int rear_right,
        int front_cover, int rear_cover);   /**< 0 close, 1 open */
    void shutDown(int is_shutdown); /**< 1 for shutdown, 0 for restart */
    void setCamPwr(int pwr_status); /**< 0 to shutdown camera power */
    void setErrVersion(int is_error); /**<  */
	void release2DButton(void);
	void release2DPie(int idx);
	void release3DButton(void);
	void release3DPie(int idx);
	void releaseAssist(void);
	void releaseRadar(void);
	void releaseRadio(void);
    void setWamDisplayStatus(int updatedStatus);
    /*
     * mute_status:  1: The logic in IHU is for mute
     *               0: The logic in IHU is for not mute
     *
     * is_rescissible : 1 mute status can be changed by WAM
     *                : 0 mute status can not be changed by WAM
     */
    void setIhuMute(int mute_status, int is_rescissible);

    /**
    * is rearview mirrow fold
    */
    void setMirrowFold(int is_left_fold, int is_right_fold);
    
public:
    static CmdRecver *m_pThis;

public:
    // cmd functions. register to comu module
    static int cmdFuncRotateView(unsigned int cmd_id,
        void *p_snd_data, int snd_data_size,
        void *p_ret_data, int *p_ret_data_size);

    static int cmdFuncChgView(unsigned int cmd_id, 
        void *p_snd_data, int snd_data_size,
        void *p_ret_data, int *p_ret_data_size);

    static int cmdFuncTouch(unsigned int cmd_id, 
        void *p_snd_data, int snd_data_size,
        void *p_ret_data, int *p_ret_data_size); 

    static int cmdFuncErrorImage(unsigned int cmd_id,
        void *p_snd_data, int snd_data_size,
        void *p_ret_data, int *p_ret_data_size);

    static int cmdFuncRadar(unsigned int cmd_id,
        void *p_snd_data, int snd_data_size,
        void *p_ret_data, int *p_ret_data_size);

    static int cmdFuncAssistLine(unsigned int cmd_id,
        void *p_snd_data, int snd_data_size,
        void *p_ret_data, int *p_ret_data_size);

    static int cmdFuncFps(unsigned int cmd_id,
        void *p_snd_data, int snd_data_size,
        void *p_ret_data, int *p_ret_data_size);

    static int cmdFuncTest(unsigned int cmd_id, 
        void *p_snd_data, int snd_data_size,
        void *p_ret_data, int *p_ret_data_size);     

    static int cmdFuncCarSpeed(unsigned int cmd_id, 
        void *p_snd_data, int snd_data_size,
        void *p_ret_data, int *p_ret_data_size);      

    static int cmdFuncTurnSignal(unsigned int cmd_id, 
        void *p_snd_data, int snd_data_size,
        void *p_ret_data, int *p_ret_data_size); 

    static int cmdFuncEmFlasher(unsigned int cmd_id, 
        void *p_snd_data, int snd_data_size,
        void *p_ret_data, int *p_ret_data_size); 

    static int cmdFuncGoBack(unsigned int cmd_id, 
        void *p_snd_data, int snd_data_size,
        void *p_ret_data, int *p_ret_data_size); 

    static int cmdFuncDoor(unsigned int cmd_id, 
        void *p_snd_data, int snd_data_size,
        void *p_ret_data, int *p_ret_data_size); 

    static int cmdFuncGetAuto(unsigned int cmd_id, 
        void *p_snd_data, int snd_data_size,
        void *p_ret_data, int *p_ret_data_size); 

    static int cmdFuncGetRadio(unsigned int cmd_id, 
        void *p_snd_data, int snd_data_size,
        void *p_ret_data, int *p_ret_data_size); 

    static int cmdFuncShutDown(unsigned int cmd_id, 
        void *p_snd_data, int snd_data_size,
        void *p_ret_data, int *p_ret_data_size);

    static int cmdFuncGetShutDown(unsigned int cmd_id, 
        void *p_snd_data, int snd_data_size,
        void *p_ret_data, int *p_ret_data_size);
        
    static int cmdFuncSetCamPwr(unsigned int cmd_id, 
        void *p_snd_data, int snd_data_size,
        void *p_ret_data, int *p_ret_data_size); 

    static int cmdFuncSetErrVersion(unsigned int cmd_id, 
        void *p_snd_data, int snd_data_size,
        void *p_ret_data, int *p_ret_data_size); 

    static int cmdFuncGetView(unsigned int cmd_id, 
        void *p_snd_data, int snd_data_size,
        void *p_ret_data, int *p_ret_data_size);         

	static int cmdFuncSimSlotSig(unsigned int cmd_id, 
        void *p_snd_data, int snd_data_size,
        void *p_ret_data, int *p_ret_data_size);

    static int cmdSetWamDisplayStatus(unsigned int cmd_id,
        void *p_snd_data, int snd_data_size,
        void *p_ret_data, int *p_ret_data_size);

    static int cmdFuncSetIhuMute(unsigned int cmd_id, 
        void *p_snd_data, int snd_data_size,
        void *p_ret_data, int *p_ret_data_size);   

    static int cmdFuncSetMirrowFold(unsigned int cmd_id, 
        void *p_snd_data, int snd_data_size,
        void *p_ret_data, int *p_ret_data_size);   

    static int cmdFuncSetSvCamFlag(unsigned int cmd_id, 
        void *p_snd_data, int snd_data_size,
        void *p_ret_data, int *p_ret_data_size); 

    static int cmdFuncGetSvCamFlag(unsigned int cmd_id, 
        void *p_snd_data, int snd_data_size,
        void *p_ret_data, int *p_ret_data_size);           
};


void shm_set_is_manual(int is_manual);

int shm_set_is_manual();

void shm_set_is_assist_off(int is_off);

int shm_get_is_assist_off();

void shm_set_is_radar_off(int is_off);

int shm_get_is_radar_off();

void shm_set_is_radio_off(int is_off);

int shm_get_is_radio_off();

void shm_set_is_closed(int is_close);

int shm_get_is_closed();

void shm_set_err_screen(int is_err_screen, int type);

void shm_set_cam_status(int is_error, int err_right, int err_front, int err_left, int err_rear);


#endif // CMD_RECVER_H
