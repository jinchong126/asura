#ifndef _WORK_CLIENT_H_
#define _WORK_CLIENT_H_
#include <pthread.h>
#include <openssl/ssl.h>
#include <ev.h>
#include "report.h"
#include "config.h"
#include "client_context.h"

#define WC_RUN_TIMEOUT 1

/* 工作线程结构体 */
typedef struct _work_client {
    int 			id; /* 当前线程下标,0开始 */
    pthread_t		thread_id;
    pthread_key_t   key; /* 存放当前线程下标 */

    int 			times;/* 回调进入次数 */
    struct timeval  start_time;
    struct timeval  stop_time;

    w_report_st *wr;/*统计*/

    //客户端结构体
    int 			do_num;/* 分配的连接数 */
    int 			do_per_num;/* 分配的每秒连接数 */
    int 			done_num;/* 做了连接数,不保证正确链接 */
    client_ctx_st   *cli_ctx;

    SSL_CTX *ssl_ctx;
    int ssl_opt;

    struct ev_loop *loop;
    ev_timer start_timer;/* 链接运行定时器, 运行完成停止*/
    int ev_state_flg;

    config_st *config; //配置
    int need_send_len;
    void *need_send_buf;
} work_client_st;

#define WORKERS_SET_ID(key, id)		(pthread_setspecific((key), (void *)(id)))

void wc_run(config_st *conf, work_client_st *wc);
work_client_st *wc_init(config_st *conf, m_report_st *mr);
void wc_deinit(config_st *conf, work_client_st *pwc);

#endif
