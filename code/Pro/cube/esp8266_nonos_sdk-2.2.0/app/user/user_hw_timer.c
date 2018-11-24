#include "ets_sys.h"
#include "osapi.h"
#include "user_interface.h"
#include "mem.h"
#include "espconn.h"

#include "driver/hw_timer.h"

void user_hw_timer_cb(void) {
	// TODO:
}

// 这个函数不需要改动
// HW timer
void hw_test_timer_cb(void) {
	LOCAL uint32 tick_now2 = 0,idx = 0;

	//硬件定时器计数的最小单位为us！
	if ((WDEV_NOW() - tick_now2) >= 1000000)   //1000 000代表1s
	{
		tick_now2 = WDEV_NOW();
		os_printf("count_second:%u\n", ++idx);

		user_hw_timer_cb();
	}
}

void ICACHE_FLASH_ATTR
user_hw_timer_init(uint32 us) {
	hw_timer_init(FRC1_SOURCE, 1);   //FRC1不能打断其它ISR
	hw_timer_set_func(hw_test_timer_cb); //回调函数hw_test_timer_cb（）前不能添加ICACHE_FLASH_ATTR宏！
	hw_timer_arm(us);
}

void ICACHE_FLASH_ATTR
user_hw_timer_delete(void) {
	hw_timer_set_func(NULL);
}
