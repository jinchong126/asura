/*
 * =====================================================================================
 *
 *       Filename:  asura.c
 *
 *    Description:  主程序
 *
 *        Version:  1.0
 *        Created:  09/21/2022 11:37:34 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  jc
 *        Company:
 *
 * =====================================================================================
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "report.h"
#include "debug_msg.h"
#include "work_client.h"
#include "asura.h"

void asr_reset_item_report(m_report_st *mr)
{
    mr->succ_connect_num = 0;
    mr->fail_connect_num = 0;
    mr->connecting_num = 0;
    mr->cur_connect_num = 0;

    mr->send_num = 0;
    mr->recv_num = 0;

    mr->send_err_num = 0;
    mr->send_size = 0;
    mr->recv_size = 0;

    mr->first_secend_send_num = 0;
    mr->first_secend_recv_num = 0;
}

void asr_collect_item_report(m_report_st *mr, w_report_st *wr, int tnum)
{
    int i;
    struct timeval tv;
    double tatal_time = 0;
    w_report_st *p;

    time_getval(&tv);
    for( i = 0; i < tnum; i++ ) {
        p = wr + i;
        mr->succ_connect_num += p->succ_connect_num;
        mr->fail_connect_num += p->fail_connect_num;
        mr->connecting_num += p->connecting_num;
        mr->cur_connect_num += p->cur_connect_num;

        mr->send_num += p->send_num;
        mr->recv_num += p->recv_num;

        mr->send_err_num += p->send_err_num;
        mr->send_size += p->send_size;
        mr->recv_size += p->recv_size;

        mr->first_secend_send_num += p->first_secend_send_num;
        mr->first_secend_recv_num += p->first_secend_recv_num;

        tatal_time += time_compare_us(&p->start_time, &tv);

        DB_EMERGE("work:----%-2d Connect All:%6d S:%6d F:%6d Cur:%6d Ing:%6d "
                  "send_num:%6d recv_num:%6d ",
                  i,
                  p->want_connect_num, p->succ_connect_num, p->fail_connect_num,
                  p->cur_connect_num, p->connecting_num,
                  p->send_num, p->recv_num);
        DB_DEBUG("work:----%-2d send_num:%6d send_size:%12d recv_num:%6d recv_size:%12d",
                 i,
                 p->send_num, p->send_size, p->recv_num, p->recv_size);
    }

    mr->avg_tps = (double)mr->recv_num/(tatal_time/1000000/tnum);

    DB_EMERGE("anum:%6d Connect All:%6d S:%6d F:%6d Cur:%6d Ing:%6d "
              "send_num:%6d recv_num:%6d ",
              mr->want_client_num, mr->want_client_num, mr->succ_connect_num, mr->fail_connect_num,
              mr->cur_connect_num, mr->connecting_num,
              mr->send_num, mr->recv_num);
    DB_DEBUG("main:------ send_num:%6d send_size:%12d recv_num:%6d recv_size:%12d",
             mr->send_num, mr->send_size, mr->recv_num, mr->recv_size);

}

void asr_report_handler(asura_main_st *am)
{
    struct timeval tv;
    double tmp_conn_rate = 0;

    config_st *conf = &am->config;
    m_report_st *mr = am->report;

    time_getval(&tv);

    asr_reset_item_report(mr);
    asr_collect_item_report(mr, mr->pwr, conf->thread_num);

    mr->run_time = time_compare_ms(&mr->start_time, &tv);

    tmp_conn_rate = (double)mr->succ_connect_num / (mr->run_time/1000);

    mr->max_connect_rate = mr->max_connect_rate > tmp_conn_rate ? mr->max_connect_rate : tmp_conn_rate;
    mr->min_connect_rate = mr->min_connect_rate < tmp_conn_rate ? mr->min_connect_rate : tmp_conn_rate;
}

void asr_report_timer_cb(struct ev_loop *loop, struct ev_timer *w, int revents)
{
    asura_main_st *am = (asura_main_st *)w->data;

    asr_report_handler(am);
}

void asr_print_report_result(m_report_st *mr)
{
    if (mr == NULL) {
        return ;
    }
    DB_EMERGE("=============report=============");
    DB_EMERGE("Want client num:%d", mr->want_client_num);
    DB_EMERGE("Connected num:%d", mr->succ_connect_num);
    DB_EMERGE("Connect err num:%d", mr->fail_connect_num);
    DB_EMERGE("Current connect num:%d", mr->cur_connect_num);
    DB_EMERGE("Sent request num:%d", mr->send_num);
    DB_EMERGE("Recv response num:%d", mr->recv_num);
    DB_EMERGE("Max connect rate:%.2lf/s", mr->max_connect_rate);
    DB_EMERGE("Min connect rate:%.2lf/s", mr->min_connect_rate);
    DB_EMERGE("Average connect rate:%.2lf/s", mr->aver_connect_rate);
    DB_EMERGE("First second send:%d recv:%d Average:%.2f/s",
              mr->first_secend_send_num, mr->first_secend_recv_num,
              (double)(mr->first_secend_send_num + mr->first_secend_recv_num)/2);
    DB_EMERGE("Average tps:%.2f/s", mr->avg_tps);
    DB_EMERGE("Run times:%.2lfs",mr->run_time/1000);
    DB_EMERGE("Success ssl auth num:%d", mr->success_ssl_auth);
}

void asr_print_report_result_int_cb(EV_P_ ev_signal *w, int revents)
{
    asura_main_st *am = (asura_main_st *)w->data;
    m_report_st *mr = am->report;
    asr_report_handler(am);

    mr->aver_connect_rate = mr->succ_connect_num/(mr->run_time/1000);

    asr_print_report_result(mr);
    exit(0);
}

asura_main_st *am_init(int argc, char *argv[])
{
    int ret, report_size;
    asura_main_st *am;
    config_st *conf;
    m_report_st *mr;

    am = malloc(sizeof(asura_main_st));
    if(!am) {
        return NULL;
    }

    memset(am, 0, sizeof(asura_main_st));
    conf = &am->config;

    ret = config_init(argc, argv, conf);
    if (ret < 0) {
        SAFE_FREE(am, free);
        return NULL;
    }
    am->loop = ev_loop_new(EVBACKEND_SELECT);

    ev_signal_init(&am->signal_exit, asr_print_report_result_int_cb, SIGINT);
    am->signal_exit.data = (void *)am;

    ev_timer_init(&am->report_timer, asr_report_timer_cb, ASR_AUDIT_PERIOD, ASR_AUDIT_PERIOD);
    am->report_timer.data = (void *)am;

    ev_timer_init(&am->stop_timer, asr_print_report_result_int_cb, conf->duration, 0);
    am->stop_timer.data = (void *)am;

    report_size = sizeof(m_report_st) + sizeof(w_report_st) * conf->thread_num;
    //am->report = (m_report_st *)malloc(report_size);
    am->report = malloc(report_size);
    if( !am->report ) {
        SAFE_FREE(am, free);
        return NULL;
    }

    mr = am->report;
    DB_DEBUG("m_report:%p, w_report:%p",  am->report, mr->pwr);
    memset(am->report, 0, report_size);
    mr->want_client_num = conf->client_num;

    return am;
}

void am_deinit(asura_main_st *am)
{
    SAFE_FREE(am->report, free);

    ev_loop_destroy(am->loop);

    SAFE_FREE(am, free);
}

void asr_wait_wc_thread_start(asura_main_st *am)
{
    int i, j;
    m_report_st *mr = am->report;
    config_st *conf = &am->config;
    w_report_st *p, *wr;

    p = mr->pwr;

    mr->min_connect_rate = 0xefffffff;

    while( 1 ) {
        usleep(10*1000);
        for( i = 0, j = 0; i < conf->thread_num; i++ ) {
            wr = p + i;
            if( wr->start_time.tv_sec != 0 ) {
                j++;
            }
        }
        if (j == i) {
            break;
        }
    }

    EV_TIMER_START_SAFE(am->loop, &am->report_timer, am->ev_state_flg, COMM_TIMER_IO_MASK);
    ev_signal_start(am->loop, &am->signal_exit);
    EV_TIMER_START_SAFE(am->loop, &am->stop_timer, am->ev_state_flg, COMM_TIMER2_IO_MASK);

    time_getval(&mr->start_time);
}

int main(int argc, char *argv[])
{
    asura_main_st *am;
    work_client_st *wc;

    /* 主线程结构体初始化 */
    am = am_init(argc, argv);
    if( !am )
        exit(1);

    /* 工作线程结构体初始化 */
    wc = wc_init(&am->config, am->report);
    if( !wc )
        exit(1);

    DB_DEBUG("create worker wc:%p", wc);
    wc_run(&am->config, wc);

    asr_wait_wc_thread_start(am);

    ev_run(am->loop, 0);

    // 未实现优雅退出
    ev_loop_destroy(am->loop);
    DB_EMERGE("main exit");
    return 0;
}
