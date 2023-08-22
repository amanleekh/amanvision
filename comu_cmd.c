
#include <fcntl.h>
#include <stdio.h>
#include <errno.h>
#include <stddef.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <semaphore.h>
#include <stdarg.h>


#include "comu_cmd_define.h"
#include "comu_cmd.h"


#define COMUCMD_SHARE_MEM_NAME  ("/comucmd_mem")
#define COMUCMD_SND_MEM_SIZE    (64*1024)
#define COMUCMD_RET_MEM_SIZE    (1*1024)
#define COMUCMD_EXT_SHARE_MEM   (6*1024)
#define COMUCMD_CMD_TIMEOUT_S   (1)
#define COMUCMD_MAX_CMD_NUM     (64)
#define COMUCMD_VERSION         (0x01)
#define COMUCMD_MAGIC           (0x34578962)


typedef struct _comucmd_struct_
{
    sem_t sem_cmd;
    sem_t sem_signal_s2r;
    sem_t sem_signal_r2s;
    int cur_cmd_num;
    comucmd_reg_s cmd[COMUCMD_MAX_CMD_NUM];
    unsigned int version;
    unsigned int magic;
    int salve_init_ready;

    unsigned int tmp_cmd_id;
    int tmp_snd_data_size;
    int tmp_ret_data_size;
    int tmp_cmd_not_exist;
}comucmd_s;


/**
* for comu printf info store
*/
typedef struct _fileprt_struct_
{
	int r_idx;
	int w_idx;
	int cur_len;
	int max_len;
	int is_status_ok;
	sem_t lock;
	unsigned char data[0];
}fileprt_s;


static comucmd_s *g_pcomucmd = NULL;

/**< for comu printf. print info to shm mem */
static fileprt_s *g_fileprt = NULL;


int comucmd_host_init(int *p_err_mem_exist, int *p_err_mem_ready)
{
    int shm_fd = -1;
    comucmd_s *p_comu = NULL;
    
    if ((NULL == p_err_mem_exist) || (NULL == p_err_mem_ready))
    {
        COMUCMD_PRT("comucmd_host_init faild. param error\n");
        return 0;
    }
    *p_err_mem_exist = 0;
    *p_err_mem_ready = 0;

    int memsize = COMUCMD_RET_MEM_SIZE + COMUCMD_SND_MEM_SIZE 
        + COMUCMD_EXT_SHARE_MEM + sizeof(comucmd_s);
    shm_fd = shm_open(COMUCMD_SHARE_MEM_NAME, O_RDWR, 0777);
    if (shm_fd < 0)
    {
        *p_err_mem_exist = 1;
        //printf("comucmd_host_init. shm_open faild. errno=%d\n", errno);
        return 1;
    }
    if (ftruncate(shm_fd, memsize) < 0)
    {
        COMUCMD_PRT("comucmd_host_init. ftruncate faild\n");
        return 1;
    }
    p_comu = (comucmd_s *)mmap(0, memsize, PROT_READ | PROT_WRITE,  MAP_SHARED, shm_fd, 0);
    if (NULL == p_comu)
    {
        COMUCMD_PRT("comucmd_host_init. mmap faild\n");
        return 1;
    }
    if (0 == p_comu->salve_init_ready)
    {
        *p_err_mem_ready = 1;
        //COMUCMD_PRT("comucmd_host_init. salve is not ready\n");
        return 1;       
    }
    p_comu->salve_init_ready = 0;
    if ((p_comu->version != COMUCMD_VERSION) || (p_comu->magic != COMUCMD_MAGIC))
    {
        COMUCMD_PRT("comucmd_host_init. version or magic is not match\n");
        return 1;
    }
    // check 'ext_share_mem_s' size
    ext_share_mem_s *p_share = (ext_share_mem_s *)((unsigned char *)p_comu 
        + sizeof(comucmd_s)
        + COMUCMD_SND_MEM_SIZE + COMUCMD_RET_MEM_SIZE);    
    if (p_share->this_struct_size != sizeof(ext_share_mem_s))
    {
        COMUCMD_PRT("comucmd_host_init. sizeof 'ext_share_mem_s' not match\n");
        return 1;        
    }
    g_pcomucmd = p_comu;
    return 0;
}


int comucmd_slave_init()
{
    int shm_fd = -1;
    comucmd_s *p_comu = NULL;

    int memsize = COMUCMD_RET_MEM_SIZE + COMUCMD_SND_MEM_SIZE 
        + COMUCMD_EXT_SHARE_MEM + sizeof(comucmd_s);
    shm_unlink(COMUCMD_SHARE_MEM_NAME);
    shm_fd = shm_open(COMUCMD_SHARE_MEM_NAME, O_CREAT | O_RDWR | O_EXCL, 0777);
    if (shm_fd < 0)
    {
        COMUCMD_PRT("comucmd_slave_init. shm_open faild. errno=%d\n", errno);
        return 1;
    }
    if (ftruncate(shm_fd, memsize) < 0)
    {
        COMUCMD_PRT("comucmd_slave_init. ftruncate faild\n");
        return 1;
    }
    p_comu = (comucmd_s *)mmap(0, memsize, PROT_READ | PROT_WRITE,  MAP_SHARED, shm_fd, 0);
    if (NULL == p_comu)
    {
        COMUCMD_PRT("comucmd_slave_init. mmap faild\n");
        return 1;
    }
    memset(p_comu, 0, memsize);
    p_comu->version = COMUCMD_VERSION;
    p_comu->magic = COMUCMD_MAGIC;

    if (sem_init(&(p_comu->sem_cmd), 1, 1) < 0)
    {
        COMUCMD_PRT("comucmd_slave_init. sem_init sem_cmd faild\n");
        return 1;
    }
    if (sem_init(&(p_comu->sem_signal_s2r), 1, 0) < 0)
    {
        COMUCMD_PRT("comucmd_slave_init. sem_init sem_s2r faild\n");
        return 1;
    }
    if (sem_init(&(p_comu->sem_signal_r2s), 1, 0) < 0)
    {
        COMUCMD_PRT("comucmd_slave_init. sem_init sem_r2s faild\n");
        return 1;
    }
    // set ext_share_mem_s struct size
    ext_share_mem_s *p_share = (ext_share_mem_s *)((unsigned char *)p_comu 
        + sizeof(comucmd_s)
        + COMUCMD_SND_MEM_SIZE + COMUCMD_RET_MEM_SIZE);
    p_share->this_struct_size = sizeof(ext_share_mem_s);

    // set ok
    p_comu->salve_init_ready = 1;

    g_pcomucmd = p_comu;
    return 0;    
}


int comucmd_slave_register_cmd(comucmd_reg_s *p_reg)
{
    int i;
    comucmd_s *p_comu = g_pcomucmd;

    if ((NULL == p_reg) || (NULL == p_comu))
    {
        COMUCMD_PRT("comucmd_slave_register_cmd. param error\n");
        return 1;       
    }

    sem_wait(&(p_comu->sem_cmd));
    if (p_comu->cur_cmd_num >= COMUCMD_MAX_CMD_NUM)
    {
        sem_post(&(p_comu->sem_cmd));
        COMUCMD_PRT("comucmd_slave_register_cmd. cur cmd num is max num\n");
        return 1;        
    }
    for (i = 0; i < p_comu->cur_cmd_num; i++)
    {
        if (p_comu->cmd[i].cmd_id == p_reg->cmd_id)
        {
            sem_post(&(p_comu->sem_cmd));
            COMUCMD_PRT("comucmd_slave_register_cmd. cmd_id is same as %d cmd\n", i);
            return 1;
        }
    }
    comucmd_reg_s *p_cur_comu_cmd = &(p_comu->cmd[p_comu->cur_cmd_num]);
    memcpy(p_cur_comu_cmd, p_reg, sizeof(comucmd_reg_s));
    p_comu->cur_cmd_num++;
    sem_post(&(p_comu->sem_cmd));

    return 0;
}


int comucmd_host_send_cmd(unsigned int cmd_id, void *p_snd_data, int snd_data_size, void *p_ret_data, int *p_ret_data_size)
{
    comucmd_s *p_comu = g_pcomucmd;

    if ((NULL == p_comu))
    {
        COMUCMD_PRT("comucmd_host_send_cmd. param error1\n");
        return 1;         
    }
    if ((snd_data_size < 0) || (snd_data_size > COMUCMD_SND_MEM_SIZE) || ((snd_data_size != 0) && (NULL == p_snd_data)))
    {
        COMUCMD_PRT("comucmd_host_send_cmd. param error2\n");
        return 1;          
    }
    if (p_ret_data_size != NULL)
    {
        if ((*p_ret_data_size < 0) || (*p_ret_data_size > COMUCMD_RET_MEM_SIZE) || ((*p_ret_data_size != 0) && (NULL == p_ret_data)))
        {
            COMUCMD_PRT("comucmd_host_send_cmd. param error3\n");
            return 1;    
        }
    }

    sem_wait(&(p_comu->sem_cmd));

    unsigned char *p_shm_send_mem = (unsigned char *)p_comu + sizeof(comucmd_s);
    unsigned char *p_shm_recv_mem = (unsigned char *)p_shm_send_mem + COMUCMD_SND_MEM_SIZE;

    p_comu->tmp_cmd_id = cmd_id;
    p_comu->tmp_snd_data_size = snd_data_size;
    if (p_ret_data_size != NULL)
    {
        p_comu->tmp_ret_data_size = *p_ret_data_size;
    }
    else
    {
        p_comu->tmp_ret_data_size = 0;
    }
    p_comu->tmp_cmd_not_exist = 0;
    memcpy(p_shm_send_mem, p_snd_data, snd_data_size);
    
    sem_post(&(p_comu->sem_signal_s2r));
    sem_wait(&(p_comu->sem_signal_r2s));
    
    // cmd not exist
    if (p_comu->tmp_cmd_not_exist)
    {
        sem_post(&(p_comu->sem_cmd));
        COMUCMD_PRT("comucmd_host_send_cmd. cmd_id=%u not exist\n", cmd_id);
        return 1;
    }
    if (p_ret_data_size != NULL)
    {
        if ((p_comu->tmp_ret_data_size < 0) || (p_comu->tmp_ret_data_size > COMUCMD_RET_MEM_SIZE))
        {
            sem_post(&(p_comu->sem_cmd));
            COMUCMD_PRT("comucmd_host_send_cmd. ret data size if not correct\n");
            return 1;
        }
        memcpy(p_ret_data, p_shm_recv_mem, p_comu->tmp_ret_data_size);
        *p_ret_data_size = p_comu->tmp_ret_data_size;
    }

    sem_post(&(p_comu->sem_cmd));

    return 0;  
}


int comucmd_slave_proc_cmd()
{
    int i;
    comucmd_s *p_comu = g_pcomucmd;
    if (NULL == p_comu)
    {
        COMUCMD_PRT("comucmd_slave_proc_cmd. param error\n");
        return 1;
    }

    sem_wait(&(p_comu->sem_signal_s2r));

    for (i = 0; i < p_comu->cur_cmd_num; i++)
    {
        if (p_comu->cmd[i].cmd_id == p_comu->tmp_cmd_id)
        {
            break;
        }
    }
    if (i == p_comu->cur_cmd_num)
    {
        p_comu->tmp_cmd_not_exist = 1;
        sem_post(&(p_comu->sem_signal_r2s));
        COMUCMD_PRT("comucmd_slave_proc_cmd. unknow cmd_id=%u\n", p_comu->tmp_cmd_id);
        return 1;
    }
    unsigned char *p_shm_send_mem = (unsigned char *)p_comu + sizeof(comucmd_s);
    unsigned char *p_shm_recv_mem = (unsigned char *)p_shm_send_mem + COMUCMD_SND_MEM_SIZE;    
    (*(p_comu->cmd[i].p_cmd_func))(p_comu->tmp_cmd_id, p_shm_send_mem, p_comu->tmp_snd_data_size,
        p_shm_recv_mem, &(p_comu->tmp_ret_data_size));

    sem_post(&(p_comu->sem_signal_r2s));

    return 0;
}


int comucmd_get_ext_share_mem(unsigned char **p_mem, int *p_mem_size)
{
    comucmd_s *p_comu = g_pcomucmd;
    
    if ((NULL == p_comu) || (NULL == p_mem) || (NULL == p_mem_size))
    {
        COMUCMD_PRT("comucmd_get_ext_share_mem. param error\n");
        return 1;         
    }
    *p_mem = (unsigned char *)p_comu + sizeof(comucmd_s)
        + COMUCMD_SND_MEM_SIZE + COMUCMD_RET_MEM_SIZE;
    *p_mem_size = COMUCMD_EXT_SHARE_MEM;

    return 0;
}


int fileprt_init(unsigned char *p_mem, int mem_len)
{
    if ((NULL == p_mem) || (mem_len < (int)(sizeof(fileprt_s))) || (g_fileprt != NULL))
    {
        return 1;
    }
    memset(p_mem, 0, mem_len);
    fileprt_s *ps = (fileprt_s *)p_mem;
    ps->r_idx = 0;
    ps->w_idx = 0;
    ps->cur_len = 0;
    ps->max_len = mem_len - sizeof(fileprt_s);
    if (sem_init(&(ps->lock), 1, 1) != 0)
    {
        return 1;
    }
    ps->is_status_ok = 1;

    g_fileprt = (fileprt_s *)p_mem;
    return 0;
}


int fileprt_prt(const char *format, ...)
{
#define FILEPRT_MAX_MSG_LEN     (256)

    fileprt_s *ps = g_fileprt;
    if (NULL == ps)
    {
        return 1;
    }
    if (0 == ps->is_status_ok)
    {
        return 1;
    }

    // print strint to tmp_str first
    char tmp_str[FILEPRT_MAX_MSG_LEN];
    int act_str_size = -1;
    
    va_list args;
    va_start(args, format);
    act_str_size = vsnprintf(tmp_str, FILEPRT_MAX_MSG_LEN, format, args);
    va_end(args);
    if (act_str_size <= 0)
    {
        return 1;
    }
    else if (act_str_size >= FILEPRT_MAX_MSG_LEN)
    {
        tmp_str[FILEPRT_MAX_MSG_LEN - 1] = '\n';
        tmp_str[FILEPRT_MAX_MSG_LEN - 2] = '.';
        tmp_str[FILEPRT_MAX_MSG_LEN - 3] = '.';
        tmp_str[FILEPRT_MAX_MSG_LEN - 4] = '.';
        act_str_size = FILEPRT_MAX_MSG_LEN;        
    }

    sem_wait(&(ps->lock));
    if (ps->cur_len + act_str_size > ps->max_len)
    {
        ps->r_idx = (ps->r_idx + ps->cur_len / 2) % ps->max_len;
        ps->cur_len = ps->cur_len - ps->cur_len / 2;
    }
    if (ps->max_len - ps->cur_len < act_str_size)
    {
        sem_post(&(ps->lock));
        return 1;
    }
    if (ps->w_idx + act_str_size <= ps->max_len)
    {
        memcpy(ps->data + ps->w_idx, tmp_str, act_str_size);
    }
    else
    {
        int len1 = ps->max_len - ps->w_idx;
        int len2 = act_str_size - len1;
        memcpy(ps->data + ps->w_idx, tmp_str, len1);
        memcpy(ps->data, tmp_str + len1, len2);
    }
    ps->w_idx = (ps->w_idx + act_str_size) % ps->max_len;
    ps->cur_len += act_str_size;
    sem_post(&(ps->lock));
    
    return 0;
}


int fileprt_dumptofile(unsigned char *p_mem, char *name)
{   
    fileprt_s *ps = (fileprt_s *)p_mem;
    if (NULL == ps)
    {
        return 1;
    }
    if (0 == ps->is_status_ok)
    {
        return 1;
    }

    FILE *fs = fopen(name, "wb+");
    if (NULL == fs)
    {
        return 1;
    }

    sem_wait(&(ps->lock));
    if (ps->cur_len <= 0)
    {
        sem_post(&(ps->lock));
        return 0;
    }
    if (ps->w_idx > ps->r_idx)
    {
        fwrite(ps->data + ps->r_idx, ps->cur_len, 1, fs);
    }
    else
    {

        int len1 = ps->max_len - ps->r_idx;
        int len2 = ps->cur_len - len1;
        fwrite(ps->data + ps->r_idx, len1, 1, fs);
        fwrite(ps->data, len2, 1, fs);
    }
    sem_post(&(ps->lock));
    fclose(fs);

    return 0;
}



