/*
 * thread2.c
 *
 *  Created on: 12 Feb 2018
 *      Author: sinanyil81
 */
#include "app.h"
#include "timer.h"
// define task-shared persistent variables.
__shared(
    uint8_t x,y;
)

__USE_CHANNEL(THREAD1,THREAD2);

TASK(task1);

// called at the very first boot
void thread2_init(){
    // create a thread with priority 15 and entry task task1
    __CREATE(THREAD2,task1);
}


TASK(task1){
    set_expire_timer(THREAD2,4000);
    if(__GET(x)==0)
        __led_toggle(LED2);

    if(__GET_CHANNEL(THREAD1,THREAD2)[5]==0x34)
        __led_toggle(LED2);

    if(__GET_CHANNEL_TIMESTAMP(THREAD1,THREAD2)==500)
        __led_toggle(LED2);
    __delay_cycles(150000);
    stop_expire_timer(THREAD2);
    // next task is task2
    return NULL;
}
