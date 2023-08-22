
#include <fcntl.h>
#include <errno.h>
#include <time.h>
#include <stdio.h>
#include <stdarg.h>
#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <semaphore.h>

#include "memlog.h"

#define MEMLOG_VERSION      (0x01)
#define MEMLOG_MAGIC        (0x12345678)
#define MEMLOG_MAX_MSG_LEN  (256)       // must be multi of sizeof(int), since align calc
#define MEMLOG_PAIR_SIZE    (128)

// total size is 32K(per blk)*16(blk)=512k
#define MEMLOG_MEMBLK_SIZE  (32*1024)
#define MEMLOG_MEMBLK_NUM   (16)
#define MEMLOG_MEM_SIZE     (MEMLOG_MEMBLK_SIZE*MEMLOG_MEMBLK_NUM)

#define MEMLOG_NAME         ("/memlog_mem")
#define MEMLOG_SEM_NAME     ("/memlog_sem")


typedef struct _memlog_line_struct_
{
    int lvl;
    unsigned int time_ms;
    unsigned int nxt_line_pos;
}memlog_line_s;


typedef struct _memlog_block_struct_
{
    unsigned int version;
    unsigned int magic;
    unsigned int fst_line_pos;
    unsigned int lst_line_pos;
}memlog_block_s;


typedef struct _memlog_struct_
{
    unsigned int version;
    unsigned int magic;
    int server_ready_flag;
    int blk_r_idx;
    int blk_w_idx;
    int log_lvl;
    int not_prt_console;
    int not_prt_shm;
    int is_prt_time_ms;
    int is_prt_lvl;
    int is_have_str;
    unsigned int blk_pos[MEMLOG_MEMBLK_NUM];
    sem_t *sem;
    unsigned char pair[MEMLOG_PAIR_SIZE];
}memlog_s;


typedef struct _memlog_client_struct_
{
    sem_t *sem;
    unsigned char *p_dump_mem;
    unsigned char *p_single_blk_mem;
}memlog_client_s;


static memlog_s *g_pmemlog = NULL;


static memlog_client_s *g_client_pmemlog = NULL;


unsigned int memlog_gettime_ms()
{
    // get time ms since system start
    struct timespec te;
    clock_gettime(CLOCK_MONOTONIC, &te);
    unsigned int milliseconds = te.tv_sec * 1000L + te.tv_nsec / 1000000L; 
    return milliseconds;
}


int memlog_server_init()
{
    int i;    
    int shm_fd = -1;
    memlog_s *p_memlog = NULL;

    // alloc shared mem
    int memsize = MEMLOG_MEM_SIZE + sizeof(memlog_s);
    shm_unlink(MEMLOG_NAME);
    shm_fd = shm_open(MEMLOG_NAME, O_CREAT | O_RDWR | O_EXCL, 0777);
    if (shm_fd < 0)
    {
        //MEMLOG_PRT("memlog_server_init. shm_open faild. errno=%d\n", errno);
        return 1;
    }
    if (ftruncate(shm_fd, memsize) < 0)
    {
        MEMLOG_PRT("memlog_server_init. ftruncate faild\n");
        return 1;
    }
    p_memlog = (memlog_s *)mmap(0, memsize, PROT_READ | PROT_WRITE,  MAP_SHARED, shm_fd, 0);
    if (NULL == p_memlog)
    {
        MEMLOG_PRT("memlog_server_init. mmap faild\n");
        return 1;
    }
    memset(p_memlog, 0, memsize);

    // init memlog struct val
    p_memlog->version = MEMLOG_VERSION;
    p_memlog->magic = MEMLOG_MAGIC;
    p_memlog->blk_r_idx = 0;
    p_memlog->blk_w_idx = 0;
    p_memlog->log_lvl = -1;
    p_memlog->not_prt_console = 1;
    p_memlog->not_prt_shm = 0;
    p_memlog->is_prt_time_ms = 1;
    p_memlog->is_prt_lvl = 0;
    
    // init every blk struct
    memlog_block_s *p_blk = (memlog_block_s *)((unsigned char *)p_memlog + sizeof(memlog_s));
    unsigned int blk_pos = sizeof(memlog_s);
    for (i = 0; i < MEMLOG_MEMBLK_NUM; i++)
    {
        p_blk->version = MEMLOG_VERSION;
        p_blk->magic = MEMLOG_MAGIC;
        p_blk->fst_line_pos = sizeof(memlog_block_s);
        p_blk->lst_line_pos = sizeof(memlog_block_s);
        p_memlog->blk_pos[i] = blk_pos;

        p_blk = (memlog_block_s *)((unsigned char *)p_blk + MEMLOG_MEMBLK_SIZE);
        blk_pos = blk_pos + MEMLOG_MEMBLK_SIZE;
    }
    sem_unlink(MEMLOG_SEM_NAME);
    p_memlog->sem = sem_open(MEMLOG_SEM_NAME, O_CREAT | O_EXCL, 0777, 1);
    if (SEM_FAILED == p_memlog->sem)
    {
        MEMLOG_PRT("memlog_server_init. sem_open faild\n");
        return 1;
    }

    p_memlog->server_ready_flag = 1;
    g_pmemlog = p_memlog;
    memlog_server_printf(1, "memlog_server_init success. version=0x%x\n", MEMLOG_VERSION);

    return 0;
}


int memlog_server_set_log_lvl(int lvl)
{
    memlog_s *p_memlog = g_pmemlog;
    if (NULL == p_memlog)
    {
        MEMLOG_PRT("memlog_server_set_log_lvl faild. param error\n");
        return 1;
    }

    p_memlog->log_lvl = lvl;

    return 0;
}

int memlog_server_set_prt_console(int flag)
{
    memlog_s *p_memlog = g_pmemlog;
    if (NULL == p_memlog)
    {
        MEMLOG_PRT("memlog_server_set_prt_console faild. param error\n");
        return 1;
    }

    if (flag)
    {
        p_memlog->not_prt_console = 0;
    }
    else
    {
        p_memlog->not_prt_console = 1;
    }

    return 0;
}


int memlog_server_set_prt_shm(int flag)
{
    memlog_s *p_memlog = g_pmemlog;
    if (NULL == p_memlog)
    {
        MEMLOG_PRT("memlog_server_set_prt_shm faild. param error\n");
        return 1;
    }

    if (flag)
    {
        p_memlog->not_prt_shm = 0;
    }
    else
    {
        p_memlog->not_prt_shm = 1;
    }

    return 0;
}

int memlog_server_set_pair(int idx, unsigned char val)
{
    memlog_s *p_memlog = g_pmemlog;
    if (NULL == p_memlog)
    {
        MEMLOG_PRT("memlog_server_set_pair faild. param error\n");
        return 1;
    }

    p_memlog->pair[idx] = val;
    return 0;
}

int memlog_server_printf(int lvl, const char *format, ...)
{
    memlog_s *p_memlog = g_pmemlog;
    if (NULL == p_memlog)
    {
        MEMLOG_PRT("memlog_server_printf. param error\n");
        return 1;
    }
    if (lvl <= p_memlog->log_lvl)
    {
        return 0;
    }
    unsigned int time_ms = memlog_gettime_ms();

    va_list args;
    if (0 == p_memlog->not_prt_console)
    {
        va_start(args, format);
        if ((0 == p_memlog->is_prt_lvl) && (0 == p_memlog->is_prt_time_ms))
        {   
            MEMLOG_PRT_ARG(format, args);
            //printf("[AVMS]");
            //vprintf(__VA_ARGS__);
        }
        else if ((0 == p_memlog->is_prt_lvl) && (1 == p_memlog->is_prt_time_ms))
        {
            printf("[AVMS][%u]", time_ms);
            vprintf(format, args);
        }
        else // if ((1 == p_memlog->is_prt_lvl) && (1 == p_memlog->is_prt_time_ms))
        {
            printf("[AVMS][%d][%u]", lvl, time_ms);
            vprintf(format, args);
        }
        va_end(args);
    }
    if (1 == p_memlog->not_prt_shm)
    {
        return 0;
    }

    // print strint to tmp_str
    char tmp_str[MEMLOG_MAX_MSG_LEN];
    int act_str_size = -1;
    
    va_start(args, format);
    act_str_size = vsnprintf(tmp_str, MEMLOG_MAX_MSG_LEN, format, args);
    va_end(args);
    if (act_str_size <= 0)
    {
        return 1;
    }
    else if (act_str_size >= MEMLOG_MAX_MSG_LEN)
    {
        tmp_str[MEMLOG_MAX_MSG_LEN - 1] = '\0';
        tmp_str[MEMLOG_MAX_MSG_LEN - 2] = '\n';
        tmp_str[MEMLOG_MAX_MSG_LEN - 3] = '.';
        tmp_str[MEMLOG_MAX_MSG_LEN - 4] = '.';
        tmp_str[MEMLOG_MAX_MSG_LEN - 5] = '.';
        act_str_size = MEMLOG_MAX_MSG_LEN;        
    }
    else
    {
        int i;
        // strlen + 1 ('\0'), so write size is act_str_size+1
        // since MEMLOG_MAX_MSG_LEN is multi of sizeof(int), so align will not bigger than MEMLOG_MAX_MSG_LEN
        int align_str_size = (act_str_size + 1 + sizeof(int) - 1) / sizeof(int) * sizeof(int);
        for (i = act_str_size; i < align_str_size; i++)
        {
            tmp_str[i] = '\0';
        }
        act_str_size = align_str_size;
    }
    
    // copy tmp_str to blk mem
    sem_wait(p_memlog->sem);
    memlog_block_s *p_blk = (memlog_block_s *)((unsigned char *)p_memlog + p_memlog->blk_pos[p_memlog->blk_w_idx]);
    int lst_str_size = MEMLOG_MEMBLK_SIZE - p_blk->lst_line_pos - sizeof(memlog_line_s);
    if (lst_str_size < act_str_size)
    {
        p_memlog->blk_w_idx++;
        if (p_memlog->blk_w_idx >= MEMLOG_MEMBLK_NUM)
        {
            p_memlog->blk_w_idx = 0;
        }
        // mem is full, inc ridx
        if (p_memlog->blk_w_idx == p_memlog->blk_r_idx)
        {
            p_memlog->blk_r_idx++;
            if (p_memlog->blk_r_idx >= MEMLOG_MEMBLK_NUM)
            {
                p_memlog->blk_r_idx = 0;
            }
        }
        p_blk = (memlog_block_s *)((unsigned char *)p_memlog + p_memlog->blk_pos[p_memlog->blk_w_idx]);
        p_blk->fst_line_pos = sizeof(memlog_block_s);
        p_blk->lst_line_pos = sizeof(memlog_block_s);
    }
    memlog_line_s *p_line = (memlog_line_s *)((unsigned char *)p_blk + p_blk->lst_line_pos);
    unsigned char *p_str = (unsigned char *)p_line + sizeof(memlog_line_s);

    p_line->lvl = lvl;
    p_line->time_ms = time_ms;
    p_line->nxt_line_pos = p_blk->lst_line_pos + sizeof(memlog_line_s) + act_str_size;
    memcpy(p_str, tmp_str, act_str_size);
    p_blk->lst_line_pos = p_line->nxt_line_pos;
    sem_post(p_memlog->sem);

    p_memlog->is_have_str = 1;

    return 0;
}


int memlog_server_dump_get_mem_size(int *p_mem_size)
{
    if (NULL == p_mem_size)
    {
        return 1;
    }
    
    *p_mem_size = MEMLOG_MEM_SIZE;
    return 0;
}


int memlog_server_dump(unsigned char *p_dump_mem, int mem_len)
{
    memlog_s *p_memlog = g_pmemlog;

    if (NULL == p_memlog)
    {
        MEMLOG_PRT("memlog_server_dump. param error\n");
        return 1;
    }
    if (mem_len != MEMLOG_MEM_SIZE)
    {
        MEMLOG_PRT("memlog_server_dump. param error2\n");
        return 1;
    }

    sem_wait(p_memlog->sem);
    unsigned char *p_fst_blk_mem = (unsigned char *)p_memlog + p_memlog->blk_pos[0];
    memcpy(p_dump_mem, p_fst_blk_mem, MEMLOG_MEM_SIZE);
    sem_post(p_memlog->sem);

    return 0;
}


int memlog_server_paser(unsigned char *p_mem, int len, void (*func)(char *str))
{
    if ((NULL == p_mem) || (len <= 0))
    {
        MEMLOG_PRT("memlog_server_paser. param error\n");
        return 1;
    }

    int i;
    int blk_num = len / MEMLOG_MEMBLK_SIZE;
    for (i = 0; i < blk_num; i++)
    {
        memlog_block_s *p_blk = (memlog_block_s *)(p_mem + i * MEMLOG_MEMBLK_SIZE);
        if ((p_blk->version != MEMLOG_VERSION) || (p_blk->magic != MEMLOG_MAGIC))
        {
            MEMLOG_PRT("memlog_server_paser. version or magic is not match\n");
            return 1;
        }

        unsigned int cur_line_pos = p_blk->fst_line_pos;
        while (cur_line_pos != p_blk->lst_line_pos)
        {
            memlog_line_s *p_cur_line = (memlog_line_s *)((unsigned char *)p_blk + cur_line_pos);
            char *p_str = (char *)p_cur_line + sizeof(memlog_line_s);
            //MEMLOG_PRT("%s", p_str);
            (*func)(p_str);

            cur_line_pos = p_cur_line->nxt_line_pos;
        }       
    }

    return 0;
}


int memlog_server_paser_time(unsigned char *p_mem, int len, void (*func)(unsigned int time, char *str))
{
    if ((NULL == p_mem) || (len <= 0))
    {
        MEMLOG_PRT("memlog_server_paser. param error\n");
        return 1;
    }

    int i;
    int blk_num = len / MEMLOG_MEMBLK_SIZE;
    for (i = 0; i < blk_num; i++)
    {
        memlog_block_s *p_blk = (memlog_block_s *)(p_mem + i * MEMLOG_MEMBLK_SIZE);
        if ((p_blk->version != MEMLOG_VERSION) || (p_blk->magic != MEMLOG_MAGIC))
        {
            MEMLOG_PRT("memlog_server_paser. version or magic is not match\n");
            return 1;
        }

        unsigned int cur_line_pos = p_blk->fst_line_pos;
        while (cur_line_pos != p_blk->lst_line_pos)
        {
            memlog_line_s *p_cur_line = (memlog_line_s *)((unsigned char *)p_blk + cur_line_pos);
            char *p_str = (char *)p_cur_line + sizeof(memlog_line_s);
            //MEMLOG_PRT("%s", p_str);
            (*func)(p_cur_line->time_ms, p_str);

            cur_line_pos = p_cur_line->nxt_line_pos;
        }       
    }

    return 0;
}


int memlog_client_init(int *p_err_mem_not_exist, int *p_err_mem_not_ready)
{   
    int shm_fd = -1;
    memlog_s *p_memlog = NULL;
    memlog_client_s *p_client = NULL;

    if ((NULL == p_err_mem_not_exist) || (NULL == p_err_mem_not_ready))
    {
        MEMLOG_PRT("memlog_client_init. param error\n");
        return 1;
    }
    *p_err_mem_not_exist = 0;
    *p_err_mem_not_ready = 0;

    // alloc shared mem
    int memsize = MEMLOG_MEM_SIZE + sizeof(memlog_s);
    shm_fd = shm_open(MEMLOG_NAME, O_RDWR, 0777);
    if (shm_fd < 0)
    {
        *p_err_mem_not_exist = 1;
        //MEMLOG_PRT("memlog_client_init. shm_open faild. errno=%d\n", errno);
        return 1;
    }
    if (ftruncate(shm_fd, memsize) < 0)
    {
        MEMLOG_PRT("memlog_client_init. ftruncate faild\n");
        return 1;
    }
    p_memlog = (memlog_s *)mmap(0, memsize, PROT_READ | PROT_WRITE,  MAP_SHARED, shm_fd, 0);
    if (NULL == p_memlog)
    {
        MEMLOG_PRT("memlog_client_init. mmap faild\n");
        return 1;
    }
    if (p_memlog->server_ready_flag != 1)
    {
        *p_err_mem_not_ready = 1;
        //MEMLOG_PRT("memlog_client_init. server is not ready\n");
        return 1;        
    }
    p_memlog->server_ready_flag = 0;    // set flag = 0, if server exist and killed, then client start
    if ((p_memlog->version != MEMLOG_VERSION) || (p_memlog->magic != MEMLOG_MAGIC))
    {
        MEMLOG_PRT("memlog_client_init. version or magic is not match\n");
        return 1;
    }

    // alloc client mem
    p_client = (memlog_client_s *)malloc(sizeof(memlog_client_s));
    if (NULL == p_client)
    {
        MEMLOG_PRT("memlog_client_init. alloc client struct faild\n");
        return 1;
    }
    memset(p_client, 0, sizeof(memlog_client_s));

    p_client->p_dump_mem = (unsigned char *)malloc(MEMLOG_MEM_SIZE);
    if (NULL == p_client->p_dump_mem)
    {
        MEMLOG_PRT("memlog_client_init. alloc memlog mem faild\n");
        return 1;
    }
    memset(p_client->p_dump_mem, 0, MEMLOG_MEM_SIZE);

    p_client->p_single_blk_mem = (unsigned char *)malloc(MEMLOG_MEMBLK_SIZE);
    if (NULL == p_client->p_single_blk_mem)
    {
        MEMLOG_PRT("memlog_client_init. alloc memlog single mem faild\n");
        return 1;
    }
    memset(p_client->p_single_blk_mem, 0, MEMLOG_MEMBLK_SIZE);

    p_client->sem = sem_open(MEMLOG_SEM_NAME, O_EXCL);
    if (SEM_FAILED == p_memlog->sem)
    {
        MEMLOG_PRT("memlog_client_init. sem_open faild. errno=%d\n", errno);
        return 1;
    }

    g_pmemlog = p_memlog;  
    g_client_pmemlog = p_client;
    MEMLOG_PRT("memlog_client_init success\n");
    return 0; 
}


int memlog_client_set_loglvl(int lvl)
{
    memlog_s *p_memlog = g_pmemlog;
    if (NULL == p_memlog)
    {
        MEMLOG_PRT("memlog_client_set_loglvl faild. param error\n");
        return 1;
    }

    p_memlog->log_lvl = lvl;

    return 0;
}

int memlog_client_set_prt_console(int flag)
{
    memlog_s *p_memlog = g_pmemlog;
    if (NULL == p_memlog)
    {
        MEMLOG_PRT("memlog_client_set_prt_console faild. param error\n");
        return 1;
    }

    if (flag)
    {
        p_memlog->not_prt_console = 0;
    }
    else
    {
        p_memlog->not_prt_console = 1;
    }

    return 0;
}

int memlog_client_ouput()
{
    memlog_s *p_memlog = g_pmemlog;
    memlog_client_s *p_client = g_client_pmemlog;

    if ((NULL == p_memlog) || (NULL == p_client))
    {
        MEMLOG_PRT("memlog_client_ouput. param error\n");
        return 1;
    }
    if (0 == p_memlog->is_have_str)
    {
        return 0;
    }

    while (1)
    {
        // cpy block mem
        sem_wait(p_client->sem);
        memlog_block_s *p_rblk = (memlog_block_s *)((unsigned char *)p_memlog + p_memlog->blk_pos[p_memlog->blk_r_idx]);
        if (p_rblk->fst_line_pos != p_rblk->lst_line_pos)
        {
            memcpy(p_client->p_single_blk_mem, p_rblk, MEMLOG_MEMBLK_SIZE);
        }
        else
        {
            p_memlog->is_have_str = 0;
            sem_post(p_client->sem);
            break;
        }
        p_rblk->lst_line_pos = p_rblk->fst_line_pos;
        if (p_memlog->blk_r_idx != p_memlog->blk_w_idx)
        {
            p_memlog->blk_r_idx++;
            if (p_memlog->blk_r_idx >= MEMLOG_MEMBLK_NUM)
            {
                p_memlog->blk_r_idx = 0;
            }
        }
        sem_post(p_client->sem);

        // prt
        memlog_block_s *p_cpy_blk = (memlog_block_s *)(p_client->p_single_blk_mem);
        unsigned int cur_line_pos = p_cpy_blk->fst_line_pos;
        while (cur_line_pos != p_cpy_blk->lst_line_pos)
        {
            memlog_line_s *p_cur_line = (memlog_line_s *)((unsigned char *)p_cpy_blk + cur_line_pos);
            char *p_str = (char *)p_cur_line + sizeof(memlog_line_s);
            printf("[AVMC]%s", p_str);

            cur_line_pos = p_cur_line->nxt_line_pos;
        }
    }

    return 0;
}

int memlog_client_get_pair(int *p_pair_arr_size, unsigned char **p_pair_arr)
{
    memlog_s *p_memlog = g_pmemlog;
    
    if ((NULL == p_memlog) || (NULL == p_pair_arr_size) || (NULL == p_pair_arr))
    {
        MEMLOG_PRT("memlog_client_get_pair. param error\n");
        return 1;
    }
    *p_pair_arr_size = MEMLOG_PAIR_SIZE;
    *p_pair_arr = (unsigned char *)(p_memlog->pair);

    return 0;
}

int memlog_client_dump(unsigned char **p_dump_mem, int *p_mem_len)
{
    memlog_s *p_memlog = g_pmemlog;
    memlog_client_s *p_client = g_client_pmemlog;

    if ((NULL == p_memlog) || (NULL == p_client))
    {
        MEMLOG_PRT("memlog_client_ouput. param error\n");
        return 1;
    }

    sem_wait(p_client->sem);
    unsigned char *p_fst_blk_mem = (unsigned char *)p_memlog + p_memlog->blk_pos[0];
    memcpy(p_client->p_dump_mem, p_fst_blk_mem, MEMLOG_MEM_SIZE);
    
    p_memlog->blk_r_idx = p_memlog->blk_w_idx;
    memlog_block_s *p_rblk = (memlog_block_s *)p_memlog + p_memlog->blk_pos[p_memlog->blk_r_idx];
    p_rblk->fst_line_pos = p_rblk->lst_line_pos;
    sem_post(p_client->sem);

    *p_dump_mem = p_client->p_dump_mem;
    *p_mem_len = MEMLOG_MEM_SIZE;
    return 0;
}


int memlog_client_paser(unsigned char *p_mem, int len)
{
    if ((NULL == p_mem) || (len <= 0))
    {
        MEMLOG_PRT("memlog_client_paser. param error\n");
        return 1;
    }

    int i;
    int blk_num = len / MEMLOG_MEMBLK_SIZE;
    for (i = 0; i < blk_num; i++)
    {
        memlog_block_s *p_blk = (memlog_block_s *)(p_mem + i * MEMLOG_MEMBLK_SIZE);
        if ((p_blk->version != MEMLOG_VERSION) || (p_blk->magic != MEMLOG_MAGIC))
        {
            MEMLOG_PRT("memlog_client_paser. version or magic is not match\n");
            return 1;
        }

        unsigned int cur_line_pos = p_blk->fst_line_pos;
        while (cur_line_pos != p_blk->lst_line_pos)
        {
            memlog_line_s *p_cur_line = (memlog_line_s *)((unsigned char *)p_blk + cur_line_pos);
            char *p_str = (char *)p_cur_line + sizeof(memlog_line_s);
            MEMLOG_PRT("%s", p_str);

            cur_line_pos = p_cur_line->nxt_line_pos;
        }       
    }

    return 0;
}
