#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include <unistd.h>
#include <sys/sysinfo.h>
#include "common.h"

int cm_get_cpu_num()
{
    int cpus;
    cpus = get_nprocs_conf();
    if (cpus <= 0) {
        cpus = 1;
    }

    return cpus;
}

inline time_t get_cur_timestamp()
{
    return time(NULL);
}


inline void time_getval(struct timeval *tv)
{
    if (tv == NULL) {
        return;
    }
    gettimeofday(tv, NULL);
}

// tv2 - tv1
inline double time_compare_ms(const struct timeval *tv1, const struct timeval *tv2)
{
    if (tv1 == NULL|| tv2 == NULL) {
        return 0;
    }

    return (1000000*(tv2->tv_sec - tv1->tv_sec)
            +tv2->tv_usec - tv1->tv_usec)/1000;
}

// tv2 - tv1
inline double time_compare_us(const struct timeval *tv1, const struct timeval *tv2)
{
    if (tv1 == NULL|| tv2 == NULL) {
        return 0;
    }

    return (1000000*(tv2->tv_sec - tv1->tv_sec)
            +tv2->tv_usec - tv1->tv_usec);
}


inline long int get_cur_usec()
{
    struct timeval tv;
    gettimeofday(&tv,NULL);
    // printf("second:%ld\n",tv.tv_sec);                              //秒
    // printf("millisecond:%ld\n",tv.tv_sec*1000 + tv.tv_usec/1000);  //毫秒
    // printf("microsecond:%ld\n",tv.tv_sec*1000000 + tv.tv_usec);    //微秒

    return tv.tv_sec*1000000 + tv.tv_usec;    //微秒
}
