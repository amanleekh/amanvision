

#ifndef _GPU_SHADER_H_
#define _GPU_SHADER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "common_def.h"
#include "GLES2/gl2.h"
#include "GLES2/gl2ext.h"

#define SHADER_LOG(...) AVM_LOG(__VA_ARGS__)
#define SHADER_ERR(...) AVM_ERR(__VA_ARGS__)


/**
* load vert shader and frag shader, return handler
*/
int shader_load(const char *p_vname, const char *p_pname, void **handler);


/**
* load vert shader and frag shader, return handler
*/
int shader_load_str(const char *p_vstr, const char *p_pstr, void **handler);


/**
* destroy shader
*/
void shader_destroy(void *handler);


/**
* get shader program hander
*/
GLuint shader_get_program(void *handler);


#ifdef __cplusplus
}
#endif

#endif

