/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the QtCore module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>

#include <QApplication>
#include <QLabel>
#include <QSurfaceFormat>
#include <QThread>
#include <QSplashScreen>
#include <fstream>

#include "common_def.h"
#include "main_widget.h"
#include "video_adapt.h"
#include "memlog.h"
#include "comu_cmd.h"
//#include "minIni.h"
#include "ini_config.h"
#include "custom_str.h"


// if main init is faild
// 0 is ok, others is error num code
static int g_main_init_faild = 0;


void set_main_init_faild(int err_num)
{
    g_main_init_faild = err_num;
}


int get_main_init_faild()
{
    return g_main_init_faild;
}


static int log_init()
{
    int ret;

    // log system 
    ret = memlog_server_init();
    if (ret != 0)
    {
        AVM_ERR("memlog_server_init faild. ret=%d\n", ret);
        return 1;
    }

    // read log config
    long ret_val;
    long def_log_to_console = 1;
    long def_log_to_shm = 1;
    ret_val = ini_get_log_to_console();
    if (-1 == ret_val)
    {
        AVM_ERR("log_init. read 'log_to_console' faild. use default value\n");
    }
    else
    {
        def_log_to_console = ret_val;
    }
    ret_val = ini_get_log_to_shm();
    if (-1 == ret_val)
    {
        AVM_ERR("log_init. read 'log_to_shm' faild. use default value\n");
    }
    else
    {
        def_log_to_shm = ret_val;
    }

    // set config
    memlog_server_set_prt_console(def_log_to_console);
    memlog_server_set_prt_shm(def_log_to_shm);

    AVM_LOG("log_init. server log_to_console=%ld, log_to_shm=%ld\n",
        def_log_to_console, def_log_to_shm);

    return 0;
}


static void get_process_name(const pid_t pid, char *name)
{
    char procfile[256];
    sprintf(procfile, "/proc/%d/cmdline", pid);
    FILE *f = fopen(procfile, "r");
    if (f)
    {
        size_t size;
        size = fread(name, sizeof(char), sizeof(procfile), f);
        if (size > 0)
        {
            if ('\n' == name[size - 1])
            {
                name[size - 1] = '\0';
            }
        }
        fclose(f);
    }
}


static int check_all_calib_file_exist()
{
    int err_num = 0;
    
    const char *db_str[9] = {
        "avm_qt_app_res/data/assist_line_calib_front.db",
        "avm_qt_app_res/data/assist_line_calib_rear.db",
        "avm_qt_app_res/data/assist_line_front.db",
        "avm_qt_app_res/data/assist_line_rear.db",
        "avm_qt_app_res/data/block_line_front.db",
        "avm_qt_app_res/data/block_line_rear.db",
        "avm_qt_app_res/data/fisheye_clip.db",
        "avm_qt_app_res/data/Table.db",
        "avm_qt_app_res/data/table_cam_two.db"
    };
    const char *calib_res_str[9] = {
        "avm_calib_res/assist_line_calib_front.db",
        "avm_calib_res/assist_line_calib_rear.db",
        "avm_calib_res/assist_line_front.db",
        "avm_calib_res/assist_line_rear.db",
        "avm_calib_res/block_line_front.db",
        "avm_calib_res/block_line_rear.db",
        "avm_calib_res/fisheye_clip.db",
        "avm_calib_res/Table.db",
        "avm_calib_res/table_cam_two.db"
    };    
    for (int i = 0; i < 9; i++)
    {
        // db file is in data dir, ok
        if (true == QFile::exists(db_str[i]))
        {
            continue;
        }
        // if file is not exist, check calib_res dir, find file and cppy to data dir
        if (true == QFile::exists(calib_res_str[i]))
        {
            QFile::copy(calib_res_str[i], db_str[i]);
            AVM_LOG("check_all_calib_file_exist. \"%s\" is not exist, but copy success\n", db_str[i]);
            continue;
        }
        AVM_LOG("check_all_calib_file_exist. \"%s\" is not exist\n", db_str[i]);
        err_num++;
    }

    return err_num;
}


//<* set factory_calib ok flag in shm memory
int set_factory_ok_shm_flag()
{
    int calib_ok = 0;

    // read calib_factory.param file
    std::ifstream in_file("avm_calib_res/calib_factory.param", std::ios::binary);
    if (in_file.is_open())
    {
        param_file_s file_st;
        in_file.read((char *)(&file_st), sizeof(file_st));
        calib_ok = file_st.calib_ok;
    }
    else
    {
        AVM_ERR("set_factory_ok_shm_flag. read calib_factory.param faild\n");
    }
    in_file.close();

    // set shm flag
    unsigned char *p_mem = NULL;
    int memsize = 0;
    comucmd_get_ext_share_mem(&p_mem, &memsize);  
    if (NULL == p_mem)
    {
        AVM_ERR("set_factory_ok_shm_flag. get ext shar mem faild\n");
        return 1;
    }
    ext_share_mem_s *p_share = (ext_share_mem_s *)p_mem;
    p_share->factory_calib_ok = calib_ok;
    AVM_LOG("set_factory_ok_shm_flag. set shm calib_ok=%d\n", calib_ok);

    return 0;
}


int main(int argc, char *argv[])
{
    int ret;

    // check config file, ensure all config
    
    // it is often more convenient to set the format for all windows once at the start of the application
    QSurfaceFormat format;
    format.setDepthBufferSize(24);
    // Anti-aliasing
    format.setSamples(4);
    QSurfaceFormat::setDefaultFormat(format);

    // The QApplication class manages the GUI application's control flow and main settings.
    QApplication app(argc, argv);
    app.setApplicationName("avm_qt_app");
    app.setApplicationVersion("0.1");

    // set language
    int country = 1;
    custstr_set_country(country);
    if (1 == country)
    {
        QFont font;
        //font.setFamily("simhei");
        font.setFamily("MYingHei_18030_C2-Medium");
        app.setFont(font);
    }
    QThread::currentThread()->setPriority(QThread::HighPriority);

    // create splash image and show
    QPixmap pixmap("avm_qt_app_res/ui/start_page.png");
    QSplashScreen splash(pixmap);
    splash.show();

    // log init, alloc shm mem
    ret = log_init();
    if (ret != 0)
    {
        AVM_ERR("log_init faild. ret=%d\n", ret);
        set_main_init_faild(1);
        //return 1;
    }

    // int comu salve, alloc shm mem
    ret = comucmd_slave_init();
    if (ret != 0)
    {
        AVM_ERR("comucmd_slave_init faild. ret=%d\n", ret);
        set_main_init_faild(2);
        //return 1;
    }

    // log shm and comu shm has been alloced and inited, 
    // send a signal to 'avm_comu', notify it can start using these two modules
    char parent_name[256];
    pid_t parent_pid = getppid();
    get_process_name(parent_pid, parent_name);
    if (strstr(parent_name, "avm_comu") != NULL)
    {
        kill(getppid(), SIGUSR1);   // use kill function to send signal
    }

    // set factory_calib ok flag in shm memory
    // avm_comu can get this value through shm and set DTC
    // 'calib_factory.calib' file is not exist in avm release package by default, so default this DTC is set
    // or, factory calib is faild, this DTC also set
    set_factory_ok_shm_flag();

    // in desktop app, we do not need to read capture video, but use video file or image file instead
#if GLOBAL_RUN_ENV_DESKTOP == 0
    // video capture and video sync etc.
    int input_video_width = 0;
    int input_video_height = 0;
    ret = video_adapt(&input_video_width, &input_video_height);
    if (ret != 0)
    {
        AVM_ERR("video_adapt faild. ret=%d\n", ret);
        set_main_init_faild(3);
        //return 1;
    }
#else
    int input_video_width = 1280;
    int input_video_height = 720;
#endif

    // check all calib db file is not missimg
    if (check_all_calib_file_exist() != 0)
    {
        AVM_ERR("check_all_calib_file_exist faild. ret=%d\n", ret);
        set_main_init_faild(4);
    }

#if GLOBAL_RUN_ENV_DESKTOP == 1
    MainWidget mainwidget(input_video_width, input_video_height);
    mainwidget.setGeometry(0, 0, 1280, 720);
    mainwidget.layout();
    mainwidget.control();
    mainwidget.show();
#else
    MainWidget mainwidget(input_video_width, input_video_height);
    // Window flags are a combination of a type (e.g. Qt::Dialog) and zero or more hints to the window system
    mainwidget.setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
    // This property holds the geometry of the widget relative to its parent and excluding the window frame.
    mainwidget.setGeometry(0, 0, 1280, 720);
    // Shows the widget and its child widgets.
    mainwidget.layout();
    mainwidget.control();
    mainwidget.show();
#endif

    // hide splash when main widget show
    splash.finish(&mainwidget);

    // It is necessary to call this function to start event handling
    return app.exec();
}

