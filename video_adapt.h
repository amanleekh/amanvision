#ifndef VIDEO_ADAPT_H
#define VIDEO_ADAPT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "common_def.h"

#define VIDEOAPT_LOG(...)   AVM_LOG(__VA_ARGS__)
#define VIDEOAPT_ERR(...)   AVM_LOG(__VA_ARGS__)

// in desktop app, we do not need camera capture, use video file instead
#if GLOBAL_RUN_ENV_DESKTOP == 0

int video_adapt(int* input_width, int* input_height);

#endif

#ifdef __cplusplus
}
#endif

#endif // VIDEO_ADAPT_H
