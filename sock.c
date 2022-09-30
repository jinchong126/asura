/*
 * =====================================================================================
 *
 *	   Filename:  sock.c
 *
 *	Description:  socket相关操作
 *
 *		Version:  1.0
 *		Created:  2022年09月20日 02时25分09秒
 *	   Revision:  none
 *	   Compiler:  gcc
 *
 *		 Author:  jc
 *		Company:
 *
 * =====================================================================================
 */
#include <stdio.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdlib.h>
#include <stdint.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <sys/un.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <ifaddrs.h>

#include "sock.h"
#include "debug_msg.h"
#include <netinet/tcp.h>


/*
 * Set the socket to non blocking -rjkaes
 */
int sock_nonblocking (int fd)
{
    int flags;

    flags = fcntl (fd, F_GETFL, 0);
    return fcntl (fd, F_SETFL, flags | O_NONBLOCK);
}

/*
 * Set the socket to blocking -rjkaes
 */
int sock_blocking (int fd)
{
    int flags;

    flags = fcntl (fd, F_GETFL, 0);
    return fcntl (fd, F_SETFL, flags & ~O_NONBLOCK);
}

/* 成功返回socket fd */
int sock_init(int family, int sock_type, char *ipstr, char *ifname)
{
    int fd;
    int reuse = 1;
    struct ifreq ifr;

    //创建socket
    fd = socket(family, sock_type, 0);
    if (-1 == fd) {
        DB_ERR("socket error, errno:%d", errno);
        return -1;
    }

    if (ifname != NULL && strlen(ifname) > 0) {
        memset(&ifr, 0x00, sizeof(ifr));
        strncpy(ifr.ifr_name, ifname, sizeof(ifr.ifr_name));
        if (-1 == setsockopt(fd, SOL_SOCKET, SO_BINDTODEVICE, (char *)&ifr, sizeof(ifr))) {
            DB_ERR("socket bind device error errno:%d", errno);
            close(fd);
            return -1;
        }
    }
    if (ipstr != NULL && strlen(ipstr) > 0) {
        struct sockaddr_in bindaddr;
        bindaddr.sin_family = family;
        bindaddr.sin_addr.s_addr = inet_addr(ipstr);
        bindaddr.sin_port = htons(0);
        if(-1 == bind(fd, (struct sockaddr *)&bindaddr, sizeof(bindaddr))) {
            DB_ERR("socket bind ip %s error errno:%d", ipstr, errno);
            close(fd);
            return -1;
        }
    }

    //设置立即释放端口并可以再次使用
    if (-1 == setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse))) {
        DB_ERR("socket set reuseaddr error errno:%d", errno);
        close(fd);
        return -1;
    }

    //设置为非阻塞
    if(sock_nonblocking (fd) < 0) {
        DB_ERR("socket set nonblocking error errno:%s",  strerror(errno));
        close(fd);
        return -1;
    }

    return fd;
}

/* socket connect */
int sock_connect(int fd, struct sockaddr *addr, socklen_t addr_len)
{
    int ret;

    if(fd <= 0) {
        DB_ERR("socket connect error fd:%d", fd);
        return -1;
    }

    //connect连接
    ret = connect(fd, addr, addr_len);
    if (ret < 0 && errno != EINPROGRESS) {
        DB_ERR("socket %d connect error errno:%d", fd, errno);
        return -1;
    }

    if (errno == EINPROGRESS) {
        return SOCK_CONNECTING;
    }

    return SOCK_CONNECTED;
}

int sock_connect_check(int fd)
{
    int ret;
    int optval = -1;
    int optlen = sizeof(optval);

    ret = getsockopt(fd, SOL_SOCKET, SO_ERROR, &optval, (socklen_t*)&optlen);
    if ( ret < 0 ) {
        DB_ERR("get sock opt error:%s!", strerror(errno));
        return SOCK_CONNECT_ERR;
    }

    if( optval != 0) {
        if( errno != EINPROGRESS ) {
            DB_ERR("get sock opt error:%s!", strerror(errno));
            return SOCK_CONNECT_ERR;
        } else
            return SOCK_CONNECTING;
    }

    DB_INFO("tcp connect to server successful!");
    return SOCK_CONNECTED;
}

/* socket accept */
int sock_accept(int in_fd, struct sockaddr *addr, socklen_t *addr_len)
{
    int fd;

    if(in_fd <= 0) {
        return -1;
    }

    //accept连接
    fd = accept(in_fd, addr, addr_len);
    if (-1 == fd) {
        DB_ERR("socket accept error errno:%d", errno);
        return -1;
    }
    return fd;
}


int sock_close(int fd)
{
    if (fd > 0) {
        close(fd);
    }
    return 0;
}

//------------------------------------------------

/* 成功返回socket fd */
int sock_server_init(int family, int sock_type, char *ifname, char *ipstr, uint16_t port)
{
    int fd;
    int reuse = 1;
    struct sockaddr_in addr;
    struct sockaddr_in6 addr6;

    fd = sock_init(family, sock_type, NULL, ifname);
    if(fd < 0) {
        close(fd);
        return -1;
    }

    if( family == AF_INET ) {
        memset(&addr, 0, sizeof(addr));
        addr.sin_family = family;
        addr.sin_port = htons(port);
        DB_INFO("bind ip:%s port:%d", ipstr == NULL?"0.0.0.0":ipstr, port);
        if (NULL == ipstr) {
            addr.sin_addr.s_addr = htonl(INADDR_ANY);
        } else {
            addr.sin_addr.s_addr = inet_addr(ipstr);
        }

        //绑定监听
        if (-1 == bind(fd, (struct sockaddr *)&addr, sizeof(addr))) {
            DB_ERR("socket bind error errno:%s", strerror(errno));
            close(fd);
            return -1;
        }
    } else {
        memset(&addr6, 0, sizeof(addr6));
        addr6.sin6_family = family;
        addr6.sin6_port = htons(port);
        DB_INFO("bind ip:%s port:%d", ipstr == NULL?"::0":ipstr, port);
        addr6.sin6_addr = in6addr_any;

        if (-1 == setsockopt(fd, IPPROTO_IPV6, IPV6_V6ONLY, &reuse, sizeof(reuse))) {
            DB_ERR("socket set ipv6 only error errno:%s", strerror(errno));
            close(fd);
            return -1;
        }

        //绑定监听
        if (-1 == bind(fd, (struct sockaddr *)&addr6, sizeof(addr6))) {
            DB_ERR("socket bind error errno:%s", strerror(errno));
            close(fd);
            return -1;
        }
    }

    if (-1 == listen(fd, 8192)) {
        DB_ERR("socket listen error errno:%s", strerror(errno));
        close(fd);
        return -1;
    }

    DB_DEBUG("Listen %s:%d success\n", ipstr == NULL?(family == AF_INET?"0.0.0.0":"::0"):ipstr, port);
    return fd;
}

/* 成功返回socket fd */
int sock_connect_server(int protocol, struct sockaddr_storage *addr, int blocking)
{
    int fd;
    int ret;
    int reuse = 1;
    char ip_str[128] = "";
    int port;

    fd = socket(addr->ss_family, SOCK_STREAM, protocol);
    if (fd < 0) {
        DB_ERR("socket errno:%d", errno);
        return -1;
    }

    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR,
                   (const void *) &reuse, sizeof(reuse)) < 0) {
        DB_ERR("setsockopt(SO_REUSEADDR) error:%s",strerror(errno));
        return -1;
    }

    //设置为非阻塞
    if( blocking != 1 ) {
        if(sock_nonblocking(fd) < 0) {
            DB_ERR("socket set nonblocking error errno:%d", errno);
            close(fd);
            return -1;
        }
    }

    if( addr->ss_family == AF_INET ) {
        inet_ntop(AF_INET, &((struct sockaddr_in *)addr)->sin_addr, ip_str, sizeof(ip_str));
        port = ((struct sockaddr_in *)addr)->sin_port;
    } else {
        inet_ntop(AF_INET6, &((struct sockaddr_in6 *)addr)->sin6_addr, ip_str, sizeof(ip_str));
        port = ((struct sockaddr_in6 *)addr)->sin6_port;
    }


    if( protocol == IPPROTO_TCP ) {
        ret = connect(fd, (struct sockaddr *)addr, sizeof(struct sockaddr_storage));
        if (ret < 0 && errno != EINPROGRESS) {
            DB_ERR("connect %s:%d error, err:%s\n", ip_str, ntohs(port), strerror(errno));
            close(fd);
            return -1;

        }

        DB_INFO("connecting... %s:%d", ip_str, ntohs(port));
    }

    return fd;
}

int sock_send_buff(int sockfd, void *buf, size_t len)
{
    return write(sockfd, buf, len);
}

int sock_recv_buff(int sockfd, void *buf, size_t len)
{
    return read(sockfd, buf, len);
}

size_t sock_safe_recv_buff(int fd, void *buffer, int count)
{
    size_t len;

    do {
        len = recv (fd, buffer, count, 0);
    } while (len < 0 && errno == EINTR);

    return len;
}

size_t sock_safe_send_buff(int fd, const void *buf, int count)
{
    size_t len;
    size_t bytestosend;
    const char *buffer = buf;

    bytestosend = count;

    while (1) {
        len = send (fd, buffer, bytestosend, MSG_NOSIGNAL);
        if (len < 0) {
            if (errno == EINTR)
                continue;
            else
                return -errno;
        }
        if ((size_t) len == bytestosend)
            break;
        buffer += len;
        bytestosend -= len;
    }

    return count;
}
