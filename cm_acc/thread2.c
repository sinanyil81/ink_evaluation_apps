//thread2.c
#include "app.h"
#include "ink.h"
#include "dsplib/include/DSPLib.h"
#include "app_intrinsics.h"
#include "adxl345_i2c_lib.h"

#define DUMMY_SAMPLES 64
/* Allocate 16-bit real FFT data vector with correct alignment */
DSPLIB_DATA(sampled_input, MSP_ALIGN_FFT_Q15(N_SAMPLES))
 _q15 sampled_input[N_SAMPLES];
__shared(

  _q15 data_array[N_SAMPLES];

)

__USE_CHANNEL(THREAD1,THREAD2);


ENTRY_TASK(task1);
TASK(task2);


//Forward declarations 
void setup_mcu();               //mcu internals
void led_signal();              //LED sing 

uint8_t acc_data[NUM_BYTES_RX];


void thread2_init(){

    // create a thread with priority 15 and entry task task1
    __CREATE(THREAD2,task1);

}


ENTRY_TASK(task1){
  

  //__disable_interrupt();
  
  setup_mcu();
  //get samples 

  //P3OUT |= BIT0;
  i2c_init();
  
  i2c_write(ADXL_345 , ADXL_CONF_REG , 0x00);
  i2c_write(ADXL_345, ADXL_CONF_REG, 0x10);
  i2c_write(ADXL_345, ADXL_CONF_REG, 0x08);
  // P3OUT &= ~BIT0;

    
  uint8_t collected = 0;
  uint16_t z;
  // P3OUT |= BIT0;
  while(N_SAMPLES-collected){

        i2c_read_multi(ADXL_345, READ_REG, NUM_BYTES_RX, &acc_data);
        z = (((int16_t)acc_data[5]) << 8) | acc_data[4];
        __SET(data_array[collected++],z);
  }
  //__enable_interrupt();
  //P3OUT &= ~BIT0;
 
  return task2;
}

TASK(task2){

  /* Function definitions */
//  uint32_t sample_count = 0;
//  uint32_t fft_count    = 0;
//  uint32_t abs_count    = 0;
//  uint32_t max_count    = 0;
  //P3OUT |= BIT0;
  msp_status status;

  uint8_t i;
  for (i = 0; i < N_SAMPLES; i++)
  {
    sampled_input[i] = __GET(data_array[i]);
  }
  //  /* Configure parameters for FFT */
  msp_fft_q15_params fftParams;
  msp_abs_q15_params absParams;
  msp_max_q15_params maxParams;

  fftParams.length = N_SAMPLES;
  fftParams.bitReverse = 1;
  fftParams.twiddleTable = msp_cmplx_twiddle_table_128_q15;

  absParams.length = N_SAMPLES;

  maxParams.length = N_SAMPLES;

  //uint16_t shift = 0;
  uint16_t max_index = 0;

  /* Perform FFT */
  status = msp_fft_fixed_q15(&fftParams, sampled_input);
  //msp_checkStatus(status);

  /* Remove DC component  */
  sampled_input[0] = 0;

  /* Compute absolute value of FFT */
  status = msp_abs_q15(&absParams, sampled_input, sampled_input);
  //msp_checkStatus(status);

  /* Get peak frequency */
  status = msp_max_q15(&maxParams, sampled_input, NULL, &max_index); 
  //msp_checkStatus(status);
  uint16_t dummy[DUMMY_SAMPLES];
  uint16_t tmp[DUMMY_SAMPLES];
  uint16_t k = 0;

  while(k < DUMMY_SAMPLES)
  {
      dummy[k] = k*2;
      tmp[DUMMY_SAMPLES - k] = k*4;
      k++;
  }
  k = 0;

      //__no_operation();

  /* Turn on red LED if peak frequency greater than threshold */

//  if (max_index > 8)
//  {
//      notify_P3P4();
//  }
//  else
//  {
//      notify_P3P4();
//  }
  //P3OUT &= ~BIT0;
  P3OUT |= BIT4;
  P3OUT &= ~BIT4;
  P1IE |= BIT4;                               // P1.4 interrupt enabled

  __SIGNAL(THREAD1);
  return NULL;

}


