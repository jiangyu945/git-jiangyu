#ifndef _USER_GPIO_H_
#define _USER_GPIO_H_


#define INT_GPIO	12

#define TIMER_GPIO	4
#define GPIO_SET_INTERVAL	1000


void user_gpio_interrupt_init(void);
void user_gpio_timer_init(void);

#endif
