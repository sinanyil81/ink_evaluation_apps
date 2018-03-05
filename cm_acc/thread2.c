//thread2.c
#include "app.h"
#include "ink.h"

#define NUM_OF_SAMPLES 10
uint8_t bound = 1;
uint8_t counter = 0;
uint8_t rcv = 0;

uint8_t data;
uint8_t data_array[NUM_OF_SAMPLES];
uint8_t RXData;
uint8_t collected = 0;


// called at the very first boot
__shared(
    uint8_t x,y;
)

__USE_CHANNEL(THREAD1,THREAD2);


TASK(task1);


void thread2_init(){

    // create a thread with priority 15 and entry task task1
    __CREATE(THREAD2,task1);
    //__SIGNAL(THREAD2);
}


void tx_flag(uint8_t flag){
    //code to sample the "fake accel"
    uint8_t *p;

    //transmit code
    p = &flag;
    UCA1TXBUF = *p; // Load data onto buffer
    while(!(UCA1IFG & UCTXIFG));
    //p++;
    //UCA1TXBUF = *p; // Load data onto buffer
    //while(!(UCA1IFG & UCTXIFG));

}

void rx_data(){

	while(UCA1IFG & 0x01);
	if (rcv)
	{
	    data_array[collected] = data;
	    rcv = 0;
	    collected++;
	 //    if (data > bound) P3OUT |= BIT4 | BIT5;
		// else P3OUT &= ~(BIT4 | BIT5);
	}
	__bis_SR_register(GIE);

}

TASK(task1){

  	P3OUT &= ~(BIT4|BIT5); 
  	P3DIR |= BIT4 | BIT5;

	// Configure GPIO
	P2SEL1 |= BIT5 | BIT6;                    // USCI_A0 UART operation
	P2SEL0 &= ~(BIT5 | BIT6);
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

	P1OUT &= ~BIT0;                           // Clear P1.0 output latch
	P1DIR |= BIT0;                            // For LED on P1.0

	
	static const uint8_t flag_start = 13;
	static const uint8_t flag_stop = 26;
	static const uint8_t request = 52;
	static const uint8_t ack = 104;
	uint16_t cnt = 0;


	tx_flag(flag_start);
	while(collected < NUM_OF_SAMPLES){
		tx_flag(request);
		rx_data();
		tx_flag(ack);	
	    P1OUT ^=  BIT0;
		if (++cnt > 20)
		{
			cnt = 0;
			tx_flag(flag_start);
		}
	}
	collected = 0;
	tx_flag(flag_stop);
	P1OUT &= BIT0;
	P1IE = BIT2;                              // P1.2 interrupt enabled
//  __CREATE(THREAD1,task1);
  __SIGNAL(THREAD1);


}

// #if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
// #pragma vector=USCI_A1_VECTOR
// __interrupt void USCI_A1_ISR(void)
// #elif defined(__GNUC__)
// void __attribute__ ((interrupt(USCI_A1_VECTOR))) USCI_A1_ISR (void)
// #else
// #error Compiler not supported!
// #endif

__interrupt(USCI_A1_VECTOR)
{
  switch(__even_in_range(UCA1IV,USCI_UART_UCTXCPTIFG))
  {
    case USCI_NONE: break;
    case USCI_UART_UCRXIFG:
    	data =(uint8_t) UCA1RXBUF;
    	__bic_SR_register(GIE);
    	rcv = 1;
      // RXData = UCA1RXBUF;                   // Read buffer
      // uint8_t *p;
      // p = &data;
      // *p = RXData;
      // p++;
      // counter++;
      // if(counter == 2)                  // Check value
      // {	
      // 	counter = 0;
      //   rcv = 1;                      // If correct capture data
      // }
      break;
    case USCI_UART_UCTXIFG: break;
    case USCI_UART_UCSTTIFG: break;
    case USCI_UART_UCTXCPTIFG: break;
  }
}
