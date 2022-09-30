/*
 * =====================================================================================
 *
 *       Filename:  debug_msg.c
 *
 *    Description:  日志
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

#ifndef _DEBUG_MSG_H_
#define _DEBUG_MSG_H_

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <pthread.h>

/* 调试等级定义 */
#ifdef LOG_EMERG
#define DBG_LEVEL_EMERGE	LOG_EMERG
#define DBG_LEVEL_ALERT		LOG_ALERT
#define DBG_LEVEL_CRIT		LOG_CRIT
#define DBG_LEVEL_ERR		LOG_ERR
#define DBG_LEVEL_WARNING	LOG_WARNING
#define DBG_LEVEL_NOTICE	LOG_NOTICE
#define DBG_LEVEL_INFO		LOG_INFO
#define DBG_LEVEL_DEBUG		LOG_DEBUG
#else
#define DBG_LEVEL_EMERGE	0
#define DBG_LEVEL_ALERT		1
#define DBG_LEVEL_CRIT		2
#define DBG_LEVEL_ERR		3
#define DBG_LEVEL_WARNING	4
#define DBG_LEVEL_NOTICE	5
#define DBG_LEVEL_INFO		6
#define DBG_LEVEL_DEBUG		7
#endif

#define DBG_LEVEL_EMERGE_TEXT	"EMERGE"
#define DBG_LEVEL_ALERT_TEXT	"ALERT"
#define DBG_LEVEL_CRIT_TEXT		"CRIT"
#define DBG_LEVEL_ERR_TEXT		"ERROR"
#define DBG_LEVEL_WARNING_TEXT	"WARNING"
#define DBG_LEVEL_NOTICE_TEXT	"NOTICE"
#define DBG_LEVEL_INFO_TEXT		"INFO"
#define DBG_LEVEL_DEBUG_TEXT	"DEBUG"


extern long unsigned int g_pthread_info;

const char *get_dev_sn(void);

/*最大调试等级*/
extern int g_debug_level;
/*调试普通信息输出目标*/
extern FILE* g_debug_fp_err;
/*调试错误信息输出目标*/
extern FILE* g_debug_fp;

#define __FILENAME__	(strrchr(__FILE__, '/')?strrchr(__FILE__, '/')+1:__FILE__)

#define dmsg(fmt, ARGS...)	printf("%s %d:"fmt"\n", __FILENAME__, __LINE__,##ARGS);

/*打印调试信息，不建议直接调用该接口*/
#define DBG(level, fp, fmt, ARGS...)	 do{	\
				if(g_debug_level >= (level) )	\
				{	\
					struct timeval tv;	\
					struct tm *now = NULL;	\
					gettimeofday(&tv, NULL);	\
					now=localtime((time_t*)&tv.tv_sec);	\
					if(now)			\
						if(g_pthread_info) \
							fprintf(fp, "[%04d-%02d-%02d %02d:%02d:%02d.%06d][%s][%lu]%s %d:"fmt"\n", 1900+now->tm_year,	\
								now->tm_mon+1, now->tm_mday, now->tm_hour, now->tm_min, now->tm_sec, (int)tv.tv_usec, level##_TEXT, pthread_self(), __FILENAME__, __LINE__,##ARGS);	\
						else \
							fprintf(fp, "[%04d-%02d-%02d %02d:%02d:%02d.%06d][%s]%s %d:"fmt"\n", 1900+now->tm_year,	\
								now->tm_mon+1, now->tm_mday, now->tm_hour, now->tm_min, now->tm_sec, (int)tv.tv_usec, level##_TEXT, __FILENAME__, __LINE__,##ARGS);	\
					else	\
						if(g_pthread_info) \
							fprintf(fp, "[%s][%lu]%s %d:"fmt"\n", level##_TEXT, pthread_self(), __FILENAME__, __LINE__,##ARGS);	\
						else \
							fprintf(fp, "[%s]%s %d:"fmt"\n", level##_TEXT, __FILENAME__, __LINE__,##ARGS);	\
					fflush(fp);	\
				}	\
			}while(0)

/* 打印不同等级的调试信息 */
#if 0
#define DB_EMERGE(fmt, ARGS...)	DBG(DBG_LEVEL_EMERGE, g_debug_fp_err?g_debug_fp_err:stderr, fmt, ##ARGS)
#define DB_ALERT(fmt, ARGS...)	DBG(DBG_LEVEL_ALERT, g_debug_fp_err?g_debug_fp_err:stderr, fmt, ##ARGS)
#define DB_CRIT(fmt, ARGS...)	DBG(DBG_LEVEL_CRIT, g_debug_fp_err?g_debug_fp_err:stderr, fmt, ##ARGS)
#define DB_ERR(fmt, ARGS...)	DBG(DBG_LEVEL_ERR, g_debug_fp_err?g_debug_fp_err:stderr, fmt, ##ARGS)
#define DB_WAR(fmt, ARGS...)	DBG(DBG_LEVEL_WARNING, g_debug_fp_err?g_debug_fp_err:stderr, fmt, ##ARGS)
#define DB_NOTICE(fmt, ARGS...)	DBG(DBG_LEVEL_NOTICE, g_debug_fp?g_debug_fp:stdout, fmt, ##ARGS)
#define DB_INFO(fmt, ARGS...)	DBG(DBG_LEVEL_INFO, g_debug_fp?g_debug_fp:stdout, fmt, ##ARGS)
#define DB_DEBUG(fmt, ARGS...)	DBG(DBG_LEVEL_DEBUG, g_debug_fp?g_debug_fp:stdout, fmt, ##ARGS)
#else
#define DB_EMERGE(fmt, ARGS...)	log_debug(DBG_LEVEL_EMERGE, g_debug_fp_err?g_debug_fp_err:stderr, __FILENAME__, __LINE__, fmt, ##ARGS)
#define DB_ALERT(fmt, ARGS...)	log_debug(DBG_LEVEL_ALERT, g_debug_fp_err?g_debug_fp_err:stderr, __FILENAME__, __LINE__, fmt, ##ARGS)
#define DB_CRIT(fmt, ARGS...)	log_debug(DBG_LEVEL_CRIT, g_debug_fp_err?g_debug_fp_err:stderr, __FILENAME__, __LINE__, fmt, ##ARGS)
#define DB_ERR(fmt, ARGS...)	log_debug(DBG_LEVEL_ERR, g_debug_fp_err?g_debug_fp_err:stderr, __FILENAME__, __LINE__, fmt, ##ARGS)
#define DB_WAR(fmt, ARGS...)	log_debug(DBG_LEVEL_WARNING, g_debug_fp_err?g_debug_fp_err:stderr, __FILENAME__, __LINE__, fmt, ##ARGS)
#define DB_NOTICE(fmt, ARGS...)	log_debug(DBG_LEVEL_NOTICE, g_debug_fp?g_debug_fp:stdout, __FILENAME__, __LINE__, fmt, ##ARGS)
#define DB_INFO(fmt, ARGS...)	log_debug(DBG_LEVEL_INFO, g_debug_fp?g_debug_fp:stdout, __FILENAME__, __LINE__, fmt, ##ARGS)
#define DB_DEBUG(fmt, ARGS...)	log_debug(DBG_LEVEL_DEBUG, g_debug_fp?g_debug_fp:stdout, __FILENAME__, __LINE__, fmt, ##ARGS)
#endif

/*
 * 描述：
 *	设置调试等级
 * 参数：
 *	level：调试等级
 * 返回值：
 *	无返回值
 */
void set_debug_level(const int level);

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
int open_debug_file(const char* path);

void log_debug(int level, FILE *fp, char *sfile, int line, const char *fmt, ...);

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
void buffer_dump(const char *tip, const void *buf, const unsigned int len);

/*
 * 描述：
 *	关闭调试文件
 *	此函数线程非安全
 * 参数：
 *	无参数
 * 返回值：
 *	无返回值
 */
void close_debug_file(void);
#endif /*_DEBUG_MSG_H_*/
