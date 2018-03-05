/*
 * thread1.c
 *
 *  Created on: 12 Feb 2018
 *      Author: sinanyil81
 */
#include "app.h"
#include "timer.h"
// define task-shared persistent variables.
__shared(
    uint8_t x,y;
    int16_t a[100];
)

__CREATE_CHANNEL(THREAD1,THREAD2,100);

TASK(task1);
TASK(task2);
TASK(task3);

// called at the very first boot
void thread1_init(){
    // create a thread with priority 15 and entry task task1
    __CREATE(THREAD1,task1);
    __SIGNAL(THREAD1);
#ifdef WKUP_TIMER
    set_wkup_timer(THREAD1,20000);
#endif
}


TASK(task1){
    uint8_t m;
#ifdef EXPR_TIMER
    set_expire_timer(THREAD1,40000);
#endif    
    m = __GET(x);

    __GET_EVENT(15);

    __SET(a[4],m + 10);

   // set_timer(20000);

    // next task is task2
    return task2;
}

TASK(task2){
    uint16_t val = __GET(a[4]);

    if(val==10)
        __led_toggle(LED1);

    __SET(a[99],0xffff);

    //there is no next task
    return task3;
}

TASK(task3){
    uint16_t val1 = __GET(a[99]);
    uint16_t val2 = __GET(a[4]);

    if(val2==10 && val1==0xffff)
        __led_toggle(LED1);

    __SET_CHANNEL_TIMESTAMP(THREAD1,THREAD2,500);
    __GET_CHANNEL(THREAD1,THREAD2)[5]= 0x34;
#ifdef EXPR_TIMER
    stop_expire_timer(THREAD1);
#endif
    __SIGNAL(THREAD2);

    //there is no next task
    return NULL;
}
