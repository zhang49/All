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

LOCAL os_timer_t motor_timer;
LOCAL uint16 set_duty=0;            /** PWM占空比变量 */
LOCAL uint8 dir=1;                /** 占空比加减标志 */
uint8 count=0;
static int motor_direction = 1;
static int motor_speed = 4;
static int pin_cur = 0;
static int motor_status;
static int motor_turn_pin_arr[] = {
		MOTOR_PIN1,MOTOR_PIN2,MOTOR_PIN3,MOTOR_PIN4,-1
};

void motor_set_pin_hight(int pin){
	int i;
	for(i=0;motor_turn_pin_arr[i]!=-1;i++)
		GPIO_OUTPUT_SET(motor_turn_pin_arr[i],0);
	GPIO_OUTPUT_SET(pin,1);
}

static void motor_turn_process(void)
{
	if(motor_direction==1){
		pin_cur = (++pin_cur%4)==0?0:pin_cur;
		motor_set_pin_hight(motor_turn_pin_arr[pin_cur]);
	}else if(motor_direction==2){
		pin_cur = (--pin_cur)<0?3:pin_cur;
		motor_set_pin_hight(motor_turn_pin_arr[pin_cur]);
	}
}

void ICACHE_FLASH_ATTR motor_status_set(int status){
	motor_status= status==0?0:1;
	if(motor_status){
		os_timer_disarm(&motor_timer);
		os_timer_arm(&motor_timer, motor_speed, 1);
	}
	else{
		os_timer_disarm(&motor_timer);
	}
}

void ICACHE_FLASH_ATTR motor_direction_set(int direction){
	motor_direction=direction;
	motor_status_set(direction);
}

void ICACHE_FLASH_ATTR motor_speed_set(int speed){
	motor_speed = speed;
	motor_status_set(motor_status);
}

void ICACHE_FLASH_ATTR comm_motor_init()
{
	int i;

	PIN_PULLDWN_EN(PERIPHS_IO_MUX_MTMS_U);
	PIN_PULLDWN_EN(PERIPHS_IO_MUX_GPIO0_U);
	PIN_PULLDWN_EN(PERIPHS_IO_MUX_GPIO4_U);
	PIN_PULLDWN_EN(PERIPHS_IO_MUX_GPIO5_U);

	PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTMS_U, FUNC_GPIO14);//选择GPIO14
	os_memset(&motor_timer,0,sizeof(os_timer_t));
	os_timer_disarm(&motor_timer);
	os_timer_setfn(&motor_timer, motor_turn_process, NULL);
	os_timer_arm(&motor_timer, motor_speed, 1);
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
}
