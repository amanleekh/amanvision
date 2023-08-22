
#include <limits>
#include <sys/time.h>

#include "common_def.h"
#include "model_draw.h"
#include "gpu_shader.h"
#include "thread.h"
//#include "minIni.h"
#include "ini_config.h"

// Include GLM core features
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
#include <glm/trigonometric.hpp>

// Include GLM extensions
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>


#define MODELDW_DOOR_ANGLE      (45.0f)    // angle door open


QMutex ModelDraw::m_mutex;   /**< mutex load car */
ModelLoader *ModelDraw::m_p_model_loader = NULL;
int ModelDraw::m_is_model_loaded  = 0;

QMutex ModelDraw::m_texture_mutex;
int ModelDraw::m_texture_loadded = 0;

int ModelDraw::m_blink_cnt = 0;
float ModelDraw::m_fps = (float)(DEFAULT_FPS);
int ModelDraw::m_blink_intv = DEFAULT_FPS_EVEN;     // // blink period, 0.5HZ

// car type, 1 for 01, 2 for 02, 3 for 03
ModelDraw::ModelDraw(int car_type)
{
    // mvp
    m_m = glm::mat4(1.0f);
    m_v = glm::mat4(1.0f);
    m_p = glm::mat4(1.0f);
    m_mvp = m_p * m_v * m_m;
    m_y_trans = 0.0f;
    m_car_type = car_type;

    float car_model_blink_r = 1.0f;
    float car_model_blink_g = 0.49f;
    float car_model_blink_b = 0.01f;
    float ini_material_specular_r = 0.2f;
    float ini_material_specular_g = 0.2f;
    float ini_material_specular_b = 0.2f;

    // light
#if GLOBAL_RUN_ENV_DESKTOP == 0
    m_light_info.Position = QVector4D(0.0f, 0.0f, 20.0f, 1.0f);
#else
    m_light_info.Position = QVector4D(0.0f, 0.0f, 20.0f, 1.0f);
#endif
    m_light_info.Intensity = QVector3D(1.0f, 1.0f, 1.0f);
    
    // material info
    ini_material_specular_r = ini_get_material_specular_r();
    ini_material_specular_g = ini_get_material_specular_g();
    ini_material_specular_b = ini_get_material_specular_b();

    m_material_info.Ambient  = QVector3D(0.05f, 0.05f, 0.05f);
    m_material_info.Diffuse  = QVector3D(0.3f, 0.3f, 0.3f);
    m_material_info.Specular = QVector3D(ini_material_specular_r, ini_material_specular_g, ini_material_specular_b);
    m_material_info.Shininess = 50.0f;

    // blink info
    car_model_blink_r = ini_get_car_model_blink_r();
    car_model_blink_g = ini_get_car_model_blink_g();
    car_model_blink_b = ini_get_car_model_blink_b();

    m_blink_material_info.Ambient = QVector3D(car_model_blink_r, car_model_blink_g, car_model_blink_b);
    m_blink_material_info.Diffuse = QVector3D(0.1f, 0.1f, 0.1f);
    m_blink_material_info.Specular = QVector3D(0.0f, 0.0f, 0.0f);
    m_blink_material_info.Shininess = 50.0f;

    // misc
    m_p_vshader_str = "avm_qt_app_res/shader/vshader_model_desktop.glsl";
    m_p_fshader_str = "avm_qt_app_res/shader/fshader_model_desktop.glsl";
    m_is_model_block_load = 0;
    m_is_vbo_gened = 0;
    m_have_uv = 0;
    m_wheel_angle = 0;
    m_wheel_speed = 0;
    m_wheel_angle_mat = glm::mat4(1.0);
    m_wheel_speed_mat = glm::mat4(1.0);
    m_wheel_anglespeed_mat = glm::mat4(1.0);

    for (int i = 0; i < 4; i++)
    {
        m_door_open[i] = 0;
        m_door_trans[i] = 1;
        m_door_close_mat[i] = glm::mat4(1.0);
        m_door_cur_mat[i] = glm::mat4(1.0); // default door is closed
        m_light_blink[i] = 0;
    }
    m_is_in_em_flasher = 0;
    m_door_open_mat[0] = glm::rotate(glm::mat4(1.0), glm::radians(MODELDW_DOOR_ANGLE*-1.0f), glm::vec3(0, 0, 1));
    m_door_open_mat[1] = glm::rotate(glm::mat4(1.0), glm::radians(MODELDW_DOOR_ANGLE*1.0f), glm::vec3(0, 0, 1));
    m_door_open_mat[2] = glm::rotate(glm::mat4(1.0), glm::radians(MODELDW_DOOR_ANGLE*-1.0f), glm::vec3(0, 0, 1));
    m_door_open_mat[3] = glm::rotate(glm::mat4(1.0), glm::radians(MODELDW_DOOR_ANGLE*1.0f), glm::vec3(0, 0, 1));

    m_alpha_change = 1;
    m_alpha = 100;
    m_window_alpha = 0.8;
    
    // create model loader
    if (NULL == ModelDraw::m_p_model_loader)
    {
        ModelDraw::m_p_model_loader = new ModelLoader(false);
    }
}


int ModelDraw::create_program()
{
    int ret;

    // load shader program. compile and load
    ret = shader_load(m_p_vshader_str, m_p_fshader_str, &m_shader_handler);
    if (ret != 0)
    {
        AVM_ERR("ModelDraw::create_program. shader_load_str faild.\n");
        return 1;
    }

    GLuint hProgram = shader_get_program(m_shader_handler);

    // get gles value location
    m_loc_vertex_position = glGetAttribLocation(hProgram, "vertexPosition");
    m_loc_vertex_normal = glGetAttribLocation(hProgram, "vertexNormal");
	m_loc_texture_coord = glGetAttribLocation(hProgram, "vsTexCoord");
    m_loc_mv = glGetUniformLocation(hProgram, "MV");
    m_loc_n = glGetUniformLocation(hProgram, "N");
    m_loc_mvp = glGetUniformLocation(hProgram, "MVP");
    m_loc_light_position = glGetUniformLocation(hProgram, "lightPosition");
    m_loc_light_intensity = glGetUniformLocation(hProgram, "lightIntensity");
    m_loc_ka = glGetUniformLocation(hProgram, "Ka");
    m_loc_kd = glGetUniformLocation(hProgram, "Kd");
    m_loc_ks = glGetUniformLocation(hProgram, "Ks");
    m_loc_shininess = glGetUniformLocation(hProgram, "shininess");
    m_loc_alpha = glGetUniformLocation(hProgram, "alpha");
	m_loc_sampler = glGetUniformLocation(hProgram, "fsSampler");
	m_loc_has_texture = glGetUniformLocation(hProgram, "hasTex");
    
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    return 0;
}


static unsigned int load_model_get_time_ms()
{
    struct timeval  tv;
    gettimeofday(&tv, NULL);

    unsigned int time_in_mill 
        = (tv.tv_sec) * 1000 + (tv.tv_usec) / 1000;
    return time_in_mill;
}


static void *load_model_thread_func(void *arg)
{
    load_thread_s *p_st = (load_thread_s *)arg;
    *(p_st->p_is_loaded) = 0;

    AVM_LOG("load_model_thread_func. start load '%s' file\n", p_st->p_path);

    unsigned int time_start = load_model_get_time_ms();
    
    if (!p_st->p_handler->Load(QString(p_st->p_path), ModelLoader::RelativePath))
    {
        AVM_ERR("load_model_thread_func. load model faild\n");
        return NULL;
    }

    if (p_st->is_scale)
    {
        p_st->p_handler->transformToCoordinates(p_st->scale_val, p_st->dst_w, p_st->dst_h, p_st->dst_z);
    }
    *(p_st->p_is_loaded) = 1;

    unsigned int time_end = load_model_get_time_ms();
    int det = time_end - time_start;
    AVM_LOG("load_model_thread_func, file '%s' cost %d ms\n", p_st->p_path, det);

    return NULL;
}


int ModelDraw::pre_load_model_data(const char *p_path, int is_scale, float scale_val,
        float dst_w, float dst_h, float dst_z, int is_block)
{
    int re;
    
    if (is_scale)
    {
        if ((dst_w <= 0.0f) && (dst_h <= 0.0f) && (dst_z <= 0.0f) && (scale_val < 0.0f))
        {
            AVM_ERR("ModelDraw::pre_load_model_data. param error, dst_w=%f, dst_h=%f\n", dst_w, dst_h);
            return 1;
        }
    }

    if (is_block)
    {
        unsigned int time_start = load_model_get_time_ms();

        if (!ModelDraw::m_p_model_loader->Load(QString(p_path), ModelLoader::RelativePath))
        {
            AVM_ERR("ModelDraw::pre_load_model_data. load model faild\n");
            return 1;
        }
        if (is_scale)
        {
            ModelDraw::m_p_model_loader->transformToCoordinates(scale_val, dst_w, dst_h, dst_z);
        }
        ModelDraw::m_is_model_loaded = 1;
        unsigned int time_end = load_model_get_time_ms();
        int det = time_end - time_start;
        AVM_LOG("ModelDraw::pre_load_model_data. file %s cost %d ms\n", p_path, det);
    }
    else
    {
        snprintf(m_model_name, 256, "%s", p_path);
        m_nonblock_thread_arg.p_handler = ModelDraw::m_p_model_loader;
        m_nonblock_thread_arg.p_path = m_model_name;
        m_nonblock_thread_arg.is_scale = is_scale;
        m_nonblock_thread_arg.scale_val = scale_val;
        m_nonblock_thread_arg.dst_w = dst_w;
        m_nonblock_thread_arg.dst_h = dst_h;
        m_nonblock_thread_arg.dst_z = dst_z;
        m_nonblock_thread_arg.p_is_loaded = &ModelDraw::m_is_model_loaded;
        re = thread_create(50, &load_model_thread_func, (void *)&m_nonblock_thread_arg);

        if (re != 0)
        {
            AVM_ERR("ModelDraw::pre_load_model_data. create thread faild\n");
            return 1; 
        }
    }

    return 0;
}


int ModelDraw::pre_load_car_data(int is_block)
{
    // async load is implemented by share gl context, so thread asyn load is not support ever
    if (0 == is_block)
    {
        AVM_ERR("ModelDraw::pre_load_car_data. current not support\n");
        return 1;
    }
    m_is_model_block_load = is_block;
    
    ModelDraw::m_mutex.lock();
    if (0 == ModelDraw::m_is_model_loaded)
    {
        // get model file name
        char str_name[256];
        if (ini_get_car_model_name(str_name, sizeof(str_name)) != 0)
        {
            AVM_ERR("ModelDraw::pre_load_car_data. read config file faild\n");
            return 1;
        }

        // pre load data and scale
        int is_scale = 1;
        float scale_val = ini_get_car_model_scale();
        if (scale_val > 0.0f)
        {
            AVM_LOG("ModelDraw::pre_load_car_data. ini file, scale_val=%.2f\n", scale_val);
            pre_load_model_data(str_name, is_scale, scale_val, -1.0f, -1.0f, -1.0f, is_block);
        }
        else
        {
            float dst_w = ini_get_car_model_dst_w();
            float dst_h = ini_get_car_model_dst_h();
            float dst_z = ini_get_car_model_dst_z();
            if ((dst_w < 0.0f) && (dst_h < 0.0f) && (dst_z < 0.0f))
            {
                is_scale = 0;
                pre_load_model_data(str_name, is_scale, -1.0f, -1.0f, -1.0f, -1.0f, is_block);
            }
            else
            {
                AVM_LOG("ModelDraw::pre_load_car_data. ini file, dst_w=%.2f, h=%.2f, z=%.2ssssf\n",
                    dst_w, dst_h, dst_z);
                pre_load_model_data(str_name, is_scale, -1.0f, dst_w, dst_h, dst_z, is_block);
            }
        }
    }
    ModelDraw::m_mutex.unlock();

    // move det
    float det = ini_get_car_model_det();
    AVM_LOG("ModelDraw::pre_load_car_data. ini file, det=%.2f\n", det);
    m_y_trans = det;
    m_m = glm::translate(m_m, glm::vec3(0.0f, 0.0f, m_y_trans));

    // change light position, at top of car
    m_light_info.Position[1] = m_y_trans;

    // get car info
    if (read_wheel_string() != 0)
    {
        AVM_ERR("ModelDraw::pre_load_car_data wheel faild\n");
        return 1;
    }
    // get door string ingo
    if (read_door_string() != 0)
    {
        AVM_ERR("ModelDraw::pre_load_car_data door faild\n");
        return 1;
    }
    // light
    if (read_light_string() != 0)
    {
        AVM_ERR("ModelDraw::pre_load_car_data light faild\n");
        return 1;        
    }

    return 0;
}


int ModelDraw::read_wheel_string()
{
    char str_name[256];
    
    // front left wheel
    if (ini_get_car_model_fl_wheel_name(str_name, sizeof(str_name)) != 0)
    {
        AVM_ERR("ModelDraw::read_wheel_string. read fl_wheel_name faild\n");
        return 1;
    }
    m_wheel_fl = QString(str_name);

    // front right wheel
    if (ini_get_car_model_fr_wheel_name(str_name, sizeof(str_name)) != 0)
    {
        AVM_ERR("ModelDraw::read_wheel_string. read fr_wheel_name faild\n");
        return 1;
    }
    m_wheel_fr = QString(str_name);    

    // bottom left
    if (ini_get_car_model_bl_wheel_name(str_name, sizeof(str_name)) != 0)
    {
        AVM_ERR("ModelDraw::read_wheel_string. read bl_wheel_name faild\n");
        return 1;
    }
    m_wheel_bl = QString(str_name);

    // bottom right
    if (ini_get_car_model_br_wheel_name(str_name, sizeof(str_name)) != 0)
    {
        AVM_ERR("ModelDraw::read_wheel_string. read br_wheel_name faild\n");
        return 1;
    }
    m_wheel_br = QString(str_name); 

    return 0;
}


int ModelDraw::read_door_string()
{
    char str_name[256];

    // front left door
    if (ini_get_car_model_fl_door_name(str_name, sizeof(str_name)) != 0)
    {
        AVM_ERR("ModelDraw::read_door_string. read fl_wheel_name faild\n");
        return 1;
    }
    m_door_str[0] = QString(str_name);

    // front right door
    if (ini_get_car_model_fr_door_name(str_name, sizeof(str_name)) != 0)
    {
        AVM_ERR("ModelDraw::read_door_string. read fr_wheel_name faild\n");
        return 1;
    }
    m_door_str[1] = QString(str_name);    

    // bottom left door
    if (ini_get_car_model_bl_door_name(str_name, sizeof(str_name)) != 0)
    {
        AVM_ERR("ModelDraw::read_door_string. read bl_wheel_name faild\n");
        return 1;
    }
    m_door_str[2] = QString(str_name);

    // bottom right door
    if (ini_get_car_model_br_door_name(str_name, sizeof(str_name)) != 0)
    {
        AVM_ERR("ModelDraw::read_door_string. read br_wheel_name faild\n");
        return 1;
    }
    m_door_str[3] = QString(str_name); 

    // front window
    if (ini_get_car_model_front_win_name(str_name, sizeof(str_name)) != 0)
    {
        AVM_ERR("ModelDraw::read_door_string. read front_win_name faild\n");
        return 1;
    }    
    m_window_str[0] = QString(str_name);

    // front left door's window
    if (ini_get_car_model_fl_win_name(str_name, sizeof(str_name)) != 0)
    {
        AVM_ERR("ModelDraw::read_door_string. read fl_win_name faild\n");
        return 1;
    }    
    m_window_str[1] = QString(str_name);

    // front right door's window
    if (ini_get_car_model_fr_win_name(str_name, sizeof(str_name)) != 0)
    {
        AVM_ERR("ModelDraw::read_door_string. read fl_win_name faild\n");
        return 1;
    }    
    m_window_str[2] = QString(str_name);

    // rear left door's window
    if (ini_get_car_model_bl_win_name(str_name, sizeof(str_name)) != 0)
    {
        AVM_ERR("ModelDraw::read_door_string. read bl_win_name faild\n");
        return 1;
    }    
    m_window_str[3] = QString(str_name);

    // rear right door's window
    if (ini_get_car_model_br_win_name(str_name, sizeof(str_name)) != 0)
    {
        AVM_ERR("ModelDraw::read_door_string. read br_win_name faild\n");
        return 1;
    }    
    m_window_str[4] = QString(str_name);

    // front window
    if (ini_get_car_model_rear_win_name(str_name, sizeof(str_name)) != 0)
    {
        AVM_ERR("ModelDraw::read_door_string. read rear_win_name faild\n");
        return 1;
    }    
    m_window_str[5] = QString(str_name);

    return 0;
}


int ModelDraw::read_light_string()
{
    char str_name[256];

    // clear all str
    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 3; j++)
        {
            m_light_str[i][j] = QString();
        }
    }

    QStringList list;
    // front left light
    if (ini_get_car_model_fl_light_name(str_name, sizeof(str_name)) != 0)
    {
        AVM_ERR("ModelDraw::read_light_string. read fl_light_name faild\n");
        return 1;
    }
    list = QString(str_name).split(' ');
    for (int i = 0; i < list.size() && i < 3; i++)
    {
        m_light_str[0][i] = list.at(i);
    }

    // front right light
    if (ini_get_car_model_fr_light_name(str_name, sizeof(str_name)) != 0)
    {
        AVM_ERR("ModelDraw::read_light_string. read fr_light_name faild\n");
        return 1;
    }
    list = QString(str_name).split(' ');
    for (int i = 0; i < list.size() && i < 3; i++)
    {
        m_light_str[1][i] = list.at(i);
    }   

    // bottom left light
    if (ini_get_car_model_bl_light_name(str_name, sizeof(str_name)) != 0)
    {
        AVM_ERR("ModelDraw::read_light_string. read bl_light_name faild\n");
        return 1;
    }
    list = QString(str_name).split(' ');
    for (int i = 0; i < list.size() && i < 3; i++)
    {
        m_light_str[2][i] = list.at(i);
    }

    // bottom right light
    if (ini_get_car_model_br_light_name(str_name, sizeof(str_name)) != 0)
    {
        AVM_ERR("ModelDraw::read_light_string. read br_light_name faild\n");
        return 1;
    }
    list = QString(str_name).split(' ');
    for (int i = 0; i < list.size() && i < 3; i++)
    {
        m_light_str[3][i] = list.at(i);
    }

    return 0;
}



int ModelDraw::check_light_node_name(int index, QString input_name)
{
    for (int i = 0; i < 3; i++)
    {
        if (m_light_str[index][i] == input_name)
        {
            return 1;
        }
    }

    return 0;
}


int ModelDraw::is_model_loaded()
{
    return ModelDraw::m_is_model_loaded;    
}


void ModelDraw::set_rotate(float angle)
{
    m_m = glm::mat4(1.0);

    m_m = glm::rotate(m_m, glm::radians(angle), glm::vec3(0, 1, 0));
    m_m = glm::translate(m_m, glm::vec3(0.0f, 0.0f, m_y_trans));
}


int ModelDraw::gen_model_vbo()
{
    int ret;
    
    if (0 == m_is_model_block_load)
    {
        return 0;
    }
    
    ret = gen_model_vbo_inner();
    if (ret != 0)
    {
        AVM_ERR("ModelDraw::gen_model_vbo. gen_model_vbo_inner faild\n");
        return 1;
    }

    ret = gen_texture();
    if (ret != 0)
    {
        AVM_ERR("ModelDraw::gen_model_vbo. gen_texture faild\n");
        return 1;
    }
    m_is_vbo_gened = 1;

    return 0;
}


int ModelDraw::gen_model_vbo_inner()
{
    if (0 == ModelDraw::m_is_model_loaded)
    {
        AVM_ERR("ModelDraw::gen_model_vbo, model is not loadded\n");
        return 1;
    }

    int i;

    QVector<float> *vertices;
    QVector<float> *normals;
    QVector<QVector<float> > *texture_uv;
    QVector<unsigned int> *indices;

    ModelDraw::m_p_model_loader->getBufferData(&vertices, &normals, &indices);
    ModelDraw::m_p_model_loader->getTextureData(&texture_uv, 0, 0);

#if MODEL_DRAW_USE_UINT == 0
    if (vertices->size() > std::numeric_limits<unsigned short>::max())
    {
        AVM_ERR("ModelDraw::gen_model_vbo. OpenGL ES 2.0 index buffer types do not support 32-bit integers\n");
        AVM_ERR("ModelDraw::gen_model_vbo. so vertex buffers cannot have more than std::numeric_limits<unsigned short>::max() vertices\n");
        AVM_ERR("ModelDraw::gen_model_vbo. this model has %d more than than std::numeric_limits<unsigned short>::max()=%d vertices so will likely not render correctly\n",
            vertices->size(), std::numeric_limits<unsigned short>::max());
        return 1;
    }
#endif

    // Create a buffer and copy the vertex data to it
    glGenBuffers(1, &m_vertex_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, m_vertex_buffer);
    glBufferData(GL_ARRAY_BUFFER, vertices->size()*sizeof(GLfloat), &(*vertices)[0], GL_STATIC_DRAW);

    // Create a buffer and copy the normal data to it
    glGenBuffers(1, &m_normal_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, m_normal_buffer);
    glBufferData(GL_ARRAY_BUFFER, normals->size()*sizeof(GLfloat), &(*normals)[0], GL_STATIC_DRAW);

    // Create a buffer and copy the texture data to it
    m_have_uv = 0;
    if ((texture_uv != 0) && (texture_uv->size() != 0))
    {
        int texSize = 0;
        glGenBuffers(1, &m_texture_uv_buffer);
        glBindBuffer(GL_ARRAY_BUFFER, m_texture_uv_buffer);
        for (i = 0; i < texture_uv->size(); i++)
        {
            texSize += texture_uv->at(i).size();
        }
        glBufferData(GL_ARRAY_BUFFER, texSize*sizeof(GLfloat), &(*texture_uv)[0][0], GL_STATIC_DRAW);
        m_have_uv = 1;
    }

    // Create a buffer and copy the index data to it
    glGenBuffers(1, &m_index_buffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_index_buffer);

    AVM_LOG("ModelDraw::load_model. vert_size=%d, normal_size=%d, uv_size=%d, idx_size=%d\n",
        vertices->size(), normals->size(), texture_uv->size(), indices->size());

#if MODEL_DRAW_USE_UINT == 0
    // OpenGL ES -- unsigned long int type indexes are not supported, use unsigned short instead
    // This means the number of vertices in a single vertex buffer is limited to std::numeric_limits<unsigned short>::max()
    QVector<unsigned short> shortindices;
    shortindices.resize(indices->size());
    std::copy(indices->begin(), indices->end(), shortindices.begin());
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, shortindices.size() * sizeof(unsigned short), &(shortindices)[0], GL_STATIC_DRAW);
#else
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices->size() * sizeof(unsigned int), &(*indices)[0], GL_STATIC_DRAW);
#endif

    m_model_root_node = ModelDraw::m_p_model_loader->getNodeData();

    return 0;
}


int ModelDraw::gen_texture()
{
    int re = 0;
    
    ModelDraw::m_texture_mutex.lock();
    if (0 == ModelDraw::m_texture_loadded)
    {
	    re = ModelDraw::m_p_model_loader->postLoadGLTexture();
	    ModelDraw::m_texture_loadded = 1;
	}
	ModelDraw::m_texture_mutex.unlock();

    return re;
}


int ModelDraw::render_process(int widget_idx)
{
    if (0 == ModelDraw::m_is_model_loaded)
    {
        return 0;
    }
    if (0 == m_is_vbo_gened)
    {
        return 0;
    }

    glUseProgram(shader_get_program(m_shader_handler));

    // set model alpha
    if (m_alpha_change)
    {
        m_alpha_change = 0;
        if (100 == m_alpha)
        {
            glUniform1f(m_loc_alpha, 1.0f);
        }
        else
        {
            glUniform1f(m_loc_alpha, m_alpha/100.0);
        }
    }
    // set shader uniforms for light information
    glUniform4f(m_loc_light_position, 
        m_light_info.Position[0], m_light_info.Position[1],
        m_light_info.Position[2], m_light_info.Position[3]);
    glUniform3f(m_loc_light_intensity, 
        m_light_info.Intensity[0], m_light_info.Intensity[1], m_light_info.Intensity[2]);

    // vertex buffer
    glBindBuffer(GL_ARRAY_BUFFER, m_vertex_buffer);
    glVertexAttribPointer(m_loc_vertex_position, 3, GL_FLOAT, GL_FALSE,
        3 * sizeof(float), (GLvoid *)(0 * sizeof(float)));

    // normal buffer
    glBindBuffer(GL_ARRAY_BUFFER, m_normal_buffer);
    glVertexAttribPointer(m_loc_vertex_normal, 3, GL_FLOAT, GL_FALSE,
        3 * sizeof(float), (GLvoid *)(0 * sizeof(float)));

    // texture buffer
    if (m_have_uv)
    {
        glBindBuffer(GL_ARRAY_BUFFER, m_texture_uv_buffer);
        glVertexAttribPointer(m_loc_texture_coord, 2, GL_FLOAT, GL_FALSE,
            2 * sizeof(float), (GLvoid *)(0 * sizeof(float)));
    }

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_index_buffer);
    glEnableVertexAttribArray(m_loc_vertex_position);
    glEnableVertexAttribArray(m_loc_vertex_normal); 
    glEnableVertexAttribArray(m_loc_texture_coord);

    // calc angle matrix and speed matrix
    // wheel 235/60 R17, R=0.36m, 2*pi*R=2.26m .speed_deg/360*fps*2.26m=speed/3.6
    float speed_deg = m_wheel_speed * 360 / (3.6*2.26*m_fps);
    m_wheel_angle_mat = glm::rotate(glm::mat4(1.0), glm::radians(m_wheel_angle*-1.0f), glm::vec3(0, 0, 1));
    m_wheel_speed_mat = glm::rotate(m_wheel_speed_mat, glm::radians(speed_deg), glm::vec3(1, 0, 0));
    m_wheel_anglespeed_mat = m_wheel_angle_mat * m_wheel_speed_mat;
    draw_node(m_model_root_node.data(), glm::mat4(1.0f), 0);

    // we assume summwidget always exist
    if (1 == widget_idx)
    {
        ModelDraw::m_blink_cnt++;
    }
    
    return 0;
}


void ModelDraw::node_door_process(int door_idx, glm::mat4 &object_matrix, glm::mat4 &m)
{
    float speed = 1.0f;  // MODELDW_DOOR_ANGLE/(speed pre sec)
    int open_single_deg_dir[4] = {-1, 1, -1, 1};

    if (m_door_trans[door_idx])
    {
        float cur_deg = glm::atan(m_door_cur_mat[door_idx][0][1], m_door_cur_mat[door_idx][0][0]) / 3.141592653 * 180.0;
        if (m_door_open[door_idx])
        {
            float single_deg = open_single_deg_dir[door_idx] * MODELDW_DOOR_ANGLE / speed / m_fps;
            if (fabs(open_single_deg_dir[door_idx] * MODELDW_DOOR_ANGLE - cur_deg) < fabs(single_deg))
            {
                m_door_trans[door_idx] = 0;
                m_door_cur_mat[door_idx] = m_door_open_mat[door_idx];
            }
            else
            {
                m_door_cur_mat[door_idx] = glm::rotate(m_door_cur_mat[door_idx], glm::radians(single_deg), glm::vec3(0, 0, 1));
            }
        }
        else
        {
            float single_deg = open_single_deg_dir[door_idx] * -1.0f * MODELDW_DOOR_ANGLE / speed / m_fps;
            if (fabs(cur_deg) < fabs(single_deg))
            {
                m_door_trans[door_idx] = 0;
                m_door_cur_mat[door_idx] = m_door_close_mat[door_idx];  
            }
            else
            {
                m_door_cur_mat[door_idx] = glm::rotate(m_door_cur_mat[door_idx], glm::radians(single_deg), glm::vec3(0, 0, 1));
            }
        }
        m = m_m * object_matrix * m_door_cur_mat[door_idx];      
    }
    else
    {
        if (0 == m_door_open[door_idx])
        {
            m = m_m * object_matrix * m_door_close_mat[door_idx];
        }
        else
        {
            m = m_m * object_matrix * m_door_open_mat[door_idx];
        }
    }
}


void ModelDraw::draw_node(const Node *node, glm::mat4 object_matrix, int door_flag)
{
    int i, j;
    
    // Prepare matrices and set mvp uniform value
    object_matrix = object_matrix * glm::make_mat4(node->transformation.data());

    // set window aplha
    if ((node->name == m_window_str[0])
        || (node->name == m_window_str[1])
        || (node->name == m_window_str[2])
        || (node->name == m_window_str[3])
        || (node->name == m_window_str[4])
        || (node->name == m_window_str[5]))
    {
        glUniform1f(m_loc_alpha, m_window_alpha*m_alpha/100.0);
    }
    else
    {
        if (100 == m_alpha)
        {
            glUniform1f(m_loc_alpha, 1.0f);
        }
        else
        {
            glUniform1f(m_loc_alpha, m_alpha/100.0);
        }
    }

    // wheel angle and speed process
    glm::mat4 m;
    if ((node->name == m_wheel_fl) || (node->name == m_wheel_fr))
    {
        m = m_m * object_matrix * m_wheel_anglespeed_mat;
    }
    else if ((node->name == m_wheel_bl) || (node->name == m_wheel_br))
    {
        m = m_m * object_matrix * m_wheel_speed_mat;
    }
    // door process. loop 
    else if ((node->name == m_door_str[0]) || (1 == door_flag))
    {
        door_flag = 1;
        node_door_process(0, object_matrix, m);
    }
    else if ((node->name == m_door_str[1]) || (2 == door_flag))
    {
        door_flag = 2;
        node_door_process(1, object_matrix, m);
    }
    else if ((node->name == m_door_str[2]) || (3 == door_flag))
    {
        door_flag = 3;
        node_door_process(2, object_matrix, m);
    }
    else if ((node->name == m_door_str[3]) || (4 == door_flag))
    {
        door_flag = 4;
        node_door_process(3, object_matrix, m);
    }    
    else
    {
        m = m_m * object_matrix;
    }

    //glm::mat4 m =m_m * object_matrix;
    glm::mat4 mv = m_v * m;
    glm::mat3 n = glm::inverseTranspose(glm::mat3(mv));
    glm::mat4 mvp = m_p * m_v * m;

    glUniformMatrix4fv(m_loc_mv, 1, GL_FALSE, glm::value_ptr(mv)); // Transforming to eye space
    glUniformMatrix3fv(m_loc_n, 1, GL_FALSE, glm::value_ptr(n));    // Transform normal to Eye space
    glUniformMatrix4fv(m_loc_mvp, 1, GL_FALSE, glm::value_ptr(mvp));  // Matrix for transforming to Clip space

    int find_light_and_blink = 0;
    for (j = 0; j < 4; j++)
    {
        int cur_blink = 0;
        if (m_is_in_em_flasher)
        {
            cur_blink = 1;
        }
        else
        {
            cur_blink = m_light_blink[j];
        }
        if ((1 == cur_blink)
            && check_light_node_name(j, node->name)
            && ((ModelDraw::m_blink_cnt / m_blink_intv) % 2))
        {
            // Draw each mesh in this node
            for (i = 0; i < node->meshes.size(); i++)
            {
                set_material_uniforms(m_blink_material_info);
        
#if MODEL_DRAW_USE_UINT == 0
                // OpenGL ES -- unsigned long int type indexes are not supported, use unsigned short instead
                glDrawElements(GL_TRIANGLES, node->meshes[i]->indexCount, 
                    GL_UNSIGNED_SHORT, (const void*)(node->meshes[i]->indexOffset * sizeof(unsigned short)));
#else
                glDrawElements(GL_TRIANGLES, node->meshes[i]->indexCount, 
                    GL_UNSIGNED_INT, (const void*)(node->meshes[i]->indexOffset * sizeof(unsigned int)));
#endif
            }
            find_light_and_blink = 1;
            break;
        }
    }
    if (0 == find_light_and_blink)
    {
        // Draw each mesh in this node
        for (i = 0; i < node->meshes.size(); i++)
        {
            set_material_uniforms(*node->meshes[i]->material);

#if MODEL_DRAW_USE_UINT == 0
            // OpenGL ES -- unsigned long int type indexes are not supported, use unsigned short instead
            glDrawElements(GL_TRIANGLES, node->meshes[i]->indexCount,
                GL_UNSIGNED_SHORT, (const void*)(node->meshes[i]->indexOffset * sizeof(unsigned short)));
#else
            glDrawElements(GL_TRIANGLES, node->meshes[i]->indexCount,
                GL_UNSIGNED_INT, (const void*)(node->meshes[i]->indexOffset * sizeof(unsigned int)));
#endif
        }
    }

    // Recursively draw this nodes children nodes
    for (int i = 0; i < node->nodes.size(); i++)
    {
        draw_node(&(node->nodes[i]), object_matrix, door_flag);
    }
}


void ModelDraw::set_material_uniforms(MaterialInfo &mater)
{
    glUniform3f(m_loc_ka, mater.Ambient[0], mater.Ambient[1], mater.Ambient[2]);
    glUniform3f(m_loc_kd, mater.Diffuse[0], mater.Diffuse[1], mater.Diffuse[2]);
    glUniform3f(m_loc_ks, m_material_info.Specular[0], m_material_info.Specular[1], m_material_info.Specular[2]);
    glUniform1f(m_loc_shininess, mater.Shininess);

    if (mater.isTexture)
    {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, mater.textureIdx);
        glUniform1i(m_loc_sampler, 0);
        glUniform1i(m_loc_has_texture, 1);
    }
    else
    {
        glUniform1i(m_loc_has_texture, 0);
    }
}


/**
* set front wheel angle, [-45, 45] deg. default is 0
*/
int ModelDraw::set_wheel_angle(int angle)
{
    if ((angle < -45) || (angle > 45))
    {
        return 0;
    }
    if (angle == m_wheel_angle)
    {
        return 0;
    }
    m_wheel_angle = angle;

    return 0;
}


/**
* set four wheel speed. [-100, 100] km/h. default is 0
*/
int ModelDraw::set_wheel_speed(int speed)
{
    if ((speed < -100) || (speed > 100))
    {
        return 0;
    }
    if (speed == m_wheel_speed)
    {
        return 0;
    }
    m_wheel_speed = speed;

    return 0;    
}


int ModelDraw::set_light_blink(int fl, int fr, int rl, int rr)
{
    m_light_blink[0] = fl;
    m_light_blink[1] = fr;
    m_light_blink[2] = rl;
    m_light_blink[3] = rr;

    return 0;
}


int ModelDraw::set_em_flashers(int is_on)
{
    m_is_in_em_flasher = is_on;
    
    return 0; 
}


/**
* set model alpha. [0, 100], 0 is invisible
*/
int ModelDraw::set_model_alpha(int alpha)
{
    if ((alpha < 0) || (alpha > 100))
    {
        return 0;
    }
    if (alpha == m_alpha)
    {
        return 0;
    }
    m_alpha = alpha;
    m_alpha_change++;

    return 0;    
}


int ModelDraw::set_door(int is_fl_open, int is_fr_open, int is_bl_open, int is_br_open)
{
    m_door_open[0] = is_fl_open;
    m_door_open[1] = is_fr_open;
    m_door_open[2] = is_bl_open;
    m_door_open[3] = is_br_open;
    m_door_trans[0] = 1;
    m_door_trans[1] = 1;
    m_door_trans[2] = 1;
    m_door_trans[3] = 1;
    
    return 0;
}


void ModelDraw::set_fps(float fps)
{
    m_fps = fps;
    unsigned int int_fps = (unsigned int)((m_fps + 0.5f) / 2.0);
    unsigned int intv = int_fps & (~0x01u);
    m_blink_intv = intv;
}


