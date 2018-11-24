#include "ets_sys.h"
#include "osapi.h"
#include "user_interface.h"
#include "mem.h"
#include "espconn.h"

#include "driver/hw_timer.h"

void user_hw_timer_cb(void) {
	// TODO:
}

// �����������Ҫ�Ķ�
// HW timer
void hw_test_timer_cb(void) {
	LOCAL uint32 tick_now2 = 0,idx = 0;

	//Ӳ����ʱ����������С��λΪus��
	if ((WDEV_NOW() - tick_now2) >= 1000000)   //1000 000����1s
	{
		tick_now2 = WDEV_NOW();
		os_printf("count_second:%u\n", ++idx);

		user_hw_timer_cb();
	}
}

void ICACHE_FLASH_ATTR
user_hw_timer_init(uint32 us) {
	hw_timer_init(FRC1_SOURCE, 1);   //FRC1���ܴ������ISR
	hw_timer_set_func(hw_test_timer_cb); //�ص�����hw_test_timer_cb����ǰ�������ICACHE_FLASH_ATTR�꣡
	hw_timer_arm(us);
}

void ICACHE_FLASH_ATTR
user_hw_timer_delete(void) {
	hw_timer_set_func(NULL);
}
