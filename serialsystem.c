#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <time.h>
#include <signal.h>
#include <net/if.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <linux/can.h>
#include <linux/can/raw.h>
#include "error.h"
#include "log.h"
#include "Hachiko.h"

extern void * thread_tcu_write_service(void *) ___THREAD_ENTRY___;
extern void * thread_tcu_read_service(void *) ___THREAD_ENTRY___;
//extern void * thread_tcu_heartbeat_service(void *)___THREAD_ENTRY___;
int main()
{
    pthread_t tid = 0;
    pthread_attr_t attr;
    int s;
    int thread_done[ 8 ] = {0};
    int errcode = 0, ret;

    printf(
            "           ___        _       ____\n"
            "          / _ \\ _ __ | |_   _|  _ \\ _____      _____ _ __\n"
            "         | | | | '_ \\| | | | | |_) / _ \\ \\ /\\ / / _ \\ '__|\n"
            "         | |_| | | | | | |_| |  __/ (_) \\ V  V /  __/ |\n"
            "          \\___/|_| |_|_|\\__, |_|   \\___/ \\_/\\_/ \\___|_|\n"
            "                        |___/\n"
           );
    printf( "           \nCopyright © Andy zhao for SerialSystem\n");
    printf( "                            %s %s\n\n", __DATE__, __TIME__);
    printf("系统启动中.....\n\n\n\n");


    s = pthread_attr_init(&attr);
    if ( 0 != s ) {
        log_printf(WRN, "could not set thread stack size, use default.");
    }

    if ( 0 == s ) {
        if ( 0 == pthread_attr_setstacksize(&attr, 1024 * 1024) ) {
            log_printf(INF, "set all thread stack size to: 1024 * 1024 KB");
        } else {
            log_printf(ERR, "set all thread stack_size to 1024 * 1024 KB faile, "
                       "use system default size.");
        }
    }

    // 启动定时器
    Hachiko_init();

#if 1
    // TCU 数据包写线程，从队列中取出要写的数据包并通过CAN总线发送出去
    ret = pthread_create( & tid, &attr, thread_tcu_write_service,
                          &thread_done[1]);
    if ( 0 != ret ) {
        errcode  = 0x1001;
        log_printf(ERR,
                   "CAN-BUS reader start up.                       FAILE!!!!");
        goto die;
    }
    log_printf(INF, "CAN-BUS reader start up.                           DONE.");
#endif
#if 1
    // TCU数据包读线程，从CAN总线读取数据包后将数据存入读入数据队列等待处理
    ret = pthread_create( & tid, &attr, thread_tcu_read_service,
                          &thread_done[2]);
    if ( 0 != ret ) {
        errcode  = 0x1002;
        log_printf(ERR,
                   "CAN-BUS writer start up.                       FAILE!!!!");
        goto die;
    }
    log_printf(INF, "CAN-BUS writer start up.                           DONE.");
#endif
#if 0
    // TCU数据包心跳线程，定时发送数据，周期1s
    ret = pthread_create( & tid, &attr, thread_tcu_heartbeat_service,
                          &thread_done[3]);
    if ( 0 != ret ) {
        errcode  = 0x1002;
        log_printf(ERR,
                   "CAN-BUS writer start up.                       FAILE!!!!");
        goto die;
    }
    log_printf(INF, "CAN-BUS heartbeatt start up.                           DONE.");
#endif
     if ( s == 0 ) {
        pthread_attr_destroy(&attr);
    }
    // 主循环中放置看门狗代码
    for ( ;; ) {
        sleep(1);
    }
    return 0;
die:
    log_printf(ERR, "going to die. system aborted!");
    return errcode;
}
