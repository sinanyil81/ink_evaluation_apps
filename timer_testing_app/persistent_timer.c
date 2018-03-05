#include "persistent_timer.h"


// 0 persistent - 1 dirty persistent 
// double buffer 
__nv per_timer_t per_timer_vars[2];

void __per_timer_init(){
	uint8_t i;
	for (i = 0; i < MAX_TIMED_THREADS; i++)
	{
			per_timer_vars[0].wkup_timing[i].status = 0; 
	}
}

void _per_timer_update_data(uint8_t idx, uint16_t time_data){
	//update the persistent timer dirty buffer
	per_timer_vars[1].wkup_timing[idx].data = time_data;
	per_timer_vars[1].wkup_timing[idx].__dirty = DIRTY;
}

void _per_timer_update_thread_id(uint8_t idx, uint8_t thread_id){
	//update the persistent timer dirty buffer
	per_timer_vars[1].wkup_timing[idx].thread_id = thread_id;
	per_timer_vars[1].wkup_timing[idx].__dirty = DIRTY;

}

void _per_timer_update_status(uint8_t idx, uint8_t status){
	//update the persistent timer dirty buffer
	per_timer_vars[1].wkup_timing[idx].status = status;
	per_timer_vars[1].wkup_timing[idx].__dirty = DIRTY;
}

void _per_timer_update_nxt_thread(ink_time_interface_t ink_time_interface ,uint8_t next_thread){
	//update the persistent timer dirty buffer
	per_timer_vars[1].next_info[ink_time_interface].next_thread = next_thread;
	per_timer_vars[1].next_info[ink_time_interface].__dirty = DIRTY;
}

void _per_timer_update_nxt_time(ink_time_interface_t ink_time_interface, uint16_t next_time){
	//update the persistent timer dirty buffer
	per_timer_vars[1].next_info[ink_time_interface].next_time = next_time;
	per_timer_vars[1].next_info[ink_time_interface].__dirty = DIRTY;
}

//timer buffer is ready to commit
void _per_timer_update_lock(){

	_tstatus = TIMER_COMMIT;
}

//commit into the persistent buffer

void _commit_timer_buffers(ink_time_interface_t interface){
	
	uint8_t i;
	
	for (i = 0; i < MAX_TIMED_THREADS; i++)
	{
		if (per_timer_vars[1].wkup_timing[i].__dirty == DIRTY)
		{
			
			per_timer_vars[0].wkup_timing[i] = per_timer_vars[1].wkup_timing[i];
			per_timer_vars[0].wkup_timing[i].__dirty = NOT_DIRTY;
		}
	}

	for (i = 0; i < TIMER_TOOLS; i++)
	{
		if (	per_timer_vars[1].next_info[interface].__dirty == DIRTY)
		{
			per_timer_vars[0].next_info[interface] = 
				per_timer_vars[1].next_info[interface];
			per_timer_vars[0].next_info[interface].__dirty = 
				NOT_DIRTY;
		}
	}
}

void _per_timer_commit(ink_time_interface_t interface){
    if (_tstatus == TIMER_COMMIT)
    {
    	_commit_timer_buffers(interface);
        _tstatus = TIMER_DONE;
    }
}

wkup_d _per_timer_get(uint8_t idx){
	return per_timer_vars[0].wkup_timing[idx];

}

uint16_t _per_timer_get_data(uint8_t idx){
	//get the persistent timer from persistent buffer
	return per_timer_vars[0].wkup_timing[idx].data ;
}

uint8_t _per_timer_get_thread_id(uint8_t idx){
	//get the persistent timer from persistent buffer
	return per_timer_vars[0].wkup_timing[idx].thread_id ;

}

uint8_t _per_timer_get_status(uint8_t idx){
	//get the persistent timer from persistent buffer
	return per_timer_vars[0].wkup_timing[idx].status ;
}
//TODO: change types
uint8_t _per_timer_get_nxt_thread(ink_time_interface_t ink_time_interface){
	//get the persistent timer from persistent buffer
	return per_timer_vars[0].next_info[ink_time_interface].next_thread;
}

uint16_t _per_timer_get_nxt_time(ink_time_interface_t ink_time_interface){
	//get the persistent timer from persistent buffer
	return per_timer_vars[0].next_info[ink_time_interface].next_time;
}
