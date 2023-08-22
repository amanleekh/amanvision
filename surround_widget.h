#ifndef SURROUND_WIDGET_H
#define SURROUND_WIDGET_H

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QThread>
#include <QLabel>
#include <QMouseEvent>
#include <QOffscreenSurface>

#include "common_def.h"
#include "avm_gpu.h"
#include "model_draw.h"
#include "overlay_draw.h"


#define SURRWGT_LOG(...)    AVM_LOG(__VA_ARGS__)
#define SURRWGT_ERR(...)    AVM_ERR(__VA_ARGS__)


/**
 * @brief The SurroundWidget class
 * The QOpenGLWidget class is a widget for rendering OpenGL graphics.
 * The QOpenGLFunctions class provides cross-platform access to the OpenGL ES 2.0 API
 */
class SurroundWidget : public QOpenGLWidget, protected QOpenGLFunctions
{
    /**< The Q_OBJECT macro must appear in the private section of a class definition that declares its own signals and slots or that uses other services provided by Qt's meta-object system. */
    Q_OBJECT

public:
    SurroundWidget(QWidget *parent, int wideangle_width, int widgetWidth, int widgetHeight,
        int tex_width, int tex_height);
    ~SurroundWidget();

public:
	void setLabel(QLabel *pLabel);
    /**< get current view enum */
    int getView();
	int get_view_by_cur_deg(void);
    int getStatus(void);
    void setPaintStatus();

public slots:
    /**< get signale from thread worker when a video sync frame is getted*/
    void stitchUpdate(void);
    /**< update slot can be called by timer interrupt */
    void stitchUpdateTime();
    /**< let widget know it's width and height have changed */
    void updateWidgetWH(int widgetWidth, int widgetHeight);
  
public:
    /**< viewEnum should be AVM_VIEW_E */
    void setView(int viewEnum);
    /**< dyn change view (rotate). only for change between 3d views. if current view and dst view is not 3d view, will do nothing */
    void setViewDyn(int viewEnum);
    /**< rotate current view. only for auto and 3d views */
    void rotateView(int det_x);
	void rotateView_omni(int det_x, int det_y);
    /**< set radar alert state. 4bit for a radar. 0x01 for first radar block level is 1. 0x13 for first radar level is 3, second radar level is 1. etc */
    void setRadar(unsigned mask, unsigned int mask2);
    /**< clr radar block by bit mask */
    void clrRadar(unsigned mask, unsigned int mask2);
    /**< show or hide radar */
    void showRadar(int isShow);
    /**< set assist line by angle deg. eg. 10 for right deg 10. -10 means left deg 10 */
    void setAssistLine(float angle);
    /**< show or hide assit line */
    void showAssistLine(int isShow);
    void showBlockLine(int isShow);
    /**< set speed km/h */
    void setSpeed(int speed);
    /**< -1 left, 0 off 1 right*/
    void setTurnSignal(int turn);
    /**< set car in emergency flashers. 0 off, 1 on */
    void setEmFlahers(int is_on);    
    /**< set model alpha.[1, 100] */
    void setModelAlpha(int alpha);
    /**< print current paintGL fsp */
    void prtFps();
    void setCarDirection(int is_front);
    /**< hide stitch camera, can be used when car's door is open */
    /**< 0 not hide, 1 hide, -1 unchanged */
    void hideCamera(int is_hide_right, int is_hide_front,
        int is_hide_left, int is_hide_rear);
    void setDoor(int fl_open, int fr_open, int bl_open, int br_open);

    int checkInPure3dView(int view_enum)
    {
        return m_avmGpu->avm_gpu_check_in_pure_3d_view(view_enum);
    }
    int checkIn3dView(int view_enum)
    {
        return m_avmGpu->avm_gpu_check_in_3d_view(view_enum);
    }

protected:
    /**< Q_DECL_OVERRIDE */
    /**< This macro can be used to declare an overriding virtual function. Use of this markup will allow the compiler to generate an error if the overriding virtual function does not in fact override anything. */
    /**< his virtual function is called once before the first call to paintGL() or resizeGL() */
    void initializeGL() Q_DECL_OVERRIDE;

    /**< This virtual function is called whenever the widget needs to be painted. Reimplement it in a subclass. */
    void paintGL() Q_DECL_OVERRIDE;

    /**< This virtual function is called whenever the widget has been resized. Reimplement it in a subclass. The new size is passed in w and h. */
    void resizeGL(int width, int height) Q_DECL_OVERRIDE;
    
private:
    void calc_fps();
    QString getAutoStr();

private:
    int m_textureWidth;     /**< texture width and height, fix to 1280*720 */
    int m_textureHeight;
    int m_widgetWidth;      /**< widget width and height, also is gl's viewport */
    int m_widgetHeight;
    int m_wideangleWidth;
    AVM_VIEW_E m_current_view;  /**< current avm view. defualt to auto. this auto exacly is bird view */
    int m_is_auto_change;   /**< if current view state can auto change when car state change */
    int m_prt_fps_flag;
    float m_fps;

private:
    AvmGpu *m_avmGpu;   /**< avm gpu alg */
    ModelDraw *m_carModel;
    OverlayDraw *m_overlay;
	QLabel *m_p_mViewLabel;
    int m_status;

    /*< static func to get fps */
public:
    static int getFps();
private:
    static int m_static_fps;
};


#endif // SURROUND_WIDGET_H

