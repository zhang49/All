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

enum MasterMsgType{
	SYS_COMMAND = 0x00,		//Such as Mode Restore
	NORMAL_CONFIG = 0x01,		//Such as Mode Restore
	SYN_CONTROL = 0x02,		//synchronization control
	SYN_STATE = 0x03,			//synchronization state
	WIFI_CONNECT = 0xfd,
	WIFI_SCAN = 0xfe
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
