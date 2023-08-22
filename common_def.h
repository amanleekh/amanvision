#ifndef COMMON_DEF_H
#define COMMON_DEF_H

#ifdef __cplusplus
extern "C" {
#endif

// is app run env is desktop or embeded linux
#ifdef APP_RUN_ON_PC
#define GLOBAL_RUN_ENV_DESKTOP  (1)
#else
#define GLOBAL_RUN_ENV_DESKTOP  (0)
#endif

#define DEFAULT_FPS                 (25)
#define DEFAULT_FPS_EVEN            (24)
// if support front camera or rear camera single camera fish eye calib view
// if not support, related files will not be load and used
#define SUPPORT_FRONT_REAR_CALIB    (0)

#include "memlog.h"

#define AVM_VEB(...) memlog_server_printf(0, __VA_ARGS__)
#define AVM_LOG(...) memlog_server_printf(1, __VA_ARGS__)
#define AVM_ERR(...) memlog_server_printf(2, __VA_ARGS__)

#ifdef __cplusplus
}
#endif

#endif // COMMON_DEF_H
