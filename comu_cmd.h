
#ifndef _COMU_CMD_H_
#define _COMU_CMD_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "common_def.h"


#define COMUCMD_PRT(...)  AVM_LOG(__VA_ARGS__)


typedef int (*comucmd_cmd_func_t)(unsigned int cmd_id, 
    void *p_snd_data, int snd_data_size, 
    void *p_ret_data, int *p_ret_data_size);


typedef struct _comucmd_reg_struct_
{
    unsigned int cmd_id;
    const char *p_desc_str;
    comucmd_cmd_func_t p_cmd_func;
}comucmd_reg_s;


int comucmd_host_init(int *p_err_mem_exist, int *p_err_mem_ready);


int comucmd_slave_init();


int comucmd_slave_register_cmd(comucmd_reg_s *p_reg);


int comucmd_host_send_cmd(unsigned int cmd_id, void *p_snd_data, int snd_data_size, void *p_ret_data, int *p_ret_data_size);


int comucmd_slave_proc_cmd();


int comucmd_get_ext_share_mem(unsigned char **p_mem, int *p_mem_size);


/**
* some comu's print information(use fileprt_prt func) can be stored in shm memory, 
* and then qt print these logs through fileprt_dumptofile func
*/
int fileprt_init(unsigned char *p_mem, int mem_len);


int fileprt_prt(const char *format, ...);

int fileprt_dumptofile(unsigned char *p_mem, char *name);

#ifdef __cplusplus
}
#endif

#endif