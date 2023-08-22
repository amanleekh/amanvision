
#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QDialog>
#include <QThread>
#include <QObject>
#include <QDebug>
#include <QTimer>
//#include <QtConcurrent>
#include <QtConcurrent/QtConcurrent>
#include <QMouseEvent>

#include <sys/time.h>

#include "common_def.h"
#include "surround_widget.h"
#include "avm_gpu.h"
#include "video_sync.h"
//#include "minIni.h"
#include "overlay_draw.h"
#include "ini_config.h"
#include "avm_gpu.h"
#include "cmd_recver.h"
#include "custom_str.h"
#include "glloaderthread.h"
#include "sync_video_getter.h"
#include "comu_cmd_define.h"
#include "comu_cmd.h"


/**< static value for fps */
int SurroundWidget::m_static_fps = 0;

/**
 * @brief SurroundWidget::SurroundWidget
 * @param parent
 * @param widgetWidth
 * @param widgetHeight
 */
SurroundWidget::SurroundWidget(QWidget *parent,int wideangle_width,
    int widgetWidth, int widgetHeight,
    int tex_width, int tex_height) : 
    QOpenGLWidget(parent)
{
    // set init val
    m_avmGpu = NULL;
    m_textureWidth = tex_width;
    m_textureHeight = tex_height;
    m_widgetWidth = widgetWidth;
    m_widgetHeight = widgetHeight;
    m_wideangleWidth = wideangle_width;
    m_prt_fps_flag = 0;
    m_current_view = AVM_VIEW_AUTO;
    m_fps = (float)(DEFAULT_FPS);

	m_p_mViewLabel = NULL;

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
    m_overlay->pre_load_block_line_data();

    m_status = 0;
}


/**
 * @brief SurroundWidget::~SurroundWidget
 */
SurroundWidget::~SurroundWidget()
{
    delete m_avmGpu;
}


void SurroundWidget::setLabel(QLabel *pLabel)
{
	m_p_mViewLabel = pLabel;
}


int SurroundWidget::getView()
{
    return m_avmGpu->avm_gpu_get_view();
}

int SurroundWidget::get_view_by_cur_deg()
{
	return m_avmGpu->avm_gpu_get_view_by_cur_deg();
}

int SurroundWidget::getStatus(void)
{
    return m_status;
}


void SurroundWidget::setView(int viewEnum)
{
    if ((viewEnum < AVM_VIEW_AUTO) || (viewEnum > AVM_VIEW_LAST))
    {
        return;
    }
    m_current_view = (AVM_VIEW_E)viewEnum;

    m_avmGpu->avm_gpu_set_view(viewEnum);

    glm::mat4 v;
    glm::mat4 p;
    m_avmGpu->avm_gpu_get_v_p(&v, &p);
    m_carModel->set_mvp(NULL, &v, &p);

    m_overlay->set_radar_mvp(NULL, &v, &p);

    m_overlay->set_assist_line_force_update();
    m_overlay->set_assist_line_mvp(NULL, &v, &p);

    update();
}


void SurroundWidget::setViewDyn(int viewEnum)
{
    int ret = m_avmGpu->avm_gpu_set_view_dyn_trans(viewEnum);
    if (0 == ret)
    {
        update();
    }
}

void SurroundWidget::rotateView(int det_x)
{    
    int ret = m_avmGpu->avm_gpu_set_rotate(0, det_x, 0);
    if (0 == ret)
    {
        glm :: mat4 p_v;
        m_avmGpu->avm_gpu_get_v_p(&p_v, NULL);
        m_carModel->set_mvp(NULL, &p_v, NULL);
        m_overlay->set_radar_mvp(NULL, &p_v, NULL);
        m_overlay->set_assist_line_mvp(NULL, &p_v, NULL);
        
        update();
    }
}


void SurroundWidget::rotateView_omni(int det_x, int det_y)
{    
    int ret = m_avmGpu->avm_gpu_omni_directional_rotate(det_x, det_y);
    if (0 == ret)
    {
        glm :: mat4 p_v;
        m_avmGpu->avm_gpu_get_v_p(&p_v, NULL);
        m_carModel->set_mvp(NULL, &p_v, NULL);
        m_overlay->set_radar_mvp(NULL, &p_v, NULL);
        m_overlay->set_assist_line_mvp(NULL, &p_v, NULL);
        
        update();
    }
}


void SurroundWidget::setRadar(unsigned mask, unsigned int mask2)
{
    m_overlay->radar_set_alert(mask, mask2, NULL);
}


void SurroundWidget::clrRadar(unsigned mask, unsigned int mask2)
{
    m_overlay->radar_clr_alert(mask, mask2, NULL);
}

void SurroundWidget::showRadar(int isShow)
{
    m_overlay->radar_show(isShow);
}


void SurroundWidget::setAssistLine(float angle)
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

    // set angle int
    m_carModel->set_wheel_angle(angle_int);
}

void SurroundWidget::showAssistLine(int isShow)
{
    m_overlay->assist_line_show(isShow);
}


void SurroundWidget::showBlockLine(int isShow)
{
    m_overlay->block_line_show(isShow);
}


void SurroundWidget::setSpeed(int speed)
{
    m_carModel->set_wheel_speed(speed);
}


void SurroundWidget::setTurnSignal(int turn)
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


void SurroundWidget::setEmFlahers(int is_on)
{
    m_carModel->set_em_flashers(is_on);
}



void SurroundWidget::setModelAlpha(int alpha)
{
    m_carModel->set_model_alpha(alpha);
}


void SurroundWidget::prtFps()
{
    m_prt_fps_flag = 1;
}


void SurroundWidget::setCarDirection(int is_front)
{
    m_overlay->assist_line_set_direction(is_front);
}


/**< hide stitch camera, can be used when car's door is open */
void SurroundWidget::hideCamera(int is_hide_right, int is_hide_front,
    int is_hide_left, int is_hide_rear)
{
    m_avmGpu->avm_gpu_hide_stitch(is_hide_right, is_hide_front, 
        is_hide_left, is_hide_rear);
}

void SurroundWidget::setDoor(int fl_open, int fr_open, int bl_open, int br_open)
{
    m_carModel->set_door(fl_open, fr_open, bl_open, br_open);
}


/**
 * @brief SurroundWidget::initializeGL, nit gl env, qopenglwidget func
 * this func be called only once before paintGl or resizeGl 
 */
void SurroundWidget::initializeGL()
{
    int ret;
    float window_alpha = 0.5;

    // QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();
    // f->glClear(GL_COLOR_BUFFER_BIT);
    // the prefixing of each and every OpenGL call can be avoided by deriving from QOpenGLFunctions instead:
    initializeOpenGLFunctions();

    // env init, compile shader and create opengl program
    ret = m_avmGpu->avmgpu_init();
    if (ret != 0)
    {
        SURRWGT_ERR("SurroundWidget::initializeGL. avmgpu_init faild\n");
        return;
    }

    m_avmGpu->avm_gpu_postion_set_auto_height(8.0f);
    // set stitch default view
    ret = m_avmGpu->avm_gpu_set_view(m_current_view, 1);
    if (ret != 0)
    {
        SURRWGT_ERR("SurroundWidget::initializeGL. avm_gpu_postion_init faild\n");
        return;
    }    
    // update view enum
    if (m_p_mViewLabel != NULL)
    {
        //QString str1(getAutoStr());
        QString str2 = custstr_get_view_str(m_current_view);
        m_p_mViewLabel->setText(str2);
    }

    // get stitch view and proj matrix
    glm::mat4 stitch_v;
    glm::mat4 stitch_p;
    m_avmGpu->avm_gpu_get_v_p(&stitch_v, &stitch_p);

    // create model program and gen vbo
    m_carModel->create_program();

    // get window alpha from config.ini
    window_alpha = ini_get_car_model_window_alpha();

    // set window alpha
    m_carModel->set_window_alpha(window_alpha);

    //m_carModel->gen_model_vbo();
    m_carModel->set_mvp(NULL, &stitch_v, &stitch_p);

    // create overlay program and gen vbo
    m_overlay->create_program();
    m_overlay->gen_block_line_vbo();
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

    SURRWGT_LOG("SurroundWidget::initializeGL. init success\n");
}


/**
 * @brief SurroundWidget::paintGL
 */
void SurroundWidget::paintGL()
{
    calc_fps();

    // Clear color and depth buffers
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // if dyn change view state, will dyn change other model's view
    int is_in_dyn_view = m_avmGpu->avm_gpu_is_in_dyn_view();
    m_avmGpu->avm_gpu_process();
    if (is_in_dyn_view)
    {
        glm::mat4 p_v;
        m_avmGpu->avm_gpu_get_v_p(&p_v, NULL);
        m_carModel->set_mvp(NULL, &p_v, NULL);
        m_overlay->set_radar_mvp(NULL, &p_v, NULL);
        m_overlay->set_assist_line_mvp(NULL, &p_v, NULL);
    }

    int view = m_avmGpu->avm_gpu_get_view();
    if ((AVM_VIEW_AUTO == view) || m_avmGpu->avm_gpu_check_in_3d_view(view))
    {
        m_overlay->overlay_render_start();
        m_overlay->bottom_render_process();
        m_overlay->assist_line_process(0);
        m_overlay->radar_render_process();
        m_overlay->overlay_render_end();

        m_carModel->render_process(0);
    }
    else if (AVM_VIEW_CAMERA_FRONT == view)
    {
        m_overlay->overlay_render_start();
        m_overlay->assist_line_process(1);
        m_overlay->block_line_process(1);
        m_overlay->overlay_render_end();
    }
    else if (AVM_VIEW_CAMERA_REAR == view)
    {
        m_overlay->overlay_render_start();
        m_overlay->assist_line_process(2);
        m_overlay->block_line_process(2);
        m_overlay->overlay_render_end(); 
    }
    else if ((AVM_VIEW_CAMERA_FISH_FRONT == view)
        || (AVM_VIEW_CAMERA_FISH_CLIP_FRONT == view))
    {
        m_overlay->overlay_render_start();
        m_overlay->assist_line_process(1);
        m_overlay->block_line_process(1);
        m_overlay->overlay_render_end();
    }
    else if ((AVM_VIEW_CAMERA_FISH_REAR == view)
        || (AVM_VIEW_CAMERA_FISH_CLIP_REAR == view))
    {
        m_overlay->overlay_render_start();
        m_overlay->assist_line_process(2);
        m_overlay->block_line_process(2);
        m_overlay->overlay_render_end();
    }

    GLenum gl_err = glGetError();
    if (gl_err != GL_NO_ERROR)
    {
        SURRWGT_LOG("SurroundWidget::paintGL. gl_error=%d\n", gl_err);
    }

    // set shm flag
    setPaintStatus();
}


/**
 * @brief SurroundWidget::resizeGL
 * @param width
 * @param height
 */
void SurroundWidget::resizeGL(int width, int height)
{
    width = width;
    height = height;
}


void SurroundWidget::calc_fps()
{
    static unsigned int s_lst_ms = 0;
    static unsigned int s_total_ms = 0;
    static int s_total_cnt = 0;
    static int s_show_cnt = 3;

    if (1 == m_prt_fps_flag)
    {
        m_prt_fps_flag = 0;
        if (s_show_cnt >= 0)
        {
            s_show_cnt++;
        }
        else
        {
            s_show_cnt = 1;
        }
    }

    // get time ms
    struct timeval tv;
    gettimeofday(&tv, NULL);
    unsigned int time_in_mill
        = (tv.tv_sec) * 1000 + (tv.tv_usec) / 1000;

    if (0 == s_lst_ms)
    {
        s_lst_ms = time_in_mill;
    }
    else
    {
        int det = time_in_mill - s_lst_ms;
        if ((det > 0) && (det < 10000))
        {
            s_total_ms += det;
            s_total_cnt++;
        }
        s_lst_ms = time_in_mill;
    }
    if (s_total_ms >= 5000)
    {
        float fps = s_total_cnt * 1000.0 / s_total_ms;
        if ((fps > 4.0f) && (fps < 50.0f))
        {
            m_fps = fps;
            m_carModel->set_fps(m_fps);
            m_avmGpu->avm_gpu_set_fps(m_fps);
            SurroundWidget::m_static_fps = (int)(m_fps);
        }
        if (s_show_cnt > 0)
        {
            s_show_cnt = s_show_cnt - 1;
            SURRWGT_LOG("SurroundWidget::calc_fps. fps=%.2f\n", m_fps);
        }

        s_lst_ms = 0;
        s_total_ms = 0;
        s_total_cnt = 0;
    }
}


/**
 * @brief SurroundWidget::stitchUpdate
 * process when get a synced frame signal
 */
void SurroundWidget::stitchUpdate()
{
    update();
}


/**
 * @brief SurroundWidget::stitchUpdateTime
 */
void SurroundWidget::stitchUpdateTime()
{
    update();
}


void SurroundWidget::updateWidgetWH(int widgetWidth, int widgetHeight)
{
    m_widgetWidth = widgetWidth;
    widgetHeight = widgetHeight;
    m_avmGpu->avm_gpu_update_wh(widgetWidth, widgetHeight);
}


QString SurroundWidget::getAutoStr()
{
    if (shm_set_is_manual())
    {
        return custstr_get_manual();
    }
    else
    {
        return custstr_get_auto();
    }
}


int SurroundWidget::getFps()
{
    return SurroundWidget::m_static_fps;
}


void SurroundWidget::setPaintStatus()
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
            p_share_s->qt_status |= 0x01; 
        }       

        // set status
        m_status = 1;
    }
}

