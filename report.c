/*
 * =====================================================================================
 *
 *       Filename:  report.c
 *
 *    Description:
 *
 *        Version:  1.0
 *        Created:  09/24/2022 13:37:00 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  jc
 *        Company:
 *
 * =====================================================================================
 */
#define _GNU_SOURCE
#include "debug_msg.h"
#include "sock.h"
#include "report.h"

inline int report_status(w_report_st *wr, int status)
{
    switch( status ) {
    case SOCK_CONNECT_ERR:
        wr->connecting_num--;
        wr->fail_connect_num++;
        break;
    case SOCK_CONNECTED:
        wr->connecting_num--;
        wr->succ_connect_num++;
        wr->cur_connect_num++;
        break;
    case SOCK_CONNECTING:
        break;
    default:
        break;
    }
    return 0;
}
