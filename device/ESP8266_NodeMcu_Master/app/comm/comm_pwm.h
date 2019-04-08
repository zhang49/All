/*
 * comm_pwm.h
 *
 *  Created on: Mar 14, 2019
 *      Author: root
 */

#ifndef COMM_PWM_H_
#define COMM_PWM_H_

typedef struct {
	uint32 led_pwm_duty;

}LightCommControl;
LightCommControl light_comm;

/*Definition of GPIO PIN params, for GPIO initialization*/
#define PWM_0_OUT_IO_MUX PERIPHS_IO_MUX_MTDI_U
#define PWM_0_OUT_IO_NUM 12
#define PWM_0_OUT_IO_FUNC  FUNC_GPIO12

#define PWM_1_OUT_IO_MUX PERIPHS_IO_MUX_MTDO_U
#define PWM_1_OUT_IO_NUM 15
#define PWM_1_OUT_IO_FUNC  FUNC_GPIO15

#define PWM_2_OUT_IO_MUX PERIPHS_IO_MUX_MTCK_U
#define PWM_2_OUT_IO_NUM 13
#define PWM_2_OUT_IO_FUNC  FUNC_GPIO13

#define PWM_3_OUT_IO_MUX PERIPHS_IO_MUX_MTMS_U
#define PWM_3_OUT_IO_NUM 14
#define PWM_3_OUT_IO_FUNC  FUNC_GPIO14

#define PWM_4_OUT_IO_MUX PERIPHS_IO_MUX_GPIO5_U
#define PWM_4_OUT_IO_NUM 5
#define PWM_4_OUT_IO_FUNC  FUNC_GPIO5

void ICACHE_FLASH_ATTR comm_pwm_init();

uint32 ICACHE_FLASH_ATTR comm_led_pwm_duty_api_get();
void ICACHE_FLASH_ATTR comm_led_pwm_duty_api_set(uint32 duty);

#endif /* COMM_PWM_H_ */
