#ifndef _CONFIG_H_
#define _CONFIG_H_

#include "common.h"

#define ASR_CLIENT_NUM_MAX 60000
#define ASR_CLIENT_NUM_DEFAULT 10000
#define ASR_PER_CLIENT_NUM_DEFAULT 1000
#define ASR_DURATION_DEFAULT 10

#define ASR_AUDIT_PERIOD 1

#define ASR_IFNAMESIZE 16
#define ASR_SEND_MAXLEN 16*1024

typedef struct _config {
    int thread_num; //工作线程数

    char ip_str[IPV4_LEN];
    int port;

    int client_num; //总数
    int per_client_num; //每秒发送数
    int duration; //测试时间
    char ifname[ASR_IFNAMESIZE];
    char sip[MAX_STR_IP_SIZE];

    int need_send_len;
    char need_send_buf[ASR_SEND_MAXLEN];
} config_st;

int config_init(int argc, char *argv[], config_st *conf);
#endif
