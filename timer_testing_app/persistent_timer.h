////persistent timers data
//#ifndef PERS_TMR_
//#define PERS_TMR_
//#include "app.h"
//
//enum{TIMER_INSERT, TIMER_COMMIT,TIMER_DONE } ;
//
//enum{DIRTY , NOT_DIRTY};
//
//enum{USED, NOT_USED};
//
//typedef enum{WKUP,PDC,EXPR}ink_time_interface_t;
//
//static volatile __nv uint8_t _tstatus = TIMER_DONE;
//
////how many timing Ink interface are used
//#define TIMER_TOOLS 3
//#define MAX_TIMED_THREADS 3
//#define MAX_EXPR_THREADS 3
//#define MAX_PDC_THREADS 1
//
//
//typedef struct
//{
//    uint8_t status;     // 1 active - 0 cleared
//    uint8_t thread_id;    // thread ID
//    uint16_t data;      // remaining time for thread execution
//    uint8_t __dirty;    //
//
//}wkup_d;
//
//typedef struct
//{
//	uint8_t status;     // USED - NOT_USED
//	uint8_t next_thread;// next thread candidate
//	uint16_t next_time; // next timing candidate
//	uint8_t __dirty;	// touched on the dirty buffer
//
//}next_d;
//
//typedef struct
//{
//	uint16_t on_time;
//	uint16_t off_time;
//	uint8_t __dirty;
//
//}per_time_d;
//
//typedef struct
//{
//	wkup_d wkup_timing[MAX_TIMED_THREADS];
//	next_d next_info[TIMER_TOOLS];
//	per_time_d time_bank;
//
//}per_timer_t;
//
//
//
////Initialize
//void __per_timer_init();
//
////update data in the dirty buffer
//void _per_timer_update_data(uint8_t idx, uint16_t time_data);
//
//void _per_timer_update_thread_id(uint8_t idx, uint8_t thread_id);
//
//void _per_timer_update_status(uint8_t idx, uint8_t status);
//
//void _per_timer_update_nxt_thread(ink_time_interface_t ink_time_interface ,uint8_t next_thread);
//
//void _per_timer_update_nxt_time(ink_time_interface_t ink_time_interface, uint16_t next_time);
//
//
////collect data from the persistent buffer
//wkup_d _per_timer_get(uint8_t idx);
//
//uint16_t _per_timer_get_data(uint8_t idx);
//
//uint8_t _per_timer_get_thread_id(uint8_t idx);
//
//uint8_t _per_timer_get_status(uint8_t idx);
//
//uint8_t _per_timer_get_nxt_thread(ink_time_interface_t ink_time_interface);
//
//uint16_t _per_timer_get_nxt_time(ink_time_interface_t ink_time_interface);
//
//
////timer buffer is ready to commit
//void _per_timer_update_lock();
//
////commit into the persistent buffer
//void _per_timer_commit(ink_time_interface_t interface);
////internal function
//void _commit_timer_buffers(ink_time_interface_t interface);
//
//#endif
