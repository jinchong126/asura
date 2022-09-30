#ifndef _COMMON_H_
#define _COMMON_H_

#include <ev.h>

#define MAX_STR_IP_SIZE 16

#define BIT(x) (1 << (x))
#define SETBIT(addr, bit) (addr |= (1<<bit))
#define CLEARBIT(addr, bit) (addr &= ~(1<<bit))
#define CHECKBIT(addr, bit) (addr & (1<<bit))


enum {
    IPV4_LEN = 16,
    IPV6_LEN = 46
};

#ifndef IPSTR
#define IPSTR					"%u.%u.%u.%u"
#define IP2STR(a)				(((a) >> 24) & 0xff), (((a) >> 16) & 0xff), \
									(((a) >> 8) & 0xff), ((a) & 0xff)
#define NETIP2STR(a)			((a) & 0xff), (((a) >> 8) & 0xff), \
									(((a) >> 16) & 0xff), (((a) >> 24) & 0xff)
#endif

#define SAFE_FREE(p, f) do{ \
						if( p != NULL ) { \
							f(p); \
							p = NULL; \
						} \
					}while(0)

#define EV_IO_STOP_SAFE(loop, io, f, mask) do{\
											if(CHECKBIT(f, mask)){\
												ev_io_stop(loop, io);\
												CLEARBIT(f, mask);\
											}\
										}while(0);

#define EV_TIMER_STOP_SAFE(loop, io, f, mask) do{\
											if(CHECKBIT(f, mask)){\
												ev_timer_stop(loop, io);\
												CLEARBIT(f, mask);\
											}\
										}while(0);

#define EV_IO_START_SAFE(loop, io, f, mask) do{\
											if(!CHECKBIT(f, mask)){\
												ev_io_start(loop, io);\
												SETBIT(f, mask);\
											}\
										}while(0);

#define EV_TIMER_START_SAFE(loop, io, f, mask) do{\
											if(!CHECKBIT(f, mask)){\
												ev_timer_start(loop, io);\
												SETBIT(f, mask);\
											}\
										}while(0);

// 注意一个结构体中多个相同类型的情况
enum ev_state {
    COMM_READ_IO_CONN_MASK,
    COMM_WRITE_IO_CONN_MASK,
    COMM_READ_IO_MASK,
    COMM_WRITE_IO_MASK,
    COMM_TIMER_IO_MASK,
    COMM_TIMER2_IO_MASK
};

// #define TIMEFMT %02d:%02d:%02d.%06d
// #define TIMEVAL(tv) (tv).tv_sec/(1000000*60*60),time_use/(1000000*60),time_use/(1000000),

int cm_get_cpu_num();
time_t get_cur_timestamp();
inline void time_getval(struct timeval *tv);
inline double time_compare_ms(const struct timeval *tv1, const struct timeval *tv2);
inline double time_compare_us(const struct timeval *tv1, const struct timeval *tv2);
inline long int get_cur_usec();
#endif
