#include <msp430.h> 
#include <stdint.h>

#define FIXED
//uint16_t data[] ={};

#ifdef FIXED
    uint16_t data[] = {
    #include "powerPatterns.txt"
    };
#elif TOWARD
    uint16_t data[] = {
    #include "powerPatterns.txt"
    };
#endif

/**
 * main.c
 */
volatile unsigned char RXData = 0;
volatile unsigned char TXData = 1;
uint16_t flag = 12;


int main(void)
{
	WDTCTL = WDTPW | WDTHOLD;	// stop watchdog timer

	// Configure GPIO
	P1OUT &= ~BIT0;                           // Clear P1.0 output latch
	P1DIR |= BIT0;                            // For LED on P1.0
  	P3OUT &= ~(BIT4|BIT5); 
  	P3DIR |= BIT4 | BIT5;

	// Configure GPIO
	P2SEL1 |= BIT5 | BIT6;                    // USCI_A0 UART operation
	P2SEL0 &= ~(BIT5 | BIT6);

	// Disable the GPIO power-on default high-impedance mode to activate
	// previously configured port settings
	PM5CTL0 &= ~LOCKLPM5;

	 // Configure USCI_A0 for UART mode
	UCA1CTLW0 = UCSWRST;                      // Put eUSCI in reset
	UCA1CTL1 |= UCSSEL__SMCLK;                // CLK = SMCLK
	UCA1BR0 = 8;                              // 1000000/115200 = 8.68
	UCA1MCTLW = 0xD600;                       // 1000000/115200 - INT(1000000/115200)=0.68
	                                        // UCBRSx value = 0xD6 (See UG)
	UCA1BR1 = 0;
	UCA1CTL1 &= ~UCSWRST;                     // release from reset
	UCA1IE |= UCRXIE;                         // Enable USCI_A0 RX interrupt
	
	uint8_t *p;
	uint8_t i  = 0;
	uint16_t bound = 290;
	
	 while (1)
	{
		
		if (data[i] >= bound)
		{
			
			P3OUT |= BIT4 | BIT5;

		}else{

			P3OUT &= ~(BIT4 | BIT5);
		}

		__delay_cycles(50);
		
		
		if (RXData == flag)
		{
			p = &data[i];
			UCA1TXBUF = *p;	// Load data onto buffer
			while(!(UCA1IFG & UCTXIFG));
			p++;
			UCA1TXBUF = *p;	// Load data onto buffer
			while(!(UCA1IFG & UCTXIFG));
		}

		if (i>=200)
		{
			i = 0;

		}else{

			i++;
		}

		//__bis_SR_register(LPM0_bits | GIE);     // Enter LPM0, interrupts enabled
	}
		
	return 0;
}

#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector=USCI_A1_VECTOR
__interrupt void USCI_A1_ISR(void)
#elif defined(__GNUC__)
void __attribute__ ((interrupt(USCI_A1_VECTOR))) USCI_A1_ISR (void)
#else
#error Compiler not supported!
#endif
{
  switch(__even_in_range(UCA1IV,USCI_UART_UCTXCPTIFG))
  {
    case USCI_NONE: break;
    case USCI_UART_UCRXIFG:
      RXData = UCA1RXBUF;                   // Read buffer
      if(RXData == flag)                  // Check value
      {
        P1OUT |= BIT0;                      // If incorrect turn on P1.0
      }
      __bic_SR_register_on_exit(LPM0_bits); // Exit LPM0 on reti
      break;
    case USCI_UART_UCTXIFG: break;
    case USCI_UART_UCSTTIFG: break;
    case USCI_UART_UCTXCPTIFG: break;
  }
}
