#ifndef GLLOADERTHREAD_H
#define GLLOADERTHREAD_H

#include <QThread>
#include <QMutex>
#include <QOpenGLContext>
#include <QOffscreenSurface>
#include "model_draw.h"


class GlLoaderThread : public QThread
{
    Q_OBJECT

public:
    QOpenGLContext *m_ctx;  /**< share context */
    QOffscreenSurface *m_surface;
    ModelDraw *m_car_model;

public:
    void run();
};

#endif // GLLOADERTHREAD_H
