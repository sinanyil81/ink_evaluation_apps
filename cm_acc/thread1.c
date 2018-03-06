#include "app.h"
#include "ink.h"
// define task-shared persistent variables.

__shared(
    uint8_t x,y;
)

__USE_CHANNEL(THREAD1,THREAD2);

ENTRY_TASK(task1);


// called at the very first boot
void thread1_init(){
    // create a thread with priority 15 and entry task task1
    __CREATE(THREAD1,task1);
    __SIGNAL(THREAD1);
}


ENTRY_TASK(task1){

  // Configure GPIO
  P1OUT &= BIT2;                            // Pull-down resistor on P1.2
  P1REN = BIT2;                             // Select down-up mode for P1.2
  P1DIR = 0xFF ^ BIT2;                      // Set all but P1.2 to output direction
  P1IES &= BIT2;                            // P1.2 Lo/Hi edge
  P1IFG = 0;                                // Clear all P1 interrupt flags
  P1IE = BIT2;                              // P1.2 interrupt enabled

    // next task is task2
    //__SIGNAL(THREAD2);

    return NULL;
}

isr_event_t timer_event;


_interrupt(PORT1_VECTOR)
{
 	P1IFG &= ~BIT2;                         // Clear P1.2 IFG
    P1IE = 0;                               // P1.2 interrupt disable
    if(!__EVENT_BUFFER_FULL(THREAD2)){
        timer_event.data = NULL;
        timer_event.size = 0;
        timer_event.timestamp = 1;

        __SIGNAL_EVENT(THREAD2,&timer_event);

    }
    __SIGNAL(THREAD2);
    /* turn on CPU */
    __bic_SR_register_on_exit(LPM3_bits);
}

