#include <msp430.h> 
#include <stdint.h>

//#define FIXED
#define STABLE 1
//uint16_t data[] ={};

#ifdef FIXED
    uint16_t data[] = {
    #include "powerPatterns.txt"
    };
#elif STABLE
    uint8_t data[] = {1,2,3,4,5,6,7,8,9,10,1,2,3,4,5,6,7,8,9,10,1,2,3,4,5,6,7,8,9,10,1,2,3,4,5,6,7,8,9,10,1,2,3,4,5,6,7,8,9,10};
#endif


/**
 * main.c
 */
volatile unsigned char RXData = 0;
volatile unsigned char TXData = 1;
volatile uint8_t Rx = 666;
uint8_t flag_start = 13;
uint8_t flag_stop = 26;

int main(void)
{
	WDTCTL = WDTPW | WDTHOLD;	// stop watchdog timer

	// Configure GPIO
	P1OUT &= ~BIT0;                           // Clear P1.0 output latch
	P1DIR |= BIT0;                            // For LED on P1.0

	P1OUT &= ~BIT1;                           // Clear P1.0 output latch
	P1DIR |= BIT1;                            // For LED on P1.0

	P3OUT &= ~(BIT4|BIT5);
   	P3DIR |= BIT4 | BIT5;

    PM5CTL0 &= ~LOCKLPM5;                   // Disable the GPIO power-on default high-impedance mode
                                            // to activate previously configured port settings

	// // Configure GPIO
	P6SEL1 &= ~(BIT0 | BIT1);
    P6SEL0 |= (BIT0 | BIT1);				// USCI_A3 UART operation

#ifdef BAUD_115
    // Disable the GPIO power-on default high-impedance mode to activate
    // previously configured port settings
    PM5CTL0 &= ~LOCKLPM5;

    // Configure USCI_A3 for UART mode
    UCA3CTLW0 = UCSWRST;                    // Put eUSCI in reset
    UCA3CTLW0 |= UCSSEL__SMCLK;             // CLK = SMCLK
    UCA3BRW = 8;                            // 1000000/115200 = 8.68
    UCA3MCTLW = 0xD600;                     // 1000000/115200 - INT(1000000/115200)=0.68
                                            // UCBRSx value = 0xD6 (See UG)
    UCA3CTLW0 &= ~UCSWRST;                  // release from reset
    UCA3IE |= UCRXIE;                       // Enable USCI_A3 RX interrupt
#else 
    // Startup clock system with max DCO setting ~8MHz
    CSCTL0_H = CSKEY_H;                     // Unlock CS registers
    CSCTL1 = DCOFSEL_3 | DCORSEL;           // Set DCO to 8MHz
    CSCTL2 = SELA__VLOCLK | SELS__DCOCLK | SELM__DCOCLK;
    CSCTL3 = DIVA__1 | DIVS__1 | DIVM__1;   // Set all dividers
    CSCTL0_H = 0;                           // Lock CS registers

        // Configure USCI_A3 for UART mode
    UCA3CTLW0 = UCSWRST;                    // Put eUSCI in reset
    UCA3CTLW0 |= UCSSEL__SMCLK;             // CLK = SMCLK
    // Baud Rate calculation
    // 8000000/(16*9600) = 52.083
    // Fractional portion = 0.083
    // User's Guide Table 21-4: UCBRSx = 0x04
    // UCBRFx = int ( (52.083-52)*16) = 1
    UCA3BRW = 52;                           // 8000000/16/9600
    UCA3MCTLW |= UCOS16 | UCBRF_1 | 0x4900;
    UCA3CTLW0 &= ~UCSWRST;                  // Initialize eUSCI
    UCA3IE |= UCRXIE;                       // Enable USCI_A3 RX interrupt
#endif
    
	uint8_t *p;
	uint8_t i  = 0;
	uint16_t bound = 1;

	//enable interrupts;
	__bis_SR_register(GIE);
	
	 while (1)
	{
		
		if (data[i] >= bound)
		{
			
			P3OUT |= BIT4 | BIT5;

		}
		else
		{

			P3OUT &= ~(BIT4 | BIT5);
		}

		
		
		if (Rx == flag_start)
		{
			P1OUT &= ~BIT0;
			P1OUT |=  BIT1;
		    //p = &data[i];
		    while(Rx != 52);
			P1OUT &= ~BIT1;
		    //UCA3TXBUF = *p;	// Load data onto buffer
		    UCA3TXBUF = data[i];
		    while(!(UCA3IFG & UCTXIFG));//Wait for TX to complete
			// p++;			// Move the data ptr 8bit
			// UCA3TXBUF = *p;	// Load data onto buffer
	  //       while(!(UCA3IFG & UCTXIFG));
	        while(Rx != 104);
	        P1OUT |=  BIT1;
	        //__delay_cycles(50);

		}else if(Rx == flag_stop){
		    P1OUT &= ~BIT1;
		    P1OUT |=  BIT0;

		}else if(Rx == 666){

		    P1OUT |= BIT0 | BIT1;
		}else{

		    P1OUT &= ~(BIT0 | BIT1);
		    __delay_cycles(50000);
		    P1OUT |= BIT0 | BIT1;
		}

		if (i > 30) i = 0; //Recycle dataset
		else i++;

		//__bis_SR_register(LPM0_bits | GIE);     // Enter LPM0, interrupts enabled
	}
		
	return 0;
}

#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector=EUSCI_A3_VECTOR
__interrupt void USCI_A3_ISR(void)
#elif defined(__GNUC__)
void __attribute__ ((interrupt(EUSCI_A3_VECTOR))) USCI_A3_ISR (void)
#else
#error Compiler not supported!
#endif
{
  switch(__even_in_range(UCA3IV, USCI_UART_UCTXCPTIFG))
  {
    case USCI_NONE: break;
    case USCI_UART_UCRXIFG:
		Rx = UCA3RXBUF;                   // Read buffer
		//P1OUT = BIT0;                      // If incorrect turn on P1.0
		//__delay_cycles(50);
		//P1OUT ^= BIT0;                      // If incorrect turn on P1.0
      break;
    case USCI_UART_UCTXIFG: break;
    case USCI_UART_UCSTTIFG: break;
    case USCI_UART_UCTXCPTIFG: break;
  }
}
