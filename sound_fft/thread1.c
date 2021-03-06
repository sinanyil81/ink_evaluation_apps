#include <msp430.h>
#include "app.h"
#include "ink.h"
#include "dsplib/include/DSPLib.h"

/*FFT characteristics definitions*/
#define FFT_SAMPLES             128
#define INPUT_AMPLITUDE         0.5
#define INPUT_FREQUENCY         64
#define SAMPL_FREQ              512
#define FFT_FREQ_THRESHOLD      1500
#define REAL_ADC_FREQ           1000000 / (4 * 4 * 16)
#define INDEX_THRESHOLD         FFT_FREQ_THRESHOLD / (REAL_ADC_FREQ / (2 * FFT_SAMPLES))


  /* Allocate 16-bit real FFT data vector with correct alignment */
DSPLIB_DATA(tb_fftd, MSP_ALIGN_FFT_Q15(FFT_SAMPLES))
 _q15 tb_fftd[FFT_SAMPLES];

__shared(

  _q15 pers_sdata[FFT_SAMPLES]

)

void ADC_config();

//__USE_CHANNEL(THREAD1,THREAD2);

ENTRY_TASK(task1);
TASK(task2);

void thread1_init(){

    // create a thread with priority " " and entry task task1
    __CREATE(THREAD1,task1);
    __SIGNAL(THREAD1);
}

uint16_t counter;
_q15 sampled_input[FFT_SAMPLES];
ENTRY_TASK(task1){

    //TODO:needed?
    __delay_cycles(100000);
  
	// save interrupt state and then disable interrupts
	uint16_t is = __get_interrupt_state();
	__disable_interrupt();

	// configure ADC
	ADC_config();

	counter = 0;

	//why enable here and restore later
	__enable_interrupt();

	while(counter < FFT_SAMPLES);
	
	// turn off the ADC to save energy
	ADC12CTL0 &= ~ADC12ON;

	// restore interrupt state
	__set_interrupt_state(is);

	uint16_t i;
	for (i = 0; i < FFT_SAMPLES; i++)
	{
		__SET(pers_sdata[i], sampled_input[i]);
	}

  return task2;
}
   
TASK(task2){

  msp_status status;

  uint16_t i;
  for (i = 0; i < FFT_SAMPLES; i++)
  {
    tb_fftd[i] = __GET(pers_sdata[i]);
  }
  //  /* Configure parameters for FFT */
  msp_fft_q15_params fftParams;
  msp_abs_q15_params absParams;
  msp_max_q15_params maxParams;

  fftParams.length = FFT_SAMPLES;
  fftParams.bitReverse = 1;
  //fftParams.twiddleTable = NULL; //msp_cmplx_twiddle_table_16_q15;
#if !defined(MSP_USE_LEA)
        fftParams.twiddleTable = msp_cmplx_twiddle_table_128_q15;
#else
        fftParams.twiddleTable = NULL;
#endif
  absParams.length = FFT_SAMPLES;

  maxParams.length = FFT_SAMPLES;

  uint16_t shift = 0;
  uint16_t max_index = 0;

  /* Perform FFT */
  status = msp_fft_auto_q15(&fftParams, tb_fftd, &shift);
  //msp_checkStatus(status);

  /* Remove DC component  */
  tb_fftd[0] = 0;

  /* Compute absolute value of FFT */
  status = msp_abs_q15(&absParams, tb_fftd, tb_fftd);
  //msp_checkStatus(status);

  /* Get peak frequency */
  status = msp_max_q15(&maxParams, tb_fftd, NULL, &max_index); 
  //msp_checkStatus(status);

  __no_operation();

  /* Turn on red LED if peak frequency greater than threshold */
//  uint16_t tmp = 1000;
//  if (max_index > INDEX_THRESHOLD)
//  {
//      //RED ON-GREEN OFF
//      P1OUT |= BIT0;
//      notify_P3P5();
//      P4OUT &= BIT6;
//  }
//  else
//  {
//      P4OUT |= BIT6;
//      while(tmp--)
//      {
//       _delay_cycles(50000);
//        P1OUT ^= BIT0;
//      }
//  }
 __SIGNAL(THREAD1);

 return NULL;

}

/**
 * Configure ADC for microphone sampling
 */
void ADC_config()
{
    //TODO:free 1.3
    // Pin P1.3 set for Ternary Module Function (which includes A3)
    P1SEL0 |= BIT3;
    P1SEL1 |= BIT3;

    // Clear ENC bit to allow register settings
    ADC12CTL0 &= ~ADC12ENC;

    // Sample-and-hold source select
    //
    // 000 -> ADC12SC bit (default)
    // ADC12CTL1 &= ~(ADC12SHS0 | ADC12SHS1 | ADC12SHS2);

    // Clock source select
    //
    // source: MCLK (DCO, 1 MHz)
    // pre-divider: 4
    // divider: 4
    ADC12CTL1 |= ADC12SSEL_2 | ADC12PDIV_1 | ADC12DIV_3;

    // sampling period select for MEM0: 64 clock cycles (*)
    // multiple sample and conversion: enabled
    // ADC module ON
    ADC12CTL0 |= ADC12SHT0_2 | ADC12MSC | ADC12ON;
    // (*) freq = MCLK / (ADC12PDIV_0 * ADC12DIV_0 * ADC12SHT0_4)
    //          = 1000000 / (4 * 4 * 16)
    //          = 3906 Hz

    // conversion sequence mode: repeat-single-channel
    // pulse-mode select: SAMPCON signal is sourced from the sampling timer
    ADC12CTL1 |= ADC12CONSEQ_2 | ADC12SHP;

    // resolution: 12 bit
    // data format: right-aligned, unsigned
    ADC12CTL2 |= ADC12RES__12BIT ;//| ADC12DF_0;
    ADC12CTL2 &= ~ADC12DF;
    // conversion start address: MEM0
    ADC12CTL3 |= ADC12CSTARTADD_0;

    // MEM0 control register
    // reference select: VR+ = AVCC (3V), VR- = AVSS (0V)
    // input channel select: A3
    ADC12MCTL0 |= ADC12VRSEL_0 | ADC12INCH_3;

    // Clear interrupt for MEM0
    ADC12IFGR0 &= ~ADC12IFG0;

    // Enable interrupt for (only) MEM0
    ADC12IER0 = ADC12IE0;

    // Trigger first conversion (Enable conversion and Start conversion)
    ADC12CTL0 |= ADC12ENC | ADC12SC;
}

/**
 * ADC Interrupt handler
 */

_interrupt(ADC12_VECTOR)
{
    // static uint16_t i = 0;
    switch(__even_in_range(ADC12IV,12))
    {
    case  0: break;                         // Vector  0:  No interrupt
    case  2: break;                         // Vector  2:  ADC12BMEMx Overflow
    case  4: break;                         // Vector  4:  Conversion time overflow
    case  6: break;                         // Vector  6:  ADC12BHI
    case  8: break;                         // Vector  8:  ADC12BLO
    case 10: break;                         // Vector 10:  ADC12BIN
    case 12:                                // Vector 12:  ADC12BMEM0 Interrupt
        if (counter < FFT_SAMPLES)
            // Read ADC12MEM0 value
            sampled_input[counter++] = ADC12MEM0;
        else {
            // disable ADC conversion and disable interrupt request for MEM0
            ADC12CTL0 &= ~ADC12ENC;
            ADC12IER0 &= ~ADC12IE0;
        }
        break;
    case 14: break;                         // Vector 14:  ADC12BMEM1
    case 16: break;                         // Vector 16:  ADC12BMEM2
    case 18: break;                         // Vector 18:  ADC12BMEM3
    case 20: break;                         // Vector 20:  ADC12BMEM4
    case 22: break;                         // Vector 22:  ADC12BMEM5
    case 24: break;                         // Vector 24:  ADC12BMEM6
    case 26: break;                         // Vector 26:  ADC12BMEM7
    case 28: break;                         // Vector 28:  ADC12BMEM8
    case 30: break;                         // Vector 30:  ADC12BMEM9
    case 32: break;                         // Vector 32:  ADC12BMEM10
    case 34: break;                         // Vector 34:  ADC12BMEM11
    case 36: break;                         // Vector 36:  ADC12BMEM12
    case 38: break;                         // Vector 38:  ADC12BMEM13
    case 40: break;                         // Vector 40:  ADC12BMEM14
    case 42: break;                         // Vector 42:  ADC12BMEM15
    case 44: break;                         // Vector 44:  ADC12BMEM16
    case 46: break;                         // Vector 46:  ADC12BMEM17
    case 48: break;                         // Vector 48:  ADC12BMEM18
    case 50: break;                         // Vector 50:  ADC12BMEM19
    case 52: break;                         // Vector 52:  ADC12BMEM20
    case 54: break;                         // Vector 54:  ADC12BMEM21
    case 56: break;                         // Vector 56:  ADC12BMEM22
    case 58: break;                         // Vector 58:  ADC12BMEM23
    case 60: break;                         // Vector 60:  ADC12BMEM24
    case 62: break;                         // Vector 62:  ADC12BMEM25
    case 64: break;                         // Vector 64:  ADC12BMEM26
    case 66: break;                         // Vector 66:  ADC12BMEM27
    case 68: break;                         // Vector 68:  ADC12BMEM28
    case 70: break;                         // Vector 70:  ADC12BMEM29
    case 72: break;                         // Vector 72:  ADC12BMEM30
    case 74: break;                         // Vector 74:  ADC12BMEM31
    case 76: break;                         // Vector 76:  ADC12BRDY
    default: break;
    }
}
