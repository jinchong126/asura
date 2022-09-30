#ifndef _ASURA_H_
#define _ASURA_H_
#include <pthread.h>
#include <ev.h>
#include "config.h"

typedef struct _asura_main {
    struct ev_loop *loop;
    ev_signal signal_exit; //退出信号
    ev_timer report_timer; //定时打印
    ev_timer stop_timer; //退出定时
    int ev_state_flg;

    config_st config;

    //统计 m_report + n*w_report
    void *report;
} asura_main_st;

#endif
