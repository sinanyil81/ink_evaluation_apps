#include "app.h"
#include "timer.h"
#include "persistent_timer.h"
//persistent timers "emulation"
typedef uint16_t __per_time_t;

__per_time_t __on_time = 0;
__per_time_t __off_time = 0;

//global
uint8_t tol = 0;
uint8_t nxt_wkup;
uint16_t min_wkup;

uint8_t nxt_expr;
uint16_t min_expr;

uint8_t nxt_pdc;
uint16_t min_pdc;
uint16_t tm_pdc; 

//set a periodic firing of an event 
void set_periodic_timer(uint8_t thread_id, uint16_t ticks){
    uint8_t i,cmpl = 0;

    //__set_expr_timer(__get_thread(thread_id), ticks);
    tm_pdc = ticks;

    for ( i = 0; i < MAX_PDC_THREADS; i++)
    {
        if (pdc_timing[i].status == 0)
        {
            pdc_timing[i].data = ticks;
            pdc_timing[i].thrd_id = thread_id;
            pdc_timing[i].status = 1;
            cmpl = 1;
            break;
        }
    }

    if (cmpl)
    {
        refresh_pdc_timers();
    
    }else{

        //failure 
        //TODO: ADD failcheck 
        pdc_timing[MAX_PDC_THREADS-1].data = ticks;
        pdc_timing[MAX_PDC_THREADS-1].thrd_id = thread_id;
        refresh_expr_timers();
    }
}

//stop the periodic firing of the event 
void stop_periodic_timer(uint8_t thread_id){
    clear_pdc_status(thread_id);

    //TODO:care infinite loop?
    refresh_pdc_timers();
}

/*Internal functions*/
void refresh_pdc_timers(){

    uint8_t i,first = 1;
    uint16_t current_time = __off_time + __on_time;

    for (i = 0; i < MAX_PDC_THREADS; i++)
    {

        if (pdc_timing[i].status == 1)
        {
            
            if (first)
            {
                min_pdc = pdc_timing[i].data;
                nxt_pdc = pdc_timing[i].thrd_id;
                first = 0;
            }
            

            pdc_timing[i].data = pdc_timing[i].data - current_time; 
            if (pdc_timing[i].data < 0 && pdc_timing[i].data > -tol)
            {
                if ((min_pdc > -pdc_timing[i].data) || (min_pdc == -pdc_timing[i].data && nxt_pdc > pdc_timing[i].thrd_id))
                {
                    min_pdc = -pdc_timing[i].data;
                    nxt_pdc = pdc_timing[i].thrd_id;
                }
            }else if (pdc_timing[i].data < 0 && pdc_timing[i].data < -tol) pdc_timing[i].status = 0;

            if (pdc_timing[i].data > 0)
            {   //TODO:priority convention
                if ((min_pdc > pdc_timing[i].data) || (min_pdc == pdc_timing[i].data && nxt_pdc > pdc_timing[i].thrd_id))
                {
                    min_pdc = pdc_timing[i].data;
                    nxt_pdc = pdc_timing[i].thrd_id;
                }
            }
       }
    }

    if (!first)
    {
        //set the new pending ISR timer.
        timerA2_set_CCR0(min_pdc);
    }
    //no pending wake up timer was found.
    else stop_timer();



}

void clear_pdc_status(uint8_t thread_id){
    
    uint8_t i; 
    for (i = 0; i < MAX_PDC_THREADS; i++)
    {
        if (pdc_timing[i].thrd_id == thread_id) pdc_timing[i].status = 0;
    }
}


void stop_timer()
{
    /* stop timer */
    TA2CCTL0 = 0x00;
    TA2CTL = 0;
}

void clear_expr_status(uint8_t thread_id){
    
    uint8_t i;
    for (i = 0; i < MAX_EXPR_THREADS; i++)
    {
        if (expr_timing[i].thrd_id == thread_id) expr_timing[i].status = 0;
    }

}

void refresh_expr_timers(){
    
    uint8_t i,first = 1;
    uint16_t current_time = __off_time + __on_time;

    for (i = 0; i < MAX_EXPR_THREADS; i++)
    {

        if (expr_timing[i].status == 1)
        {
            
            if (first)
            {
                min_expr = expr_timing[i].data;
                nxt_expr = expr_timing[i].thrd_id;
                first = 0;
            }
            

            expr_timing[i].data = expr_timing[i].data - current_time; 
            if (expr_timing[i].data < 0)
            {
                //evict thread 
                __stop_thread(__get_thread(expr_timing[i].thrd_id));
                //update struct
                clear_expr_status(expr_timing[i].thrd_id);
            }

            if (expr_timing[i].data > 0)
            {   //TODO:priority convention
                if ((min_expr > expr_timing[i].data) || (min_expr == expr_timing[i].data && nxt_expr < expr_timing[i].thrd_id))
                {
                    min_expr = expr_timing[i].data;
                    nxt_expr = expr_timing[i].thrd_id;
                }
            }
       }
    }

    if (!first)
    {
        //set the new pending ISR timer.
        timerA2_set_CCR0(min_expr);
    }
    //no pending wake up timer was found.
    else stop_timer();

}


void set_expire_timer(uint8_t thread_id, uint16_t ticks){

    uint8_t i,cmpl = 0;

    __set_expr_timer(__get_thread(thread_id), ticks);

    for ( i = 0; i < MAX_EXPR_THREADS; i++)
    {
        if (expr_timing[i].status == 0)
        {
            expr_timing[i].data = ticks;
            expr_timing[i].thrd_id = thread_id;
            expr_timing[i].status = 1;
            cmpl = 1;
            break;
        }
    }

    if (cmpl)
    {
        refresh_expr_timers();
    
    }else{

        //failure 
        //TODO: ADD failcheck 
        expr_timing[MAX_EXPR_THREADS-1].data = ticks;
        expr_timing[MAX_EXPR_THREADS-1].thrd_id = thread_id;
        refresh_expr_timers();
    }
}


void stop_expire_timer(uint8_t thread_id){
   
    __set_expr_timer(__get_thread(thread_id),0);

    clear_expr_status(thread_id);

    //TODO:care infinite loop?
    refresh_expr_timers();
}


//clears the status flag on wkup_d struct containing the thread/timing information for the one shot timer
void clear_wkup_status(uint8_t thread_id){
    
    uint8_t i; 
    for (i = 0; i < MAX_TIMED_THREADS; i++)
    {
        if (wkup_timing[i].thrd_id == thread_id) wkup_timing[i].status = 0;
    }
}

wkup_d wkup_timing[MAX_TIMED_THREADS];

//unload persistent buffer to local variables for fewer fram accesses
void unpack_to_local(){

    uint8_t i;
    for (i = 0; i < MAX_TIMED_THREADS; i++)
    {
        wkup_timing[i] = _per_timer_get(i);  
    }

}
//updates the information on which thread is scheduled to execute next based on timing 
void refresh_wkup_timers(){

    uint8_t i,first = 1;
    uint16_t current_time = __off_time + __on_time;

    for (i = 0; i < MAX_TIMED_THREADS; i++)
    {

        if (wkup_timing[i].status == 1)
        {
            
            if (first)
            {
                min_wkup = wkup_timing[i].data;
                nxt_wkup = wkup_timing[i].thrd_id;
                first = 0;
            }
            

            wkup_timing[i].data = wkup_timing[i].data - current_time; 
            _per_timer_update_data(i,wkup_timing[i].data);

            if (wkup_timing[i].data < 0 && wkup_timing[i].data > -tol)
            {
                if ((min_wkup > -wkup_timing[i].data) || (min_wkup == -wkup_timing[i].data && nxt_wkup > wkup_timing[i].thrd_id))
                {
                    min_wkup = -wkup_timing[i].data;
                    nxt_wkup = wkup_timing[i].thrd_id;
                }
            }else if (wkup_timing[i].data < 0 && wkup_timing[i].data < -tol) wkup_timing[i].status = 0;

            if (wkup_timing[i].data > 0)
            {   //TODO:priority convention
                if ((min_wkup > wkup_timing[i].data) || (min_wkup == wkup_timing[i].data && nxt_wkup > wkup_timing[i].thrd_id))
                {
                    min_wkup = wkup_timing[i].data;
                    nxt_wkup = wkup_timing[i].thrd_id;
                }
            }
       }
    }

    if (!first)
    {
        _per_timer_update_nxt_thread(WKUP,nxt_wkup);
        _per_timer_update_nxt_time(WKUP,min_wkup);
        //set the new pending ISR timer.
        timerA2_set_CCR0(min_wkup);
    }
    //no pending wake up timer was found.
    else stop_timer();


}

// sets a one-shot timer using Timer A2 
void set_wkup_timer(uint8_t thread_id, uint16_t ticks)
{
    uint8_t i,cmpl = 0;
    
    //set wkup_timing information on the thread 
    //__set_sing_timer(__get_thread(thread_id), ticks);

    //fetch from persistent buffer to local variables 
    unpack_to_local();

    //init the timer 
    stop_timer();

    //set the compare register on the device
    for (i = 0; i < MAX_TIMED_THREADS; i++)
    {
        if (wkup_timing[i].status == 0)
        {
            wkup_timing[i].data = ticks;
            _per_timer_update_data(i,ticks);
            wkup_timing[i].thrd_id = thread_id;
            _per_timer_update_thrd_id(i,thread_id);
            wkup_timing[i].status = 1;
            _per_timer_update_status(i,1);
            cmpl = 1;
            break;
        }
    }

    if (cmpl)
    {
        refresh_wkup_timers();
    
    }else{

        //failure 
        //TODO: ADD failcheck 
        wkup_timing[2].data = ticks;
        _per_timer_update_data(2,ticks);
        wkup_timing[2].thrd_id = thread_id;
        _per_timer_update_thrd_id(i,thread_id);
        refresh_wkup_timers();
    }

    _per_timer_update_lock();
    _per_timer_commit();
}

/* stop timer A2 */
void timerA2_init()
{
    /* stop timer */
    TA2CCTL0 = 0x00;
    TA2CTL = 0;
}

void timerA2_set_CCR0(uint16_t ticks)
{
    TA2CCR0 = ticks; // Take current count + desired count
    TA2CCTL0 = CCIE; // Enable Interrupts on Comparator register
    TA2CTL = TASSEL__ACLK | MC__UP | TACLR; // start timer
}


// the event that will be used to register the uart event
isr_event_t timer_event;

/* sets a one-shot timer using Timer A2 */
void set_timer(uint16_t ticks)
{
    timerA2_set_CCR0(ticks);
}


#ifdef WKUP_TIMER
/* the timer interrupt handler */
//wkup_interrupt
__interrupt(TIMER2_A0_VECTOR)
{
    /* stop timer */
    TA2CCTL0 = 0x00;
    TA2CTL = 0;

    if(!__EVENT_BUFFER_FULL(_per_timer_get_nxt_thrd())){
        timer_event.ISRtype = 0;
        timer_event.data = NULL;
        timer_event.size = 0;
        timer_event.timestamp = TA2CCR0;
        __on_time = TA2CCR0;
        __SIGNAL_EVENT(_per_timer_get_nxt_thrd(),&timer_event);
        
        //TODO: intermittent protection
        unpack_to_local();
        clear_wkup_status(_per_timer_get_nxt_thrd());
        refresh_wkup_timers();
        _per_timer_update_lock();
        _per_timer_commit();
    }

    
    /* turn on CPU */
    __bic_SR_register_on_exit(LPM3_bits);
}
#endif

#ifdef EXPR_TIMER
/* the timer interrupt handler */
//expr_interrupt
__interrupt(TIMER2_A0_VECTOR)
{
    /* stop timer */
    TA2CCTL0 = 0x00;
    TA2CTL = 0;

    __on_time = TA2CCR0;
        
    //TODO: intermittent protection
    __stop_thread(__get_thread(nxt_expr));
    clear_expr_status(nxt_expr);
    refresh_expr_timers();
    
    /* turn on CPU */
    __bic_SR_register_on_exit(LPM3_bits);
}
#endif

#ifdef PDC_TIMER
/* the timer interrupt handler */
//pdc_interrupt
uint16_t pdc_count = 1;
__interrupt(TIMER2_A0_VECTOR)
{
    /* stop timer */
    TA2CCTL0 = 0x00;
    TA2CTL = 0;

    if(!__EVENT_BUFFER_FULL(nxt_pdc)){
        timer_event.ISRtype = 0;
        timer_event.data = NULL;
        timer_event.size = 0;
        timer_event.timestamp = TA2CCR0;
        __on_time = TA2CCR0;
        __SIGNAL_EVENT(nxt_pdc,&timer_event);
        
        //TODO: intermittent protection
        pdc_count++;
        clear_pdc_status(nxt_pdc);
        set_periodic_timer(nxt_pdc,pdc_count*tm_pdc);
    }

    
    /* turn on CPU */
    __bic_SR_register_on_exit(LPM3_bits);
}
#endif
