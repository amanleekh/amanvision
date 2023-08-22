

#include <stdio.h>
#include <errno.h>
#include <string.h>

#define __USE_GNU             /* See feature_test_macros(7) */
#include <pthread.h>
#include "thread.h"


static int thread_create_inner(int priority, const char *name, void *(func_t)(void *arg), void *arg)
{
    int re;
    
    if (NULL == func_t)
    {
        THREAD_PRT("thread_create_inner faild. param error\n");
        return THREAD_ERR;
    }
    
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
    pthread_attr_setschedpolicy(&attr, SCHED_RR);

    int policy = 0;
    int max_priority = 0; 
    int min_priority = 0;
    pthread_attr_getschedpolicy(&attr,&policy);
    max_priority = sched_get_priority_max(policy);
    min_priority = sched_get_priority_min(policy);
    if ((priority < min_priority) || (priority > max_priority))
    {
        THREAD_PRT("thread_create_inner faild. priority=%d, min=%d, max=%d\n",
            priority, min_priority, min_priority);
        return THREAD_ERR;  
    }
    struct sched_param param;
    param.sched_priority = priority;
    pthread_attr_setschedparam(&attr,&param);  

    pthread_t pid;
    re = pthread_create(&pid, &attr, func_t, arg);
    // can't set param
    if (EPERM == re)
    {
        pthread_attr_destroy(&attr);
        THREAD_PRT("warning:thread_create_inner. pthread_create faild. will use default param to create\n");

        int re_tmp = 0;
        pthread_attr_t attr_tmp;
        pthread_attr_init(&attr_tmp);
        re_tmp = pthread_create(&pid, &attr_tmp, func_t, arg);
        pthread_attr_destroy(&attr_tmp);
        if (re_tmp != 0)
        {
            THREAD_PRT("thread_create_inner. pthread_create with default param faild.\n");
            return THREAD_ERR;
        }
    }
    else if (re != 0)
    {
        pthread_attr_destroy(&attr);
        THREAD_PRT("thread_create_inner. pthread_create faild. re=%dd\n", re);
        return THREAD_ERR;
    }

    if (name != NULL)
    {
        char new_name[16] = {'\0'};
        strncpy (new_name, name, 15);
        new_name[15] = '\0';
        pthread_setname_np(pid, new_name);
    }
    
    pthread_attr_destroy(&attr);
    return THREAD_OK;
}


int thread_create(int priority, void *(func_t)(void *arg), void *arg)
{
    return thread_create_inner(priority, NULL, func_t, arg);
}


int thread_create_name(int priority, const char *name, void *(func_t)(void *arg), void *arg)
{
    return thread_create_inner(priority, name, func_t, arg);
}


