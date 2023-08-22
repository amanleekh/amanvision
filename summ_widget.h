#ifndef SUMM_WIDGET_H
#define SUMM_WIDGET_H

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QLabel>

#include "common_def.h"
#include "avm_gpu.h"
#include "model_draw.h"
#include "overlay_draw.h"

#define SUMMWGT_LOG(...)    AVM_LOG(__VA_ARGS__)
#define SUMMWGT_ERR(...)    AVM_ERR(__VA_ARGS__)


class SummWidget : public QOpenGLWidget, protected QOpenGLFunctions
{
    Q_OBJECT

public:
    SummWidget(QWidget *parent, int wideangle_width, int widgetWidth, int widgetHeight,
        int tex_width, int tex_height);
    ~SummWidget();

public slots:
    /**
     * @brief SummWidget::stitchUpdate, used in device env
     *        process when get a synced frame signal
     *        releated code below:
     *        connect(syncVideoGetter, SIGNAL(finishGetOneFrame()), m_summWidget, SLOT(stitchUpdate()));
     */
    void stitchUpdate(void);

    /**
     * @brief SummWidget::stitchUpdateTime, used in pc env
     *        releated code below:
     *        connect(m_checkGlTimer, SIGNAL(timeout()), m_summWidget, SLOT(stitchUpdateTime()));
     *        time inv is 25fps
     */
    void stitchUpdateTime();

public:
    void showRadar(int isShow);
    void showAssistLine(int isShow);
    /**< set assist line by angle deg. eg. 10 for right deg 10. -10 means left deg 10 */
    void setAssistLine(float angle);
    void setRadar(unsigned mask, unsigned int mask2);
    /**< clr radar block by bit mask */
    void clrRadar(unsigned mask, unsigned int mask2);
    /**< set speed km/h */
    void setSpeed(int speed);
    /**< -1 left, 0 off 1 right*/
    void setTurnSignal(int turn);
    /**< set car in emergency flashers. 0 off, 1 on */
    void setEmFlahers(int is_on);
    void setCarDirection(int is_front);
    /**< hide stitch camera, can be used when car's door is open */
    /**< 0 not hide, 1 hide, -1 unchanged */
    void hideCamera(int is_hide_right, int is_hide_front,
        int is_hide_left, int is_hide_rear);
    void setMirrowFold(int is_left_fold, int is_right_fold);
    void initMirrowFoldLabel();
    void processMirrowFoldLabel();
    void setDoor(int fl_open, int fr_open, int bl_open, int br_open);

public:
    void mouseMoveEvent(QMouseEvent *event);
    int getStatus(void);
    void setPaintStatus();

protected:
    void initializeGL() Q_DECL_OVERRIDE;
    void paintGL() Q_DECL_OVERRIDE;
    void resizeGL(int width, int height) Q_DECL_OVERRIDE;

private:
    int m_textureWidth;     /**< texture width and height, fix to 1280*720 */
    int m_textureHeight;
    int m_widgetWidth;
    int m_widgetHeight;
    int m_wideangleWidth;
    int m_left_mirrow_fold;
    int m_right_mirrow_fold;
    AVM_VIEW_E m_current_view;  /**< current avm view. defualt to auto */
    
private:
    AvmGpu *m_avmGpu;   /**< avm gpu alg */
    ModelDraw *m_carModel;
    OverlayDraw *m_overlay;  
    QPixmap *m_pixmap_mirrow_left;
    QPixmap *m_pixmap_mirrow_right;
    QLabel  *m_label_mirrow_left;
    QLabel  *m_label_mirrow_right;
    int m_status;
};


#endif // SUMM_WIDGET_H
