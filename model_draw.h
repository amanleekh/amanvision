

#ifndef _MODEL_DRAW_H_
#define _MODEL_DRAW_H_

#include "common_def.h"
#include "GLES2/gl2.h"
#include "modelloader.h"
#include <glm/mat4x4.hpp>
#include <QMutex>

// if vbo index buff is unsigned short or unsigned int
// opengl es 2.0 can only be unsigned short, but we use uint have no problem
#define MODEL_DRAW_USE_UINT     (1)


typedef struct _load_thread_struct_
{
    ModelLoader *p_handler;
    const char *p_path;
    int is_scale;
    float scale_val;
    float dst_w;
    float dst_h;
    float dst_z;
    int *p_is_loaded;
}load_thread_s;


class ModelDraw
{
public:
    // car type, 1 for 01, 2 for 02, 3 for 03
    ModelDraw(int car_type);

public:
    // compile shader program
    int create_program();

    // pre load model data
    int pre_load_model_data(const char *p_path, int is_scale, float scale_val,
        float dst_w, float dst_h, float dst_z, int is_block);

    // load car model data
    int pre_load_car_data(int is_block = 0);

    // if model have loaded
    int is_model_loaded();
    
    // load model to gen vbo
    int gen_model_vbo();

    void get_mvp(glm::mat4 *p_m, glm::mat4 *p_v, glm::mat4 *p_p)
    {
        if (p_m != NULL) *p_m = m_m;
        if (p_v != NULL) *p_v = m_v;
        if (p_p != NULL) *p_p = m_p;
    }

    void set_mvp(glm::mat4 *p_m, glm::mat4 *p_v, glm::mat4 *p_p)
    {
        if (p_m != NULL) m_m = *p_m;
        if (p_v != NULL) m_v = *p_v;
        if (p_p != NULL) m_p = *p_p;
    }

    void set_rotate(float angle);

    void set_light_pos(float x, float y, float z)
    {
        m_light_info.Position = QVector4D(x, y, z, 1.0f);
    }

    /**
    *
    */
    int render_process(int widget_idx);

    /**
    * set front wheel angle, [-45, 45] deg. default is 0
    */
    int set_wheel_angle(int angle);

    /**
    * set four wheel speed. [-100, 100] km/h. default is 0
    */
    int set_wheel_speed(int speed);

    /**
    * set light blink, 4 light
    */
    int set_light_blink(int fl, int fr, int rl, int rr);

    /**
    * set emergency flashers state
    */
    int set_em_flashers(int is_on);
    
    /**
    * set model alpha. [0, 100], 0 is invisible
    */
    int set_model_alpha(int alpha);

    /**
    * set model alpha. [0.0, 1.0], 0 is invisible
    */
    void set_window_alpha(float alpha)
    {
        m_window_alpha = alpha;
    }

    int set_door(int is_fl_open, int is_fr_open, int is_bl_open, int is_br_open);

    void set_fps(float fps);

private:
    int gen_model_vbo_inner();    
    int gen_texture();
    /**< 0, not a door, 1 front_left, 2 front right, 3 rear left, 4 rear right */
    void draw_node(const Node *node, glm::mat4 object_matrix, int door_flag);
    void set_material_uniforms(MaterialInfo &mater);
    void node_door_process(int door_idx, glm::mat4 &object_matrix, glm::mat4 &m);
    int read_wheel_string();
    int read_door_string();
    int read_light_string();
    /**< index 0,1,2,3 front-left, front-right, rear-left, rear-right. */
    int check_light_node_name(int index, QString input_name);

private:
    void *m_shader_handler;         /**< shader handler */
    GLint m_loc_vertex_position;    /**< attr and uniform location */
    GLint m_loc_vertex_normal;
	GLint m_loc_texture_coord;
    GLint m_loc_mv;
    GLint m_loc_n;
    GLint m_loc_mvp;
    GLint m_loc_light_position;
    GLint m_loc_light_intensity;
    GLint m_loc_ka;
    GLint m_loc_kd;
    GLint m_loc_ks;
    GLint m_loc_shininess;
    GLint m_loc_alpha;
	GLint m_loc_sampler;
	GLint m_loc_has_texture;

private:
    GLuint m_vertex_buffer;
    GLuint m_normal_buffer;
    GLuint m_texture_uv_buffer; /**< if model have texture */
    GLuint m_index_buffer;

private: 
    glm::mat4 m_m;  /**< model, view, proj matrix */
    glm::mat4 m_v;
    glm::mat4 m_p;
    glm::mat4 m_mvp;
    float m_y_trans;      /**< y det read from ini file */
    int m_car_type;
    LightInfo m_light_info;     /**< default light and material info */
    MaterialInfo m_material_info;
    MaterialInfo m_blink_material_info;

private:
    const char *m_p_vshader_str;
    const char *m_p_fshader_str;
    static ModelLoader *m_p_model_loader;   /**< just load once */
    int m_is_model_block_load;
    static int m_is_model_loaded;      /**< if model been loaded */
    int m_is_vbo_gened;
    int m_have_uv;  /**< if this model have uv texture. current not support uv tex */
    load_thread_s m_nonblock_thread_arg;
    char m_model_name[256];
    QSharedPointer<Node> m_model_root_node;
    static QMutex m_mutex;    /**< mutex load car */

    QString m_wheel_fl;     /**< wheel string name in model file */
    QString m_wheel_fr;
    QString m_wheel_bl;
    QString m_wheel_br;   
    int m_wheel_angle;
    int m_wheel_speed;
    glm::mat4 m_wheel_angle_mat;        /**< front wheel turn angle matrix */
    glm::mat4 m_wheel_speed_mat;        /**< whell speed matrix */
    glm::mat4 m_wheel_anglespeed_mat;   /**< angle_mat*speed_mat */

    QString m_door_str[4];     /**< door string name in model file.fl, fr, bl, br */
    int m_door_open[4];     /**< door is open or not */
    int m_door_trans[4];    /** is door transing */
    float m_window_alpha;   /**< windows alpha value. 1 not trnas */
    QString m_window_str[6];   /**< window string name. front, fl_door, fr_door, bl_door, br_door, rear */
    glm::mat4 m_door_open_mat[4];
    glm::mat4 m_door_close_mat[4];
    glm::mat4 m_door_cur_mat[4];

    int m_alpha;
    int m_alpha_change;

    int m_light_blink[4];
    int m_is_in_em_flasher;     /**< is currnet emergency flashers on or off */
    QString m_light_str[4][3];  /**< light string can have at max 3 string */
    static int m_blink_cnt;
    static float m_fps;
    static int m_blink_intv; 
private:
    static QMutex m_texture_mutex;
    static int m_texture_loadded;
};


#endif


