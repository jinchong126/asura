/*
 * =====================================================================================
 *
 *       Filename:  client_context.c
 *
 *    Description:  客户端操作相关
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
#include <string.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "common.h"
#include "debug_msg.h"
#include "cJSON.h"
#include "sock.h"
#include "client_ssl.h"
#include "work_client.h"

int cc_connected_handle(client_ctx_st* ctx)
{
    w_report_st *wr = ctx->wr;

    DB_DEBUG("tcp connected");
    report_status(wr, SOCK_CONNECTED);
    EV_IO_STOP_SAFE(ctx->loop, &ctx->rio_handshake, ctx->ev_state_flg, COMM_READ_IO_CONN_MASK);
    EV_IO_STOP_SAFE(ctx->loop, &ctx->wio_handshake, ctx->ev_state_flg, COMM_WRITE_IO_CONN_MASK);

    EV_IO_START_SAFE(ctx->loop, &ctx->rio, ctx->ev_state_flg, COMM_READ_IO_MASK);
    EV_IO_START_SAFE(ctx->loop, &ctx->wio, ctx->ev_state_flg, COMM_WRITE_IO_MASK);
    return cc_send_data(ctx);
}

static void cc_check_tcp_connect_cb(struct ev_loop *loop, struct ev_io *w, int revents)
{
    int ret;
    client_ctx_st *ctx = (client_ctx_st *)w->data;
    w_report_st *wr = ctx->wr;

    ret = sock_connect_check(ctx->fd);
    switch (ret) {
    case SOCK_CONNECT_ERR:
        report_status(wr, ret);
        EV_IO_STOP_SAFE(loop, &ctx->rio_handshake, ctx->ev_state_flg, COMM_READ_IO_CONN_MASK);
        EV_IO_STOP_SAFE(loop, &ctx->wio_handshake, ctx->ev_state_flg, COMM_WRITE_IO_CONN_MASK);
        break;
    case SOCK_CONNECTING:
        break;
    case SOCK_CONNECTED:

#if 1
        // if(ctx->ssl) {
        //     DB_ERR("start ssl connect...");
        //     ret = sc_ssl_connect(ctx);
        //     if( ret == 0 ) {
        //         /* SSL连接成功 */
        //         protocol = SIM_SSL;
        //     }else if(ret < 0){
        //         sc_ssl_connect_error(ctx);
        //         return;
        //     }
        // }

        cc_connected_handle(ctx);
#endif
        break;
    default:
        break;
    }

    return;
}

static void cc_write_cb(struct ev_loop *loop, struct ev_io *w, int revents)
{
    client_ctx_st* ctx = (client_ctx_st*)w->data;
    cc_send_data(ctx);
    return;
}

static void cc_read_cb(struct ev_loop *loop, struct ev_io *w, int revents)
{
    int ret;
    struct timeval tv;
    char recv_buf[READ_MAXLEN] = {0};
    client_ctx_st* ctx = (client_ctx_st*)w->data;
    w_report_st *wr = ctx->wr;

    /* TODO 读取数据 */
    ret = sock_recv_buff(ctx->fd, recv_buf, READ_MAXLEN);
    if( ret < 0 ) {
        EV_IO_STOP_SAFE(loop, &(ctx->wio), ctx->ev_state_flg, COMM_WRITE_IO_MASK);
        EV_IO_STOP_SAFE(loop, &(ctx->rio), ctx->ev_state_flg, COMM_READ_IO_MASK);
        wr->cur_connect_num--;
        return;
    } else if (ret == 0) {
        return;
    }

    wr->recv_num++;
    wr->recv_size += ret;

    //一秒内成功数量
    time_getval(&tv);
    if (time_compare_ms(&ctx->start_time, &tv) <= 1000) {
        wr->first_secend_recv_num++;
    }

    DB_DEBUG("tcp recv success size:%d", ret);

    return;
}

int cc_connect(client_ctx_st* ctx, char *ip_str, int port)
{
    struct sockaddr_in addr;
    unsigned int addr_len = sizeof(addr);

    //只连接一次
    DB_DEBUG("cc_connect fd:%d connect_status:%d", ctx->fd, ctx->connect_status);
    if (ctx->fd <= 0 || ctx->connect_status != 0) {
        DB_ERR("cc_connect error");
        return -1;
    }
    addr.sin_family = AF_INET;
    inet_pton(AF_INET, ip_str, &addr.sin_addr.s_addr);
    addr.sin_port = htons(port);

    DB_DEBUG("cc_connect fd:%d", ctx->fd);
    ctx->connect_status = sock_connect(ctx->fd, (struct sockaddr*)&addr, addr_len);
    return ctx->connect_status;
}

int cc_send_data(client_ctx_st* ctx)
{
    int ret;
    struct timeval tv;
    w_report_st *wr = ctx->wr;

    if (ctx->send_len >= ctx->need_send_len) {
        // 发送完成
        EV_IO_STOP_SAFE(ctx->loop, &(ctx->wio), ctx->ev_state_flg, COMM_WRITE_IO_MASK);
        return 0;
    }

    ret = sock_safe_send_buff(ctx->fd, ctx->need_send_buf + ctx->send_len, ctx->need_send_len - ctx->send_len);
    if( ret < 0 ) {
        EV_IO_STOP_SAFE(ctx->loop, &(ctx->wio), ctx->ev_state_flg, COMM_WRITE_IO_MASK);
        EV_IO_STOP_SAFE(ctx->loop, &(ctx->rio), ctx->ev_state_flg, COMM_READ_IO_MASK);
        wr->cur_connect_num--;
        return -1;
    }

    ctx->send_len = ret;
    ctx->send_times++;

    wr->send_num++;
    wr->send_size += ret;

    //一秒内成功数量
    time_getval(&tv);
    if (time_compare_ms(&ctx->start_time, &tv) <= 1000) {
        wr->first_secend_send_num++;
    }

    DB_DEBUG("tcp send success size:%d", ret);

    return ret;
}

client_ctx_st* cc_client_init(struct ev_loop *loop, int do_num, char *sip, char *ifname, int send_len, char *send_buf, w_report_st *wr)
{
    int i;
    client_ctx_st *ctx, *pctx;

    pctx = (client_ctx_st *)malloc(sizeof(client_ctx_st) * do_num);
    if( !pctx ) {
        return NULL;
    }
    memset(pctx, 0, sizeof(client_ctx_st) * do_num);

    DB_DEBUG("ctx----------------:%p", pctx);

    for (i = 0; i < do_num; i++) {
        ctx = pctx + i;
        ctx->fd = sock_init(AF_INET, SOCK_STREAM, sip, ifname);
        if (0 > ctx->fd) {
            SAFE_FREE(pctx, free);
            DB_ERR("socket init error, ifname:%s success num:%d", ifname, i);
            return NULL;
        }

        DB_DEBUG("fd:%d", ctx->fd);
        ev_io_init(&ctx->rio, cc_read_cb, ctx->fd, EV_READ);
        ev_io_init(&ctx->wio, cc_write_cb, ctx->fd, EV_WRITE);
        ctx->wio.data = (void *)ctx;
        ctx->rio.data = (void *)ctx;

        ev_io_init(&ctx->rio_handshake, cc_check_tcp_connect_cb, ctx->fd, EV_READ);
        ev_io_init(&ctx->wio_handshake, cc_check_tcp_connect_cb, ctx->fd, EV_WRITE);
        ctx->rio_handshake.data = (void *)ctx;
        ctx->wio_handshake.data = (void *)ctx;

        ctx->loop = loop;
        ctx->need_send_len = send_len;
        ctx->need_send_buf = send_buf;
        ctx->wr = wr;
    }

    return pctx;
}

int cc_client_deinit(client_ctx_st *cli_ctx, int do_num)
{
    int i;
    client_ctx_st *ctx;

    for (i = 0; i < do_num; i++) {
        ctx = cli_ctx + i;
        sock_close(ctx->fd);
    }

    SAFE_FREE(cli_ctx, free);
    return 0;
}
