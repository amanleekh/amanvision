
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "version.h"
//#include "minIni.h"
#include "ini_config.h"


void ver_get_date(char **p_char)
{
    static int s_flag = 0;
    static char s_str[128] = {'\0'};

    if (0 == s_flag)
    {
        s_flag = 1;
        //snprintf(s_str, 128, "%s %s\n", __DATE__, __TIME__);
        snprintf(s_str, 128, "%s\n", __DATE__);
    }

    if (p_char != NULL)
    {
        *p_char = s_str;
    }
}

void ver_get_version(char **p_char, char **p_char2, char **p_char3, char **p_char4)
{
#define VER_STR_MAX_NUM (64)
    static int s_flag = 0;
    static char s_str[VER_STR_MAX_NUM] = {'\0'};
    static char s_str1[VER_STR_MAX_NUM] = {'\0'};
    static char s_str2[VER_STR_MAX_NUM] = {'\0'};
    static char s_str3[VER_STR_MAX_NUM] = {'\0'};

    if (0 == s_flag)
    {
        s_flag = 1;

        FILE *fp = fopen("version.txt", "r");
        if (fp != NULL)
        {
            int cnt = 0;
            char str[VER_STR_MAX_NUM];
            char name[VER_STR_MAX_NUM];
            
            while (!feof(fp))
            {
                fgets(str, VER_STR_MAX_NUM, fp);
                if (0 == cnt)
                {
                    sscanf(str, "%s %s", name, s_str);
                }
                else if (1 == cnt)
                {
                    sscanf(str, "%s %s", name, s_str1);
                }
                else if (2 == cnt)
                {
                    sscanf(str, "%s %s", name, s_str2);
                }
                else if (3 == cnt)
                {
                    sscanf(str, "%s %s", name, s_str3);
                }

                cnt++;
                if (cnt >= 4)
                {
                    break;
                }
            }
            fclose(fp);
        }
    }

    *p_char = s_str;
    *p_char2 = s_str1;
    *p_char3 = s_str2;
    *p_char4 = s_str3;
}

void ver_get_vp_ver(char **p_char)
{
	static int s_flag = 0;
	static char s_str[VER_STR_MAX_NUM] = {'\0'};

	if (0 == s_flag)
    {
        FILE *fp = fopen("vpsw_version.txt", "r");
        if (fp != NULL)
        {
            char str[VER_STR_MAX_NUM];
            //char name[VER_STR_MAX_NUM];
            
            fgets(str, VER_STR_MAX_NUM, fp);
            sscanf(str, "vp_version:%s", s_str);
			//printf("####VP:%s\n", s_str);

            fclose(fp);

			s_flag = 1;
        }
    }

    *p_char = s_str;
}
