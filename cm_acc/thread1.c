#include "app.h"
#include "ink.h"
#include "app_intrinsics.h"

// define task-shared persistent variables.

 __shared(
     uint16_t x;
 )

__USE_CHANNEL(THREAD1,THREAD2);

ENTRY_TASK(task1);


// called at the very first boot
void thread1_init(){
    // create a thread with priority 15 and entry task task1
    __CREATE(THREAD1,task1);
    __SIGNAL(THREAD1);
//    P3OUT |= BIT6;
//    P3OUT &= ~BIT6;
}


ENTRY_TASK(task1){


    P1IE |= BIT4;                              // P1.4 interrupt enabled low priority thread
    P1IE |= BIT2;                              // P1.2 interrupt enabled high priority thread
    __delay_cycles(15);

    uint16_t tmp = 100;
    __enable_interrupt();
    while(tmp--) 
    {
        rand();
    }

    //if interrupts are not available use this section
//    if ((rand() % 2) == 0) {
//
//        __SIGNAL(THREAD2);
//    }
//
//    else {
//        __SIGNAL(THREAD3);
//   }

    return NULL;
}

isr_event_t timer_event;


_interrupt(PORT1_VECTOR)
{
  switch(__even_in_range(P1IV, P1IV_P1IFG7))
    {
        case P1IV_NONE:    break;                   // Vector  0:  No interrupt
        case P1IV_P1IFG0:  break;                   // Vector  2:  P1.0 interrupt flag
        case P1IV_P1IFG1:  break;                   // Vector  4:  P1.1 interrupt flag
        case P1IV_P1IFG2:
            P1IE &= ~BIT2;
            P1IFG &= ~BIT2;                         // Clear P1.2 IFG
            //P3OUT |= BIT6;
            if (valid_signal())
            {
              if(!__EVENT_BUFFER_FULL(THREAD3))
              {
                timer_event.data = NULL;
                timer_event.size = 0;
                timer_event.timestamp = 2;

              }
            //P3OUT &=~BIT6;
              __SIGNAL(THREAD3);
            }
            /* turn on CPU */
            __bic_SR_register_on_exit(LPM3_bits);
            break;                                  // Vector  6:  P1.2 interrupt flag
        case P1IV_P1IFG3:  break;                   // Vector  8:  P1.3 interrupt flag
        case P1IV_P1IFG5:  break;                   // Vector  12:  P1.5 interrupt flag
        case P1IV_P1IFG4:                           // Vector  10:  P1.4 interrupt flag
            //P1IE = 0;                               // interrupt disable
            P1IE &= ~BIT4;
            //__disable_interrupt();
            P1IFG &= ~BIT4;                         // Clear P1.4 IFG
            P3OUT |= BIT0;
            if(!__EVENT_BUFFER_FULL(THREAD2))
            {
              timer_event.data = NULL;
              timer_event.size = 0;
              timer_event.timestamp = 1;

            }
            P3OUT &= ~BIT0;

            //if the same thread is executing finish it 
            if (__next_thread() != __get_thread(THREAD2))
            {
                __SIGNAL(THREAD2);
            }
            /* turn on CPU */
            __bic_SR_register_on_exit(LPM3_bits);
            break;
        case P1IV_P1IFG6:  break;          // Vector  14:  P1.6 interrupt flag
        case P1IV_P1IFG7:  break;          // Vector  16:  P1.7 interrupt flag
        default: break;
    }
 	  
}

