#include "ink.h"

//cm_acc Header
#ifndef APP_INTR_H_
#define APP_INTR_H_

/* FFT characteristics definitions */
//Dummy values 
#define N_SAMPLES               16
#define INPUT_AMPLITUDE         0.5
#define INPUT_FREQUENCY         64
#define SAMPL_FREQ              512
#define PI                      3.1415926536
#define FREQ_THRESHOLD          1500

//UART communication codes
#define ACC 13
#define SIGNAL 52
#define FREE 166
#define STOP 26

//Boundaries in "amplitude" for raising a pin 
#define BOUND_SINGAL 1000
#define BOUND_ACC 1250

// Queue
#define QUEUE_SIZE 256
typedef struct {
    uint8_t Data[QUEUE_SIZE];
    uint16_t first,last;
    uint16_t count; 
} queue;



//mcu settings 
void setup_mcu();

//UART communication
void tx_data(uint8_t flag);

void init_queue(queue *q);

void enqueue(queue *q,char x);

char dequeue(queue *q);

//for visual debugging 
void led_signal();

//logic analyzer
void notify_P3P5();
void notify_P3P4();

#endif //APP_INTR_H_
