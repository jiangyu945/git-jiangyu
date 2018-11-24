#include "ets_sys.h"
#include "osapi.h"
#include "user_interface.h"

void ICACHE_FLASH_ATTR
my_sntp_init(void)
{
	sntp_setservername(0,"0.cn.pool.ntp.org");
	sntp_setservername(1,"1.cn.pool.ntp.org");
	sntp_setservername(2,"2.cn.pool.ntp.org");
	sntp_init();

	uint32_t time = sntp_get_current_timestamp();
	os_printf("time:%d\r\n",time);
	os_printf("date:%s\r\n",sntp_get_real_time(time));

}
