
#ifndef _MEMLOG_H_
#define _MEMLOG_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>

#define MEMLOG_PRT(...) printf("[AVMS]");printf(__VA_ARGS__)
#define MEMLOG_PRT_ARG(...) printf("[AVMS]");vprintf(__VA_ARGS__)

int memlog_server_init();

int memlog_server_set_log_lvl(int lvl);

int memlog_server_set_prt_console(int flag);

int memlog_server_set_prt_shm(int flag);

int memlog_server_set_pair(int idx, unsigned char val);

int memlog_server_printf(int lvl, const char * format, ...);

int memlog_server_dump_get_mem_size(int *p_mem_size);

int memlog_server_dump(unsigned char *p_dump_mem, int mem_len);

int memlog_server_paser(unsigned char *p_mem, int len, void (*func)(char *str));

int memlog_server_paser_time(unsigned char *p_mem, int len, void (*func)(unsigned int time, char *str));

int memlog_client_init(int *p_err_mem_not_exist, int *p_err_mem_not_ready);

int memlog_client_set_loglvl(int lvl);

int memlog_client_set_prt_console(int flag);

int memlog_client_ouput();

int memlog_client_get_pair(int *p_pair_arr_size, unsigned char **p_pair_arr);

int memlog_client_dump(unsigned char **p_dump_mem, int *p_mem_len);

int memlog_client_paser(unsigned char *p_mem, int len);

unsigned int memlog_gettime_ms();
    

#ifdef __cplusplus
}
#endif

#endif
