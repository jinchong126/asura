/*
 * =====================================================================================
 *
 *       Filename:  work_client.c
 *
 *    Description:  工作线程相关
 *
 *        Version:  1.0
 *        Created:  09/22/2022 13:37:00 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  jc
 *        Company:
 *
 * =====================================================================================
 */
#define _GNU_SOURCE
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "common.h"
#include "debug_msg.h"
#include "sock.h"
#include "config.h"
#include "work_client.h"

void wc_run_cb(struct ev_loop *loop, struct ev_timer *w, int revents)
{
    int ret, i;
    struct timeval tv;
    struct tm *tm;
    work_client_st *wc = (work_client_st *)w->data;
    w_report_st *wr = wc->wr;
    config_st *conf = wc->config;
    client_ctx_st *ctx;

    // 回调执行次数
    wc->times++;

    DB_INFO("wc_run_cb done_num:%d, do_num:%d", wc->done_num, wc->do_num);
    if(wc->done_num >= wc->do_num) {
        DB_INFO("wc_run_cb timer stop");
        EV_TIMER_STOP_SAFE(loop, w, wc->ev_state_flg, COMM_TIMER_IO_MASK);
    }

    for(i = 0; i < wc->do_per_num; i++ ) {
        if (wc->done_num >= wc->do_num) {
            EV_TIMER_STOP_SAFE(loop, w, wc->ev_state_flg, COMM_TIMER_IO_MASK);
            break;
        }
        ctx = wc->cli_ctx + wc->done_num;
        time_getval(&ctx->start_time);

        wr->connecting_num++;
        ret = cc_connect(ctx, conf->ip_str, conf->port);
        if (ret == SOCK_CONNECT_ERR) {
            report_status(wr, ret);
        } else if (ret == SOCK_CONNECTED) {
            cc_connected_handle(ctx);
        } else {
            //connecting
            EV_IO_START_SAFE(loop, &ctx->rio_handshake, ctx->ev_state_flg, COMM_READ_IO_CONN_MASK);
            EV_IO_START_SAFE(loop, &ctx->wio_handshake, ctx->ev_state_flg, COMM_WRITE_IO_CONN_MASK);
        }

        wc->done_num += 1;
    }

    if (wc->times == 1) {
        time_getval(&tv);
        tm = localtime((time_t*)&tv);
        DB_EMERGE("wc_thread  start, wc:%p, id:%d send ok connect:%d[%02d:%02d:%02d.%06u] spend:%.fms",
                  wc, wc->id, wc->done_num, tm->tm_hour, tm->tm_min, tm->tm_sec, (tv.tv_usec), time_compare_ms(&wc->start_time, &tv));

    }
}

void cpu_thread_bind(work_client_st *wc)
{
    cpu_set_t mask;
    cpu_set_t get;

    CPU_ZERO(&mask);
    CPU_SET(wc->id, &mask);

    if (pthread_setaffinity_np(pthread_self(), sizeof(mask), &mask) < 0) {
        fprintf(stderr, "set thread affinity failed\n");
    }

    CPU_ZERO(&get);

    if (pthread_getaffinity_np(pthread_self(), sizeof(get), &get) < 0) {
        fprintf(stderr, "get thread affinity failed\n");
    }

    if (CPU_ISSET(wc->id, &get)) {
        printf("thread %lu is running in processor %d\n", (long unsigned)pthread_self(),wc->id);
    }
}

static void *wc_thread(void *arg)
{
    struct tm *tm;
    work_client_st *wc = (work_client_st *)arg;
    config_st *conf = wc->config;
    w_report_st *wr = wc->wr;

    DB_DEBUG("wc_thread start, wc:%p, id:%d loop:%p", wc, wc->id, wc->loop);
    if (conf->thread_num <= cm_get_cpu_num()) {
        cpu_thread_bind(wc);
    }

    //每秒运行一次
    ev_timer_init(&wc->start_timer, wc_run_cb, WC_RUN_TIMEOUT, WC_RUN_TIMEOUT);
    wc->start_timer.data = (void *)wc;

    wr->want_connect_num = wc->do_num;
    time_getval(&wr->start_time);
    time_getval(&wc->start_time);

    tm = localtime((time_t*)&wc->start_time);
    DB_EMERGE("wc_thread start, wc:%p, id:%d loop:%p start[%02d:%02d:%02d.%06d]",
              wc, wc->id, wc->loop, tm->tm_hour, tm->tm_min, tm->tm_sec, wc->start_time.tv_usec);

    //首次运行
    wc_run_cb(wc->loop, &wc->start_timer, 0);

    EV_TIMER_START_SAFE(wc->loop, &wc->start_timer, wc->ev_state_flg, COMM_TIMER_IO_MASK);
    ev_run(wc->loop, 0);
    return NULL;
}

static void wc_create_thread(work_client_st *wc)
{
    int ret;
    char name[16];
    pthread_attr_t attr;
    pthread_attr_init(&attr);

    DB_DEBUG("worker id:%d wc p:%p ctx:%p", wc->id, wc, wc->cli_ctx);
    ret = pthread_create(&wc->thread_id, &attr, wc_thread, (void*)wc);
    if (ret != 0) {
        perror("Failed to create thread.\n");
        exit(1);
    }
    name[0] = '\0';
    sprintf(name, "work%d", wc->id);
    pthread_setname_np(wc->thread_id, name);
    //DB_DEBUG("create worker id:%d thread_id:%lu", item->id, item->thread_id);
    return;
}

int wc_ctx_init(config_st *conf, work_client_st *wc)
{
    //余数
    int remainder;

    remainder = conf->client_num % conf->thread_num;
    wc->do_num = conf->client_num/conf->thread_num;
    if (remainder > wc->id) {
        wc->do_num += 1;
    }

    remainder = conf->per_client_num % conf->thread_num;
    wc->do_per_num = conf->per_client_num/conf->thread_num;
    if (remainder > wc->id) {
        wc->do_per_num += 1;
    }

    DB_EMERGE("worker id:%d do_num:%d do_per_num:%d wc:%p ", wc->id, wc->do_num, wc->do_per_num, wc);

    wc->cli_ctx = cc_client_init(wc->loop, wc->do_num, conf->sip, conf->ifname, conf->need_send_len, conf->need_send_buf, wc->wr);
    if (wc->cli_ctx == NULL) {
        return -1;
    }

    return 0;
}

int wc_ctx_deinit(work_client_st *wc)
{
    int ret;
    ret = cc_client_deinit(wc->cli_ctx, wc->do_num);
    return ret;
}

void wc_run(config_st *conf, work_client_st *wc)
{
    int i;

    DB_DEBUG("create worker num:%d wc:%p", conf->thread_num, wc);
    for (i = 0; i < conf->thread_num; i++) {
        wc_create_thread(wc+i);
    }

    return;
}

work_client_st *wc_init(config_st *conf, m_report_st *mr)
{
    int i, ret;
    work_client_st *wc, *pwc;

    pwc = (work_client_st *)malloc(sizeof(work_client_st) * conf->client_num);
    DB_DEBUG("create worker pwc:%p", pwc);
    if( !pwc ) {
        return NULL;
    }
    memset(pwc, 0, sizeof(work_client_st) * conf->client_num);

    for (i = 0; i < conf->thread_num; i++) {
        wc = pwc + i;
        wc->id = i;
        wc->loop = ev_loop_new(EVBACKEND_EPOLL);
        wc->config = conf;
        wc->need_send_len = conf->need_send_len;
        wc->need_send_buf = conf->need_send_buf;

        wc->wr = mr->pwr + i;
        ret = wc_ctx_init(conf, wc);
        if( 0 > ret ) {
            SAFE_FREE(pwc, free);
            return NULL;
        }
    }

    return pwc;
}

void wc_deinit(config_st *conf, work_client_st *pwc)
{
    int i;
    work_client_st *wc;

    for (i = 0; i < conf->thread_num; i++) {
        wc = pwc + i;
        ev_loop_destroy(wc->loop);
        wc_ctx_deinit(wc);
    }

    SAFE_FREE(pwc, free);
}
