

#include <stdio.h>
#include <stdlib.h>

#include "common_def.h"
#include "GLES2/gl2.h"
#include "GLES2/gl2ext.h"
#include "gpu_shader.h"

#if GLOBAL_RUN_ENV_DESKTOP == 0
#include "EGL/egl.h"
#endif

typedef struct shader_struct_
{
    GLuint vertShaderNum;   /**< */ 
    GLuint pixelShaderNum;
    GLuint programHandle;   /**< program handler */
}shader_s;


static int shader_compile(int is_file, const char *p_name, GLuint shader_num)
{
    if (is_file)
    {
    	FILE * fptr = NULL;
        
    	fptr = fopen(p_name, "rb");
    	if (NULL == fptr)
    	{
            SHADER_ERR("shader_compile. fopen faild\n");
    		return 1;
    	}

    	int length;
    	fseek(fptr, 0, SEEK_END);
    	length = ftell(fptr);
    	fseek(fptr, 0 ,SEEK_SET);

    	char * shader_source = (char *)malloc(sizeof (char) * length);
    	if (NULL == shader_source)
    	{
            SHADER_ERR("shader_compile. malloc faild\n");
    		return 1;
    	}
    	fread(shader_source, length, 1, fptr);

    	glShaderSource(shader_num, 1, (const char **)&shader_source, &length);
    	glCompileShader(shader_num);

    	free(shader_source);
    	fclose(fptr);
	}
	else
	{
    	glShaderSource(shader_num, 1, (const char **)&p_name, NULL);
    	glCompileShader(shader_num);
	}

	GLint compiled = 0;
	glGetShaderiv(shader_num, GL_COMPILE_STATUS, &compiled);
	if (!compiled)
	{
	    SHADER_ERR("shader_compile. compile faild\n");
	    
		// Retrieve error buffer size.
		GLint error_buf_size;
		glGetShaderiv(shader_num, GL_INFO_LOG_LENGTH, &error_buf_size);
        if ((error_buf_size <= 0) || (error_buf_size >= 512))
        {
            SHADER_ERR("shader_compile. error_buf_size=%d\n", error_buf_size);
            return 1;
        }
        char str[512] = {'\0'};
        glGetShaderInfoLog(shader_num, 511, &error_buf_size, str);
        str[error_buf_size] = '\0';
        SHADER_ERR("shader_compile. get shader faild. %s\n", str);

		return 1;
	}

	return 0;    
}


void * shader_load_comm(int is_file, const char *p_vname, const char *p_pname)
{
    if ((NULL == p_vname) || (NULL == p_pname))
    {
        return NULL;
    }

    shader_s *p_handler = (shader_s *)malloc(sizeof(shader_s));
    if (NULL == p_handler)
    {
        return NULL;
    }
    p_handler->vertShaderNum = glCreateShader(GL_VERTEX_SHADER);
    p_handler->pixelShaderNum = glCreateShader(GL_FRAGMENT_SHADER);

	if (shader_compile(is_file, p_vname, p_handler->vertShaderNum) != 0)
	{
        SHADER_ERR("shader_load. ps compile faild, p_vname=%s\n", p_vname);
        free(p_handler);
		return NULL;
	}
	if (shader_compile(is_file, p_pname, p_handler->pixelShaderNum) != 0)
	{
        SHADER_ERR("shader_load. vs compile faild, p_pname=%s\n", p_pname);
        free(p_handler);
		return NULL;
	}

	p_handler->programHandle = glCreateProgram();

	glAttachShader(p_handler->programHandle, p_handler->vertShaderNum);
	glAttachShader(p_handler->programHandle, p_handler->pixelShaderNum);

	glLinkProgram(p_handler->programHandle);
	// Check if linking succeeded.
	GLint linked = 0;
	glGetProgramiv(p_handler->programHandle, GL_LINK_STATUS, &linked);
	if (!linked)
	{
		SHADER_ERR("shader_load. link failed.\n");

		GLint error_buf_size;
        glGetProgramiv(p_handler->programHandle, GL_INFO_LOG_LENGTH, &error_buf_size);
        if ((error_buf_size <= 0) || (error_buf_size >= 256))
        {
            free(p_handler);
            return NULL;
        }
        char str[256] = {'\0'};
        glGetProgramInfoLog(p_handler->programHandle, 255, &error_buf_size, str);
        str[error_buf_size + 1] = '\0';
        SHADER_ERR("shader_load. %s\n", str);
        
        free(p_handler);
		return NULL;
	}
    
	glUseProgram(p_handler->programHandle);
    
    return ((void *)p_handler);
}


int shader_load(const char *p_vname, const char *p_pname, void **handler)
{
    if (NULL == handler)
    {
        SHADER_ERR("shader_load faild. param error\n");
        return 1;
    }
    
    *handler = shader_load_comm(1, p_vname, p_pname);
    if (NULL == *handler)
    {
        SHADER_ERR("shader_load faild. shader_load_comm faild\n");
        return 1;
    }
    
    return 0;
}


int shader_load_str(const char *p_vstr, const char *p_pstr, void **handler)
{
    if (NULL == handler)
    {
        SHADER_ERR("shader_load_str faild. param error\n");
        return 1;
    }
    
    *handler = shader_load_comm(0, p_vstr, p_pstr);
    if (NULL == *handler)
    {
        SHADER_ERR("shader_load_str faild. shader_load_comm faild\n");
        return 1;
    }
    
    return 0;    
}


void shader_destroy(void *handler)
{
    if (NULL == handler)
    {
        return;
    }

    shader_s *p_handler = (shader_s *)handler;

    if (p_handler->programHandle)
    {
		glDeleteShader(p_handler->vertShaderNum);
		glDeleteShader(p_handler->pixelShaderNum);
		glDeleteProgram(p_handler->programHandle);
		glUseProgram(0);
		p_handler->programHandle = 0;
    }
}



GLuint shader_get_program(void *handler)
{
    if (NULL == handler)
    {
        return 0;
    }

    shader_s *p_handler = (shader_s *)handler;
    
    return p_handler->programHandle;
}

