#ifndef _REPORT_H_
#define _REPORT_H_
#include <time.h>

//工作线程统计
typedef struct _w_report {
    struct timeval start_time;
    struct timeval end_time;//完成一次完整收包时间
    int want_connect_num;//用户设定的连接
    int succ_connect_num;//连接成功数
    int fail_connect_num;
    int connecting_num;
    int cur_connect_num;//当前连接数
    int send_num;//发送的请求数
    int recv_num;//接收的响应数
    int send_err_num;//发送失败数量
    int send_size;//发送的数据大小
    int recv_size;//接收的数据大小
    int finished_client_num;

    int first_secend_send_num;//一秒内数据
    int first_secend_recv_num;//一秒内数据
} w_report_st;

//主线程统计
typedef struct _m_report {
    struct timeval start_time;
    double run_time;// 毫秒
    int want_client_num;//用户设定的连接
    int succ_connect_num;//连接成功数
    int fail_connect_num;
    int connecting_num;
    int cur_connect_num;//当前连接数
    int send_num;//发送的请求数
    int recv_num;//接收的响应数
    int send_err_num;//发送失败数量
    int send_size;//发送的数据大小
    int recv_size;//接收的数据大小
    double max_connect_rate;//最大连接速率
    double min_connect_rate;//最小连接速率
    double aver_connect_rate; //平均连接速率
    int finished_client_num;

    int first_secend_send_num;//一秒内数据
    int first_secend_recv_num;//一秒内数据
    double avg_tps;//平均连接速率

    int timeout;
    int success_ssl_auth;//ssl建链成功并且认证成功的数量
    w_report_st pwr[0];
} m_report_st;

inline int report_status(w_report_st *wr, int status);

#endif
