/*
 * =====================================================================================
 *
 *       Filename:  config.c
 *
 *    Description:
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
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "config.h"
#include "debug_msg.h"

static const char g_config_send_buf[] =
    "GET / HTTP/1.1\r\n"
    "User-Agent: Asura/0.1\r\n"
    "Host: %s:%d\r\n"
    "Connection: keep-alive\r\n"
    "Accept-Encoding: gzip, deflate\r\n"
    "Accept-Language: zh-CN,zh;q=0.9\r\n"
    "\r\n";


int read_send_data(config_st *conf, char *file)
{
    FILE *fp;

    /* 打开文件用于读写 */
    fp = fopen(file, "r");
    if (fp == NULL) {
        return -1;
    }

    conf->need_send_len = fread(conf->need_send_buf, ASR_SEND_MAXLEN, 1, fp);

    fclose(fp);

    return conf->need_send_len;
}

void usage(const char *path)
{
    const char *p = strrchr(path, '/');
    if(p)
        p++;
    else
        p = "asura";

    printf("Asura for performence.\n");
    printf("Usage %s ip port [OPTIONS].\n", p);
    printf("Options:\n");
    printf("	-n: the number of client (default:1w max:60w).\n");//发送总数
    printf("	-p: the number of client per second (default:1k).\n");//每秒发送数量
    printf("	-t: time of duration (default:10s).\n");//持续时间
    printf("	-f: need send data in file.\n");
    printf("	-w: worker num.\n");
    printf("	-s: client bind ip address.\n");
    printf("	-b: client bind interface.\n");
    printf("	-d: <level>: Special debug level, 0 to 7.\n");
    printf("	-h: Show this help information.\n");
    exit(0);
}

int config_arg_parser(int argc, char *argv[], config_st *conf)
{
    int opt = 0;
    char par_info[]="d:n:p:t:f:w:s:b:h";

    if (argc < 3) {
        //usage(argv[0]);
        //return -1;
    } else {
        // ip & port
        strcpy(conf->ip_str, argv[1]);
        conf->port = atoi(argv[2]);
    }

    while(-1 != (opt = getopt(argc, argv, par_info))) {
        switch(opt) {
        case 'd':
            set_debug_level(atoi(optarg));
            break;
        case 'n':
            conf->client_num = atoi(optarg);
            if (conf->client_num < 0 || conf->client_num > ASR_CLIENT_NUM_MAX) {
                usage(argv[0]);
            }
            break;
        case 'p':
            conf->per_client_num = atoi(optarg);
            break;
        case 't':
            conf->duration = atoi(optarg);
            break;
        case 'f':
            if (0 > read_send_data(conf, optarg)) {
                return -1;
            }
            break;
        case 'w':
            conf->thread_num = atoi(optarg);
            break;
        case 's':
            strcpy(conf->sip, optarg);
            break;
        case 'b':
            strcpy(conf->ifname, optarg);
            break;
        case 'h':
            usage(argv[0]);
        default:
            printf("invalid option:[%c]\n", opt);
            usage(argv[0]);
        }
    }

    return 0;
}

int config_init(int argc, char *argv[], config_st *conf)
{

    conf->client_num = ASR_CLIENT_NUM_DEFAULT;
    conf->per_client_num = ASR_PER_CLIENT_NUM_DEFAULT;
    conf->duration = ASR_DURATION_DEFAULT;

    if (0 > config_arg_parser(argc, argv, conf)) {
        return -1;
    }

    if (conf->thread_num == 0) {
        conf->thread_num = cm_get_cpu_num();
    }

    if (conf->ip_str[0] == '\0') {
        strcpy(conf->ip_str, "127.0.0.1");
    }
    if (conf->port == 0) {
        conf->port = 80;
    }

    if (conf->need_send_len == 0) {
        conf->need_send_len = sprintf(conf->need_send_buf, g_config_send_buf, conf->ip_str, conf->port);
    }

    DB_EMERGE("----connect:%s:%d thread_num:%d",	conf->ip_str, conf->port, conf->thread_num);
    DB_EMERGE("----client_num:%d per_client_num:%d",	conf->client_num, conf->per_client_num);
    return 0;
}