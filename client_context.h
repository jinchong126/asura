#ifndef _CLIENT_CONTEXT_H_
#define _CLIENT_CONTEXT_H_

#include <ev.h>
#include <openssl/ssl.h>
#include <sys/time.h>

#define READ_MAXLEN (1024*16)

typedef enum _AUTH_MODE {
    AUTH_USER_PWD = 1,
    AUTH_KEEPALIVE,
    AUTH_LOGOUT
} AUTH_MODE;

typedef struct _client_context {
    struct ev_loop *loop;
    ev_io wio;
    ev_io rio;
    ev_io rio_handshake;
    ev_io wio_handshake;
    int ev_state_flg;

    SSL *ssl;
    int fd;

    int connect_status;

    int need_send_len;
    char *need_send_buf;

    int send_len;
    int send_times;

    //开始时间
    struct timeval start_time;
    //链接成功时间
    struct timeval connected_time;
    //最后接收响应时间
    struct timeval last_recv_time;
    struct timeval close_time;
    w_report_st *wr;
} client_ctx_st;

int cc_connect(client_ctx_st* ctx, char *ip_str, int port);
int cc_connected_handle(client_ctx_st* ctx);
int cc_send_data(client_ctx_st* ctx);
client_ctx_st* cc_client_init(struct ev_loop *loop, int do_num, char *sip, char *ifname, int send_len, char *send_buf, w_report_st *wr);
int cc_client_deinit(client_ctx_st *cli_ctx, int do_num);

#endif
