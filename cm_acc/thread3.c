//thread2.c
#include "app.h"
#include "ink.h"
#include "dsplib/include/DSPLib.h"
#include "app_intrinsics.h"

  /* Allocate 16-bit real FFT data vector with correct alignment */
DSPLIB_DATA(signal_sampled_input, MSP_ALIGN_FFT_Q15(N_SAMPLES))
 _q15 signal_sampled_input[N_SAMPLES];
__shared(

  _q15 signal_data_array[N_SAMPLES]

)

//__USE_CHANNEL(THREAD1,THREAD2);

ENTRY_TASK(task1);
TASK(task2);

queue tx_queue;
queue rx_queue;
unsigned char rx_buff;
uint16_t data;


void thread3_init(){

    // create a thread with priority 15 and entry task task1
    __CREATE(THREAD3,task1);

}


ENTRY_TASK(task1){
     //setup UART communication and hardware

    __delay_cycles(100000);
    setup_mcu();
  //setup the receive queue
  __bic_SR_register(GIE);
   while(rx_queue.count) dequeue(&rx_queue);
   __bis_SR_register(GIE);
   init_queue(&rx_queue);


  //flags for uart communication
	static const uint8_t flag_start = ACC;
	static const uint8_t flag_free = FREE;
  static const uint8_t flag_stop = STOP;
  static const uint8_t flag_signal = SIGNAL;

  unsigned char *ptr;
  uint8_t collected = 0;
  uint16_t buff;


  //send uart request until data are collected
  tx_data(SIGNAL);
  
  while(collected < N_SAMPLES)
  {
    if (rx_queue.count >= 2)
    {
        ptr = &buff;
        
        __bic_SR_register(GIE);
        *ptr = dequeue(&rx_queue);
        ptr++;
        *ptr = dequeue(&rx_queue);
        __bis_SR_register(GIE);
        
        __SET(signal_data_array[collected++],(uint16_t) buff);
        
        buff = 0;        
      }

      //RED LED successful collection
      P4OUT |=  BIT6;
  }

  collected = 0;
  tx_data(flag_stop);
  tx_data(flag_free);

  return task2;
}
   
TASK(task2){


  /* Function definitions */
  uint32_t sample_count = 0;
  uint32_t fft_count    = 0;
  uint32_t abs_count    = 0;
  uint32_t max_count    = 0;

  msp_status status;

  uint8_t i;
  for (i = 0; i < N_SAMPLES; i++)
  {
    signal_sampled_input[i] = __GET(signal_data_array[i]);
  }
  //  /* Configure parameters for FFT */
  msp_fft_q15_params fftParams;
  msp_abs_q15_params absParams;
  msp_max_q15_params maxParams;

  fftParams.length = N_SAMPLES;
  fftParams.bitReverse = 1;
  fftParams.twiddleTable = msp_cmplx_twiddle_table_16_q15;

  absParams.length = N_SAMPLES;

  maxParams.length = N_SAMPLES;

  uint16_t shift = 0;
  uint16_t max_index = 0;

  /* Perform FFT */
  status = msp_fft_fixed_q15(&fftParams, signal_sampled_input);
  msp_checkStatus(status);

  /* Remove DC component  */
  signal_sampled_input[0] = 0;

  /* Compute absolute value of FFT */
  status = msp_abs_q15(&absParams, signal_sampled_input, signal_sampled_input);
  msp_checkStatus(status);

  /* Get peak frequency */
  status = msp_max_q15(&maxParams, signal_sampled_input, NULL, &max_index); 
  msp_checkStatus(status);

  __no_operation();

  /* Turn on red LED if peak frequency greater than threshold */
  uint16_t tmp = 1000;
  if (max_index < 5)
  {
      //RED ON-GREEN OFF
      P1OUT |= BIT0;
      notify_P3P5();
      P4OUT &= BIT6;
  }
  else
  {
      P4OUT |= BIT6;
      while(tmp--)
      {  
       _delay_cycles(50000);
        P1OUT ^= BIT0;
      }
  }
 __SIGNAL(THREAD1);

 return NULL;

}


