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

#define EspNowChannel 3


#define DHT22_MACINDEX 0
#define RELAY1_MACINDEX 1
#define RELAY2_MACINDEX 2
#define RELAY3_MACINDEX 3
#define RAY_MACINDEX 4

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

#endif /* COMM_PUBDEF_H_ */
