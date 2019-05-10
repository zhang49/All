/*
 * comm_pwm.c
 *
 *  Created on: Mar 14, 2019
 *      Author: root
 */

#include "ets_sys.h"
#include "osapi.h"
#include "user_interface.h"
#include "gpio.h"
#include "comm/comm_pwm.h"
#include "user_config.h"
#include "pwm.h"

LOCAL os_timer_t pwm_timer;
LOCAL uint16 set_duty=0;            /** PWM占空比变量 */
uint8 count=0;
static void ESP8266_PWM_RUN(void)
{

}
void ICACHE_FLASH_ATTR comm_pwm_init()
{
	uint32 io_info[1][3]={                                                // 该参数在ESP8266 SDK的user_light.h中
		{PWM_0_OUT_IO_MUX,PWM_0_OUT_IO_FUNC,PWM_0_OUT_IO_NUM},          //GPIO12
	};

	uint32 duty[1]= {0};
	pwm_init(500,duty,1,io_info);


	pwm_set_duty(light_comm.led_pwm_duty,0);
	pwm_start();
	/*
	os_memset(&pwm_timer,0,sizeof(os_timer_t));
	os_timer_disarm(&pwm_timer);
	os_timer_setfn(&pwm_timer, ESP8266_PWM_RUN, NULL);
	os_timer_arm(&pwm_timer, 30, 1);
	*/
}

uint32 ICACHE_FLASH_ATTR comm_led_pwm_duty_api_get(){
	if(light_comm.led_pwm_duty<=300)
		return 0;
	else
		return (int)(light_comm.led_pwm_duty-300)/(((500*1000/45)-300)/300);
}

void ICACHE_FLASH_ATTR comm_led_pwm_duty_api_set(uint32 duty){
	if(duty!=0)
		light_comm.led_pwm_duty=300+((500*1000/45)-300)/300*duty;
	else
		light_comm.led_pwm_duty=0;
	pwm_set_duty(light_comm.led_pwm_duty,0);
	pwm_start();
}
