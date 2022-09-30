/*
 * =====================================================================================
 *
 *       Filename:  sock.h
 *
 *    Description:  socket操作相关头文件
 *
 *        Version:  1.0
 *        Created:  2019年11月27日 03时13分46秒
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  jc
 *        Company:
 *
 * =====================================================================================
 */
#ifndef __SOCK_H__
#define __SOCK_H__

#include <sys/types.h>
#include <sys/socket.h>
#include <ev.h>
#include <stdint.h>

enum {
    SOCK_TYPE_TCP = 1,
    SOCK_TYPE_UDP
};

enum {
    SOCK_CONNECT_ERR = -1,
    SOCK_CONNECTED = 1,
    SOCK_CONNECTING
};

int sock_nonblocking (int fd);

int sock_blocking (int fd);

int sock_connect(int fd, struct sockaddr *addr, socklen_t addr_len);
int sock_connect_check(int fd);

int sock_accept(int in_fd, struct sockaddr *addr, socklen_t *addr_len);

int sock_server_init(int family, int sock_type, char *ifname, char *ipstr, uint16_t port);

int sock_send_buff(int fd, void *buf, size_t len);
int sock_recv_buff(int fd, void *buf, size_t len);

size_t sock_safe_recv_buff(int fd, void *buffer, int count);
size_t sock_safe_send_buff(int fd, const void *buf, int count);

int sock_init(int family, int sock_type, char *ip, char *ifname);
int sock_close(int fd);

#endif  // __SOCK_H__

