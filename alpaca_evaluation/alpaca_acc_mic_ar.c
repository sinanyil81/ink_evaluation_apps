#include <msp430.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "/Users/Carlo/Developer/MSP430/Repositories/alpaca-oopsla2017/ext/libi2c/src/include/libi2c/adxl345_i2c_lib.h"
#include "/Users/Carlo/Developer/MSP430/Repositories/alpaca-oopsla2017/ext/libappspecs/src/include/libappspecs/app_intrinsics.h"
#include "/Users/Carlo/Developer/MSP430/Repositories/alpaca-oopsla2017/ext/libtester/src/include/libtester/tester.h"

#include <libalpaca/alpaca.h>
#include <libmspbuiltins/builtins.h>

//#define NO_INT_SOURCE

/*FFT characteristics definitions*/
#define FFT_SAMPLES             128
#define INPUT_AMPLITUDE         0.5
#define INPUT_FREQUENCY         64
#define SAMPL_FREQ              512
#define FFT_FREQ_THRESHOLD      1500
#define REAL_ADC_FREQ           1000000 / (4 * 4 * 16)
#define INDEX_THRESHOLD         FFT_FREQ_THRESHOLD / (REAL_ADC_FREQ / (2 * FFT_SAMPLES))

// Number of samples to discard before recording training set
#define NUM_WARMUP_SAMPLES 3

#define ACCEL_WINDOW_SIZE 3
#define MODEL_SIZE 16
#define SAMPLE_NOISE_FLOOR 10 // TODO: made up value

// Number of classifications to complete in one experiment
#define SAMPLES_TO_COLLECT 512


//delay(us) to simulate hardware blocks
//values from logic analyzer traces with Ink 
#define FILTER_DURATION 5714
#define SAMPLE_MIC_DURATION 70830
#define FFT_MIC_DURATION 5366
#define FFT_ACC_DURATION 4200 
#define SINGLE_ACC_SAMPLE_DURATION 2000
#define SAMPLE_ACC_DURATION 128300

#define POWER_TEST
#ifdef POWER_TEST
//use for reseter 
uint16_t noise[] = {
  209,90,678,329,34,691,616,527,846,793,683,259,448,938,539,653,936,610,1118,1324,632,1081,486,617,764,1016,1006,989,1526,906,1027,1760,1633,1568,1275,2210,2022,1824,2436,2200,1990,1731,2634,2550,2763,2185,2886,2614,2415,2543,3040,2951,2806,3739,3058,3991,3423,3736,3796,3545,3739,4705,4340,4182,4729,4977,4829,5154,4862,5378,5717,5324,6022,5846,6380,6026,6174,6912,6895,6745,6912,7058,7272,7316,8038,7277,8335,8227,8109,8478,8148,9042,9390,9339,9300,9370,9575,10253,9705,10043,10324,11196,10423,10636,11020,11319,11713,11906,12174,12062,12818,12808,12917,13070,13283,14145,14065,13930,14676,14647,15018,15197,15615,15645,15502,15893,16317,17056,16699,17477,17792,17196,17920,18656,18377,18318,19222,18981,19238,20275,20408,20551,20927,21208,21731,21528,22128,22077,22394,22812,22984,23192,23677,23456,24431,24205,24834,25267,25267,25744,26536,25986,27203,26817,27112,27987,27899,28812,28869,29278,29542,29641,29978,30878,30711,31128,31320,31972,32577,32345,33244,32942,34071,33527,34495,34391,34888,34998,36105,36112,36725,37299,36960,37576,38467,38517,39301,38882,39475,40234
};
#endif



typedef int16_t _q15;

typedef struct
{
    int16_t x,y,z;

}threeAxis_t_8;

typedef threeAxis_t_8 accelReading;

typedef accelReading accelWindow[ACCEL_WINDOW_SIZE];

typedef struct {
	unsigned meanmag;
	unsigned stddevmag;
} features_t;

typedef enum {
	CLASS_STATIONARY,
	CLASS_MOVING,
} class_t;


typedef enum {
	MODE_IDLE = 3,
	MODE_TRAIN_STATIONARY = 2,
	MODE_TRAIN_MOVING = 1,
	MODE_RECOGNIZE = 0, // default
} run_mode_t;

//tasks
TASK(2, task_schedule)
TASK(3, task_sample_accel)
TASK(4, task_fft_accel)
TASK(5, task_sample_mic)
TASK(6, task_fft_mic)
TASK(7, task_filter_data)
TASK(8, task_init)
TASK(9, task_selectMode)
TASK(10, task_resetStats)
TASK(11, task_sample)
TASK(12, task_transform)
TASK(13, task_featurize)
TASK(14, task_classify)
TASK(15, task_stats)
TASK(16, task_warmup)
TASK(17, task_train)
TASK(18, task_idle)
TASK(19, task_update)

uint16_t noise[] = {
                    209,90,678,329,34,691,616,527,846,793,683,259,448,938,539,653,936,610,1118,1324,632,1081,486,617,764,1016,1006,989,1526,906,1027,1760,1633,1568,1275,2210,2022,1824,2436,2200,1990,1731,2634,2550,2763,2185,2886,2614,2415,2543,3040,2951,2806,3739,3058,3991,3423,3736,3796,3545,3739,4705,4340,4182,4729,4977,4829,5154,4862,5378,5717,5324,6022,5846,6380,6026,6174,6912,6895,6745,6912,7058,7272,7316,8038,7277,8335,8227,8109,8478,8148,9042,9390,9339,9300,9370,9575,10253,9705,10043,10324,11196,10423,10636,11020,11319,11713,11906,12174,12062,12818,12808,12917,13070,13283,14145,14065,13930,14676,14647,15018,15197,15615,15645,15502,15893,16317,17056,16699,17477,17792,17196,17920,18656,18377,18318,19222,18981,19238,20275,20408,20551,20927,21208,21731,21528,22128,22077,22394,22812,22984,23192,23677,23456,24431,24205,24834,25267,25267,25744,26536,25986,27203,26817,27112,27987,27899,28812,28869,29278,29542,29641,29978,30878,30711,31128,31320,31972,32577,32345,33244,32942,34071,33527,34495,34391,34888,34998,36105,36112,36725,37299,36960,37576,38467,38517,39301,38882,39475,40234
};
//custom sqrt !!untested
unsigned isqr_Newton(unsigned x);

static uint8_t _ISR_flag = 0;

GLOBAL_SB(uint8_t,inited);

//scheduler
GLOBAL_SB(uint8_t,flag);

//SOUND FFT
GLOBAL_SB(_q15,pers_sdata,FFT_SAMPLES);

//ACCEL FFT
GLOBAL_SB(_q15,data_array,N_SAMPLES);

//pesistent vars
GLOBAL_SB(uint16_t, pinState);
GLOBAL_SB(unsigned, discardedSamplesCount);
GLOBAL_SB(run_mode_t, class);
GLOBAL_SB(unsigned, totalCount);
GLOBAL_SB(unsigned, movingCount);
GLOBAL_SB(unsigned, stationaryCount);
GLOBAL_SB(accelReading, window, ACCEL_WINDOW_SIZE);
GLOBAL_SB(features_t, features);
GLOBAL_SB(features_t, model_stationary, MODEL_SIZE);
GLOBAL_SB(features_t, model_moving, MODEL_SIZE);
GLOBAL_SB(unsigned, trainingSetSize);
GLOBAL_SB(unsigned, samplesInWindow);
GLOBAL_SB(run_mode_t, mode);
GLOBAL_SB(unsigned, seed);
GLOBAL_SB(unsigned, count);

void configure_mcu()
{
  WDTCTL = WDTPW | WDTHOLD; // Stop watchdog timer

  __delay_cycles(3);

  // Configure GPIO
  // Pin 1.2 will be the input to fire ISR1
  //enable_P1P2();
  P1OUT &= ~BIT2;                             // Pull-down resistor on P1.2
  P1REN |= BIT2;                              // Select down-up mode for P1.2
  P1DIR &= ~BIT2;                             // Set P1.2 to output direction
  P1IES &= ~BIT2;                             // P1.2 Lo/Hi edge

  P1OUT &= ~BIT4;                             // Pull-down resistor on P1.4
  P1REN |= BIT4;                              // Select down-up mode for P1.4
  P1DIR &= ~BIT4;                              // Set all but P1.4 to output direction
  P1IES &= ~BIT4;                             // P1.4 Lo/Hi edge                                               // Clear all P1 interrupt flags
  P1IFG = 0;                                  // Clear all P1 interrupt flags
  
  //Logic analyzer pins
  P3OUT &= ~(BIT5); 
  P3DIR |=  BIT5;
  //Logic analyzer pins
  P3OUT &= ~(BIT4); 
  P3DIR |=  BIT4;

  P4OUT &= ~(BIT3 | BIT2);
  P4DIR |=  (BIT3 | BIT2);

  P3OUT &= ~(BIT6|BIT0); 
  P3DIR |=  (BIT6|BIT0);

  P2OUT &= ~(BIT6|BIT5|BIT2|BIT4); 
  P2DIR |=  (BIT6|BIT5|BIT2|BIT4);

  //LED
  P1OUT &= ~BIT0;                           // Clear P1.0 output latch
  P1DIR |= BIT0;                            // For LED on P1.0
  P4OUT &= ~BIT6;                           // Clear P4.6 output latch
  P4DIR |= BIT6;                            // For LED on P4.6


  PM5CTL0 &= ~LOCKLPM5; // Disable the GPIO power-on default high-impedance mode

  // Clock System Setup
  CSCTL0_H = CSKEY >> 8;   // Unlock CS registers
  CSCTL2 |= SELA__VLOCLK;
  CSCTL3 |= DIVA__1 ;     // Set all dividers to 1
  CSCTL0_H = 0;           // Lock CS registe

}

void init(){

	 configure_mcu();
#ifdef POWER_TEST
    eb_tester_start(noise);
#endif

    __delay_cycles(15);
    
    //trigger
    P3OUT |= BIT5;
    P3OUT &=~BIT5;
    GV(inited) = 1;
    GV(flag) = 0;
    
    __enable_interrupt();

}

//not needed in Alpaca
void mcu_sleep()
{	
#ifdef POWER_TEST
    eb_tester_reseter(30);
	P2OUT |= BIT2;
	__bis_SR_register(LPM3_bits | GIE);
    P2OUT &= ~BIT2;
    eb_tester_reseter(100);
#else
	__bis_SR_register(LPM3_bits | GIE);

#endif
	TRANSITION_TO(task_schedule);
}

uint8_t tmp_flag;


//main task 
void task_schedule()
{

    P4OUT |= BIT3;
#ifdef NO_INT_SOURCE
	
	static uint8_t tmp;
  
  //high priority 
	if (P1IN & BIT2)
	{
		tmp = 1;
	}
  //low priority 
	else if (P1IN & BIT4)
	{
		tmp = 3;
	}
  //no signal
	else
	{
		tmp = 0;
	}

  //choose path
	switch (tmp)
	{
		case 2:
			P4OUT &= ~BIT3;
			TRANSITION_TO(task_sample_mic);
			break;
		case 3:
			P4OUT &= ~BIT3;
			TRANSITION_TO(task_sample_accel);
			break;
		case 1:
			P4OUT &= ~BIT3;
			TRANSITION_TO(task_filter_data);
			break;
		case 0:
			 //mcu_sleep();
			 //break;
			P4OUT &= ~BIT3;
			TRANSITION_TO(task_schedule);
			break;
		default:
			P4OUT &= ~BIT3;
			TRANSITION_TO(task_schedule);
	}

#else
  	//GV(flag) = _ISR_flag;
  	// uint8_t tmp = GV(flag);
#endif

}

void task_update(){

	GV(flag) = _ISR_flag;

	TRANSITION_TO(task_schedule);
}


void task_filter_data()
{
	P3OUT |= BIT5;

#ifdef FITLER
	if (valid_signal())
		GV(flag) = MIC;
	else
		GV(flag) = WAIT;
#else

  __delay_cycles(FILTER_DURATION)

#endif
	P3OUT &= ~BIT5;

	TRANSITION_TO(task_sample_mic);
}



_q15 tb_fftd[FFT_SAMPLES];

uint16_t counter;
uint16_t is;
_q15 sampled_input[FFT_SAMPLES];

void task_sample_mic()
{  
  	P3OUT |= BIT5;
#ifdef SAMPLE_MIC
  	__disable_interrupt();
	// save interrupt state and then disable interrupts
	// is = __get_interrupt_state();

	// configure ADC
	ADC_config();

	counter = 0;

	__enable_interrupt();

	while(counter < FFT_SAMPLES);
	
	// disable interrupt for (only) MEM0
	ADC12IER0 &= ~ADC12IE0;


	// turn off the ADC to save energy
	ADC12CTL0 &= ~ADC12ON;
  
	// restore interrupt state
	// __set_interrupt_state(is);

 	uint8_t i;
	for (i = 0; i < FFT_SAMPLES; i++)
	{
    	GV(pers_sdata[i]) = sampled_input[i];
	}
#else

  __delay_cycles(SAMPLE_MIC_DURATION)

#endif
  P3OUT &= ~BIT5;
	TRANSITION_TO(task_fft_mic);
}

void task_fft_mic()
{
  P3OUT |= BIT5;
#ifdef DSPLIB_ON
  msp_status status;

  uint8_t i;
  for (i = 0; i < FFT_SAMPLES; i++)
  {
    tb_fftd[i] = GV(pers_sdata[i]);
  }
  //  /* Configure parameters for FFT */
  msp_fft_q15_params fftParams;
  msp_abs_q15_params absParams;
  msp_max_q15_params maxParams;

  fftParams.length = FFT_SAMPLES;
  fftParams.bitReverse = 1;
  fftParams.twiddleTable = NULL;
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

  /* Remove DC component  */
  tb_fftd[0] = 0;

  /* Compute absolute value of FFT */
  status = msp_abs_q15(&absParams, tb_fftd, tb_fftd);
  //msp_checkStatus(status);

  /* Get peak frequency */
  status = msp_max_q15(&maxParams, tb_fftd, NULL, &max_index); 
#else

  __delay_cycles(FFT_MIC_DURATION);
  // uint8_t i;
  // for (i = 0; i < FFT_SAMPLES; i++)
  // {
  //   tb_fftd[i] = GV(pers_sdata[i]);
  // }
  
  // uint16_t tmp = 10000;
  // i = 0;
  // while (--tmp)
  // {
  // 	 if (tb_fftd[i++] > 42)
  // 	 {
  // 	 	tb_fftd[i] = 42 ;
  // 	 }
  // }
#endif
  P3OUT &= ~BIT5;
  //Flag completion of this "Thread"
  P3OUT |= BIT6;
  P3OUT &= ~BIT6;
  
  // P1IE |= BIT2;                              // P1.2 interrupt enabled

  GV(flag) = 0;
  TRANSITION_TO(task_schedule);
}

/**
 * ADC Interrupt handler
 */
#ifdef __clang__
__attribute__((interrupt(0))) 
#else
__attribute__((interrupt(ADC12_VECTOR)))
#endif
void ADC12_ISR(void)
{
    switch(__even_in_range(ADC12IV,12))
    {
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
    default: break;
    }
}

#ifdef __clang__
__attribute__((section("__interrupt_vector_adc12"),aligned(2)))
void(*__vector_adc12)(void) = ADC12_ISR;
#endif


_q15 accel_input[N_SAMPLES];

uint8_t acc_data[NUM_BYTES_RX];

void task_sample_accel()
{

  
  P3OUT |= BIT0;
#ifdef SAMPLE_ACC  
  
  setup_mcu();

  i2c_init();
  
  i2c_write(ADXL_345 , ADXL_CONF_REG , 0x00);
  i2c_write(ADXL_345, ADXL_CONF_REG, 0x10);
  i2c_write(ADXL_345, ADXL_CONF_REG, 0x08);

    
  //get samples 
  uint8_t collected = 0;
  uint16_t z;
  while(N_SAMPLES-collected){

        // i2c_read_multi(ADXL_345, READ_REG, NUM_BYTES_RX, acc_data);
        // z = (((int16_t)acc_data[5]) << 8) | acc_data[4];
        GV(data_array[collected++]) = 11000;
  }
#else

  __delay_cycles(SAMPLE_ACC_DURATION);

#endif

  P3OUT &= ~BIT0;
 
  TRANSITION_TO(task_fft_accel);

}

void task_fft_accel()
{


  P3OUT |= BIT0;

#ifdef DSPLIB_ON
  msp_status status;

  uint8_t i;
  for (i = 0; i < N_SAMPLES; i++)
  {
    accel_input[i] = GV(data_array[i]);
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

  uint16_t max_index = 0;

  /* Perform FFT */
  status = msp_fft_fixed_q15(&fftParams, accel_input);

  /* Remove DC component  */
  accel_input[0] = 0;

  /* Compute absolute value of FFT */
  status = msp_abs_q15(&absParams, accel_input, accel_input);

  /* Get peak frequency */
  status = msp_max_q15(&maxParams, accel_input, NULL, &max_index); 
#else

  __delay_cycles(FFT_ACC_DURATION);
  // uint8_t i;
  // for (i = 0; i < N_SAMPLES; i++)
  // {
  //   accel_input[i] = GV(data_array[i]);
  // }
  
  // uint16_t tmp = 10000;
  // i = 0;

  // while (--tmp)
  // {
  // 	 if (accel_input[i++] > 42)
  // 	 {
  // 	 	accel_input[i] = 42 ;
  // 	 }
  // }

#endif

  P3OUT &= ~BIT0;

  //Initialize the AR-app 
  //AR test app is used here for an estimation 
  //of a high computation application 
  TRANSITION_TO(task_init);
}

//Dummy data sampling
void ACCEL_singleSample_(threeAxis_t_8* result){

#ifdef SAMPLE_ACC

  i2c_init();

  i2c_write(ADXL_345 , ADXL_CONF_REG , 0x00);
  i2c_write(ADXL_345, ADXL_CONF_REG, 0x10);
  i2c_write(ADXL_345, ADXL_CONF_REG, 0x08);

  i2c_read_multi(ADXL_345, READ_REG, NUM_BYTES_RX, acc_data);
#else

  __delay_cycles(SINGLE_ACC_SAMPLE_DURATION);

  acc_data[0] = 1;
  acc_data[1] = 2;
  acc_data[2] = 3;
  acc_data[3] = 4;
  acc_data[4] = 5;
  acc_data[5] = 6;

#endif
  
  result->x = (((int16_t)acc_data[1]) << 8) | acc_data[0];//(__GET(_v_seed)*17)%85;
  result->y = (((int16_t)acc_data[3]) << 8) | acc_data[2];//(__GET(_v_seed)*17*17)%85;
  result->z = (((int16_t)acc_data[5]) << 8) | acc_data[4];;//(__GET(_v_seed)*17*17*17)%85;
        
}


void task_init()
{
    P3OUT |= BIT0;

	GV(pinState) = MODE_IDLE;

	GV(count) = 0;
	GV(seed) = 1;
	P3OUT &= ~BIT0;
	TRANSITION_TO(task_selectMode);

}
void task_selectMode()
{
	P3OUT |= BIT0;

	uint16_t pin_state=1;
	++GV(count);
	if(GV(count) >= 3) pin_state=2;
	if(GV(count)>=5) pin_state=0;

	run_mode_t mode;
	class_t class;


	// Don't re-launch training after finishing training
	if ((pin_state == MODE_TRAIN_STATIONARY ||
				pin_state == MODE_TRAIN_MOVING) &&
			pin_state == GV(pinState)) {
		pin_state = MODE_IDLE;
	} else {
		GV(pinState) = pin_state;
	}


	switch(pin_state) {
		case MODE_TRAIN_STATIONARY:		
			GV(discardedSamplesCount) = 0;
			GV(mode) = MODE_TRAIN_STATIONARY;
			GV(class) = CLASS_STATIONARY;
			GV(samplesInWindow) = 0;

			P3OUT &= ~BIT0;
	
			TRANSITION_TO(task_warmup);
			break;

		case MODE_TRAIN_MOVING:
			GV(discardedSamplesCount) = 0;
			GV(mode) = MODE_TRAIN_MOVING;
			GV(class) = CLASS_MOVING;
			GV(samplesInWindow) = 0;

			P3OUT &= ~BIT0;
	
			TRANSITION_TO(task_warmup);
			break;

		case MODE_RECOGNIZE:
			GV(mode) = MODE_RECOGNIZE;

			P3OUT &= ~BIT0;
	
			TRANSITION_TO(task_resetStats);
			break;

		default:
			P3OUT &= ~BIT0;
	
			TRANSITION_TO(task_idle);
	}
}

void task_resetStats()
{
	P3OUT |= BIT0;

	// NOTE: could roll this into selectMode task, but no compelling reason


	// NOTE: not combined into one struct because not all code paths use both
	GV(movingCount) = 0;
	GV(stationaryCount) = 0;
	GV(totalCount) = 0;

	GV(samplesInWindow) = 0;

			P3OUT &= ~BIT0;
	TRANSITION_TO(task_sample);
}

void task_sample()
{
	 P3OUT |= BIT0;


	accelReading sample;
	ACCEL_singleSample_(&sample);
	GV(window, _global_samplesInWindow) = sample;
	++GV(samplesInWindow);


	if (GV(samplesInWindow) < ACCEL_WINDOW_SIZE) {
			P3OUT &= ~BIT0;

		TRANSITION_TO(task_sample);
	} else {
		GV(samplesInWindow) = 0;
			P3OUT &= ~BIT0;

		TRANSITION_TO(task_transform);
	}
}

void task_transform()
{
	 P3OUT |= BIT0;

	unsigned i;

	accelReading *sample;
	accelReading transformedSample;

	for (i = 0; i < ACCEL_WINDOW_SIZE; i++) {
		if (GV(window, i).x < SAMPLE_NOISE_FLOOR ||
				GV(window, i).y < SAMPLE_NOISE_FLOOR ||
				GV(window, i).z < SAMPLE_NOISE_FLOOR) {

			GV(window, i).x = (GV(window, i).x > SAMPLE_NOISE_FLOOR)
				? GV(window, i).x : 0;
			GV(window, i).y = (GV(window, i).y > SAMPLE_NOISE_FLOOR)
				? GV(window, i).y : 0;
			GV(window, i).z = (GV(window, i).z > SAMPLE_NOISE_FLOOR)
				? GV(window, i).z : 0;
		}
	}
	P3OUT &= ~BIT0;
	TRANSITION_TO(task_featurize);
}

void task_featurize()
{
	P3OUT |= BIT0;

	accelReading mean, stddev;
	mean.x = mean.y = mean.z = 0;
	stddev.x = stddev.y = stddev.z = 0;
	features_t features;


	int i;
	for (i = 0; i < ACCEL_WINDOW_SIZE; i++) {
		mean.x += GV(window, i).x;
		mean.y += GV(window, i).y;
		mean.z += GV(window, i).z;
	}
	mean.x >>= 2;
	mean.y >>= 2;
	mean.z >>= 2;

	for (i = 0; i < ACCEL_WINDOW_SIZE; i++) {
		stddev.x += GV(window, i).x > mean.x ? GV(window, i).x - mean.x
			: mean.x - GV(window, i).x;
		stddev.y += GV(window, i).y > mean.y ? GV(window, i).y - mean.y
			: mean.y - GV(window, i).y;
		stddev.z += GV(window, i).z > mean.z ? GV(window, i).z - mean.z
			: mean.z - GV(window, i).z;
	}
	stddev.x >>= 2;
	stddev.y >>= 2;
	stddev.z >>= 2;

	unsigned meanmag = mean.x*mean.x + mean.y*mean.y + mean.z*mean.z;
	unsigned stddevmag = stddev.x*stddev.x + stddev.y*stddev.y + stddev.z*stddev.z;
	features.meanmag   = isqr_Newton(meanmag);
	features.stddevmag = isqr_Newton(stddevmag);

	switch (GV(mode)) {
		case MODE_TRAIN_STATIONARY:
		case MODE_TRAIN_MOVING:
			GV(features) = features;
			P3OUT &= ~BIT0;
	
			TRANSITION_TO(task_train);
			break;
		case MODE_RECOGNIZE:
			GV(features) = features;
			P3OUT &= ~BIT0;
	
			TRANSITION_TO(task_classify);
			break;
		default:
			// TODO: abort
			break;
	}
}

void task_classify() {
	
	P3OUT |= BIT0;
int move_less_error = 0;
	int stat_less_error = 0;
	int i;
	class_t class;
	long int meanmag;
	long int stddevmag;
	meanmag = GV(features).meanmag;
	stddevmag = GV(features).stddevmag;

	for (i = 0; i < MODEL_SIZE; ++i) {
		long int stat_mean_err = (GV(model_stationary, i).meanmag > meanmag)
			? (GV(model_stationary, i).meanmag - meanmag)
			: (meanmag - GV(model_stationary, i).meanmag);

		long int stat_sd_err = (GV(model_stationary, i).stddevmag > stddevmag)
			? (GV(model_stationary, i).stddevmag - stddevmag)
			: (stddevmag - GV(model_stationary, i).stddevmag);

		long int move_mean_err = (GV(model_moving, i).meanmag > meanmag)
			? (GV(model_moving, i).meanmag - meanmag)
			: (meanmag - GV(model_moving, i).meanmag);

		long int move_sd_err = (GV(model_moving, i).stddevmag > stddevmag)
			? (GV(model_moving, i).stddevmag - stddevmag)
			: (stddevmag - GV(model_moving, i).stddevmag);
		if (move_mean_err < stat_mean_err) {
			move_less_error++;
		} else {
			stat_less_error++;
		}

		if (move_sd_err < stat_sd_err) {
			move_less_error++;
		} else {
			stat_less_error++;
		}
	}

	GV(class) = (move_less_error > stat_less_error) ? CLASS_MOVING : CLASS_STATIONARY;


	P3OUT &= ~BIT0;
	TRANSITION_TO(task_stats);
}

void task_stats()
{
	P3OUT |= BIT0;

	unsigned movingCount = 0, stationaryCount = 0;


	++GV(totalCount);

	switch (GV(class)) {
		case CLASS_MOVING:

			++GV(movingCount);
			break;
		case CLASS_STATIONARY:

			++GV(stationaryCount);
			break;
	}

	if (GV(totalCount) == SAMPLES_TO_COLLECT) {

		unsigned resultStationaryPct = GV(stationaryCount) * 100 / GV(totalCount);
		unsigned resultMovingPct = GV(movingCount) * 100 / GV(totalCount);

		unsigned sum = GV(stationaryCount) + GV(movingCount);
		P3OUT &= ~BIT0;

		TRANSITION_TO(task_idle);
	} else {
		P3OUT &= ~BIT0;

		TRANSITION_TO(task_sample);
	}
}

void task_warmup()
{
	P3OUT |= BIT0;

	threeAxis_t_8 sample;

	if (GV(discardedSamplesCount) < NUM_WARMUP_SAMPLES) {

		ACCEL_singleSample_(&sample);
		++GV(discardedSamplesCount);
			P3OUT &= ~BIT0;

		TRANSITION_TO(task_warmup);
	} else {
		GV(trainingSetSize) = 0;
		P3OUT &= ~BIT0;

		TRANSITION_TO(task_sample);
	}
}

void task_train()
{
	P3OUT |= BIT0;

	unsigned trainingSetSize;;
	unsigned class;

	switch (GV(class)) {
		case CLASS_STATIONARY:
			GV(model_stationary, _global_trainingSetSize) = GV(features);
			break;
		case CLASS_MOVING:
			GV(model_moving, _global_trainingSetSize) = GV(features);
			break;
	}

	++GV(trainingSetSize);

	if (GV(trainingSetSize) < MODEL_SIZE) {
		P3OUT &= ~BIT0;

		TRANSITION_TO(task_sample);
	} else {
		//        PRINTF("train: class %u done (mn %u sd %u)\r\n",
		//               class, features.meanmag, features.stddevmag);
		P3OUT &= ~BIT0;

		TRANSITION_TO(task_idle);
	}
}

void task_idle() {

  	P3OUT |= BIT0;

	//flag the completion of "thread2"
	P3OUT |= BIT4;
  	P3OUT &=~BIT4;

	// P1IE |= BIT4;                               // P1.4 interrupt enabled
	
	P3OUT &= ~BIT0;
	// GV(flag) = 0;
	TRANSITION_TO(task_schedule);
}

INIT_FUNC(init)
ENTRY_TASK(task_schedule)


unsigned isqr_Newton(unsigned x) {
  unsigned rt[3] = {1,0,0};  // current and previous 2 root candidates

  unsigned x2 = x; // Form initial guess
  while (x2 > rt[0]) {
    x2 >>= 1;
    rt[0] <<= 1;
  }

  do {
    rt[2] = rt[1];
    rt[1] = rt[0];
    rt[0] = (x/rt[0] + rt[0])/2;
    } while (rt[1] != rt[0] && rt[2] != rt[0]);
  return (rt[0] + rt[1])/2;
}