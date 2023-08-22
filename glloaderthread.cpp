
#include "glloaderthread.h"


void GlLoaderThread::run()
{
    // load data
    m_car_model->pre_load_car_data(1);

    // gen vbo
    m_ctx->makeCurrent(m_surface);
    m_car_model->gen_model_vbo();
    m_ctx->doneCurrent();
}


