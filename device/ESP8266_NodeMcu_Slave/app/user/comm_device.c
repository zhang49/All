/*
 * comm_device.c
 *
 *  Created on: Mar 26, 2019
 *      Author: root
 */
#include "comm_device.h"
#include "sensor/dht22.h"
#include "osapi.h"
#include "ets_sys.h"
#include "user_interface.h"
#include "user_config.h"
#include "espnow.h"

#include "gpio.h"

#include "comm_pub_def.h"

dht_data dhtData;
uint8_t ray_value;
uint8_t ray_alarm_value;
//second, set 0 for test
uint8_t ray_alarm_duration = 0;
uint8_t ray_alarm_time_cur;
LOCAL os_timer_t motor_timer;
os_timer_t statusTimer;
int timer_tick=0;
extern u8 ControllerMacAddr[6];

static motor_turn_status motor_direction = Motor_Stop;
static int motor_speed = 4;
static int pin_cur = 0;
static int motor_status;
static int motor_turn_pin_arr[] = {
		MOTOR_PIN1,MOTOR_PIN2,MOTOR_PIN3,MOTOR_PIN4,-1
};
//100 ms multi,Max 255 * 100	== 0 cancel auto, == 1 manual move
//need a distance sensor??
static uint32_t motor_duration = 10240;
static uint32_t motor_move_time_cur = 0;

static void motor_turn_process(void)
{
	motor_move_time_cur++;
	if(motor_move_time_cur >= motor_duration){
		motor_direction_set(Motor_Stop);
	}
	if(motor_direction==Motor_Pos){
		pin_cur = (++pin_cur%4)==0?0:pin_cur;
		motor_set_pin_hight(motor_turn_pin_arr[pin_cur]);
	}else if(motor_direction==Motor_Pas){
		pin_cur = (--pin_cur)<0?3:pin_cur;
		motor_set_pin_hight(motor_turn_pin_arr[pin_cur]);
	}
}
static char displaybuf[50]={0};
static int counttest=0;
//touch when every 1000ms
static void deviceStatusTimerCb(void *arg){
uint8 retdata[5];
#ifdef DHT22_OPEN
	timer_tick++;
	if(timer_tick <= 8)return;
	timer_tick=0;
	dht22_read(&dhtData);
	dht22_temperature_read_api(retdata);
	dht22_humidity_read_api(retdata+3);
	esp_now_send_api(ControllerMacAddr,ReplyDht22,retdata);
	os_sprintf(displaybuf, "temperature:%-10f,hum:%-10f",dhtData.temp,dhtData.hum);
	NODE_DBG("%d_%s",counttest++,displaybuf);

#elif defined RELAY1_OPEN
#elif defined RELAY2_OPEN
#elif defined RELAY3_OPEN
#elif defined RAY_OPEN
	uint16 readdata=system_adc_read();
	ray_value = readdata/1024.0*255;
	retdata[0]=ray_value;
	esp_now_send_api(ControllerMacAddr,ReplyRay,retdata);
#endif
}

void comm_device_init(void){
int op=0;
#ifdef DHT22_OPEN
	dht22_init();
#elif defined RELAY1_OPEN
	op=1;
#elif defined RELAY2_OPEN
	op=1;
#elif defined RELAY3_OPEN
	op=1;
#elif defined LIGHT_OPEN

#elif defined RAY_OPEN
	PIN_PULLDWN_EN(PERIPHS_IO_MUX_MTMS_U);
	PIN_PULLDWN_EN(PERIPHS_IO_MUX_GPIO0_U);
	PIN_PULLDWN_EN(PERIPHS_IO_MUX_GPIO4_U);
	PIN_PULLDWN_EN(PERIPHS_IO_MUX_GPIO5_U);

	PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTMS_U, FUNC_GPIO14);//选择GPIO14

	//PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO0_U, RELAY_PIN);//选择GPIO0
	GPIO_OUTPUT_SET(GPIO_ID_PIN(16), 0);
	os_memset(&motor_timer,0,sizeof(os_timer_t));
	os_timer_disarm(&motor_timer);
	os_timer_setfn(&motor_timer, motor_turn_process, NULL);
#endif

	switch(op){
	case 1:
		comm_relay_init();
		break;
	}
	uint8 retdata[5];
	esp_now_send_api(ControllerMacAddr,DeviceOnline,retdata);
	os_memset(&statusTimer,0,sizeof(os_timer_t));
	os_timer_disarm(&statusTimer);
	os_timer_setfn(&statusTimer, (os_timer_func_t *)deviceStatusTimerCb, NULL);
	os_timer_arm(&statusTimer, 1000, 1);
}

void ICACHE_FLASH_ATTR comm_relay_init(){
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO0_U, RELAY_PIN);//选择GPIO0
}

void ICACHE_FLASH_ATTR comm_relay_status_set(u8 level){
	GPIO_OUTPUT_SET(RELAY_PIN, level);
}

void dht22_humidity_read_api(u8 *data){
	data[0]=((int)(dhtData.hum/1));
	data[1]=((int)((dhtData.hum-data[0])*100));
	NODE_DBG("Humidity    Integet:%d,Point:%d",data[0],data[1]);
}

void dht22_temperature_read_api(u8 *data){
	if(dhtData.temp<0){
		dhtData.temp*=-1;
		data[0]='-';
	}else{
		data[0]='+';
	}
	data[1]=((int)(dhtData.temp/1));
	data[2]=((int)((dhtData.temp-data[1])*100));
	NODE_DBG("Temperature sign:%c, Integet:%d, Point:%d",data[0],data[1],data[2]);
}

uint8 ray_value_read(){
	return ray_value;
}

void ICACHE_FLASH_ATTR motor_duration_set(uint16_t time){
	motor_duration = time;
}

void ICACHE_FLASH_ATTR motor_speed_set(int speed){
	motor_speed = speed;
	motor_direction_set(motor_status);
}

void ICACHE_FLASH_ATTR motor_direction_set(motor_turn_status direction){
	motor_direction=direction;
	os_timer_disarm(&motor_timer);
	switch(direction){
	case Motor_Stop:
		motor_move_time_cur = 0;
		break;
	case Motor_Pos:
	case Motor_Pas:
		os_timer_arm(&motor_timer, motor_speed, 1);
		break;
	}
}

void motor_set_pin_hight(int pin){
	int i;
	for(i=0;motor_turn_pin_arr[i]!=-1;i++)
		GPIO_OUTPUT_SET(motor_turn_pin_arr[i],0);
	GPIO_OUTPUT_SET(pin,1);
}

void ICACHE_FLASH_ATTR motor_move_start(uint8 speed,uint16 duration,motor_turn_status direction){
	NODE_DBG("speed:%d,duration:%d,direction:%d",speed,duration*10,direction);
	motor_speed_set(speed);
	motor_duration_set(duration*10*4);
	motor_direction_set(direction);
}






