
#include "common_def.h"

#include <sys/time.h>
#include <malloc.h>
#include <sys/stat.h>


#include "overlay_draw.h"
#include "gpu_shader.h"
//#include "minIni.h"
#include "ini_config.h"
#include "modelloader.h"

#include <glm/mat4x4.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define BOTTOM_Z_VAL    (0.001f)
//#define ASSIST_LINE_Z_VAL   (0.002f)
//#define BLOCK_LINE_Z_VAL    (0.003f)
#define RADAR_Z_VAL     (0.004f)
#define RADAR_ALPHA_ALL (0.0f)


#define OVERLAY_DATA_INITED         (0)
#define OVERLAY_DATA_LOADDED        (1)
#define OVERLAY_VBO_LOADDED         (2)


OverlayDraw::OverlayDraw()
{
    int i;

    m_radar_m = glm::mat4(1.0f);
    m_radar_m = glm::rotate(m_radar_m, glm::radians(-90.0f), glm::vec3(1, 0, 0));
    m_radar_m = glm::rotate(m_radar_m, glm::radians(180.0f), glm::vec3(0, 0, 1));
    m_radar_v = glm::mat4(1.0f);
    m_radar_p = glm::mat4(1.0f);
    m_radar_mvp = m_radar_p * m_radar_v * m_radar_m;

    m_assist_line_m = glm::mat4(1.0f);
    m_assist_line_m = glm::rotate(m_assist_line_m, glm::radians(-90.0f), glm::vec3(1, 0, 0));
    m_assist_line_m = glm::rotate(m_assist_line_m, glm::radians(180.0f), glm::vec3(0, 0, 1));
    m_assist_line_v = glm::mat4(1.0f);
    m_assist_line_p = glm::mat4(1.0f);
    m_assist_line_mvp = m_assist_line_p * m_assist_line_v * m_assist_line_m;

    m_p_vshader_str = "avm_qt_app_res/shader/vshader_overlay.glsl";
    m_p_fshader_str = "avm_qt_app_res/shader/fshader_overlay.glsl";

    m_p_bottom_vert_arr = NULL;
    m_p_bottom_color_arr = NULL;
    m_bottom_point_num = 0;
    m_bottom_load = OVERLAY_DATA_INITED;
    m_radar_state[0] = 0xFFFFFFFF;  // FF means all radar are hided
    m_radar_state[1] = 0xFFFFFFFF;
    m_radar_change = 1;
    m_radar_2d_front_change = 1;
    m_radar_2d_rear_change = 1;
    m_p_radar_color_arr = NULL;
    m_radar_load = OVERLAY_DATA_INITED;
    m_radar_alpha = 0.4; //[csj 20180929-6]
    m_is_radar_show = 1;

    // block line
    m_block_line_pair_cnt = 0;
    m_block_line_front_vert_arr = NULL;
    m_block_line_front_num = 0;
    m_block_line_rear_vert_arr = NULL;
    m_block_line_rear_num = 0;
    m_block_line_front_color_arr = NULL;
    m_block_line_rear_color_arr = NULL;
    m_is_block_line_show = 1;
    m_block_line_load = OVERLAY_DATA_INITED;

    m_assist_line_pair_cnt = 20;
    for (i = 0; i < ASSIST_LINE_DEG_CNT; i++)
    {
        // front wheel's ahead line
        m_left_assist_line_vert_arr[i] = NULL;
        m_left_assist_line_num[i] = 0;
        m_right_assist_line_vert_arr[i] = NULL;
        m_right_assist_line_num[i] = 0;
        // front wheel's back line
        m_left_back_assist_line_vert_arr[i] = NULL;
        m_left_back_assist_line_num[i] = 0;
        m_right_back_assist_line_vert_arr[i] = NULL;
        m_right_back_assist_line_num[i] = 0;
        // front wheel's ahead line
        m_rear_wheel_left_assist_line_vert_arr[i] = NULL;
        m_rear_wheel_left_assist_line_num[i] = 0;
        m_rear_wheel_right_assist_line_vert_arr[i] = NULL;
        m_rear_wheel_right_assist_line_num[i] = 0;
        // front wheel's back line
        m_rear_wheel_left_back_assist_line_vert_arr[i] = NULL;
        m_rear_wheel_left_back_assist_line_num[i] = 0;
        m_rear_wheel_right_back_assist_line_vert_arr[i] = NULL;
        m_rear_wheel_right_back_assist_line_num[i] = 0;

        // single camera calib. front camera
        m_front_cam_left_assist_line_vert_arr[i] = NULL;
        m_front_cam_left_assist_line_num[i] = 0;
        m_front_cam_right_assist_line_vert_arr[i] = NULL;
        m_front_cam_right_assist_line_num[i] = 0;
        // single camera calib. front camera
        m_rear_cam_left_assist_line_vert_arr[i] = NULL;
        m_rear_cam_left_assist_line_num[i] = 0;
        m_rear_cam_right_assist_line_vert_arr[i] = NULL;
        m_rear_cam_right_assist_line_num[i] = 0;        
    }
    m_p_assist_line_color_arr = NULL;
    m_assist_line_load = OVERLAY_DATA_INITED;
    m_current_deg = 0;
    m_assist_dir = 0;   /**< default deiection is front */
    m_assist_change = 1;
    m_is_assist_show = 1;
}


unsigned int OverlayDraw::get_time_ms()
{
    struct timeval  tv;
    gettimeofday(&tv, NULL);

    unsigned int time_in_mill 
        = (tv.tv_sec) * 1000 + (tv.tv_usec) / 1000;
    return time_in_mill;
}


int OverlayDraw::create_program()
{
    int ret;

    // load shader program. compile and load
    ret = shader_load(m_p_vshader_str, m_p_fshader_str, &m_shader_handler);
    if (ret != 0)
    {
        AVM_ERR("OverlayDraw::create_program. shader_load_str faild.\n");
        return 1;
    }

    GLuint hProgram = shader_get_program(m_shader_handler);

    // get gles value location
    m_loc_vertex_position = glGetAttribLocation(hProgram, "vsVertex");
    m_loc_color = glGetAttribLocation(hProgram, "vsColor");
    m_loc_mvp = glGetUniformLocation(hProgram, "vsTransformMatrix");

    return 0;
}


int OverlayDraw::init_bottom_point_num(bottom_data_param_s *p_param)
{
    int i, j, k;
    int pt_idx = 0;   
    
    // left & right & top & down
    for (j = 0; j < 4; j++) 
    {
        for (i = 0; i < p_param->depth - 1; i++)
        {
            pt_idx += 6; 
        }
    }
    // left_top, right_top, bottom_left, bottom_right
    for (i = 0; i < 4; i++)
    {
        for (j = 0; j < p_param->depth - 1; j++)
        {
            for (k = 0; k < p_param->r_slice; k++)
            {
                if (j == p_param->depth - 2)
                {
                    pt_idx += 3; 
                }
                else
                {
                    pt_idx += 6; 
                }
            }
        }
    }
    // center
    pt_idx += 6;

    p_param->point_num = pt_idx;

    return 0;
}


int OverlayDraw::gen_bottom_data(bottom_data_param_s *p_param)
{
    int i, j, k;
    int pt_idx = 0;

    // left & right & top & down
    for (j = 0; j < 4; j++)
    {
        float step = p_param->r / (p_param->depth - 1);

        for (i = 0; i < p_param->depth - 1; i++)
        {
            float *p_vert = p_param->p_vert + pt_idx * 3;
            float *p_color =  p_param->p_color + pt_idx * 4;
            float p[4][2];
            // left
            if (j == 0)
            {
                p[0][0] = -1*p_param->w / 2 - p_param->r + i*step;
                p[0][1] = p_param->h / 2;
                p[1][0] = -1*p_param->w / 2 - p_param->r + i*step;
                p[1][1] = -1 * p_param->h / 2;
                p[2][0] = -1*p_param->w / 2 - p_param->r + (i+1)*step;
                p[2][1] = -1 * p_param->h / 2;
                p[3][0] = -1*p_param->w / 2 - p_param->r + (i+1)*step;
                p[3][1] = p_param->h / 2;
            }
            // right
            else if (j == 1)
            {
                p[0][0] = p_param->w / 2 + p_param->r - i*step;
                p[0][1] = -1 * p_param->h / 2;
                p[1][0] = p_param->w / 2 + p_param->r - i*step;
                p[1][1] = p_param->h / 2;
                p[2][0] = p_param->w / 2 + p_param->r - (i+1)*step;
                p[2][1] = p_param->h / 2;
                p[3][0] = p_param->w / 2 + p_param->r - (i+1)*step;
                p[3][1] = -1 * p_param->h / 2;
            }
            // top
            else if (j == 2)
            {
                p[0][0] = p_param->w / 2;
                p[0][1] = p_param->h / 2 + p_param->r - i*step;
                p[1][0] = -1 * p_param->w / 2;
                p[1][1] = p_param->h / 2 + p_param->r - i*step;
                p[2][0] = -1*p_param->w / 2;
                p[2][1] = p_param->h / 2 + p_param->r - (i+1)*step;
                p[3][0] = p_param->w / 2;
                p[3][1] = p_param->h / 2 + p_param->r - (i+1)*step;
            }
            // down
            else
            {
                p[0][0] = -1 * p_param->w / 2;
                p[0][1] = -1 * p_param->h / 2 - p_param->r + i*step;
                p[1][0] = p_param->w / 2;
                p[1][1] = -1 * p_param->h / 2 - p_param->r + i*step;
                p[2][0] = p_param->w / 2;
                p[2][1] = -1 * p_param->h / 2 - p_param->r + (i+1)*step;
                p[3][0] = -1 * p_param->w / 2;
                p[3][1] = -1 * p_param->h / 2 - p_param->r + (i+1)*step;
            }

            *p_vert++ = p[0][0];
            *p_vert++ = p[0][1] + p_param->y_det;
            *p_vert++ = p_param->z;
            *p_vert++ = p[1][0];
            *p_vert++ = p[1][1] + p_param->y_det;
            *p_vert++ = p_param->z;     
            *p_vert++ = p[2][0];
            *p_vert++ = p[2][1] + p_param->y_det;
            *p_vert++ = p_param->z; 
            *p_vert++ = p[0][0];
            *p_vert++ = p[0][1] + p_param->y_det;
            *p_vert++ = p_param->z;
            *p_vert++ = p[2][0];
            *p_vert++ = p[2][1] + p_param->y_det;   
            *p_vert++ = p_param->z; 
            *p_vert++ = p[3][0];
            *p_vert++ = p[3][1] + p_param->y_det;   
            *p_vert++ = p_param->z; 

            *p_color++ = p_param->rgb[0];
            *p_color++ = p_param->rgb[1];
            *p_color++ = p_param->rgb[2];
            *p_color++ = p_param->alpth_depth[i];
            *p_color++ = p_param->rgb[0];
            *p_color++ = p_param->rgb[1];
            *p_color++ = p_param->rgb[2];
            *p_color++ = p_param->alpth_depth[i];
            *p_color++ = p_param->rgb[0];
            *p_color++ = p_param->rgb[1];
            *p_color++ = p_param->rgb[2];
            *p_color++ = p_param->alpth_depth[i+1]; 
            *p_color++ = p_param->rgb[0];
            *p_color++ = p_param->rgb[1];
            *p_color++ = p_param->rgb[2];
            *p_color++ = p_param->alpth_depth[i];
            *p_color++ = p_param->rgb[0];
            *p_color++ = p_param->rgb[1];
            *p_color++ = p_param->rgb[2];
            *p_color++ = p_param->alpth_depth[i+1];
            *p_color++ = p_param->rgb[0];
            *p_color++ = p_param->rgb[1];
            *p_color++ = p_param->rgb[2];
            *p_color++ = p_param->alpth_depth[i+1]; 

            pt_idx += 6;        
        }
    }

    // left_top, right_top, bottom_left, bottom_right
    for (i = 0; i < 4; i++)
    {
        float step = p_param->r / (p_param->depth - 1);
        float single_deg = 3.1415927 / 2 / p_param->r_slice;

        for (j = 0; j < p_param->depth - 1; j++)
        {
            float r1 = p_param->r - step*j;
            float r2 = p_param->r - step*(j+1);

            for (k = 0; k < p_param->r_slice; k++)
            {
                float p[4][2];
                float tri_pt[2];
                // left_top
                if (i == 0)
                {
                    p[0][0] = -1 * p_param->w / 2 - r1*cos((k+1)*single_deg);
                    p[0][1] = p_param->h / 2 + r1*sin((k+1)*single_deg);
                    p[1][0] = -1 * p_param->w / 2 - r1*cos(k*single_deg);
                    p[1][1] = p_param->h / 2 + r1*sin(k*single_deg);
                    p[2][0] = -1 * p_param->w / 2 - r2*cos(k*single_deg);
                    p[2][1] = p_param->h / 2 + r2*sin(k*single_deg);
                    p[3][0] = -1 * p_param->w / 2 - r2*cos((k+1)*single_deg);
                    p[3][1] = p_param->h / 2 + r2*sin((k+1)*single_deg);
                    tri_pt[0] = -1 * p_param->w / 2;
                    tri_pt[1] = p_param->h / 2;
                }
                // right_top
                else if (i == 1)
                {
                    p[0][0] = p_param->w / 2 + r1*cos(k*single_deg);
                    p[0][1] = p_param->h / 2 + r1*sin(k*single_deg);
                    p[1][0] = p_param->w / 2 + r1*cos((k+1)*single_deg);
                    p[1][1] = p_param->h / 2 + r1*sin((k+1)*single_deg);
                    p[2][0] = p_param->w / 2 + r2*cos((k+1)*single_deg);
                    p[2][1] = p_param->h / 2 + r2*sin((k+1)*single_deg);
                    p[3][0] = p_param->w / 2 + r2*cos(k*single_deg);
                    p[3][1] = p_param->h / 2 + r2*sin(k*single_deg);    
                    tri_pt[0] = p_param->w / 2;
                    tri_pt[1] = p_param->h / 2;             
                }
                // left_bottom
                else if (i == 2)
                {
                    p[0][0] = -1 * p_param->w / 2 - r1*cos(k*single_deg);
                    p[0][1] = -1 * p_param->h / 2 - r1*sin(k*single_deg);
                    p[1][0] = -1 * p_param->w / 2 - r1*cos((k+1)*single_deg);
                    p[1][1] = -1 * p_param->h / 2 - r1*sin((k+1)*single_deg);
                    p[2][0] = -1 * p_param->w / 2 - r2*cos((k+1)*single_deg);
                    p[2][1] = -1 * p_param->h / 2 - r2*sin((k+1)*single_deg);
                    p[3][0] = -1 * p_param->w / 2 - r2*cos(k*single_deg);
                    p[3][1] = -1 * p_param->h / 2 - r2*sin(k*single_deg);
                    tri_pt[0] = -1 * p_param->w / 2;
                    tri_pt[1] = -1 * p_param->h / 2;
                }
                // right_bottom
                else
                {
                    p[0][0] = p_param->w / 2 + r1*cos((k+1)*single_deg);
                    p[0][1] = -1 * p_param->h / 2 - r1*sin((k+1)*single_deg);
                    p[1][0] = p_param->w / 2 + r1*cos(k*single_deg);
                    p[1][1] = -1 * p_param->h / 2 - r1*sin(k*single_deg);
                    p[2][0] = p_param->w / 2 + r2*cos(k*single_deg);
                    p[2][1] = -1 * p_param->h / 2 - r2*sin(k*single_deg);
                    p[3][0] = p_param->w / 2 + r2*cos((k+1)*single_deg);
                    p[3][1] = -1 * p_param->h / 2 - r2*sin((k+1)*single_deg);   
                    tri_pt[0] = p_param->w / 2;
                    tri_pt[1] = -1 * p_param->h / 2;
                }

                float *p_vert = p_param->p_vert + pt_idx * 3;
                float *p_color =  p_param->p_color + pt_idx * 4;
                if (j == p_param->depth - 2)
                {
                    *p_vert++ = p[0][0];
                    *p_vert++ = p[0][1] + p_param->y_det;
                    *p_vert++ = p_param->z;
                    *p_vert++ = p[1][0];
                    *p_vert++ = p[1][1] + p_param->y_det;
                    *p_vert++ = p_param->z;     
                    *p_vert++ = tri_pt[0];
                    *p_vert++ = tri_pt[1] + p_param->y_det;
                    *p_vert++ = p_param->z; 

                    *p_color++ = p_param->rgb[0];
                    *p_color++ = p_param->rgb[1];
                    *p_color++ = p_param->rgb[2];
                    *p_color++ = p_param->alpth_depth[j];
                    *p_color++ = p_param->rgb[0];
                    *p_color++ = p_param->rgb[1];
                    *p_color++ = p_param->rgb[2];
                    *p_color++ = p_param->alpth_depth[j];
                    *p_color++ = p_param->rgb[0];
                    *p_color++ = p_param->rgb[1];
                    *p_color++ = p_param->rgb[2];
                    *p_color++ = p_param->alpth_depth[j+1]; 

                    pt_idx += 3;                    
                }
                else
                {
                    *p_vert++ = p[0][0];
                    *p_vert++ = p[0][1] + p_param->y_det;
                    *p_vert++ = p_param->z;
                    *p_vert++ = p[1][0];
                    *p_vert++ = p[1][1] + p_param->y_det;
                    *p_vert++ = p_param->z;     
                    *p_vert++ = p[2][0];
                    *p_vert++ = p[2][1] + p_param->y_det;
                    *p_vert++ = p_param->z; 
                    *p_vert++ = p[0][0];
                    *p_vert++ = p[0][1] + p_param->y_det;
                    *p_vert++ = p_param->z;
                    *p_vert++ = p[2][0];
                    *p_vert++ = p[2][1] + p_param->y_det;   
                    *p_vert++ = p_param->z; 
                    *p_vert++ = p[3][0];
                    *p_vert++ = p[3][1] + p_param->y_det;   
                    *p_vert++ = p_param->z; 

                    *p_color++ = p_param->rgb[0];
                    *p_color++ = p_param->rgb[1];
                    *p_color++ = p_param->rgb[2];
                    *p_color++ = p_param->alpth_depth[j];       
                    *p_color++ = p_param->rgb[0];
                    *p_color++ = p_param->rgb[1];
                    *p_color++ = p_param->rgb[2];
                    *p_color++ = p_param->alpth_depth[j];       
                    *p_color++ = p_param->rgb[0];
                    *p_color++ = p_param->rgb[1];
                    *p_color++ = p_param->rgb[2];
                    *p_color++ = p_param->alpth_depth[j+1]; 
                    *p_color++ = p_param->rgb[0];
                    *p_color++ = p_param->rgb[1];
                    *p_color++ = p_param->rgb[2];
                    *p_color++ = p_param->alpth_depth[j];   
                    *p_color++ = p_param->rgb[0];
                    *p_color++ = p_param->rgb[1];
                    *p_color++ = p_param->rgb[2];
                    *p_color++ = p_param->alpth_depth[j+1];
                    *p_color++ = p_param->rgb[0];
                    *p_color++ = p_param->rgb[1];
                    *p_color++ = p_param->rgb[2];
                    *p_color++ = p_param->alpth_depth[j+1];  

                    pt_idx += 6;                        
                }
            }
        }
    }

    // center
    float *p_vert = p_param->p_vert + pt_idx * 3;
    float *p_color =  p_param->p_color + pt_idx * 4;
    *p_vert++ = -1 * p_param->w / 2;
    *p_vert++ = p_param->h / 2 + p_param->y_det;
    *p_vert++ = p_param->z; 
    *p_vert++ = -1 * p_param->w / 2;
    *p_vert++ = -1 * p_param->h / 2 + p_param->y_det;
    *p_vert++ = p_param->z; 
    *p_vert++ = p_param->w / 2;
    *p_vert++ = -1 * p_param->h / 2 + p_param->y_det;
    *p_vert++ = p_param->z; 
    *p_vert++ = -1 * p_param->w / 2;
    *p_vert++ = p_param->h / 2 + p_param->y_det;
    *p_vert++ = p_param->z; 
    *p_vert++ = p_param->w / 2;
    *p_vert++ = -1 * p_param->h / 2 + p_param->y_det;
    *p_vert++ = p_param->z; 
    *p_vert++ = p_param->w / 2;
    *p_vert++ = p_param->h / 2 + p_param->y_det;
    *p_vert++ = p_param->z;     

    for (i = 0; i < 6; i++)
    {
        *p_color++ = p_param->rgb[0];
        *p_color++ = p_param->rgb[1];
        *p_color++ = p_param->rgb[2];
        *p_color++ = p_param->alpth_depth[p_param->depth-1];
    }   
    pt_idx += 6;
    
    if (p_param->point_num != pt_idx)
    {
        return 1;
    }

    return 0;   
}


int OverlayDraw::pre_load_bottom_data()
{
    bottom_data_param_s param;
    param.w = ini_get_bottom_w() - 0.2f;
    param.h = ini_get_bottom_h() - 0.2f;
    param.z = BOTTOM_Z_VAL;
    param.y_det = ini_get_bottom_det();
    param.r = 0.2f;
    param.r_slice = 4;
    param.depth = 3;
    param.rgb[0] = 0.313f;
    param.rgb[1] = 0.313f;
    param.rgb[2] = 0.313f;
    param.alpth_depth[0] = 0.0f;
    param.alpth_depth[1] = 1.0f;
    param.alpth_depth[2] = 1.0f;
    param.p_vert = NULL;
    param.p_color = NULL;
    init_bottom_point_num(&param);
    m_p_bottom_vert_arr = (float *)malloc(param.point_num * 3 * sizeof(float));
    m_p_bottom_color_arr = (float *)malloc(param.point_num * 4 * sizeof(float));
    if ((NULL == m_p_bottom_vert_arr) || (NULL == m_p_bottom_color_arr))
    {
        AVM_ERR("OverlayDraw::pre_load_bottom_data. alloc mem faild\n");
        return 1;
    }
    param.p_vert = m_p_bottom_vert_arr;
    param.p_color = m_p_bottom_color_arr;
    if (gen_bottom_data(&param) != 0)
    {
        AVM_ERR("OverlayDraw::pre_load_bottom_data. gen data faild\n");
        return 1;        
    }
    m_p_bottom_vert_arr = (float *)param.p_vert;
    m_p_bottom_color_arr = (float *)param.p_color;
    m_bottom_point_num = param.point_num;
    m_bottom_load = OVERLAY_DATA_LOADDED;

    return 0;    
}


int OverlayDraw::gen_bottom_vbo()
{
#if 0
    if (m_bottom_load != OVERLAY_DATA_LOADDED)
    {
        AVM_ERR("OverlayDraw::gen_bottom_vbo. bottom data not loadded\n");
        return 1;
    }

    // gen vert vbo buffer
    int float_num = m_p_bottom_loader.total_point_num * m_p_bottom_loader.point_dim;
    float *p_vert_arr = m_p_bottom_loader.p_idv_obj_arr[0].p_vert_arr;
    glGenBuffers(1, &m_bottom_vert_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, m_bottom_vert_buffer);
    glBufferData(GL_ARRAY_BUFFER, float_num * sizeof(GLfloat), p_vert_arr, GL_STATIC_DRAW);
    
    // gen color vbo buffer
    int color_float_num = m_p_bottom_loader.total_point_num * 4;
    glGenBuffers(1, &m_bottom_color_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, m_bottom_color_buffer);
    glBufferData(GL_ARRAY_BUFFER, color_float_num * sizeof(GLfloat), m_p_bottom_color_arr, GL_STATIC_DRAW);
         
    // free vert array and color array
    free(m_p_bottom_loader.p_idv_obj_arr[0].p_vert_arr);
    free(m_p_bottom_loader.p_idv_obj_arr);
    free(m_p_bottom_color_arr);

    m_bottom_load = OVERLAY_VBO_LOADDED;
#endif
    if (m_bottom_load != OVERLAY_DATA_LOADDED)
    {
        AVM_ERR("OverlayDraw::gen_bottom_vbo. bottom data not loadded\n");
        return 1;
    } 
    glGenBuffers(1, &m_bottom_vert_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, m_bottom_vert_buffer);
    glBufferData(GL_ARRAY_BUFFER, m_bottom_point_num * 3 * sizeof(GLfloat), 
        m_p_bottom_vert_arr, GL_STATIC_DRAW);

    glGenBuffers(1, &m_bottom_color_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, m_bottom_color_buffer);
    glBufferData(GL_ARRAY_BUFFER, m_bottom_point_num * 4 * sizeof(GLfloat), 
        m_p_bottom_color_arr, GL_STATIC_DRAW);
   
    m_bottom_load = OVERLAY_VBO_LOADDED;
    return 0;
}


int OverlayDraw::bottom_render_process()
{
    if (m_bottom_load != OVERLAY_VBO_LOADDED)
    {
        AVM_ERR("OverlayDraw::bottom_render_process. data not load or vbo not gen\n");
        return 1;
    }

    // vertex buffer
    glBindBuffer(GL_ARRAY_BUFFER, m_bottom_vert_buffer);
    glVertexAttribPointer(m_loc_vertex_position, 3, GL_FLOAT, GL_FALSE,
        3 * sizeof(float), (GLvoid *)(0 * sizeof(float)));    

    // color buffer
    glBindBuffer(GL_ARRAY_BUFFER, m_bottom_color_buffer);
    glVertexAttribPointer(m_loc_color, 4, GL_FLOAT, GL_FALSE,
        4 * sizeof(float), (GLvoid *)(0 * sizeof(float)));    

    glEnableVertexAttribArray(m_loc_vertex_position);
    glEnableVertexAttribArray(m_loc_color);

    glm::mat4 mvp = m_radar_p * m_radar_v * m_radar_m;
    glUniformMatrix4fv(m_loc_mvp, 1, GL_FALSE, glm::value_ptr(mvp));

    //glDrawArrays(GL_TRIANGLES, 0, m_p_bottom_loader.total_point_num);
    glDrawArrays(GL_TRIANGLES, 0, m_bottom_point_num);
    //glDrawArrays(GL_LINES, 0, m_bottom_point_num);

    return 0;
}


int OverlayDraw::pre_load_radar_data()
{
    int ret;
    
    // get model file name
    char str_name[256];
    if (ini_get_radar_name(str_name, sizeof(str_name)) != 0)
    {
        AVM_ERR("OverlayDraw::pre_load_radar_data. read config file faild\n");
        return 1;
    }

    unsigned int time_start = get_time_ms();

    // load model
    ret = load_obj_file(str_name, 3, &m_p_radar_loader);
    if (ret != 0)
    {
        AVM_ERR("OverlayDraw::pre_load_radar_data. load_obj_file faild\n");
        return 1;
    }

    // scale model
    float radar_w = ini_get_radar_w();
    float radar_h = ini_get_radar_h();
    float det_y = ini_get_radar_det();;
    if ((radar_w <= 0.0f) || (radar_h <= 0.0f))
    {
        trans_obj(&m_p_radar_loader, -1.0f, -1.0f, -1.0f, det_y, RADAR_Z_VAL);
    }
    else
    {
        AVM_LOG("OverlayDraw::pre_load_radar_data. read ini file. dst_w=%.2f, dst_h=.2f\n",
            radar_w, radar_h);
        AVM_LOG("OverlayDraw::pre_load_radar_data. det=%.2f\n", det_y);
        trans_obj(&m_p_radar_loader, -1.0f, radar_w, radar_h, det_y, RADAR_Z_VAL);
    }

    // alloc color buffer and set color
    int i, j;
    int color_float_num = m_p_radar_loader.total_point_num * 4;
    m_p_radar_color_arr = (float *)malloc(color_float_num * sizeof(float));
    if (NULL == m_p_radar_color_arr)
    {
        AVM_ERR("OverlayDraw::gen_radar_vbo. alloc mem faild\n");
        return 1;
    }
    float *p_start = m_p_radar_color_arr;
    float color_white[4] = {0.99f, 0.99f, 0.99f, RADAR_ALPHA_ALL};
    float color_red[4] = {0.93f, 0.12f, 0.14f, RADAR_ALPHA_ALL};
    float color_yellow[4] = {0.96f, 0.71f, 0.10f, RADAR_ALPHA_ALL};
    for (i = 0; i < m_p_radar_loader.idv_obj_num; i++)
    {
        char name[256] = {'\0'};
        char *p_src_name = m_p_radar_loader.p_idv_obj_arr[i].name;
        int str_len = strlen(p_src_name);
        memcpy(name, p_src_name, str_len);

        // name have format "alert%c%d_path%c%d"
        char lvl;
        sscanf(name, "alert%c", &lvl);
        lvl -= '0';

        if (0 == lvl)
        {
            for (j = 0; j < m_p_radar_loader.p_idv_obj_arr[i].vert_num; j++)
            {
                *p_start++ = color_red[0];
                *p_start++ = color_red[1];
                *p_start++ = color_red[2];
                *p_start++ = color_red[3];
            }  
        }
        else if (1 == lvl)
        {
            for (j = 0; j < m_p_radar_loader.p_idv_obj_arr[i].vert_num; j++)
            {
                *p_start++ = color_yellow[0];
                *p_start++ = color_yellow[1];
                *p_start++ = color_yellow[2];
                *p_start++ = color_yellow[3];
            }  
        }
        else
        {
            for (j = 0; j < m_p_radar_loader.p_idv_obj_arr[i].vert_num; j++)
            {
                *p_start++ = color_white[0];
                *p_start++ = color_white[1];
                *p_start++ = color_white[2];
                *p_start++ = color_white[3];
            }              
        }
    }

    unsigned int time_end = get_time_ms();
    int det_time = time_end - time_start;
    AVM_LOG("OverlayDraw::pre_load_radar_data. file '%s' cost %d ms\n", str_name, det_time);
    m_radar_load = OVERLAY_DATA_LOADDED;

    return 0;
}


int OverlayDraw::gen_radar_vbo()
{
    if (m_radar_load != OVERLAY_DATA_LOADDED)
    {
        AVM_ERR("OverlayDraw::gen_radar_vbo. radar data not loadded\n");
        return 1;
    }
    
    // gen vert vbo buffer
    int float_num = m_p_radar_loader.total_point_num * m_p_radar_loader.point_dim;
    float *p_vert_arr = m_p_radar_loader.p_idv_obj_arr[0].p_vert_arr;
    glGenBuffers(1, &m_radar_vert_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, m_radar_vert_buffer);
    glBufferData(GL_ARRAY_BUFFER, float_num * sizeof(GLfloat), p_vert_arr, GL_STATIC_DRAW);
    
    // gen color vbo buffer
    int color_float_num = m_p_radar_loader.total_point_num * 4;
    glGenBuffers(1, &m_radar_color_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, m_radar_color_buffer);
    glBufferData(GL_ARRAY_BUFFER, color_float_num * sizeof(GLfloat), m_p_radar_color_arr, GL_DYNAMIC_DRAW);
       
    // free vert array and color array
    //free(m_p_radar_loader.p_idv_obj_arr[0].p_vert_arr);
    //free(m_p_radar_loader.p_idv_obj_arr);
    //free(m_p_radar_color_arr);

    m_radar_load = OVERLAY_VBO_LOADDED;
    
    return 0;
}


void OverlayDraw::set_radar_mvp(glm::mat4 *p_m, glm::mat4 *p_v, glm::mat4 *p_p)
{
    if (p_m != NULL)
    {
        m_radar_m = *p_m;
    }
    if (p_v != NULL)
    {
        m_radar_v = *p_v;
    }
    if (p_p != NULL)
    {
        m_radar_p = *p_p;
    }
}


void OverlayDraw::set_radar_rotate(float angle)
{
    m_radar_m = glm::mat4(1.0f);
    m_radar_m = glm::rotate(m_radar_m, glm::radians(angle), glm::vec3(0, 1, 0));
    m_radar_m = glm::rotate(m_radar_m, glm::radians(-90.0f), glm::vec3(1, 0, 0));
    m_radar_m = glm::rotate(m_radar_m, glm::radians(180.0f), glm::vec3(0, 0, 1));
}


int OverlayDraw::radar_render_process()
{
    if (m_radar_load != OVERLAY_VBO_LOADDED)
    {
        AVM_ERR("OverlayDraw::radar_render_process. data not load or vbo not gen\n");
        return 1;
    }
    if (0 == m_is_radar_show)
    {
        return 0;
    }

    // vertex buffer
    glBindBuffer(GL_ARRAY_BUFFER, m_radar_vert_buffer);
    glVertexAttribPointer(m_loc_vertex_position, 3, GL_FLOAT, GL_FALSE,
        3 * sizeof(float), (GLvoid *)(0 * sizeof(float)));    

    // color buffer
    glBindBuffer(GL_ARRAY_BUFFER, m_radar_color_buffer);
    // if radar data update, change dyn data
    if (m_radar_change)
    {
        m_radar_change = 0;
        glBufferSubData(GL_ARRAY_BUFFER, 0,
            m_p_radar_loader.total_point_num * 4 * sizeof(GLfloat), m_p_radar_color_arr);
    }
    glVertexAttribPointer(m_loc_color, 4, GL_FLOAT, GL_FALSE,
        4 * sizeof(float), (GLvoid *)(0 * sizeof(float)));    

    glEnableVertexAttribArray(m_loc_vertex_position);
    glEnableVertexAttribArray(m_loc_color);

    glm::mat4 mvp = m_radar_p * m_radar_v * m_radar_m;
    glUniformMatrix4fv(m_loc_mvp, 1, GL_FALSE, glm::value_ptr(mvp));

    glDrawArrays(GL_TRIANGLES, 0, m_p_radar_loader.total_point_num);

    return 0;
}


void OverlayDraw::radar_set_alert(unsigned int mask, unsigned int mask2, int *p_chg)
{
    int v, i, j, k;

    if (p_chg != NULL)
    {
        *p_chg = 0;
    }
    if ((mask == m_radar_state[0]) && (mask2 == m_radar_state[1]))
    {
        return;
    }
    if (p_chg != NULL)
    {
        *p_chg = 1;
    }    

    // mask
    // v for mask and mask2
    // j for mask's id 0-6
    int lst_mask_arr[12];
    int cur_mask_arr[12];
    for (v = 0; v < 2; v++)
    {
        for (i = 0; i < 6; i++)
        {   
            unsigned int lst_mask;
            unsigned int cur_mask;
            if (0 == v)
            {
                lst_mask = (m_radar_state[0] >> (4*i)) & 0x0F;
                cur_mask = (mask >> (4*i)) & 0x0F;
                lst_mask_arr[i] = lst_mask;
                cur_mask_arr[i] = cur_mask;
            }
            else
            {
                lst_mask = (m_radar_state[1] >> (4*i)) & 0x0F;
                cur_mask = (mask2 >> (4*i)) & 0x0F;
                lst_mask_arr[i+6] = lst_mask;
                cur_mask_arr[i+6] = cur_mask;   
            }
        }
    }

    // 3d view vert change
    for (i = 0; i < 12; i++)
    {
        if (lst_mask_arr[i] == cur_mask_arr[i])
        {
            continue;
        }
        // name have format "alert%d%d_path%d%d"
        // mask for level, i for id
        // if mask is 0x0F, means all radar is hided
        char lst_name[256];
        char cur_name[256];
        sprintf(lst_name, "alert%d%d_path%d%d", lst_mask_arr[i], i, lst_mask_arr[i], i);
        sprintf(cur_name, "alert%d%d_path%d%d", cur_mask_arr[i], i, cur_mask_arr[i], i);
        
        int vert_det = 0;
        for (j = 0; j < m_p_radar_loader.idv_obj_num; j++)
        {
            int cur_vert_num = m_p_radar_loader.p_idv_obj_arr[j].vert_num;
            int cmp0 = strcmp(lst_name, m_p_radar_loader.p_idv_obj_arr[j].name);
            int cmp1 = strcmp(cur_name, m_p_radar_loader.p_idv_obj_arr[j].name);

            if ((cmp0 != 0) && (cmp1 != 0))
            {
                vert_det += cur_vert_num;
                continue;
            }
            else if (0 == cmp0)
            {
                float *p_start = m_p_radar_color_arr + vert_det * 4;
                vert_det += cur_vert_num;
                for (k = 0; k < m_p_radar_loader.p_idv_obj_arr[j].vert_num; k++)
                {
                    *(p_start + k*4 + 3) = RADAR_ALPHA_ALL;
                }  
            }
            else
            {
                float *p_start = m_p_radar_color_arr + vert_det * 4;
                vert_det += cur_vert_num;
                for (k = 0; k < m_p_radar_loader.p_idv_obj_arr[j].vert_num; k++)
                {
                    *(p_start + k*4 + 3) = m_radar_alpha;
                } 
            }
        }        
    }

    // 2d view color buffer change
    float color_white[4] = {0.99f, 0.99f, 0.99f, 0.4f};
    float color_red[4] = {0.93f, 0.12f, 0.14f, 0.4f};
    float color_yellow[4] = {0.96f, 0.71f, 0.10f, 0.4f};
    float color_cur[4];
    int front_2d_iddex[4] = {1, 0, 11, 10};
    int rear_2d_iddex[4] = {4, 5, 6, 7};
    for (i = 0; i < 8; i++)
    {
        int idx;
        if (i < 4)
        {
            idx = front_2d_iddex[i];
        }
        else
        {
            idx = rear_2d_iddex[i-4];
        }
        if (lst_mask_arr[idx] == cur_mask_arr[idx])
        {
            continue;
        }
        if (0 == cur_mask_arr[idx])
        {
            color_cur[0] = color_red[0];
            color_cur[1] = color_red[1];
            color_cur[2] = color_red[2];
            color_cur[3] = color_red[3];
        }
        else if (1 == cur_mask_arr[idx])
        {
            color_cur[0] = color_yellow[0];
            color_cur[1] = color_yellow[1];
            color_cur[2] = color_yellow[2];
            color_cur[3] = color_yellow[3];            
        }
        else
        {
            color_cur[0] = color_white[0];
            color_cur[1] = color_white[1];
            color_cur[2] = color_white[2];
            color_cur[3] = color_white[3];              
        }
        int single_block_pair = m_block_line_pair_cnt / 4;
        float *p_start;
        if (i < 4)
        {
            p_start = m_block_line_front_color_arr 
                + single_block_pair * i * 2 * 4;
        }
        else
        {
            p_start = m_block_line_rear_color_arr
                + single_block_pair * (i-4) * 2 * 4;
        }
        for (j = 0; j < single_block_pair*2; j++)
        {
            *p_start++ = color_cur[0];
            *p_start++ = color_cur[1];
            *p_start++ = color_cur[2];
            *p_start++ = color_cur[3];
        }
    }

    m_radar_state[0] = mask;
    m_radar_state[1] = mask2;
    m_radar_change++;
    m_radar_2d_front_change++;
    m_radar_2d_rear_change++;
}


// this func is deprected
void OverlayDraw::radar_clr_alert(unsigned int mask, unsigned int mask2, int *p_chg)
{
    mask = mask;
    mask2 = mask2;
    if (p_chg != NULL)
    {
        p_chg = p_chg;
    }
}


void OverlayDraw::radar_show(int is_show)
{
    if (is_show == m_is_radar_show)
    {
        return;
    }

#if 0 
    int i, j, k;
    float alpha;

    if (is_show)
    {
        alpha = m_radar_alpha;
    }
    else
    {
        alpha = 0.0f;
    }
    
    for (i = 0; i < 12; i++)
    {
        char name[256];
        sprintf(name, "alert%d_Plane%d", i, i);

        int vert_det = 0;
        for (j = 0; j < m_p_radar_loader.idv_obj_num; j++)
        {
            int cur_vert_num = m_p_radar_loader.p_idv_obj_arr[j].vert_num;
            if (strcmp(name, m_p_radar_loader.p_idv_obj_arr[j].name) != 0)
            {
                vert_det += cur_vert_num;
                continue;
            }
            float *p_start = m_p_radar_color_arr + vert_det * 4;
            vert_det += cur_vert_num;
            for (k = 0; k < m_p_radar_loader.p_idv_obj_arr[j].vert_num; k++)
            {
                *p_start++ = 1.0f;
                *p_start++ = 1.0f;
                *p_start++ = 1.0f;
                *p_start++ = alpha;
            }
        }
    }
    m_radar_change++;
#endif

    if (is_show)
    {
        m_radar_change++;
        m_radar_2d_front_change++;
        m_radar_2d_rear_change++;
    }
    m_is_radar_show = is_show;
}


int OverlayDraw::pre_load_block_line_front_data()
{
    char str_name[256];

    // front block line
    if (ini_get_block_line_front_name(str_name, sizeof(str_name)) != 0)
    {
        AVM_ERR("OverlayDraw::pre_load_block_line_front_data. get assist name faild\n");
        return 1;
    }
    
    // alloc mem
    struct stat statbuff;
    if (stat(str_name, &statbuff) < 0)
    {
        AVM_ERR("OverlayDraw::pre_load_block_line_front_data. stat file faild\n");
        return 1;
    }
    unsigned long filesize = statbuff.st_size;
    unsigned char *p_blk_mem = (unsigned char *)memalign(128, filesize);
    if (NULL == p_blk_mem)
    {
        AVM_ERR("OverlayDraw::pre_load_block_line_front_data. alloc mem faild\n");
        return 1;
    }
    
    // read file and copy data to mem
    FILE *fp = NULL;
    fp = fopen(str_name, "rb");
    if (NULL == fp)
    {
        AVM_ERR("OverlayDraw::pre_load_block_line_front_data. open db file faild\n");
        return 1;
    }
    fread(p_blk_mem, filesize, 1, fp);
    fclose(fp);

    block_line_s *p_line = (block_line_s *)p_blk_mem;
    unsigned char *p_arr = (unsigned char *)p_line + p_line->point_arr_det;
    m_block_line_pair_cnt = p_line->pair_num;
    m_block_line_front_vert_arr = (float *)malloc(6*sizeof(float)*p_line->act_pair_num);
    if (NULL == m_block_line_front_vert_arr)
    {
        AVM_ERR("OverlayDraw::pre_load_block_line_front_data. alloc vert\n");
        return 1;
    }
    memcpy(m_block_line_front_vert_arr, p_arr, p_line->act_pair_num*sizeof(float)*6);
    m_block_line_front_num = p_line->act_pair_num;

    // set color
    m_block_line_front_color_arr = (float *)malloc(8*sizeof(float)*m_block_line_pair_cnt);
    if (NULL == m_block_line_front_color_arr)
    {
        AVM_ERR("OverlayDraw::pre_load_block_line_front_data. alloc front color vert\n");
        return 1; 
    }
    for (int i = 0; i < m_block_line_pair_cnt*2; i++)
    {
        m_block_line_front_color_arr[i*4+0] = p_line->color[0];
        m_block_line_front_color_arr[i*4+1] = p_line->color[1];
        m_block_line_front_color_arr[i*4+2] = p_line->color[2];
        m_block_line_front_color_arr[i*4+3] = p_line->color[3];
    }

    free(p_blk_mem);
    return 0;
}


int OverlayDraw::pre_load_block_line_rear_data()
{
    char str_name[256];

    // front block line
    if (ini_get_block_line_rear_name(str_name, sizeof(str_name)) != 0)
    {
        AVM_ERR("OverlayDraw::pre_load_block_line_rear_data. get assist name faild\n");
        return 1;
    }
    
    // alloc mem
    struct stat statbuff;
    if (stat(str_name, &statbuff) < 0)
    {
        AVM_ERR("OverlayDraw::pre_load_block_line_rear_data. stat file faild\n");
        return 1;
    }
    unsigned long filesize = statbuff.st_size;
    unsigned char *p_blk_mem = (unsigned char *)memalign(128, filesize);
    if (NULL == p_blk_mem)
    {
        AVM_ERR("OverlayDraw::pre_load_block_line_rear_data. alloc mem faild\n");
        return 1;
    }
    
    // read file and copy data to mem
    FILE *fp = NULL;
    fp = fopen(str_name, "rb");
    if (NULL == fp)
    {
        AVM_ERR("OverlayDraw::pre_load_block_line_rear_data. open db file faild\n");
        return 1;
    }
    fread(p_blk_mem, filesize, 1, fp);
    fclose(fp);

    block_line_s *p_line = (block_line_s *)p_blk_mem;
    unsigned char *p_arr = (unsigned char *)p_line + p_line->point_arr_det;
    m_block_line_pair_cnt = p_line->pair_num;
    m_block_line_rear_vert_arr = (float *)malloc(6*sizeof(float)*p_line->act_pair_num);
    if (NULL == m_block_line_rear_vert_arr)
    {
        AVM_ERR("OverlayDraw::pre_load_block_line_rear_data. alloc vert\n");
        return 1;
    }
    memcpy(m_block_line_rear_vert_arr, p_arr, p_line->act_pair_num*sizeof(float)*6);
    m_block_line_rear_num = p_line->act_pair_num;

    // set color
    m_block_line_rear_color_arr = (float *)malloc(8*sizeof(float)*m_block_line_pair_cnt);
    if (NULL == m_block_line_rear_color_arr)
    {
        AVM_ERR("OverlayDraw::pre_load_block_line_rear_data. alloc rear color vert\n");
        return 1; 
    }
    for (int i = 0; i < m_block_line_pair_cnt*2; i++)
    {
        m_block_line_rear_color_arr[i*4+0] = p_line->color[0];
        m_block_line_rear_color_arr[i*4+1] = p_line->color[1];
        m_block_line_rear_color_arr[i*4+2] = p_line->color[2];
        m_block_line_rear_color_arr[i*4+3] = p_line->color[3];
    }

    free(p_blk_mem);
    return 0;

}

int OverlayDraw::pre_load_block_line_data()
{
    int re = 0;
    
    re = pre_load_block_line_front_data();
    if (re != 0)
    {
        AVM_ERR("OverlayDraw::pre_load_block_line_front_data faild\n");
        return 1;         
    }
    re = pre_load_block_line_rear_data();
    if (re != 0)
    {
        AVM_ERR("OverlayDraw::pre_load_block_line_rear_data faild\n");
        return 1;         
    }
    m_block_line_load = OVERLAY_DATA_LOADDED;

    return 0;
}


int OverlayDraw::gen_block_line_vbo()
{
    if (m_block_line_load != OVERLAY_DATA_LOADDED)
    {
        AVM_ERR("OverlayDraw::gen_block_line_vbo, data not loadded\n");
        return 1;  
    }

    int vert_size;
    int color_size;

    vert_size = m_block_line_front_num*6*sizeof(float);
    glGenBuffers(1, &m_block_line_front_vert_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, m_block_line_front_vert_buffer);
    glBufferData(GL_ARRAY_BUFFER, vert_size, m_block_line_front_vert_arr, GL_STATIC_DRAW);    

    vert_size = m_block_line_rear_num*6*sizeof(float);
    glGenBuffers(1, &m_block_line_rear_vert_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, m_block_line_rear_vert_buffer);
    glBufferData(GL_ARRAY_BUFFER, vert_size, m_block_line_rear_vert_arr, GL_STATIC_DRAW);    

    color_size = m_block_line_pair_cnt*8*sizeof(float);
    glGenBuffers(1, &m_block_line_front_color_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, m_block_line_front_color_buffer);
    glBufferData(GL_ARRAY_BUFFER, color_size, m_block_line_front_color_arr, GL_DYNAMIC_DRAW);

    color_size = m_block_line_pair_cnt*8*sizeof(float);
    glGenBuffers(1, &m_block_line_rear_color_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, m_block_line_rear_color_buffer);
    glBufferData(GL_ARRAY_BUFFER, color_size, m_block_line_rear_color_arr, GL_DYNAMIC_DRAW);

    m_block_line_load = OVERLAY_VBO_LOADDED;
    return 0;
}


// 0 for 3d stitch space, 1 for front, 2 for rear camera
int OverlayDraw::block_line_process(int space)
{
    if (0 == m_is_block_line_show)
    {
        return 0;
    }
    if (m_block_line_load != OVERLAY_VBO_LOADDED)
    {
        AVM_ERR("OverlayDraw::block_line_process. data not load or vbo not gen\n");
        return 1;
    }

    // in 2d space, we do not use module matrix for rotate, and not use view matrix
    glm::mat4 mvp = m_assist_line_p; // * m_assist_line_v * m_assist_line_m;
    glUniformMatrix4fv(m_loc_mvp, 1, GL_FALSE, glm::value_ptr(mvp));

    if (1 == space)
    {
        // color buffer
        glBindBuffer(GL_ARRAY_BUFFER, m_block_line_front_color_buffer);
        // if radar data update, change dyn data
        if (m_radar_2d_front_change)
        {
            m_radar_2d_front_change = 0;
            glBufferSubData(GL_ARRAY_BUFFER, 0,
                m_block_line_pair_cnt * 8 * sizeof(GLfloat), m_block_line_front_color_arr);
        }
        glVertexAttribPointer(m_loc_color, 4, GL_FLOAT, GL_FALSE,
            4 * sizeof(float), (GLvoid *)(0 * sizeof(float)));
        // left vertex buffer
        glBindBuffer(GL_ARRAY_BUFFER, m_block_line_front_vert_buffer);
        glVertexAttribPointer(m_loc_vertex_position, 3, GL_FLOAT, GL_FALSE,
            3 * sizeof(float), (GLvoid *)(0 * sizeof(float)));

        glDrawArrays(GL_TRIANGLES, 0, m_block_line_front_num *2);
    }
    else if (2 == space)
    {
        // color buffer
        glBindBuffer(GL_ARRAY_BUFFER, m_block_line_rear_color_buffer);
        // if radar data update, change dyn data
        if (m_radar_2d_rear_change)
        {
            m_radar_2d_rear_change = 0;
            glBufferSubData(GL_ARRAY_BUFFER, 0,
                m_block_line_pair_cnt * 8 * sizeof(GLfloat), m_block_line_rear_color_arr);
        }
        glVertexAttribPointer(m_loc_color, 4, GL_FLOAT, GL_FALSE,
            4 * sizeof(float), (GLvoid *)(0 * sizeof(float)));
        // left vertex buffer
        glBindBuffer(GL_ARRAY_BUFFER, m_block_line_rear_vert_buffer);
        glVertexAttribPointer(m_loc_vertex_position, 3, GL_FLOAT, GL_FALSE,
            3 * sizeof(float), (GLvoid *)(0 * sizeof(float)));

        glDrawArrays(GL_TRIANGLES, 0, m_block_line_rear_num *2);
    }

    return 0;
}


int OverlayDraw::block_line_show(int is_show)
{
    m_is_block_line_show = is_show;
    return 0;
}


// front wheel's ahead and back line
int OverlayDraw::pre_gen_assist_line_3d_data()
{
    char str_name[256];
    if (ini_get_assist_line_name(str_name, sizeof(str_name)) != 0)
    {
        AVM_ERR("OverlayDraw::pre_gen_assist_line_3d_data. get assist name faild\n");
        return 1;
    }

    // alloc mem
    struct stat statbuff;
    if (stat(str_name, &statbuff) < 0)
    {
        AVM_ERR("OverlayDraw::pre_gen_assist_line_3d_data. stat file faild\n");
        return 1;
    }
    unsigned long filesize = statbuff.st_size;
    unsigned char *p_blk_mem = (unsigned char *)memalign(128, filesize);
    if (NULL == p_blk_mem)
    {
        AVM_ERR("OverlayDraw::pre_gen_assist_line_3d_data. alloc mem faild\n");
        return 1;
    }
    
    // read file and copy data to mem
    FILE *fp = NULL;
    fp = fopen(str_name, "rb");
    if (NULL == fp)
    {
        AVM_ERR("OverlayDraw::pre_gen_assist_line_3d_data. open db file faild\n");
        return 1;
    }
    fread(p_blk_mem, filesize, 1, fp);
    fclose(fp);

    // alloc vert mem and color mem, and copy db file's data to this mem
    assist_line_db_header_s *p_header = (assist_line_db_header_s *)p_blk_mem;
    m_assist_line_pair_cnt = p_header->pair_num;
    int alloc_size = m_assist_line_pair_cnt * 2 * 3 * sizeof(float);    // pair*2(x,y,z)
    
    for (int deg = ASSIST_LINE_MIN_DEG; deg <= ASSIST_LINE_MAX_DEG; deg++)
    {
        // front wheel's ahead line. left
        m_left_assist_line_vert_arr[deg+ASSIST_LINE_MAX_DEG] = (float *)malloc(alloc_size);
        if (NULL == m_left_assist_line_vert_arr[deg+ASSIST_LINE_MAX_DEG])
        {
            AVM_ERR("OverlayDraw::pre_gen_assist_line_3d_data. alloc left %d ang vert mem faild\n", deg);
            return 1;
        }
        float *p_left_dst = m_left_assist_line_vert_arr[deg+ASSIST_LINE_MAX_DEG];
        float *p_left_src = (float *)((unsigned char *)p_blk_mem 
            + p_header->left_line_arr_det[deg+ASSIST_LINE_MAX_DEG]);
        float left_pair_size = p_header->left_line_num[deg+ASSIST_LINE_MAX_DEG];
        memcpy(p_left_dst, p_left_src, left_pair_size * 2 * 3 * sizeof(float));
        m_left_assist_line_num[deg+ASSIST_LINE_MAX_DEG] = left_pair_size;

        // front wheel's meter line. left
        m_left_meter_vert_arr[deg+ASSIST_LINE_MAX_DEG] = (float *)malloc(p_header->meter_num*p_header->each_meter_block_pt_num*7*sizeof(float));
        if (NULL == m_left_meter_vert_arr[deg+ASSIST_LINE_MAX_DEG])
        {
            AVM_ERR("OverlayDraw::pre_gen_assist_line_3d_data. alloc left %d ang meter mem faild\n", deg);
            return 1;            
        }
        float *p_left_meter_dst = m_left_meter_vert_arr[deg+ASSIST_LINE_MAX_DEG];
        float *p_left_meter_src = (float *)((unsigned char *)p_blk_mem 
            + p_header->left_meter_arr_det[deg+ASSIST_LINE_MAX_DEG]);
        memcpy(p_left_meter_dst, p_left_meter_src, 
            p_header->meter_num*p_header->each_meter_block_pt_num*7*sizeof(float));
        m_left_meter_num[deg+ASSIST_LINE_MAX_DEG] = p_header->left_meter_num[deg+ASSIST_LINE_MAX_DEG];        

        // front wheel's ahead line. right
        m_right_assist_line_vert_arr[deg+ASSIST_LINE_MAX_DEG] = (float *)malloc(alloc_size);
        if (NULL == m_right_assist_line_vert_arr[deg+ASSIST_LINE_MAX_DEG])
        {
            AVM_ERR("OverlayDraw::pre_gen_assist_line_3d_data. alloc right %d ang vert mem faild\n", deg);
            return 1;
        }
        float *p_right_dst = m_right_assist_line_vert_arr[deg+ASSIST_LINE_MAX_DEG];
        float *p_right_src = (float *)((unsigned char *)p_blk_mem 
            + p_header->right_line_arr_det[deg+ASSIST_LINE_MAX_DEG]);
        float right_pair_size = p_header->right_line_num[deg+ASSIST_LINE_MAX_DEG];
        memcpy(p_right_dst, p_right_src, right_pair_size * 2 * 3 * sizeof(float));
        m_right_assist_line_num[deg+ASSIST_LINE_MAX_DEG] = right_pair_size;

        // front wheel's meter line. right
        m_right_meter_vert_arr[deg+ASSIST_LINE_MAX_DEG] = (float *)malloc(p_header->meter_num*p_header->each_meter_block_pt_num*7*sizeof(float));
        if (NULL == m_right_meter_vert_arr[deg+ASSIST_LINE_MAX_DEG])
        {
            AVM_ERR("OverlayDraw::pre_gen_assist_line_3d_data. alloc right %d ang meter mem faild\n", deg);
            return 1;            
        }
        float *p_right_meter_dst = m_right_meter_vert_arr[deg+ASSIST_LINE_MAX_DEG];
        float *p_right_meter_src = (float *)((unsigned char *)p_blk_mem 
            + p_header->right_meter_arr_det[deg+ASSIST_LINE_MAX_DEG]);
        memcpy(p_right_meter_dst, p_right_meter_src, 
            p_header->meter_num*p_header->each_meter_block_pt_num*7*sizeof(float));
        m_right_meter_num[deg+ASSIST_LINE_MAX_DEG] = p_header->right_meter_num[deg+ASSIST_LINE_MAX_DEG];           

        // front wheel's back line. left
        m_left_back_assist_line_vert_arr[deg+ASSIST_LINE_MAX_DEG] = (float *)malloc(alloc_size);
        if (NULL == m_left_back_assist_line_vert_arr[deg+ASSIST_LINE_MAX_DEG])
        {
            AVM_ERR("OverlayDraw::pre_gen_assist_line_3d_data. alloc left %d ang vert mem faild\n", deg);
            return 1;
        }
        float *p_left_back_dst = m_left_back_assist_line_vert_arr[deg+ASSIST_LINE_MAX_DEG];
        float *p_left_back_src = (float *)((unsigned char *)p_blk_mem 
            + p_header->left_back_line_arr_det[deg+ASSIST_LINE_MAX_DEG]);
        float left_back_pair_size = p_header->left_back_line_num[deg+ASSIST_LINE_MAX_DEG];
        memcpy(p_left_back_dst, p_left_back_src, left_back_pair_size * 2 * 3 * sizeof(float));
        m_left_back_assist_line_num[deg+ASSIST_LINE_MAX_DEG] = left_back_pair_size;

        // front wheel's back line. right
        m_right_back_assist_line_vert_arr[deg+ASSIST_LINE_MAX_DEG] = (float *)malloc(alloc_size);
        if (NULL == m_right_back_assist_line_vert_arr[deg+ASSIST_LINE_MAX_DEG])
        {
            AVM_ERR("OverlayDraw::pre_gen_assist_line_3d_data. alloc right %d ang vert mem faild\n", deg);
            return 1;
        }
        float *p_right_back_dst = m_right_back_assist_line_vert_arr[deg+ASSIST_LINE_MAX_DEG];
        float *p_right_back_src = (float *)((unsigned char *)p_blk_mem 
            + p_header->right_back_line_arr_det[deg+ASSIST_LINE_MAX_DEG]);
        float right_back_pair_size = p_header->right_back_line_num[deg+ASSIST_LINE_MAX_DEG];
        memcpy(p_right_back_dst, p_right_back_src, right_back_pair_size * 2 * 3 * sizeof(float));
        m_right_back_assist_line_num[deg+ASSIST_LINE_MAX_DEG] = right_pair_size;
    }
    int alloc_color_size = m_assist_line_pair_cnt * 2 * 4 *sizeof(float);
    m_p_assist_line_color_arr = (float *)malloc(alloc_color_size);
    if (NULL == m_p_assist_line_color_arr)
    {
        AVM_ERR("OverlayDraw::pre_gen_assist_line_3d_data. alloc color mem faild\n");
        return 1; 
    }
    float *p_color_dst = m_p_assist_line_color_arr;
    float *p_color_src = (float *)((unsigned char *)p_blk_mem
        + p_header->color_arr_det);
    memcpy(p_color_dst, p_color_src, alloc_color_size);

    // free p_blk_mem
    free(p_blk_mem);
    
    return 0;
}


// rear wheel's ahead and back line
int OverlayDraw::pre_gen_assist_line_rear_wheel_3d_data()
{
    char str_name[256];
    if (ini_get_assist_line_rear_wheel_name(str_name, sizeof(str_name)) != 0)
    {
        AVM_ERR("OverlayDraw::pre_gen_assist_line_rear_wheel_3d_data. get assist name faild\n");
        return 1;
    }

    // alloc mem
    struct stat statbuff;
    if (stat(str_name, &statbuff) < 0)
    {
        AVM_ERR("OverlayDraw::pre_gen_assist_line_rear_wheel_3d_data. stat file faild\n");
        return 1;
    }
    unsigned long filesize = statbuff.st_size;
    unsigned char *p_blk_mem = (unsigned char *)memalign(128, filesize);
    if (NULL == p_blk_mem)
    {
        AVM_ERR("OverlayDraw::pre_gen_assist_line_rear_wheel_3d_data. alloc mem faild\n");
        return 1;
    }
    
    // read file and copy data to mem
    FILE *fp = NULL;
    fp = fopen(str_name, "rb");
    if (NULL == fp)
    {
        AVM_ERR("OverlayDraw::pre_gen_assist_line_rear_wheel_3d_data. open db file faild\n");
        return 1;
    }
    fread(p_blk_mem, filesize, 1, fp);
    fclose(fp);

    // alloc vert mem and color mem, and copy db file's data to this mem
    assist_line_db_header_s *p_header = (assist_line_db_header_s *)p_blk_mem;
    m_assist_line_pair_cnt = p_header->pair_num;
    int alloc_size = m_assist_line_pair_cnt * 2 * 3 * sizeof(float);    // pair*2(x,y,z)
    
    for (int deg = ASSIST_LINE_MIN_DEG; deg <= ASSIST_LINE_MAX_DEG; deg++)
    {
        // rear wheel's ahead line. left
        m_rear_wheel_left_assist_line_vert_arr[deg+ASSIST_LINE_MAX_DEG] = (float *)malloc(alloc_size);
        if (NULL == m_rear_wheel_left_assist_line_vert_arr[deg+ASSIST_LINE_MAX_DEG])
        {
            AVM_ERR("OverlayDraw::pre_gen_assist_line_rear_wheel_3d_data. alloc left %d ang vert mem faild\n", deg);
            return 1;
        }
        float *p_left_dst = m_rear_wheel_left_assist_line_vert_arr[deg+ASSIST_LINE_MAX_DEG];
        float *p_left_src = (float *)((unsigned char *)p_blk_mem 
            + p_header->left_line_arr_det[deg+ASSIST_LINE_MAX_DEG]);
        float left_pair_size = p_header->left_line_num[deg+ASSIST_LINE_MAX_DEG];
        memcpy(p_left_dst, p_left_src, left_pair_size * 2 * 3 * sizeof(float));
        m_rear_wheel_left_assist_line_num[deg+ASSIST_LINE_MAX_DEG] = left_pair_size;

        // rear wheel's ahead line. right
        m_rear_wheel_right_assist_line_vert_arr[deg+ASSIST_LINE_MAX_DEG] = (float *)malloc(alloc_size);
        if (NULL == m_rear_wheel_right_assist_line_vert_arr[deg+ASSIST_LINE_MAX_DEG])
        {
            AVM_ERR("OverlayDraw::pre_gen_assist_line_rear_wheel_3d_data. alloc right %d ang vert mem faild\n", deg);
            return 1;
        }
        float *p_right_dst = m_rear_wheel_right_assist_line_vert_arr[deg+ASSIST_LINE_MAX_DEG];
        float *p_right_src = (float *)((unsigned char *)p_blk_mem 
            + p_header->right_line_arr_det[deg+ASSIST_LINE_MAX_DEG]);
        float right_pair_size = p_header->right_line_num[deg+ASSIST_LINE_MAX_DEG];
        memcpy(p_right_dst, p_right_src, right_pair_size * 2 * 3 * sizeof(float));
        m_rear_wheel_right_assist_line_num[deg+ASSIST_LINE_MAX_DEG] = right_pair_size;

        // rear wheel's back line. left
        m_rear_wheel_left_back_assist_line_vert_arr[deg+ASSIST_LINE_MAX_DEG] = (float *)malloc(alloc_size);
        if (NULL == m_rear_wheel_left_back_assist_line_vert_arr[deg+ASSIST_LINE_MAX_DEG])
        {
            AVM_ERR("OverlayDraw::pre_gen_assist_line_rear_wheel_3d_data. alloc left %d ang vert mem faild\n", deg);
            return 1;
        }
        float *p_left_back_dst = m_rear_wheel_left_back_assist_line_vert_arr[deg+ASSIST_LINE_MAX_DEG];
        float *p_left_back_src = (float *)((unsigned char *)p_blk_mem 
            + p_header->left_back_line_arr_det[deg+ASSIST_LINE_MAX_DEG]);
        float left_back_pair_size = p_header->left_back_line_num[deg+ASSIST_LINE_MAX_DEG];
        memcpy(p_left_back_dst, p_left_back_src, left_back_pair_size * 2 * 3 * sizeof(float));
        m_rear_wheel_left_back_assist_line_num[deg+ASSIST_LINE_MAX_DEG] = left_back_pair_size;

        // rear wheel's left meter line. left
        m_rear_wheel_left_back_meter_vert_arr[deg+ASSIST_LINE_MAX_DEG] = (float *)malloc(p_header->meter_num*p_header->each_meter_block_pt_num*7*sizeof(float));
        if (NULL == m_rear_wheel_left_back_meter_vert_arr[deg+ASSIST_LINE_MAX_DEG])
        {
            AVM_ERR("OverlayDraw::pre_gen_assist_line_rear_wheel_3d_data. alloc left %d ang meter mem faild\n", deg);
            return 1;            
        }
        float *p_left_meter_dst = m_rear_wheel_left_back_meter_vert_arr[deg+ASSIST_LINE_MAX_DEG];
        float *p_left_meter_src = (float *)((unsigned char *)p_blk_mem 
            + p_header->left_meter_arr_det[deg+ASSIST_LINE_MAX_DEG]);
        memcpy(p_left_meter_dst, p_left_meter_src, 
            p_header->meter_num*p_header->each_meter_block_pt_num*7*sizeof(float));
        m_rear_wheel_left_back_meter_num[deg+ASSIST_LINE_MAX_DEG] = p_header->left_meter_num[deg+ASSIST_LINE_MAX_DEG]; 

        // rear wheel's back line. right
        m_rear_wheel_right_back_assist_line_vert_arr[deg+ASSIST_LINE_MAX_DEG] = (float *)malloc(alloc_size);
        if (NULL == m_rear_wheel_right_back_assist_line_vert_arr[deg+ASSIST_LINE_MAX_DEG])
        {
            AVM_ERR("OverlayDraw::pre_gen_assist_line_rear_wheel_3d_data. alloc right %d ang vert mem faild\n", deg);
            return 1;
        }
        float *p_right_back_dst = m_rear_wheel_right_back_assist_line_vert_arr[deg+ASSIST_LINE_MAX_DEG];
        float *p_right_back_src = (float *)((unsigned char *)p_blk_mem 
            + p_header->right_back_line_arr_det[deg+ASSIST_LINE_MAX_DEG]);
        float right_back_pair_size = p_header->right_back_line_num[deg+ASSIST_LINE_MAX_DEG];
        memcpy(p_right_back_dst, p_right_back_src, right_back_pair_size * 2 * 3 * sizeof(float));
        m_rear_wheel_right_back_assist_line_num[deg+ASSIST_LINE_MAX_DEG] = right_back_pair_size;

        // rear wheel's meter back line. right
        m_rear_wheel_right_back_meter_vert_arr[deg+ASSIST_LINE_MAX_DEG] = (float *)malloc(p_header->meter_num*p_header->each_meter_block_pt_num*7*sizeof(float));
        if (NULL == m_rear_wheel_right_back_meter_vert_arr[deg+ASSIST_LINE_MAX_DEG])
        {
            AVM_ERR("OverlayDraw::pre_gen_assist_line_rear_wheel_3d_data. alloc right %d ang meter mem faild\n", deg);
            return 1;            
        }
        float *p_right_meter_dst = m_rear_wheel_right_back_meter_vert_arr[deg+ASSIST_LINE_MAX_DEG];
        float *p_right_meter_src = (float *)((unsigned char *)p_blk_mem 
            + p_header->right_meter_arr_det[deg+ASSIST_LINE_MAX_DEG]);
        memcpy(p_right_meter_dst, p_right_meter_src, 
            p_header->meter_num*p_header->each_meter_block_pt_num*7*sizeof(float));
        m_rear_wheel_right_back_meter_num[deg+ASSIST_LINE_MAX_DEG] = p_header->right_meter_num[deg+ASSIST_LINE_MAX_DEG];
    }

    // color array is shared by all coord space line
    // so we need not load color data second time
#if 0
    int alloc_color_size = m_assist_line_pair_cnt * 2 * 4 *sizeof(float);
    m_p_assist_line_color_arr = (float *)malloc(alloc_color_size);
    if (NULL == m_p_assist_line_color_arr)
    {
        AVM_ERR("OverlayDraw::pre_gen_assist_line_3d_data. alloc color mem faild\n");
        return 1; 
    }
    float *p_color_dst = m_p_assist_line_color_arr;
    float *p_color_src = (float *)((unsigned char *)p_blk_mem
        + p_header->color_arr_det);
    memcpy(p_color_dst, p_color_src, alloc_color_size);
#endif

    // free p_blk_mem
    free(p_blk_mem);
    
    return 0;
}



int OverlayDraw::pre_gen_assist_line_front_cam_data()
{
    char str_name[256];
    if (ini_get_assist_line_front_cam_name(str_name, sizeof(str_name)) != 0)
    {
        AVM_ERR("OverlayDraw::pre_gen_assist_line_front_cam_data. get assist name faild\n");
        return 1;
    }

    // alloc mem
    struct stat statbuff;
    if (stat(str_name, &statbuff) < 0)
    {
        AVM_ERR("OverlayDraw::pre_gen_assist_line_front_cam_data. stat file faild\n");
        return 1;
    }
    unsigned long filesize = statbuff.st_size;
    unsigned char *p_blk_mem = (unsigned char *)memalign(128, filesize);
    if (NULL == p_blk_mem)
    {
        AVM_ERR("OverlayDraw::pre_gen_assist_line_front_cam_data. alloc mem faild\n");
        return 1;
    }
    
    // read file and copy data to mem
    FILE *fp = NULL;
    fp = fopen(str_name, "rb");
    if (NULL == fp)
    {
        AVM_ERR("OverlayDraw::pre_gen_assist_line_front_cam_data. open db file faild\n");
        return 1;
    }
    fread(p_blk_mem, filesize, 1, fp);
    fclose(fp);

    // alloc vert mem and color mem, and copy db file's data to this mem
    assist_line_db_header_s *p_header = (assist_line_db_header_s *)p_blk_mem;
    m_assist_line_pair_cnt = p_header->pair_num;
    int alloc_size = m_assist_line_pair_cnt * 2 * 3 * sizeof(float);    // pair*2(x,y,z)
    int meter_alloc_size = p_header->meter_num * p_header->each_meter_block_pt_num * 7 * sizeof(float);

    for (int deg = ASSIST_LINE_MIN_DEG; deg <= ASSIST_LINE_MAX_DEG; deg++)
    {
        // left line
        m_front_cam_left_assist_line_vert_arr[deg+ASSIST_LINE_MAX_DEG] = (float *)memalign(128, alloc_size);
        if (NULL == m_front_cam_left_assist_line_vert_arr[deg+ASSIST_LINE_MAX_DEG])
        {
            AVM_ERR("OverlayDraw::pre_gen_assist_line_front_cam_data. alloc left %d ang vert mem faild\n", deg);
            return 1;
        }
        float *p_left_dst = m_front_cam_left_assist_line_vert_arr[deg+ASSIST_LINE_MAX_DEG];
        float *p_left_src = (float *)((unsigned char *)p_blk_mem 
            + p_header->left_line_arr_det[deg+ASSIST_LINE_MAX_DEG]);
        float left_pair_size = p_header->left_line_num[deg+ASSIST_LINE_MAX_DEG];
        memcpy(p_left_dst, p_left_src, left_pair_size * 2 * 3 * sizeof(float));
        m_front_cam_left_assist_line_num[deg+ASSIST_LINE_MAX_DEG] = left_pair_size;

        // right line
        m_front_cam_right_assist_line_vert_arr[deg+ASSIST_LINE_MAX_DEG] = (float *)memalign(128, alloc_size);
        if (NULL == m_front_cam_right_assist_line_vert_arr[deg+ASSIST_LINE_MAX_DEG])
        {
            AVM_ERR("OverlayDraw::pre_gen_assist_line_front_cam_data. alloc right %d ang vert mem faild\n", deg);
            return 1;
        }
        float *p_right_dst = m_front_cam_right_assist_line_vert_arr[deg+ASSIST_LINE_MAX_DEG];
        float *p_right_src = (float *)((unsigned char *)p_blk_mem 
            + p_header->right_line_arr_det[deg+ASSIST_LINE_MAX_DEG]);
        float right_pair_size = p_header->right_line_num[deg+ASSIST_LINE_MAX_DEG];
        memcpy(p_right_dst, p_right_src, right_pair_size * 2 * 3 * sizeof(float));
        m_front_cam_right_assist_line_num[deg+ASSIST_LINE_MAX_DEG] = right_pair_size;

        // left meter
        m_front_cam_left_meter_vert_arr[deg+ASSIST_LINE_MAX_DEG] = (float *)memalign(128, meter_alloc_size);
        if (NULL == m_front_cam_left_meter_vert_arr[deg+ASSIST_LINE_MAX_DEG])
        {
            AVM_ERR("OverlayDraw::pre_gen_assist_line_front_cam_data. alloc left %d ang meter mem faild\n", deg);
            return 1;
        }
        float *p_left_meter_dst = m_front_cam_left_meter_vert_arr[deg+ASSIST_LINE_MAX_DEG];
        float *p_left_meter_src = (float *)((unsigned char *)p_blk_mem 
            + p_header->left_meter_arr_det[deg+ASSIST_LINE_MAX_DEG]);
        memcpy(p_left_meter_dst, p_left_meter_src, meter_alloc_size);
        m_front_cam_left_meter_num[deg+ASSIST_LINE_MAX_DEG] = p_header->left_meter_num[deg+ASSIST_LINE_MAX_DEG];
        
        // right meter
        m_front_cam_right_meter_vert_arr[deg+ASSIST_LINE_MAX_DEG] = (float *)memalign(128, meter_alloc_size);
        if (NULL == m_front_cam_right_meter_vert_arr[deg+ASSIST_LINE_MAX_DEG])
        {
            AVM_ERR("OverlayDraw::pre_gen_assist_line_front_cam_data. alloc right %d ang meter mem faild\n", deg);
            return 1;
        }
        float *p_right_meter_dst = m_front_cam_right_meter_vert_arr[deg+ASSIST_LINE_MAX_DEG];
        float *p_right_meter_src = (float *)((unsigned char *)p_blk_mem 
            + p_header->right_meter_arr_det[deg+ASSIST_LINE_MAX_DEG]);
        memcpy(p_right_meter_dst, p_right_meter_src, meter_alloc_size);
        m_front_cam_right_meter_num[deg+ASSIST_LINE_MAX_DEG] = p_header->right_meter_num[deg+ASSIST_LINE_MAX_DEG];        
    }

    // color array is shared by all coord space line
    // so we need not load color data second time
#if 0
    int alloc_color_size = m_assist_line_pair_cnt * 2 * 4 *sizeof(float);
    m_p_assist_line_color_arr = (float *)malloc(alloc_color_size);
    if (NULL == m_p_assist_line_color_arr)
    {
        AVM_ERR("OverlayDraw::pre_gen_assist_line_front_cam_data. alloc color mem faild\n");
        return 1; 
    }
    float *p_color_dst = m_p_assist_line_color_arr;
    float *p_color_src = (float *)((unsigned char *)p_blk_mem
        + p_header->color_arr_det);
    memcpy(p_color_dst, p_color_src, alloc_color_size);
#endif
    // free p_blk_mem
    free(p_blk_mem);
    
    return 0;
}


int OverlayDraw::pre_gen_assist_line_rear_cam_data()
{
    char str_name[256];
    if (ini_get_assist_line_rear_cam_name(str_name, sizeof(str_name)) != 0)
    {
        AVM_ERR("OverlayDraw::pre_gen_assist_line_rear_cam_data. get assist name faild\n");
        return 1;
    }

    // alloc mem
    struct stat statbuff;
    if (stat(str_name, &statbuff) < 0)
    {
        AVM_ERR("OverlayDraw::pre_gen_assist_line_front_cam_data. stat file faild\n");
        return 1;
    }
    unsigned long filesize = statbuff.st_size;
    unsigned char *p_blk_mem = (unsigned char *)memalign(128, filesize);
    if (NULL == p_blk_mem)
    {
        AVM_ERR("OverlayDraw::pre_gen_assist_line_rear_cam_data. alloc mem faild\n");
        return 1;
    }
    
    // read file and copy data to mem
    FILE *fp = NULL;
    fp = fopen(str_name, "rb");
    if (NULL == fp)
    {
        AVM_ERR("OverlayDraw::pre_gen_assist_line_rear_cam_data. open db file faild\n");
        return 1;
    }
    fread(p_blk_mem, filesize, 1, fp);
    fclose(fp);

    // alloc vert mem and color mem, and copy db file's data to this mem
    assist_line_db_header_s *p_header = (assist_line_db_header_s *)p_blk_mem;
    m_assist_line_pair_cnt = p_header->pair_num;
    int alloc_size = m_assist_line_pair_cnt * 2 * 3 * sizeof(float);    // pair*2(x,y,z)
    int meter_alloc_size = p_header->meter_num * p_header->each_meter_block_pt_num * 7 * sizeof(float);

    for (int deg = ASSIST_LINE_MIN_DEG; deg <= ASSIST_LINE_MAX_DEG; deg++)
    {
        // left line
        m_rear_cam_left_assist_line_vert_arr[deg+ASSIST_LINE_MAX_DEG] = (float *)memalign(128, alloc_size);
        if (NULL == m_rear_cam_left_assist_line_vert_arr[deg+ASSIST_LINE_MAX_DEG])
        {
            AVM_ERR("OverlayDraw::pre_gen_assist_line_front_cam_data. alloc left %d ang vert mem faild\n", deg);
            return 1;
        }
        float *p_left_dst = m_rear_cam_left_assist_line_vert_arr[deg+ASSIST_LINE_MAX_DEG];
        float *p_left_src = (float *)((unsigned char *)p_blk_mem 
            + p_header->left_line_arr_det[deg+ASSIST_LINE_MAX_DEG]);
        float left_pair_size = p_header->left_line_num[deg+ASSIST_LINE_MAX_DEG];
        memcpy(p_left_dst, p_left_src, left_pair_size * 2 * 3 * sizeof(float));
        m_rear_cam_left_assist_line_num[deg+ASSIST_LINE_MAX_DEG] = left_pair_size;

        // right line
        m_rear_cam_right_assist_line_vert_arr[deg+ASSIST_LINE_MAX_DEG] = (float *)memalign(128, alloc_size);
        if (NULL == m_rear_cam_right_assist_line_vert_arr[deg+ASSIST_LINE_MAX_DEG])
        {
            AVM_ERR("OverlayDraw::pre_gen_assist_line_front_cam_data. alloc right %d ang vert mem faild\n", deg);
            return 1;
        }
        float *p_right_dst = m_rear_cam_right_assist_line_vert_arr[deg+ASSIST_LINE_MAX_DEG];
        float *p_right_src = (float *)((unsigned char *)p_blk_mem 
            + p_header->right_line_arr_det[deg+ASSIST_LINE_MAX_DEG]);
        float right_pair_size = p_header->right_line_num[deg+ASSIST_LINE_MAX_DEG];
        memcpy(p_right_dst, p_right_src, right_pair_size * 2 * 3 * sizeof(float));
        m_rear_cam_right_assist_line_num[deg+ASSIST_LINE_MAX_DEG] = right_pair_size;

        // left meter
        m_rear_cam_left_meter_vert_arr[deg+ASSIST_LINE_MAX_DEG] = (float *)memalign(128, meter_alloc_size);
        if (NULL == m_rear_cam_left_meter_vert_arr[deg+ASSIST_LINE_MAX_DEG])
        {
            AVM_ERR("OverlayDraw::pre_gen_assist_line_front_cam_data. alloc left %d ang meter mem faild\n", deg);
            return 1;
        }
        float *p_left_meter_dst = m_rear_cam_left_meter_vert_arr[deg+ASSIST_LINE_MAX_DEG];
        float *p_left_meter_src = (float *)((unsigned char *)p_blk_mem 
            + p_header->left_meter_arr_det[deg+ASSIST_LINE_MAX_DEG]);
        memcpy(p_left_meter_dst, p_left_meter_src, meter_alloc_size);
        m_rear_cam_left_meter_num[deg+ASSIST_LINE_MAX_DEG] = p_header->left_meter_num[deg+ASSIST_LINE_MAX_DEG];
        
        // right meter
        m_rear_cam_right_meter_vert_arr[deg+ASSIST_LINE_MAX_DEG] = (float *)memalign(128, meter_alloc_size);
        if (NULL == m_rear_cam_right_meter_vert_arr[deg+ASSIST_LINE_MAX_DEG])
        {
            AVM_ERR("OverlayDraw::pre_gen_assist_line_front_cam_data. alloc right %d ang meter mem faild\n", deg);
            return 1;
        }
        float *p_right_meter_dst = m_rear_cam_right_meter_vert_arr[deg+ASSIST_LINE_MAX_DEG];
        float *p_right_meter_src = (float *)((unsigned char *)p_blk_mem 
            + p_header->right_meter_arr_det[deg+ASSIST_LINE_MAX_DEG]);
        memcpy(p_right_meter_dst, p_right_meter_src, meter_alloc_size);
        m_rear_cam_right_meter_num[deg+ASSIST_LINE_MAX_DEG] = p_header->right_meter_num[deg+ASSIST_LINE_MAX_DEG];  
    }

    // color array is shared by all coord space line
    // so we need not load color data second time
#if 0
    int alloc_color_size = m_assist_line_pair_cnt * 2 * 4 *sizeof(float);
    m_p_assist_line_color_arr = (float *)malloc(alloc_color_size);
    if (NULL == m_p_assist_line_color_arr)
    {
        AVM_ERR("OverlayDraw::pre_gen_assist_line_front_cam_data. alloc color mem faild\n");
        return 1; 
    }
    float *p_color_dst = m_p_assist_line_color_arr;
    float *p_color_src = (float *)((unsigned char *)p_blk_mem
        + p_header->color_arr_det);
    memcpy(p_color_dst, p_color_src, alloc_color_size);
#endif
    // free p_blk_mem
    free(p_blk_mem);
    
    return 0;
}


int OverlayDraw::pre_gen_assist_line_data()
{
    int re;
    re = pre_gen_assist_line_3d_data();
    if (re != 0)
    {
        AVM_ERR("OverlayDraw::pre_gen_assist_line_data. load 3d data faild\n");
        return 1;
    }
    re = pre_gen_assist_line_rear_wheel_3d_data();
    if (re != 0)
    {
        AVM_ERR("OverlayDraw::pre_gen_assist_line_data. load rear wheel 3d data faild\n");
        return 1;
    }    
    re = pre_gen_assist_line_front_cam_data();
    if (re != 0)
    {
        AVM_ERR("OverlayDraw::pre_gen_assist_line_data. load front cam data faild\n");
        return 1;
    }
    re = pre_gen_assist_line_rear_cam_data();
    if (re != 0)
    {
        AVM_ERR("OverlayDraw::pre_gen_assist_line_data. load front cam data faild\n");
        return 1;
    }
    m_assist_line_load = OVERLAY_DATA_LOADDED;

    return 0;
}


int OverlayDraw::gen_assist_line_vbo()
{
    if (m_assist_line_load != OVERLAY_DATA_LOADDED)
    {
        AVM_ERR("OverlayDraw::gen_assist_line_vbo. assist line data not loaded\n");
        return 1;
    }

    // vert buffer
    int idx = m_current_deg + ASSIST_LINE_MAX_DEG;
    int vert_size;
    float *p_left_vert_arr = m_left_assist_line_vert_arr[idx];
    vert_size = m_assist_line_pair_cnt * 2 * 3 * sizeof(GLfloat);  // alloc max size
    glGenBuffers(1, &m_left_assist_line_vert_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, m_left_assist_line_vert_buffer);
    glBufferData(GL_ARRAY_BUFFER, vert_size, p_left_vert_arr, GL_DYNAMIC_DRAW);

    float *p_right_vert_arr = m_right_assist_line_vert_arr[idx];
    vert_size = m_assist_line_pair_cnt * 2 * 3 * sizeof(GLfloat);  // alloc max size
    glGenBuffers(1, &m_right_assist_line_vert_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, m_right_assist_line_vert_buffer);
    glBufferData(GL_ARRAY_BUFFER, vert_size, p_right_vert_arr, GL_DYNAMIC_DRAW);

    float *p_rear_wheel_left_vert_arr = m_rear_wheel_left_assist_line_vert_arr[idx];
    vert_size = m_assist_line_pair_cnt * 2 * 3 * sizeof(GLfloat);  // alloc max size
    glGenBuffers(1, &m_rear_wheel_left_assist_line_vert_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, m_rear_wheel_left_assist_line_vert_buffer);
    glBufferData(GL_ARRAY_BUFFER, vert_size, p_rear_wheel_left_vert_arr, GL_DYNAMIC_DRAW);

    float *p_rear_wheel_right_vert_arr = m_rear_wheel_right_assist_line_vert_arr[idx];
    vert_size = m_assist_line_pair_cnt * 2 * 3 * sizeof(GLfloat);  // alloc max size
    glGenBuffers(1, &m_rear_wheel_right_assist_line_vert_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, m_rear_wheel_right_assist_line_vert_buffer);
    glBufferData(GL_ARRAY_BUFFER, vert_size, p_rear_wheel_right_vert_arr, GL_DYNAMIC_DRAW);
    
    // color buffer
    int color_size = m_assist_line_pair_cnt * 2 * 4 * sizeof(GLfloat);
    glGenBuffers(1, &m_assist_line_color_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, m_assist_line_color_buffer);
    glBufferData(GL_ARRAY_BUFFER, color_size, m_p_assist_line_color_arr, GL_STATIC_DRAW);

    // meter
    float *p_left_meter_vert_arr = m_left_meter_vert_arr[idx];
    vert_size = m_left_meter_num[idx] *7 * sizeof(GLfloat);  // alloc max size
    glGenBuffers(1, &m_left_meter_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, m_left_meter_buffer);
    glBufferData(GL_ARRAY_BUFFER, vert_size, p_left_meter_vert_arr, GL_DYNAMIC_DRAW);    

    float *p_right_meter_vert_arr = m_right_meter_vert_arr[idx];
    vert_size = m_right_meter_num[idx] *7 * sizeof(GLfloat);  // alloc max size
    glGenBuffers(1, &m_right_meter_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, m_right_meter_buffer);
    glBufferData(GL_ARRAY_BUFFER, vert_size, p_right_meter_vert_arr, GL_DYNAMIC_DRAW); 

    m_assist_line_load = OVERLAY_VBO_LOADDED;
    
    return 0;    
}


void OverlayDraw::set_assist_line_mvp(glm::mat4 *p_m, glm::mat4 *p_v, glm::mat4 *p_p)
{
    if (p_m != NULL)
    {
        m_assist_line_m = *p_m;
    }
    if (p_v != NULL)
    {
        m_assist_line_v = *p_v;
    }
    if (p_p != NULL)
    {
        m_assist_line_p = *p_p;
    }
}

void OverlayDraw::assist_line_show(int is_show)
{
    m_is_assist_show = is_show;
}


void OverlayDraw::assist_line_set_direction(int is_front)
{
    int dst_dir;
    
    if (is_front)
    {
        dst_dir = 0;
    }
    else
    {
        dst_dir = 1;
    }
    
    if (dst_dir != m_assist_dir)
    {
        m_assist_change++;
    }
    m_assist_dir = dst_dir;
}


int OverlayDraw::assist_line_3d_process_front_left()
{
    if (0 == m_assist_dir)
    {
        // assist line
        // color buffer
        glBindBuffer(GL_ARRAY_BUFFER, m_assist_line_color_buffer);
        glVertexAttribPointer(m_loc_color, 4, GL_FLOAT, GL_FALSE,
            4 * sizeof(float), (GLvoid *)(0 * sizeof(float)));
        // left vertex buffer
        glBindBuffer(GL_ARRAY_BUFFER, m_left_assist_line_vert_buffer);
        if (m_assist_change)
        {
            //m_assist_change = 0;
            glBufferSubData(GL_ARRAY_BUFFER, 0,
                m_assist_line_pair_cnt*2*3*sizeof(float),
                m_left_assist_line_vert_arr[m_current_deg+ASSIST_LINE_MAX_DEG]);
        }
        glVertexAttribPointer(m_loc_vertex_position, 3, GL_FLOAT, GL_FALSE,
            3 * sizeof(float), (GLvoid *)(0 * sizeof(float)));    

        glDrawArrays(GL_TRIANGLE_STRIP, 0, m_left_assist_line_num[m_current_deg+ASSIST_LINE_MAX_DEG]*2);

        // meter block
        // color buffer
        glBindBuffer(GL_ARRAY_BUFFER, m_left_meter_buffer);
        if (m_assist_change)
        {
            //m_assist_change = 0;
            glBufferSubData(GL_ARRAY_BUFFER, 0,
                m_left_meter_num[m_current_deg+ASSIST_LINE_MAX_DEG]*7*sizeof(float),
                m_left_meter_vert_arr[m_current_deg+ASSIST_LINE_MAX_DEG]);
        }        
        glVertexAttribPointer(m_loc_vertex_position, 3, GL_FLOAT, GL_FALSE,
            7 * sizeof(float), (GLvoid *)(0 * sizeof(float)));    
        glVertexAttribPointer(m_loc_color, 4, GL_FLOAT, GL_FALSE,
            7 * sizeof(float), (GLvoid *)(3 * sizeof(float)));

        glDrawArrays(GL_TRIANGLES, 0, m_left_meter_num[m_current_deg+ASSIST_LINE_MAX_DEG]);
    }
    else if ((1 == m_assist_dir) && (m_current_deg > 0))
    {
        // color buffer
        glBindBuffer(GL_ARRAY_BUFFER, m_assist_line_color_buffer);
        glVertexAttribPointer(m_loc_color, 4, GL_FLOAT, GL_FALSE,
            4 * sizeof(float), (GLvoid *)(0 * sizeof(float)));
        // left vertex buffer
        glBindBuffer(GL_ARRAY_BUFFER, m_left_assist_line_vert_buffer);
        if (m_assist_change)
        {
            //m_assist_change = 0;
            glBufferSubData(GL_ARRAY_BUFFER, 0,
                m_assist_line_pair_cnt*2*3*sizeof(float),
                m_left_back_assist_line_vert_arr[m_current_deg+ASSIST_LINE_MAX_DEG]);
        }
        glVertexAttribPointer(m_loc_vertex_position, 3, GL_FLOAT, GL_FALSE,
            3 * sizeof(float), (GLvoid *)(0 * sizeof(float)));    

        glDrawArrays(GL_TRIANGLE_STRIP, 0, m_left_back_assist_line_num[m_current_deg+ASSIST_LINE_MAX_DEG]*2);    
    }

    return 0;
}


int OverlayDraw::assist_line_3d_process_front_right()
{
    if (0 == m_assist_dir)
    {
        // assist line
        // color buffer
        glBindBuffer(GL_ARRAY_BUFFER, m_assist_line_color_buffer);
        glVertexAttribPointer(m_loc_color, 4, GL_FLOAT, GL_FALSE,
            4 * sizeof(float), (GLvoid *)(0 * sizeof(float)));
        // right vertex buffer
        glBindBuffer(GL_ARRAY_BUFFER, m_right_assist_line_vert_buffer);
        if (m_assist_change)
        {
            glBufferSubData(GL_ARRAY_BUFFER, 0,
                m_assist_line_pair_cnt*2*3*sizeof(float),
                m_right_assist_line_vert_arr[m_current_deg+ASSIST_LINE_MAX_DEG]);
        }
        glVertexAttribPointer(m_loc_vertex_position, 3, GL_FLOAT, GL_FALSE,
            3 * sizeof(float), (GLvoid *)(0 * sizeof(float)));    

        glDrawArrays(GL_TRIANGLE_STRIP, 0, m_right_assist_line_num[m_current_deg+ASSIST_LINE_MAX_DEG]*2);

        // meter block
        // color buffer
        glBindBuffer(GL_ARRAY_BUFFER, m_right_meter_buffer);
        if (m_assist_change)
        {
            //m_assist_change = 0;
            glBufferSubData(GL_ARRAY_BUFFER, 0,
                m_right_meter_num[m_current_deg+ASSIST_LINE_MAX_DEG]*7*sizeof(float),
                m_right_meter_vert_arr[m_current_deg+ASSIST_LINE_MAX_DEG]);
        }        
        glVertexAttribPointer(m_loc_vertex_position, 3, GL_FLOAT, GL_FALSE,
            7 * sizeof(float), (GLvoid *)(0 * sizeof(float)));    
        glVertexAttribPointer(m_loc_color, 4, GL_FLOAT, GL_FALSE,
            7 * sizeof(float), (GLvoid *)(3 * sizeof(float)));

        glDrawArrays(GL_TRIANGLES, 0, m_right_meter_num[m_current_deg+ASSIST_LINE_MAX_DEG]);
    }
    else if ((1 == m_assist_dir) && (m_current_deg < 0))
    {
        // color buffer
        glBindBuffer(GL_ARRAY_BUFFER, m_assist_line_color_buffer);
        glVertexAttribPointer(m_loc_color, 4, GL_FLOAT, GL_FALSE,
            4 * sizeof(float), (GLvoid *)(0 * sizeof(float)));
        // right vertex buffer
        glBindBuffer(GL_ARRAY_BUFFER, m_right_assist_line_vert_buffer);
        if (m_assist_change)
        {
            glBufferSubData(GL_ARRAY_BUFFER, 0,
                m_assist_line_pair_cnt*2*3*sizeof(float),
                m_right_back_assist_line_vert_arr[m_current_deg+ASSIST_LINE_MAX_DEG]);
        }
        glVertexAttribPointer(m_loc_vertex_position, 3, GL_FLOAT, GL_FALSE,
            3 * sizeof(float), (GLvoid *)(0 * sizeof(float)));    

        glDrawArrays(GL_TRIANGLE_STRIP, 0, m_right_back_assist_line_num[m_current_deg+ASSIST_LINE_MAX_DEG]*2);  
    }

    return 0;
}


int OverlayDraw::assist_line_3d_process_rear_left()
{
    if ((0 == m_assist_dir) && (m_current_deg < 0))
    {
        // color buffer
        glBindBuffer(GL_ARRAY_BUFFER, m_assist_line_color_buffer);
        glVertexAttribPointer(m_loc_color, 4, GL_FLOAT, GL_FALSE,
            4 * sizeof(float), (GLvoid *)(0 * sizeof(float)));
        // left vertex buffer
        glBindBuffer(GL_ARRAY_BUFFER, m_rear_wheel_left_assist_line_vert_buffer);
        if (m_assist_change)
        {
            glBufferSubData(GL_ARRAY_BUFFER, 0,
                m_assist_line_pair_cnt*2*3*sizeof(float),
                m_rear_wheel_left_assist_line_vert_arr[m_current_deg+ASSIST_LINE_MAX_DEG]);
        }
        glVertexAttribPointer(m_loc_vertex_position, 3, GL_FLOAT, GL_FALSE,
            3 * sizeof(float), (GLvoid *)(0 * sizeof(float)));    

        glDrawArrays(GL_TRIANGLE_STRIP, 0, m_rear_wheel_left_assist_line_num[m_current_deg+ASSIST_LINE_MAX_DEG]*2);
    }
    else if (1 == m_assist_dir)
    {
        // assist line
        // color buffer
        glBindBuffer(GL_ARRAY_BUFFER, m_assist_line_color_buffer);
        glVertexAttribPointer(m_loc_color, 4, GL_FLOAT, GL_FALSE,
            4 * sizeof(float), (GLvoid *)(0 * sizeof(float)));
        // left vertex buffer
        glBindBuffer(GL_ARRAY_BUFFER, m_rear_wheel_left_assist_line_vert_buffer);
        if (m_assist_change)
        {
            glBufferSubData(GL_ARRAY_BUFFER, 0,
                m_assist_line_pair_cnt*2*3*sizeof(float),
                m_rear_wheel_left_back_assist_line_vert_arr[m_current_deg+ASSIST_LINE_MAX_DEG]);
        }
        glVertexAttribPointer(m_loc_vertex_position, 3, GL_FLOAT, GL_FALSE,
            3 * sizeof(float), (GLvoid *)(0 * sizeof(float)));    

        glDrawArrays(GL_TRIANGLE_STRIP, 0, m_rear_wheel_left_back_assist_line_num[m_current_deg+ASSIST_LINE_MAX_DEG]*2);

        // meter block
        // color buffer
        glBindBuffer(GL_ARRAY_BUFFER, m_left_meter_buffer);
        if (m_assist_change)
        {
            //m_assist_change = 0;
            glBufferSubData(GL_ARRAY_BUFFER, 0,
                m_rear_wheel_left_back_meter_num[m_current_deg+ASSIST_LINE_MAX_DEG]*7*sizeof(float),
                m_rear_wheel_left_back_meter_vert_arr[m_current_deg+ASSIST_LINE_MAX_DEG]);
        }        
        glVertexAttribPointer(m_loc_vertex_position, 3, GL_FLOAT, GL_FALSE,
            7 * sizeof(float), (GLvoid *)(0 * sizeof(float)));    
        glVertexAttribPointer(m_loc_color, 4, GL_FLOAT, GL_FALSE,
            7 * sizeof(float), (GLvoid *)(3 * sizeof(float)));

        glDrawArrays(GL_TRIANGLES, 0, m_rear_wheel_left_back_meter_num[m_current_deg+ASSIST_LINE_MAX_DEG]);
    }
    
    return 0;
}


int OverlayDraw::assist_line_3d_process_rear_right()
{
    // go ahead and turn right, show rear wheel's right line
    if ((0 == m_assist_dir) && (m_current_deg > 0))
    {
        // color buffer
        glBindBuffer(GL_ARRAY_BUFFER, m_assist_line_color_buffer);
        glVertexAttribPointer(m_loc_color, 4, GL_FLOAT, GL_FALSE,
            4 * sizeof(float), (GLvoid *)(0 * sizeof(float)));

        // left vertex buffer
        glBindBuffer(GL_ARRAY_BUFFER, m_rear_wheel_right_assist_line_vert_buffer);
        if (m_assist_change)
        {
            glBufferSubData(GL_ARRAY_BUFFER, 0,
                m_assist_line_pair_cnt*2*3*sizeof(float),
                m_rear_wheel_right_assist_line_vert_arr[m_current_deg+ASSIST_LINE_MAX_DEG]);
        }
        glVertexAttribPointer(m_loc_vertex_position, 3, GL_FLOAT, GL_FALSE,
            3 * sizeof(float), (GLvoid *)(0 * sizeof(float)));    
        
        glDrawArrays(GL_TRIANGLE_STRIP, 0, m_rear_wheel_right_assist_line_num[m_current_deg+ASSIST_LINE_MAX_DEG]*2);
    }
    // go back, always show rear wheel's right line
    else if (1 == m_assist_dir)
    {
        // color buffer
        glBindBuffer(GL_ARRAY_BUFFER, m_assist_line_color_buffer);
        glVertexAttribPointer(m_loc_color, 4, GL_FLOAT, GL_FALSE,
            4 * sizeof(float), (GLvoid *)(0 * sizeof(float)));

        // left vertex buffer
        glBindBuffer(GL_ARRAY_BUFFER, m_rear_wheel_right_assist_line_vert_buffer);
        if (m_assist_change)
        {
            glBufferSubData(GL_ARRAY_BUFFER, 0,
                m_assist_line_pair_cnt*2*3*sizeof(float),
                m_rear_wheel_right_back_assist_line_vert_arr[m_current_deg+ASSIST_LINE_MAX_DEG]);
        }
        glVertexAttribPointer(m_loc_vertex_position, 3, GL_FLOAT, GL_FALSE,
            3 * sizeof(float), (GLvoid *)(0 * sizeof(float)));    
        
        glDrawArrays(GL_TRIANGLE_STRIP, 0, m_rear_wheel_right_back_assist_line_num[m_current_deg+ASSIST_LINE_MAX_DEG]*2);

        // meter block
        // color buffer
        glBindBuffer(GL_ARRAY_BUFFER, m_right_meter_buffer);
        if (m_assist_change)
        {
            //m_assist_change = 0;
            glBufferSubData(GL_ARRAY_BUFFER, 0,
                m_rear_wheel_right_back_meter_num[m_current_deg+ASSIST_LINE_MAX_DEG]*7*sizeof(float),
                m_rear_wheel_right_back_meter_vert_arr[m_current_deg+ASSIST_LINE_MAX_DEG]);
        }        
        glVertexAttribPointer(m_loc_vertex_position, 3, GL_FLOAT, GL_FALSE,
            7 * sizeof(float), (GLvoid *)(0 * sizeof(float)));    
        glVertexAttribPointer(m_loc_color, 4, GL_FLOAT, GL_FALSE,
            7 * sizeof(float), (GLvoid *)(3 * sizeof(float)));

        glDrawArrays(GL_TRIANGLES, 0, m_rear_wheel_right_back_meter_num[m_current_deg+ASSIST_LINE_MAX_DEG]);
    }

    return 0;
}


int OverlayDraw::assist_line_3d_process()
{
    glEnableVertexAttribArray(m_loc_vertex_position);
    glEnableVertexAttribArray(m_loc_color);

    glm::mat4 mvp = m_assist_line_p * m_assist_line_v * m_assist_line_m;
    glUniformMatrix4fv(m_loc_mvp, 1, GL_FALSE, glm::value_ptr(mvp));

    assist_line_3d_process_front_left();
    assist_line_3d_process_front_right();
    assist_line_3d_process_rear_left();
    assist_line_3d_process_rear_right();    

    if (m_assist_change)
    {
        m_assist_change = 0;
    }

    return 0;
}


int OverlayDraw::assist_line_2d_process_front()
{
    // line left
    // color buffer
    glBindBuffer(GL_ARRAY_BUFFER, m_assist_line_color_buffer);
    glVertexAttribPointer(m_loc_color, 4, GL_FLOAT, GL_FALSE,
        4 * sizeof(float), (GLvoid *)(0 * sizeof(float)));
    // left vertex buffer
    glBindBuffer(GL_ARRAY_BUFFER, m_left_assist_line_vert_buffer);
    if (m_assist_change)
    {
        //m_assist_change = 0;
        glBufferSubData(GL_ARRAY_BUFFER, 0,
            m_assist_line_pair_cnt*2*3*sizeof(float),
            m_front_cam_left_assist_line_vert_arr[m_current_deg+ASSIST_LINE_MAX_DEG]);
    }
    glVertexAttribPointer(m_loc_vertex_position, 3, GL_FLOAT, GL_FALSE,
        3 * sizeof(float), (GLvoid *)(0 * sizeof(float)));    

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 
        m_front_cam_left_assist_line_num[m_current_deg+ASSIST_LINE_MAX_DEG]*2);

    // line right
    // color buffer
    glBindBuffer(GL_ARRAY_BUFFER, m_assist_line_color_buffer);
    glVertexAttribPointer(m_loc_color, 4, GL_FLOAT, GL_FALSE,
        4 * sizeof(float), (GLvoid *)(0 * sizeof(float)));
    // right vertex buffer
    glBindBuffer(GL_ARRAY_BUFFER, m_right_assist_line_vert_buffer);
    if (m_assist_change)
    {
        //m_assist_change = 0;
        glBufferSubData(GL_ARRAY_BUFFER, 0,
            m_assist_line_pair_cnt*2*3*sizeof(float),
            m_front_cam_right_assist_line_vert_arr[m_current_deg+ASSIST_LINE_MAX_DEG]);
    }
    glVertexAttribPointer(m_loc_vertex_position, 3, GL_FLOAT, GL_FALSE,
        3 * sizeof(float), (GLvoid *)(0 * sizeof(float)));    

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 
        m_front_cam_right_assist_line_num[m_current_deg+ASSIST_LINE_MAX_DEG]*2);

    // meter left
    // color buffer
    glBindBuffer(GL_ARRAY_BUFFER, m_left_meter_buffer);
    if (m_assist_change)
    {
        //m_assist_change = 0;
        glBufferSubData(GL_ARRAY_BUFFER, 0,
            m_front_cam_left_meter_num[m_current_deg+ASSIST_LINE_MAX_DEG]*7*sizeof(float),
            m_front_cam_left_meter_vert_arr[m_current_deg+ASSIST_LINE_MAX_DEG]);
    }        
    glVertexAttribPointer(m_loc_vertex_position, 3, GL_FLOAT, GL_FALSE,
        7 * sizeof(float), (GLvoid *)(0 * sizeof(float)));    
    glVertexAttribPointer(m_loc_color, 4, GL_FLOAT, GL_FALSE,
        7 * sizeof(float), (GLvoid *)(3 * sizeof(float)));
    
    glDrawArrays(GL_TRIANGLES, 0, m_front_cam_left_meter_num[m_current_deg+ASSIST_LINE_MAX_DEG]);
    
    // meter right
    // color buffer
    glBindBuffer(GL_ARRAY_BUFFER, m_right_meter_buffer);
    if (m_assist_change)
    {
        //m_assist_change = 0;
        glBufferSubData(GL_ARRAY_BUFFER, 0,
            m_front_cam_right_meter_num[m_current_deg+ASSIST_LINE_MAX_DEG]*7*sizeof(float),
            m_front_cam_right_meter_vert_arr[m_current_deg+ASSIST_LINE_MAX_DEG]);
    }        
    glVertexAttribPointer(m_loc_vertex_position, 3, GL_FLOAT, GL_FALSE,
        7 * sizeof(float), (GLvoid *)(0 * sizeof(float)));    
    glVertexAttribPointer(m_loc_color, 4, GL_FLOAT, GL_FALSE,
        7 * sizeof(float), (GLvoid *)(3 * sizeof(float)));
    
    glDrawArrays(GL_TRIANGLES, 0, m_front_cam_right_meter_num[m_current_deg+ASSIST_LINE_MAX_DEG]);
        
    return 0;
}


int OverlayDraw::assist_line_2d_process_rear()
{
    // color buffer
    glBindBuffer(GL_ARRAY_BUFFER, m_assist_line_color_buffer);
    glVertexAttribPointer(m_loc_color, 4, GL_FLOAT, GL_FALSE,
        4 * sizeof(float), (GLvoid *)(0 * sizeof(float)));
    // left vertex buffer
    glBindBuffer(GL_ARRAY_BUFFER, m_rear_wheel_left_assist_line_vert_buffer);
    if (m_assist_change)
    {
        glBufferSubData(GL_ARRAY_BUFFER, 0,
            m_assist_line_pair_cnt*2*3*sizeof(float),
            m_rear_cam_left_assist_line_vert_arr[m_current_deg+ASSIST_LINE_MAX_DEG]);
    }
    glVertexAttribPointer(m_loc_vertex_position, 3, GL_FLOAT, GL_FALSE,
        3 * sizeof(float), (GLvoid *)(0 * sizeof(float)));    

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 
        m_rear_cam_left_assist_line_num[m_current_deg+ASSIST_LINE_MAX_DEG]*2);

    // color buffer
    glBindBuffer(GL_ARRAY_BUFFER, m_assist_line_color_buffer);
    glVertexAttribPointer(m_loc_color, 4, GL_FLOAT, GL_FALSE,
        4 * sizeof(float), (GLvoid *)(0 * sizeof(float)));
    // right vertex buffer
    glBindBuffer(GL_ARRAY_BUFFER, m_rear_wheel_right_assist_line_vert_buffer);
    if (m_assist_change)
    {
        glBufferSubData(GL_ARRAY_BUFFER, 0,
            m_assist_line_pair_cnt*2*3*sizeof(float),
            m_rear_cam_right_assist_line_vert_arr[m_current_deg+ASSIST_LINE_MAX_DEG]);
    }
    glVertexAttribPointer(m_loc_vertex_position, 3, GL_FLOAT, GL_FALSE,
        3 * sizeof(float), (GLvoid *)(0 * sizeof(float)));    

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 
        m_rear_cam_right_assist_line_num[m_current_deg+ASSIST_LINE_MAX_DEG]*2);

    // meter left
    // color buffer
    glBindBuffer(GL_ARRAY_BUFFER, m_left_meter_buffer);
    if (m_assist_change)
    {
        //m_assist_change = 0;
        glBufferSubData(GL_ARRAY_BUFFER, 0,
            m_rear_cam_left_meter_num[m_current_deg+ASSIST_LINE_MAX_DEG]*7*sizeof(float),
            m_rear_cam_left_meter_vert_arr[m_current_deg+ASSIST_LINE_MAX_DEG]);
    }        
    glVertexAttribPointer(m_loc_vertex_position, 3, GL_FLOAT, GL_FALSE,
        7 * sizeof(float), (GLvoid *)(0 * sizeof(float)));    
    glVertexAttribPointer(m_loc_color, 4, GL_FLOAT, GL_FALSE,
        7 * sizeof(float), (GLvoid *)(3 * sizeof(float)));
    
    glDrawArrays(GL_TRIANGLES, 0, m_rear_cam_left_meter_num[m_current_deg+ASSIST_LINE_MAX_DEG]);

    // meter right
    // color buffer
    glBindBuffer(GL_ARRAY_BUFFER, m_right_meter_buffer);
    if (m_assist_change)
    {
        //m_assist_change = 0;
        glBufferSubData(GL_ARRAY_BUFFER, 0,
            m_rear_cam_right_meter_num[m_current_deg+ASSIST_LINE_MAX_DEG]*7*sizeof(float),
            m_rear_cam_right_meter_vert_arr[m_current_deg+ASSIST_LINE_MAX_DEG]);
    }        
    glVertexAttribPointer(m_loc_vertex_position, 3, GL_FLOAT, GL_FALSE,
        7 * sizeof(float), (GLvoid *)(0 * sizeof(float)));    
    glVertexAttribPointer(m_loc_color, 4, GL_FLOAT, GL_FALSE,
        7 * sizeof(float), (GLvoid *)(3 * sizeof(float)));
    
    glDrawArrays(GL_TRIANGLES, 0, m_rear_cam_right_meter_num[m_current_deg+ASSIST_LINE_MAX_DEG]);
    
    return 0;
}


int OverlayDraw::assist_line_2d_process(int is_front)
{
    glEnableVertexAttribArray(m_loc_vertex_position);
    glEnableVertexAttribArray(m_loc_color);

    // in 2d space, we do not use module matrix for rotate, and not use view matrix
    glm::mat4 mvp = m_assist_line_p; // * m_assist_line_v * m_assist_line_m;
    glUniformMatrix4fv(m_loc_mvp, 1, GL_FALSE, glm::value_ptr(mvp));

    if (is_front)
    {
        assist_line_2d_process_front();
    }
    else
    {
        assist_line_2d_process_rear();
    }

    // fot test
#if 0
    static int s_flag = 0;
    static const GLfloat s_vertex_buffer_data[] = {
       -1.0f, -1.0f, 0.0f,
       1.0f, -1.0f, 0.0f,
       0.0f,  1.0f, 0.0f,
    };
    static const GLfloat s_color_buffer_data[] = {
       1.0f, 1.0f, 1.0f, 1.0f,
       1.0f, 1.0f, 1.0f, 1.0f,
       1.0f, 1.0f, 1.0f, 1.0f
    };
    GLuint s_vertexbuffer;
    GLuint s_colorbuffer;
    if (s_flag == 0)
    {
        glGenBuffers(1, &s_vertexbuffer);
        glBindBuffer(GL_ARRAY_BUFFER, s_vertexbuffer);
        glBufferData(GL_ARRAY_BUFFER, sizeof(s_vertex_buffer_data), s_vertex_buffer_data, GL_STATIC_DRAW);

        glGenBuffers(1, &s_colorbuffer);
        glBindBuffer(GL_ARRAY_BUFFER, s_colorbuffer);
        glBufferData(GL_ARRAY_BUFFER, sizeof(s_color_buffer_data), s_color_buffer_data, GL_STATIC_DRAW);
    }
    glBindBuffer(GL_ARRAY_BUFFER, s_vertexbuffer);
    glVertexAttribPointer(m_loc_vertex_position,3,GL_FLOAT,GL_FALSE,0,(void*)0);
    glBindBuffer(GL_ARRAY_BUFFER, s_colorbuffer);
    glVertexAttribPointer(s_colorbuffer,4,GL_FLOAT,GL_FALSE,0,(void*)0);
    glDrawArrays(GL_TRIANGLES, 0, 3);
#endif

    if (m_assist_change)
    {
        m_assist_change = 0;
    }

    return 0;
}

/**
* @param space 0 for 3d stitch space, 1 for front, 2 for rear camera
*/
int OverlayDraw::assist_line_process(int space)
{
    if (m_assist_line_load != OVERLAY_VBO_LOADDED)
    {
        AVM_ERR("OverlayDraw::assist_line_process. assist line data not load or vbo\n");
        return 1;
    }
    if (0 == m_is_assist_show)
    {
        return 0;
    }

    if (space == 0)
    {
        assist_line_3d_process();
    }
    else if (space == 1)
    {
        assist_line_2d_process(1);
    }
    else
    {
        assist_line_2d_process(0);
    }

    return 0;    
}


void OverlayDraw::set_assist_line_rotate(float angle)
{
    m_assist_line_m = glm::mat4(1.0f);
    m_assist_line_m = glm::rotate(m_assist_line_m, glm::radians(angle), glm::vec3(0, 1, 0));
    m_assist_line_m = glm::rotate(m_assist_line_m, glm::radians(-90.0f), glm::vec3(1, 0, 0));
    m_assist_line_m = glm::rotate(m_assist_line_m, glm::radians(180.0f), glm::vec3(0, 0, 1));   
}


void OverlayDraw::set_assist_line_angle(float ang)
{
    if (m_current_deg != ang)
    {
        m_assist_change++;
    }
    m_current_deg = ang;
}

void OverlayDraw::set_assist_line_force_update()
{
    m_assist_change++;
}


void OverlayDraw::overlay_render_start()
{
    glUseProgram(shader_get_program(m_shader_handler));
}


void OverlayDraw::overlay_render_end()
{

}


int OverlayDraw::load_obj_file(const char *p_obj_file_name, int point_dim, obj_info_s *p_info)
{
    int i;
    char line[1024];

    if ((NULL == p_obj_file_name) || (NULL == p_info) || (point_dim <= 0))
    {
        AVM_ERR("OverlayDraw::load_obj_file. param error\n");
        return 1;
    }
    p_info->p_obj_file_name = p_obj_file_name;
    p_info->total_point_num = 0;
    p_info->point_dim = point_dim;
    p_info->min_x = 999.0f;
    p_info->min_y = 999.0f;
    p_info->min_z = 999.0f;
    p_info->max_x = -999.0f;
    p_info->max_y = -999.0f;
    p_info->max_z = -999.0f;    
    p_info->idv_obj_num = 0;

    // read file
    FILE *fp = NULL;
    fp = fopen(p_info->p_obj_file_name, "r");
    if (NULL == fp)
    {
        AVM_ERR("OverlayDraw::load_obj_file. open obj file faild\n");
        return 1;
    }
    
    // get how many idv object in this file, and how many face in obj file
    int obj_num = 0;
    int total_face_num = 0;
    fgets(line, 1024, fp);
    while (!(feof(fp)))
    {
        // get object start flag
        if ((line[0] == 'o') && (line[1] == ' '))
        {
            obj_num++;
        }
        else if ((line[0] == 'f') && (line[1] == ' '))
        {
            total_face_num++;
        }
        fgets(line, 1024, fp);
    }
    if ((obj_num <= 0) || (total_face_num <= 0))
    {
        AVM_ERR("OverlayDraw::load_obj_file. obj num or total face num error\n");
        return 1;
    }
    p_info->idv_obj_num = obj_num;
    
    // alloc struct mem, and alloc vert point mem
    p_info->p_idv_obj_arr = (idv_obj_s *)memalign(128, p_info->idv_obj_num * sizeof(idv_obj_s));
    if (NULL == p_info->p_idv_obj_arr)
    {
        AVM_ERR("OverlayDraw::load_obj_file. malloc idv obj arr faild\n");
        return 1;
    }  
    int point_cnt = (total_face_num * 3) * point_dim;
    int malloc_mem_size = point_cnt * sizeof(float);
    float *p_vert_total_mem = (float *)memalign(128, malloc_mem_size);
    if (NULL == p_vert_total_mem)
    {
        AVM_ERR("OverlayDraw::load_obj_file. malloc vert total mem faild\n");
        return 1;
    }

    // alloc tmp mem, and read all vert point to tmp mem
    float f_x, f_y, f_z;
    float *p_temp = (float *)memalign(128, malloc_mem_size);
    float *p_temp_start = p_temp;
    if (NULL == p_temp)
    {
        AVM_ERR("OverlayDraw::load_obj_file. malloc temp mem faild\n");
        return 1;
    }
    fseek(fp, 0, SEEK_SET);
    fgets(line, 1024, fp);
    while (!(feof(fp)))
    {
        if ((line[0] == 'v') && (line[1] == ' '))
        {
            char tmp_char[16];
            sscanf(line, "%s %f %f %f", tmp_char, &f_x, &f_y, &f_z);
            *(p_temp_start + 0) = f_x;
            *(p_temp_start + 1) = f_y;
            *(p_temp_start + 2) = f_z;
            
            if (f_x > p_info->max_x) p_info->max_x = f_x;
            if (f_x < p_info->min_x) p_info->min_x = f_x;
            
            if (f_y > p_info->max_y) p_info->max_y = f_y;
            if (f_y < p_info->min_y) p_info->min_y = f_y;
            
            if (f_z > p_info->max_z) p_info->max_z = f_z;
            if (f_z < p_info->min_z) p_info->min_z = f_z;

            p_temp_start += 3;
        }
        fgets(line, 1024, fp);
    }

    // set all object's vert point data
    idv_obj_s *p_cur_obj = NULL;
    fseek(fp, 0, SEEK_SET);
    fgets(line, 1024, fp);
    while (!(feof(fp)))
    {
        // find a single obj start flag
        if ((line[0] == 'o') && (line[1] == ' '))
        {
            char tmp_char[16];
            if (NULL == p_cur_obj)
            {
                p_cur_obj = &((p_info->p_idv_obj_arr)[0]);
            }
            else
            {
                p_cur_obj++;
            }
            sscanf(line, "%s %s", tmp_char, p_cur_obj->name);
            p_cur_obj->vert_num = 0;
            p_cur_obj->p_vert_arr = p_vert_total_mem;
        }
        else if ((line[0] == 'f') && (line[1] == ' '))
        {
            char tmp_char[16];
            int point[3];
            sscanf(line, "%s %d/%s %d/%s %d/%s", tmp_char,
                &(point[0]), tmp_char,
                &(point[1]), tmp_char,
                &(point[2]), tmp_char);

            for (i = 0; i < 3; i++)
            {
                *(p_vert_total_mem + 0) = p_temp[(point[i] - 1) * 3 + 0];
                *(p_vert_total_mem + 1) = p_temp[(point[i] - 1) * 3 + 1];
                *(p_vert_total_mem + 2) = p_temp[(point[i] - 1) * 3 + 2];
                p_vert_total_mem += point_dim;
            }
            p_cur_obj->vert_num += 3;
            p_info->total_point_num += 3;
        }
        fgets(line, 1024, fp);
    }

    // free tmp mem
    free(p_temp);
    fclose(fp);

    AVM_LOG("OverlayDraw::load_obj_file. min_x=%.2f,min_y=%.2f,min_z=%.2f\n",
        p_info->min_x, p_info->min_y, p_info->min_z);
    AVM_LOG("OverlayDraw::load_obj_file. max_x=%.2f,max_y=%.2f,max_z=%.2f\n",
        p_info->max_x, p_info->max_y, p_info->max_z);

#if 0
    // print result
    int j;
    printf("obj file name=%s, idv_obj_num=%d\n", p_info->p_obj_file_name, p_info->idv_obj_num);
    for (i = 0; i < p_info->idv_obj_num; i++)
    {
        printf("obj[%d], name=%s, vert_num=%d\n", 
            i, 
            p_info->p_idv_obj_arr[i].name, 
            p_info->p_idv_obj_arr[i].vert_num);
        for (j = 0; j < p_info->p_idv_obj_arr[i].vert_num; j++)
        {
            printf("%f %f %f\n", 
                p_info->p_idv_obj_arr[i].p_vert_arr[point_dim * j], 
                p_info->p_idv_obj_arr[i].p_vert_arr[point_dim * j + 1], 
                p_info->p_idv_obj_arr[i].p_vert_arr[point_dim * j + 2]);
        }
    }
#endif

    return 0;
}


int OverlayDraw::trans_obj(obj_info_s *p_info, 
    float scale_val, float dst_w, float dst_h, float det, float z_val)
{   
    // calc scale value
    float scx = 1.0f;
    float scy = 1.0f;
    if (scale_val > 0.0f)
    {
        scx = scale_val;
        scy = scale_val;
    }
    else
    {
        float width = p_info->max_x - p_info->min_x;
        float height = p_info->max_y - p_info->min_y;
        if (width <= 0.0f) width = 1.0f;
        if (height <= 0.0f) height = 1.0f;
        
        if (dst_w > 0.0f)
        {
            scx = dst_w / width;
        }
        if (dst_h >= 0.0f)
        {
            scy = dst_h / height;
        }
    }

    // scale
    int i, j;
    for (i = 0; i < p_info->idv_obj_num; i++)
    {
        for (j = 0; j < p_info->p_idv_obj_arr[i].vert_num; j++)
        {
            float *p_vert = &(p_info->p_idv_obj_arr[i].p_vert_arr[j*p_info->point_dim]);

            p_vert[0] = p_vert[0] * scx;
            p_vert[1] = p_vert[1] * scy + det;
            p_vert[2] = z_val;   // 
        }
    }
    
    return 0;
}




