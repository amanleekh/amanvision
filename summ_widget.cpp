
#include <stdio.h>

#include "common_def.h"
#include "summ_widget.h"
#include "video_sync.h"
#include "ini_config.h"
#include "glloaderthread.h"
#include "sync_video_getter.h"
#include "comu_cmd_define.h"
#include "comu_cmd.h"


/**
 * @brief SummWidget::SummWidget
 * @param parent widget param
 * @param widgetWidth
 * @param widgetHeight
 */
SummWidget::SummWidget(QWidget *parent, int wideangle_width, int widgetWidth, int widgetHeight,
    int tex_width, int tex_height) : QOpenGLWidget(parent)
{
    m_avmGpu = NULL;
    m_textureWidth = tex_width;
    m_textureHeight = tex_height;
    m_widgetWidth = widgetWidth;
    m_widgetHeight = widgetHeight;
    m_wideangleWidth = wideangle_width;
    m_current_view = AVM_VIEW_AUTO;
    m_left_mirrow_fold = 0;
    m_right_mirrow_fold = 0;

    initMirrowFoldLabel();

    // create alg class
    int car_type = ini_get_car_type();

#if GLOBAL_RUN_ENV_DESKTOP == 0
        m_avmGpu = new AvmGpu(car_type, 
            m_widgetWidth, m_widgetHeight,
            m_wideangleWidth, m_widgetHeight,
            m_textureWidth, m_textureHeight,
            &(SyncVideoGetter::getChnImgAddr_ex),
            &(SyncVideoGetter::releaseImage),
            1, 1);
#else
        m_avmGpu = new AvmGpu(car_type, 
            m_widgetWidth, m_widgetHeight,
            m_wideangleWidth, m_widgetHeight,
            m_textureWidth, m_textureHeight,
            NULL, NULL,
            1, 1);   
#endif

    // create car model and pre load data
    m_carModel = new ModelDraw(car_type);

    m_overlay = new OverlayDraw();
    m_overlay->pre_load_bottom_data();
    m_overlay->pre_load_radar_data();
    m_overlay->pre_gen_assist_line_data();

    m_status = 0;
}

/**
 * @brief SummWidget::~SummWidget
 */
SummWidget::~SummWidget()
{
}


void SummWidget::showRadar(int isShow)
{
    m_overlay->radar_show(isShow);
}


void SummWidget::setRadar(unsigned mask, unsigned mask2)
{
    m_overlay->radar_set_alert(mask, mask2, NULL);
}


void SummWidget::clrRadar(unsigned mask, unsigned mask2)
{
    m_overlay->radar_clr_alert(mask, mask2, NULL);
}


void SummWidget::showAssistLine(int isShow)
{
    m_overlay->assist_line_show(isShow);
}

void SummWidget::setAssistLine(float angle)
{
    int angle_int = (int)angle;
    if (angle_int < ASSIST_LINE_MIN_DEG)
    {
        angle_int = ASSIST_LINE_MIN_DEG;
    }
    else if (angle_int > ASSIST_LINE_MAX_DEG)
    {
        angle_int = ASSIST_LINE_MAX_DEG;
    }
    m_overlay->set_assist_line_angle(angle_int);
}


void SummWidget::setSpeed(int speed)
{
    m_carModel->set_wheel_speed(speed);
}


void SummWidget::setTurnSignal(int turn)
{
    if (-1 == turn)
    {
        m_carModel->set_light_blink(1, 0, 1, 0);
    }
    else if (0 == turn)
    {
        m_carModel->set_light_blink(0, 0, 0, 0);
    }
    else if (1 == turn)
    {
        m_carModel->set_light_blink(0, 1, 0, 1);
    }
    else
    {
    }
}

void SummWidget::setEmFlahers(int is_on)
{
    m_carModel->set_em_flashers(is_on);
}


void SummWidget::setCarDirection(int is_front)
{
    m_overlay->assist_line_set_direction(is_front);
}


void SummWidget::hideCamera(int is_hide_right, int is_hide_front,
    int is_hide_left, int is_hide_rear)
{
    m_avmGpu->avm_gpu_hide_stitch(is_hide_right, is_hide_front, 
        is_hide_left, is_hide_rear);
}

void SummWidget::setMirrowFold(int is_left_fold, int is_right_fold)
{
    m_left_mirrow_fold = is_left_fold;
    m_right_mirrow_fold = is_right_fold;
}

void SummWidget::initMirrowFoldLabel()
{

#define LAYOUT_SUMM_MIRPIC_WIDTH       (80)
#define LAYOUT_SUMM_MIRPIC_HEIGHT      (80)       
#define LAYOUT_SUMM_WIDTH              (399)
#define LAYOUT_SUMM_HEIGHT             (656)
#define LAYOUT_SUMM_L_MIRPIC_DISTANCE_X  (40)
#define LAYOUT_SUMM_L_MIRPIC_DISTANCE_Y  (278)
#define LAYOUT_SUMM_R_MIRPIC_DISTANCE_X  (LAYOUT_SUMM_WIDTH-LAYOUT_SUMM_L_MIRPIC_DISTANCE_X-LAYOUT_SUMM_MIRPIC_WIDTH)
#define LAYOUT_SUMM_R_MIRPIC_DISTANCE_Y  (LAYOUT_SUMM_L_MIRPIC_DISTANCE_Y)

    m_pixmap_mirrow_left = new QPixmap("avm_qt_app_res/ui/new/mirrow-fold-left.png");
    m_label_mirrow_left  = new QLabel(this);
    m_label_mirrow_left->setGeometry(LAYOUT_SUMM_L_MIRPIC_DISTANCE_X, LAYOUT_SUMM_L_MIRPIC_DISTANCE_Y, 
        LAYOUT_SUMM_MIRPIC_WIDTH, LAYOUT_SUMM_MIRPIC_HEIGHT);
    m_label_mirrow_left->setPixmap(*m_pixmap_mirrow_left);
    m_label_mirrow_left->hide();

    m_pixmap_mirrow_right = new QPixmap("avm_qt_app_res/ui/new/mirrow-fold-right.png");
    m_label_mirrow_right = new QLabel(this);
    m_label_mirrow_right->setGeometry(LAYOUT_SUMM_R_MIRPIC_DISTANCE_X, LAYOUT_SUMM_R_MIRPIC_DISTANCE_Y,
        LAYOUT_SUMM_MIRPIC_WIDTH,LAYOUT_SUMM_MIRPIC_HEIGHT);
    m_label_mirrow_right->setPixmap(*m_pixmap_mirrow_right);
    m_label_mirrow_right->hide();

}


void SummWidget::processMirrowFoldLabel()
{

    if(m_left_mirrow_fold)
    {
        m_label_mirrow_left->show();
    }
    else
    {
        m_label_mirrow_left->hide();
    }

    if(m_right_mirrow_fold)
    {
        m_label_mirrow_right->show();
    }
    else
    {
        m_label_mirrow_right->hide();
    }

}



void SummWidget::setDoor(int fl_open, int fr_open, int bl_open, int br_open)
{
    m_carModel->set_door(fl_open, fr_open, bl_open, br_open);
}


/**
 * @brief SummWidget::initializeGL
 */
void SummWidget::initializeGL()
{
    int ret;

    // QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();
    // f->glClear(GL_COLOR_BUFFER_BIT);
    // the prefixing of each and every OpenGL call can be avoided by deriving from QOpenGLFunctions instead:
    initializeOpenGLFunctions();

    ret = m_avmGpu->avmgpu_init();
    if (ret != 0)
    {
        SUMMWGT_ERR("SummWidget::initializeGL. avmgpu_init faild\n");
        return;
    }

    // set stitch default view
    ret = m_avmGpu->avm_gpu_set_view(m_current_view, 1);
    if (ret != 0)
    {
        SUMMWGT_ERR("SummWidget::initializeGL. avm_gpu_postion_init faild\n");
        return;
    }     
    // get stitch view and proj matrix
    glm::mat4 stitch_v;
    glm::mat4 stitch_p;
    m_avmGpu->avm_gpu_get_v_p(&stitch_v, &stitch_p);

    // create model program and gen vbo
    m_carModel->create_program();
    // set window alpha
    m_carModel->set_window_alpha(1.0);
    //m_carModel->gen_model_vbo();
    m_carModel->set_mvp(NULL, &stitch_v, &stitch_p);

    // create overlay program and gen vbo
    m_overlay->create_program();
    m_overlay->gen_bottom_vbo();
    m_overlay->gen_radar_vbo();    
    m_overlay->set_radar_mvp(NULL, &stitch_v, &stitch_p);
    m_overlay->gen_assist_line_vbo();
    m_overlay->set_assist_line_mvp(NULL, &stitch_v, &stitch_p);    

    // create share context, and start worker thread
    GlLoaderThread *workerThread = new GlLoaderThread();
    QOpenGLContext* sharectx = QOpenGLContext::currentContext();;
    workerThread->m_ctx = new QOpenGLContext();
    workerThread->m_ctx->setFormat(sharectx->format());
    workerThread->m_ctx->setShareContext(sharectx);
    workerThread->m_ctx->create();
    workerThread->m_ctx->moveToThread(workerThread);
    workerThread->m_surface = new QOffscreenSurface();
    workerThread->m_surface->setFormat(sharectx->format());
    workerThread->m_surface->create();
    workerThread->m_car_model = m_carModel;
    workerThread->start();

    SUMMWGT_LOG("SummWidget::initializeGL. init success\n");
}


/**
 * @brief SummWidget::paintGL
 */
void SummWidget::paintGL()
{
    // Clear color and depth buffers
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    m_avmGpu->avm_gpu_process();
    processMirrowFoldLabel();

    m_overlay->overlay_render_start();
    m_overlay->bottom_render_process();
    m_overlay->assist_line_process(0);
    m_overlay->radar_render_process();
    m_overlay->overlay_render_end();

    m_carModel->render_process(1);

    GLenum gl_err = glGetError();
    if (gl_err != GL_NO_ERROR)
    {
        SUMMWGT_LOG("SummWidget::paintGL. gl_error=%d\n", gl_err);
    }

    // set shm flag
    setPaintStatus();
}

/**
 * @brief SummWidget::resizeGL
 * @param width
 * @param height
 */
void SummWidget::resizeGL(int width, int height)
{
    width = width;
    height = height;
}


/**
 * @brief SummWidget::stitchUpdate
 * process when get a synced frame signal
 */
void SummWidget::stitchUpdate()
{
    update();
}


/**
 * @brief SummWidget::stitchUpdateTime
 */
void SummWidget::stitchUpdateTime()
{
    update();
}

void SummWidget::mouseMoveEvent(QMouseEvent *event)
{
    event = event;
}

int SummWidget::getStatus(void)
{
    return m_status;
}

void SummWidget::setPaintStatus()
{
    static int s_flag = 0;

    if (0 == s_flag)
    {
        s_flag = 1;

        // set shm
        unsigned char *p_mem = NULL;
        int memsize = 0;
        comucmd_get_ext_share_mem(&p_mem, &memsize);
        if (p_mem != NULL)
        {
            ext_share_mem_s *p_share_s = (ext_share_mem_s *)p_mem;
            p_share_s->qt_status |= 0x02;
        }

        // set status
        m_status = 1;
    }
}

