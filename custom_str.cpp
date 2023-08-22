
#include "custom_str.h"
#include "avm_gpu.h"

static int g_country = 0;

int custstr_set_country(int is_chinese)
{
    g_country = is_chinese;

    return 0;
}

QString custstr_get_view_str(int view_enum)
{
    if (1 == g_country)
    {
        if (view_enum == AVM_VIEW_AUTO)
        {
            return "鸟瞰";
        }
        else if (view_enum == AVM_VIEW_3D_FRONT)
        {
            return "3D前";
        }
        else if (view_enum == AVM_VIEW_3D_REAR)
        {
            return "3D后";
        }
        else if (view_enum == AVM_VIEW_3D_LEFT)
        {
            return "3D左后";
        }
        else if (view_enum == AVM_VIEW_3D_RIGHT)
        {
            return "3D右后";
        }
        else if (view_enum == AVM_VIEW_2D_FRONT)
        {
            return "2D前";
        }
        else if (view_enum == AVM_VIEW_2D_REAR)
        {
            return "2D后";
        }
        else if (view_enum == AVM_VIEW_CAMERA_FRONT)
        {
            return "前";
        }
        else if (view_enum == AVM_VIEW_CAMERA_REAR)
        {
            return "后";
        }
        else if ((view_enum == AVM_VIEW_CAMERA_LEFTRIGHT)
            || (view_enum == AVM_VIEW_CAMERA_LEFTRIGHT_CPY))
        {
            return "左右";
        }
        else if (view_enum == AVM_VIEW_3D_LEFT_FRONT)
        {
            return "3D左前";
        }
        else if (view_enum == AVM_VIEW_3D_LEFT_CENTER)
        {
            return "3D左";
        }
        else if (view_enum == AVM_VIEW_3D_RIGHT_FRONT)
        {
            return "3D右前";
        }
        else if (view_enum == AVM_VIEW_3D_RIGHT_CENTER)
        {
            return "3D右";
        }
        else if (view_enum == AVM_VIEW_CAMERA_FISH_FRONT)
        {
            return "前广角";
        }
        else if (view_enum == AVM_VIEW_CAMERA_FISH_REAR)
        {
            return "后广角";
        }
        else if (view_enum == AVM_VIEW_CAMERA_FISH_CLIP_FRONT)
        {
            return "前";
        }
        else if (view_enum == AVM_VIEW_CAMERA_FISH_CLIP_REAR)
        {
            return "后";
        }
        else
        {
            return "未定";
        }
    }
    else
    {
        if (view_enum == AVM_VIEW_AUTO)
        {
            return "BIRDS-VIEW";
        }
        else if (view_enum == AVM_VIEW_3D_FRONT)
        {
            return "3D FRONT";
        }
        else if (view_enum == AVM_VIEW_3D_REAR)
        {
            return "3D RAER";
        }
        else if (view_enum == AVM_VIEW_3D_LEFT)
        {
            return "3D LEFT-REAR";
        }
        else if (view_enum == AVM_VIEW_3D_RIGHT)
        {
            return "3D RIGHT-REAR";
        }
        else if (view_enum == AVM_VIEW_2D_FRONT)
        {
            return "2D FRONT";
        }
        else if (view_enum == AVM_VIEW_2D_REAR)
        {
            return "2D REAR";
        }
        else if (view_enum == AVM_VIEW_CAMERA_FRONT)
        {
            return "FRONT";
        }
        else if (view_enum == AVM_VIEW_CAMERA_REAR)
        {
            return "REAR";
        }
        else if ((view_enum == AVM_VIEW_CAMERA_LEFTRIGHT)
            || (view_enum == AVM_VIEW_CAMERA_LEFTRIGHT_CPY))
        {
            return "LEFT-RIGHT";
        }
        else if (view_enum == AVM_VIEW_3D_LEFT_FRONT)
        {
            return "3D LEFT-FRONT";
        }
        else if (view_enum == AVM_VIEW_3D_LEFT_CENTER)
        {
            return "3D LEFT";
        }
        else if (view_enum == AVM_VIEW_3D_RIGHT_FRONT)
        {
            return "3D RIGHT-FRONT";
        }
        else if (view_enum == AVM_VIEW_3D_RIGHT_CENTER)
        {
            return "3D RIGHT";
        }
        else if (view_enum == AVM_VIEW_CAMERA_FISH_FRONT)
        {
            return "FISHEYE-FRONT";
        }
        else if (view_enum == AVM_VIEW_CAMERA_FISH_REAR)
        {
            return "FISHEYE-REAR";
        }
        else if (view_enum == AVM_VIEW_CAMERA_FISH_CLIP_FRONT)
        {
            return "FRONT";
        }
        else if (view_enum == AVM_VIEW_CAMERA_FISH_CLIP_REAR)
        {
            return "REAR";
        }
        else
        {
            return "UNKNOW";
        }
    }
}


QString custstr_get_auto()
{
    if (1 == g_country)
    {
        return "自动：";
    }
    else
    {
        return "AUTO:";
    }
}

QString custstr_get_manual()
{
    if (1 == g_country)
    {
        return "手动：";
    }
    else
    {
        return "MANUAL:";
    }
}

QString custstr_get_assist_str()
{
    if (1 == g_country)
    {
        return "辅助线";
    }
    else
    {
        return "ASSIST-LINE";
    }
}

QString custstr_get_radar_str()
{
    if (1 == g_country)
    {
        return "雷达";
    }
    else
    {
        return "RADAR";
    }
}

QString custstr_get_radio_str()
{
    if (1 == g_country)
    {
        return "静音";
    }
    else
    {
        return "MUTE";
    }
}

QString custstr_get_speed_alert_str()
{
    if (1 == g_country)
    {
        return "车速过快，无法显示全景影像";
    }
    else
    {
        return "speeding";
    }
}

QString custstr_get_cam_err_alert_str()
{
    if (1 == g_country)
    {
        return "泊车摄像头未响应";
    }
    else
    {
        return "DEVICE ERROR";
    }
}

QString custstr_get_version_alert_str()
{
    if (1 == g_country)
    {
        return "内部版本错误";
    }
    else
    {
        return "INNER VERSION ERROR";
    }
}

QString custstr_get_init_alert_str(int err_num)
{
    if (1 == g_country)
    {
        return QString("初始化错误（错误号：") + QString::number(err_num) + QString("）");
    }
    else
    {
        return QString("INIT FAILD(CODE:") + QString::number(err_num) + QString(")");
    }
}

QString custstr_get_alert_ok_str()
{
    if (1 == g_country)
    {
        return "知道了";
    }
    else
    {
        return "OK";
    }
}


QString custstr_get_3dpie_str(int idx)
{
    if ((idx < 0) || (idx > 8))
    {
        return NULL;
    }
    const char *p_str_chn[] = {
        "3D顶视",
        "右视",
        "右前",
        "前视",
        "左前",
        "左视",
        "左后",
        "后视",
        "右后"
    };
    const char *p_str_eng[] = {
        "3D-TOP",
        "RIGHT",
        "RIGHT-FRONT",
        "FRONT",
        "LEFT-FRONT",
        "LEFT",
        "LEFT-REAR",
        "REAR",
        "RIGHT-REAR",
        "RIGHT"
    };
    if (1 == g_country)
    {
        return QString(p_str_chn[idx]);
    }
    else
    {
        return QString(p_str_eng[idx]);
    }
}


QString custstr_get_2dpie_str(int idx)
{
    if ((idx < 0) || (idx > 5))
    {
        return NULL;
    }
    const char *p_str_chn[] = {
        "",
        "侧视图",
        "前视图",
        "前广角",
        "后视图",
        "后广角"
    };
    const char *p_str_eng[] = {
        "",
        "LEFT-RIGHT",
        "FRONT",
        "FISHEYE-FRONT",
        "REAR",
        "FISHEYE-REAR"
    };
    if (1 == g_country)
    {
        return QString(p_str_chn[idx]);
    }
    else
    {
        return QString(p_str_eng[idx]);
    }
}

QString custstr_get_careful_str()
{
    if (1 == g_country)
    {
        return "欢迎使用全景影像，请检查四周情况以确保安全";
    }
    else
    {
        return "Please be careful";
    }
}

QString custstr_get_remd_manual_3d_str()
{
    if (1 == g_country)
    {
        return "点击选择视角或滑动切换视角";
    }
    else
    {
        return "Press to change view";
    }
}

QString custstr_get_remd_manual_2d_str()
{
    if (1 == g_country)
    {
        return "点击选择视角";
    }
    else
    {
        return "Press to change view";
    }
}


QString custstr_get_remd_mute_str()
{
    if (1 == g_country)
    {
        return "暂时不能更改静音状态";
    }
    else
    {
        return "unable to change the mute state";
    }
}


QString custstr_get_errinfo_title()
{
    if (1 == g_country)
    {
        return "前视 后视 左视 右视";
    }
    else
    {
        return "F   B   L   R";
    }
}


// value
// 0 ok
// 1 disconnect
// 2 bad
// 3 unknow
QString custstr_get_err_dyn_info(int front, int rear, int left, int right)
{
    QString status[4] = {
        QString("正常 "),
        QString("断开 "),
        QString("损坏 "),
        QString("未知 ")
    };
    QString status_eng[4] = {
        QString("OK  "),
        QString("CON "),
        QString("BAD "),
        QString("UNK ")
    };

#define CUST_STR_LIMIT(val)    \
    if (val < 0 || val > 3)    \
    {                          \
        val = 3;               \
    }

    CUST_STR_LIMIT(front)
    CUST_STR_LIMIT(rear)
    CUST_STR_LIMIT(left)
    CUST_STR_LIMIT(right)

    if (1 == g_country)
    {
        return status[front] + status[rear] + status[left] + status[right];
    }
    else
    {
        return status_eng[front] + status_eng[rear] + status_eng[left] + status_eng[right];
    }
}




