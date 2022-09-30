/*
 * =====================================================================================
 *
 *       Filename:  debug_msg.c
 *
 *    Description:  调试相关
 *
 *        Version:  1.0
 *        Created:  09/21/2022 13:37:00 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  jc
 *        Company:
 *
 * =====================================================================================
 */

#define _GNU_SOURCE
#include <stdio.h>
#include <stdarg.h>
#include "debug_msg.h"
//#include "common.h"

/*最大调试等级*/
int g_debug_level = DBG_LEVEL_ERR;
/*调试输出目标*/
FILE* g_debug_fp = NULL;
/*调试错误输出目标*/
FILE* g_debug_fp_err = NULL;

#define NAMELEN 16

#ifdef DEBUG_PTHREAD_ID
long unsigned int g_pthread_info = 1;
#else
long unsigned int g_pthread_info = 0;
#endif



/*
 * 描述：
 *	设置调试等级
 * 参数：
 *	level：调试等级
 * 返回值：
 *	无返回值
 */
void set_debug_level(const int level)
{
    g_debug_level = level;

    if(level > DBG_LEVEL_DEBUG )
        g_debug_level = DBG_LEVEL_DEBUG;
    else if(level < DBG_LEVEL_EMERGE)
        g_debug_level = DBG_LEVEL_EMERGE;
}

/*
 * 描述：
 *	打开调试文件，并将文件句柄传递给g_debug_fp与g_debug_fp_err
 *	此函数线程非安全
 *	必须调用close_debug_file关闭调试文件
 * 参数：
 *	file：待打开的调试文件路径
 * 返回值：
 *	打开成功返回0，否则返回-1
 */
int open_debug_file(const char* path)
{
    FILE *fp = NULL;
    char date[128] = "";

    snprintf(date, strlen(path) + 2, "%s_", path);

    fp = fopen(date, "a+");
    if(fp == NULL) {
        perror("open file");
        return -1;
    }

    g_debug_fp = fp;
    g_debug_fp_err = fp;

    return 0;
}

/*
 * 描述：
 *	关闭调试文件
 *	此函数线程非安全
 * 参数：
 *	无参数
 * 返回值：
 *	无返回值
 */
void close_debug_file(void)
{
    int same_fp = 0;


    if(g_debug_fp && g_debug_fp != stdout) {
        same_fp = (g_debug_fp == g_debug_fp_err)?1:0;
        fclose(g_debug_fp);
        g_debug_fp = stdout;
    }

    if(g_debug_fp_err && g_debug_fp_err != stderr && !same_fp) {
        fclose(g_debug_fp_err);
        g_debug_fp_err = stderr;
    }
}

//pthread_getname_np()
static const inline char *log_get_level_text(int level)
{
    switch(level) {
    case DBG_LEVEL_EMERGE:
        return DBG_LEVEL_EMERGE_TEXT;
    case DBG_LEVEL_ALERT:
        return DBG_LEVEL_ALERT_TEXT;
    case DBG_LEVEL_CRIT:
        return DBG_LEVEL_CRIT_TEXT;
    case DBG_LEVEL_ERR:
        return DBG_LEVEL_ERR_TEXT;
    case DBG_LEVEL_WARNING:
        return DBG_LEVEL_WARNING_TEXT;
    case DBG_LEVEL_NOTICE:
        return DBG_LEVEL_NOTICE_TEXT;
    case DBG_LEVEL_INFO:
        return DBG_LEVEL_INFO_TEXT;
    case DBG_LEVEL_DEBUG:
        return DBG_LEVEL_DEBUG_TEXT;
    }
    return "NULL";
}

void log_debug(int level, FILE *fp, char *sfile, int line, const char *fmt, ...)
{
    if(g_debug_level >= (level) ) {
        struct timeval tv;
        struct tm *now = NULL;
        va_list args;
        char thread_name[NAMELEN];

        thread_name[0] = '\0';
        pthread_getname_np(pthread_self(), thread_name, NAMELEN);

        va_start(args, fmt);
        gettimeofday(&tv, NULL);

        now = localtime((time_t*)&tv.tv_sec);

        if(now)
            if(g_pthread_info) {
                fprintf(fp, "[%04d-%02d-%02d %02d:%02d:%02d.%06d][%s][%lu]%s %d ", 1900+now->tm_year,
                        now->tm_mon+1, now->tm_mday, now->tm_hour, now->tm_min, now->tm_sec, (int)tv.tv_usec, log_get_level_text(level), pthread_self(), sfile, line);
                vfprintf(fp, fmt, args);
                fprintf(fp, "\n");
            } else {
                fprintf(fp, "[%04d-%02d-%02d %02d:%02d:%02d.%06d][%s][%s]%s %d ", 1900+now->tm_year,
                        now->tm_mon+1, now->tm_mday, now->tm_hour, now->tm_min, now->tm_sec, (int)tv.tv_usec, thread_name, log_get_level_text(level), sfile, line);
                vfprintf(fp, fmt, args);
                fprintf(fp, "\n");
            } else if(g_pthread_info) {
            fprintf(fp, "[%s][%lu]%s %d \n", log_get_level_text(level), pthread_self(), sfile, line);
            vfprintf(fp, fmt, args);
            fprintf(fp, "\n");
        } else {
            fprintf(fp, "[%s]%s %d \n", log_get_level_text(level), sfile, line);
            vfprintf(fp, fmt, args);
            fprintf(fp, "\n");
        }
        fflush(fp);
        va_end(args);
    }
}


/*
 * 描述：
 *	以16进制打印一段buffer
 *
 * 参数：
 *	tip：打印抬头
 *	buf：待打印的buffer
 *	len：待打印的buffer的大小
 * 返回值：
 *	无返回值
 */
void buffer_dump(const char *tip, const void *buf, const unsigned int len)
{
    int i = 0;
    const unsigned char *p = buf;

    if(g_debug_fp == NULL)
        g_debug_fp = stdout;

    if(g_debug_level < DBG_LEVEL_INFO ) {
        return;
    }

    if(tip)
        fprintf(g_debug_fp, "#####BEGIN:%s,len:%d#####\n", tip,len);
    else
        fprintf(g_debug_fp, "#####BEGIN len:%d#####\n", len);
    for (i=0; i < len; i++) {
        fprintf(g_debug_fp, "%02x ", p[i]);

        if(i % 16 == 15)
            fprintf(g_debug_fp, "\n");
    }

    if(i % 16 != 0)
        fprintf(g_debug_fp, "\n");

    if(tip)
        fprintf(g_debug_fp, "##########END:%s#########\n",tip);
    else
        fprintf(g_debug_fp, "########## END ##########\n");

    fflush(g_debug_fp);
}


