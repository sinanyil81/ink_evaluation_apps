//cm_acc Header
#include "app_intrinsics.h"
#include "ink.h"

extern  queue rx_queue;
extern  unsigned char rx_buff;


_interrupt(USCI_A1_VECTOR)
{
  switch(__even_in_range(UCA1IV,USCI_UART_UCTXCPTIFG))
  {
    case USCI_NONE: break;
    case USCI_UART_UCRXIFG:
    	

      rx_buff = UCA1RXBUF;
      enqueue(&rx_queue , rx_buff);

      break;
    case USCI_UART_UCTXIFG: break;
    case USCI_UART_UCSTTIFG: break;
    case USCI_UART_UCTXCPTIFG: break;
  }
}

void led_signal(){
  P1OUT |= BIT0;
  __delay_cycles(100000);
  P1OUT &= ~BIT0;

}

void tx_data(uint8_t flag){

  //code to sample the "fake" signal source
  uint8_t *p;

  //transmit code
  p = &flag;
  UCA1TXBUF = *p; // Load data onto buffer
  while(!(UCA1IFG & UCTXIFG));
}

void setup_mcu(){

  //Logic analyzer pins
  P3OUT &= ~(BIT4|BIT5); 
  P3DIR |= BIT4 | BIT5;

  // Configure GPIO
  P2SEL1 |= BIT5 | BIT6;                    // USCI_A0 UART operation
  P2SEL0 &= ~(BIT5 | BIT6);

  //Led
  P1OUT &= ~BIT0;                           // Clear P1.0 output latch
  P1DIR |= BIT0;                            // For LED on P1.0
  P4OUT &= ~BIT6;                           // Clear P4.6 output latch
  P4DIR |= BIT6;                            // For LED on P4.6

  PM5CTL0 &= ~LOCKLPM5;                   // Disable the GPIO power-on default high-impedance mode
                                          // to activate previously configured port settings

#ifdef BAUD_115
   // Configure USCI_A0 for UART mode
  UCA1CTLW0 = UCSWRST;                      // Put eUSCI in reset
  UCA1CTL1 |= UCSSEL__SMCLK;                // CLK = SMCLK
  UCA1BR0 = 8;                              // 1000000/115200 = 8.68
  UCA1MCTLW = 0xD600;                       // 1000000/115200 - INT(1000000/115200)=0.68
                                          // UCBRSx value = 0xD6 (See UG)
  UCA1BR1 = 0;
  UCA1CTL1 &= ~UCSWRST;                     // release from reset
  UCA1IE |= UCRXIE;                         // Enable USCI_A0 RX interrupt
#else
  // Startup clock system with max DCO setting ~8MHz
  CSCTL0_H = CSKEY >> 8;                    // Unlock clock registers
  CSCTL1 = DCOFSEL_3 | DCORSEL;             // Set DCO to 8MHz
  CSCTL2 = SELA__VLOCLK | SELS__DCOCLK | SELM__DCOCLK;
  CSCTL3 = DIVA__1 | DIVS__1 | DIVM__1;     // Set all dividers
  CSCTL0_H = 0;                             // Lock CS registers

  // Configure USCI_A0 for UART mode
  UCA1CTLW0 = UCSWRST;                      // Put eUSCI in reset
  UCA1CTLW0 |= UCSSEL__SMCLK;               // CLK = SMCLK
  // Baud Rate calculation
  // 8000000/(16*9600) = 52.083
  // Fractional portion = 0.083
  // User's Guide Table 21-4: UCBRSx = 0x04
  // UCBRFx = int ( (52.083-52)*16) = 1
  UCA1BR0 = 52;                             // 8000000/16/9600
  UCA1BR1 = 0x00;
  UCA1MCTLW |= UCOS16 | UCBRF_1;
  UCA1CTLW0 &= ~UCSWRST;                    // Initialize eUSCI
  UCA1IE |= UCRXIE;                         // Enable USCI_A0 RX interrupt

#endif
}

void init_queue(queue *q){
  
  q->first = 0;
  q->last = QUEUE_SIZE - 1;
  q->count = 0;
}

void enqueue(queue *q,char x){

  q->last = (q->last + 1) % QUEUE_SIZE;
  q->Data[ q->last ] = x;
  q->count += 1;
}

char dequeue(queue *q){

  char x = q->Data[ q->first ];
  q->first = (q->first + 1) % QUEUE_SIZE;
  q->count -= 1;
  return x;
}


void notify_P3P4(){

	P3OUT |= BIT4;
	__delay_cycles(50000);
	P3OUT &= BIT4;
}


void notify_P3P5(){

	P3OUT |= BIT5;
	__delay_cycles(50000);
	P3OUT &= BIT5;
}