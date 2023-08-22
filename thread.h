

#ifndef _THREAD_H_
#define _THREAD_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "common_def.h"

#define THREAD_OK       (0)
#define THREAD_ERR      (1)

#define THREAD_PRT(...)    AVM_LOG(__VA_ARGS__)

/**
* sched RR
*/
int thread_create(int priority, void *(func_t)(void *arg), void *arg);


/**
* sched RR with name
*/
int thread_create_name(int priority, const char *name, void *(func_t)(void *arg), void *arg);


#ifdef __cplusplus
}
#endif


#endif


