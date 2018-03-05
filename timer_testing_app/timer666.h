//#ifndef INC_TMR_
//#define INC_TMR_
////clears the status flag on wkup_d struct containing the thread/timing information for the one shot timer
//void clear_wkup_status(uint8_t thread_id);
////updates the information on which thread is scheduled to execute next based on timing
//void refresh_wkup_timers();
//// sets a one-shot timer using Timer A2
//void set_wkup_timer(uint8_t thread_id, uint16_t ticks);
//void refresh_expr_timers();
//void set_expire_timer(uint8_t thread_id, uint16_t ticks);
//void stop_expire_timer(uint8_t thread_id);
//
//void clear_expr_status(uint8_t thread_id);
///* stop timer A2 */
//void timerA2_init();
//void timerA2_set_CCR0(uint16_t ticks);
//void stop_timer();
///* sets a one-shot timer using Timer A2 */
//void set_timer(uint16_t ticks);
//
///*****************************************************************************/
////Interface for "periodic" thread firing
///*****************************************************************************/
//
////set a periodic firing of an event
//void set_periodic_timer(uint8_t thread_id, uint16_t);
////stop the periodic firing of the event
//void stop_periodic_timer(uint8_t thread_id);
//
///*Internal functions*/
//void refresh_pdc_timers();
//void clear_pdc_status(uint8_t thread_id);
//
//void timerA2_set_CCR0(uint16_t ticks);
//#endif
//
