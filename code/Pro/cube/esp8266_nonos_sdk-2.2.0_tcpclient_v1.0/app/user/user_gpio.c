#include "ets_sys.h"
#include "osapi.h"
#include "user_interface.h"

#include "user_gpio.h"

/*
 * function: user_gpio_interrupt
 * description: GPIO�жϴ�����
 */
void user_gpio_interrupt(void *arg)
{
	u32 gpio_status;
	gpio_status = GPIO_REG_READ(GPIO_STATUS_ADDRESS);
	//clear interrupt status
	GPIO_REG_WRITE(GPIO_STATUS_W1TC_ADDRESS, gpio_status);

	u8 gpio = GPIO_INPUT_GET(INT_GPIO); //��ȡ�жϹܽŵ�ƽ״̬
	os_delay_us(20 * 1000);		// ��ʱ20ms��ȥ����
	if (gpio == GPIO_INPUT_GET(INT_GPIO))    //ȷ�ϴ���
	{
		os_printf("GPIO %d: %d\r\n", INT_GPIO, gpio);
	}
}

/*
 * function: user_gpio_interrupt_init
 * description: GPIO�жϴ����ʼ��
 */
void ICACHE_FLASH_ATTR
user_gpio_interrupt_init(void)
{
	// �ܽŹ���ѡ��
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTDI_U, FUNC_GPIO12);
	// ���ùܽ�Ϊ����
	GPIO_DIS_OUTPUT(INT_GPIO);
	/*
	 * GPIO_PIN_INTR_DISABLE - ����
	 * GPIO_PIN_INTR_POSEDGE - ������
	 * GPIO_PIN_INTR_NEGEDGE - �½���
	 * GPIO_PIN_INTR_ANYEDGE - ˫����
	 * GPIO_PIN_INTR_LOLEVEL - �͵�ƽ
	 * GPIO_PIN_INTR_HILEVEL - �ߵ�ƽ
	 */

	gpio_pin_intr_state_set(GPIO_ID_PIN(INT_GPIO), GPIO_PIN_INTR_POSEDGE);

	// ע�� GPIO �жϴ�����
	ETS_GPIO_INTR_ATTACH(user_gpio_interrupt, NULL);

	// �� GPIO �ж�
	//ETS_GPIO_INTR_DISABLE();

	// �� GPIO �ж�
	ETS_GPIO_INTR_ENABLE();

	os_printf("user_gpio_interrupt_init\r\n");
}

/*
 * function: gpio_timer_cb
 * description: ��ʱ���ص�����GPIO
 */
void ICACHE_FLASH_ATTR
gpio_timer_cb(void *arg)
{
	u8 gpio = GPIO_INPUT_GET(TIMER_GPIO);
	GPIO_OUTPUT_SET(TIMER_GPIO, 1 - gpio);
	//os_printf("GPIO 0: %d\r\n", gpio);
}

/*
 * function: user_gpio_timer_init
 * description: ��ʱ�ı�GPIO��״̬
 */
LOCAL  os_timer_t gpio_timer;

void ICACHE_FLASH_ATTR
user_gpio_timer_init(void)
{
	// �ܽŹ���ѡ��
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO4_U, FUNC_GPIO4);

	os_timer_disarm(&gpio_timer);
	os_timer_setfn(&gpio_timer, (os_timer_func_t *) gpio_timer_cb, NULL);
	os_timer_arm(&gpio_timer, GPIO_SET_INTERVAL, 1);

	os_printf("user_gpio_timer_init\r\n");
}

