/*
 * comm_pubDef.h
 *
 *  Created on: Mar 27, 2019
 *      Author: root
 */

#ifndef COMM_PUB_DEF_H_
#define COMM_PUB_DEF_H_


#define COMM_NOREFRESH 0
#define COMM_REFRESHING 1
#define COMM_REFRESHED 2

#define EspNowChannel 1

#define DHT22_MACINDEX 0
#define RELAY1_MACINDEX 1
#define RELAY2_MACINDEX 2
#define RELAY3_MACINDEX 3
#define RAY_MACINDEX 4

enum CommMsgType{
	SYS_COMMAND = 0x00,		//Such as Mode Restore
	NORMAL_CONFIG = 0x01,		//Such as Mode Restore
	SYN_CONTROL = 0x02,		//synchronization control
	SYN_STATE = 0x03,			//synchronization state
	APIVERSION_REQ			= 0x80,
	WIFI_CONFIG_READ_REQ,
	WIFI_CONFIG_WRITE_REQ,
	WIFI_CONNECT,
	WIFI_SCAN,
	CONNECT_SYN_DATA,
	LIGHT_DUTY_VALUE_READ_REQ,
	LIGHT_DUTY_VALUE_WRITE_REQ,
	RAY_MOTOR_START_REQ,
	RAY_MOTOR_STOP_REQ,
	RAY_MOTOR_CW_REQ,
	RAY_MOTOR_CCW_REQ,
	RAY_VALUE_READ_REQ,
	RAY_ALARM_VALUE_READ_REQ
}master_msg_type;


typedef enum {
	RequestMotorMove = 0x01,
	RequestRelay
}EspNowRequestMsgType;

typedef enum {
	ReplyDht22 = 0x81,
	ReplyRay,
	ReplyRelay,
	DeviceOnline
}EspNowReplyMsgType;

enum DeviceType{
	DT_Relay
};
#endif /* COMM_PUBDEF_H_ */
