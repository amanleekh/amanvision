

#include <stdio.h>
#include <stdlib.h>

#if 0

#include <EGL/egl.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

#include "FSL/fsl_egl.h"
#include "gpu_egl.h"


// handler returned by fsl_getNativeDisplay()
static EGLNativeDisplayType eglNativeDisplayType;

// handler returned by eglGetDisplay()
static EGLDisplay egldisplay;

// drawing window, from fsl_createwindow()
static EGLNativeWindowType eglNativeWindow;

// drawing surface
static EGLSurface eglsurface; 

// drawing eglcontext
static EGLContext eglcontext;


int egl_init(void)
{
    static const EGLint s_configAttribs[] =
    {
		EGL_SAMPLES,      0,
		EGL_RED_SIZE,     8,
		EGL_GREEN_SIZE,   8,
		EGL_BLUE_SIZE,    8,
		EGL_ALPHA_SIZE,   EGL_DONT_CARE,
		EGL_DEPTH_SIZE,   0,
		EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
		EGL_NONE,
    };

    eglNativeDisplayType = fsl_getNativeDisplay();
    egldisplay = eglGetDisplay(eglNativeDisplayType);
    eglInitialize(egldisplay, NULL, NULL);
    if (eglGetError() != EGL_SUCCESS)
    {
        printf("egl_init. eglInitialize faild\n");
        return 1;
    }
    eglBindAPI(EGL_OPENGL_ES_API);

    EGLint numconfigs;
    EGLConfig eglconfig;

    eglChooseConfig(egldisplay, s_configAttribs, &eglconfig, 1, &numconfigs);
    if ((eglGetError() != EGL_SUCCESS) || (numconfigs != 1))
    {
        printf("egl_init. eglChooseConfig faild\n");
        return 1;
    }
   
    eglNativeWindow = fsl_createwindow(egldisplay, eglNativeDisplayType);
    if (!eglNativeWindow)
    {
        printf("egl_init. eglNativeWindow is null\n");
        return 1;
    }
    eglsurface = eglCreateWindowSurface(egldisplay, eglconfig, eglNativeWindow, NULL);
    if (eglGetError() != EGL_SUCCESS)
    {
        printf("egl_init. eglCreateWindowSurface faild\n");
        return 1;
    }

    EGLint ContextAttribList[] = {EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE};

    eglcontext = eglCreateContext( egldisplay, eglconfig, EGL_NO_CONTEXT, ContextAttribList);
    if (eglGetError() != EGL_SUCCESS)
    {
        printf("egl_init. eglCreateContext faild\n");
        return 1;
    }
    eglMakeCurrent(egldisplay, eglsurface, eglsurface, eglcontext);
    if (eglGetError() != EGL_SUCCESS)
    {
        printf("egl_init. eglMakeCurrent faild\n");
        return 1;
    }

    return 0;
}


int egl_deinit(void)
{
	eglMakeCurrent(egldisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
    if (eglGetError() != EGL_SUCCESS)
    {
        printf("egl_deinit. eglMakeCurrent faild\n");
        return 1;
    }
	eglDestroyContext(egldisplay, eglcontext);
	eglDestroySurface(egldisplay, eglsurface);	
	fsl_destroywindow(eglNativeWindow, eglNativeDisplayType);
	eglTerminate(egldisplay);
	if (eglGetError() != EGL_SUCCESS)
	{
	    printf("egl_deinit. destroy faild\n");
        return 1;
	}
	eglReleaseThread();

    return 0;
}



int egl_get_surface_wh(int *p_width, int *p_height)
{
    if ((NULL == p_width) || (NULL == p_height))
    {
        printf("egl_get_surface_wh faild. param error\n");
        return 1;
    }

    eglQuerySurface(egldisplay, eglsurface, EGL_WIDTH, p_width);
    eglQuerySurface(egldisplay, eglsurface, EGL_HEIGHT, p_height);

    return 0;
}


void egl_swap(void)
{
    eglSwapBuffers(egldisplay, eglsurface);
}


#endif
