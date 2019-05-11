/*
 * aliyun_mqtt.h
 */

#ifndef _ALIYUN_MQTT_H_
#define _ALIYUN_MQTT_H_

/******************************************************************************/
/* 从阿里云平台获取的三元组 */

#if 0
	#define PRODUCT_KEY 	"PRODUCT_KEY"
	#define DEVICE_NAME 	"DEVICE_NAME"
	#define DEVICE_SECRET	"DEVICE_SECRET"
#else	// 从 user_config.h 导入
	#include "user_config.h"
#endif

// 用户自定义的device id
#define DEVICE_ID		PRODUCT_KEY"."DEVICE_NAME

// 是否使用随机生成的timestamp
//#define RAMDOM_TIMESTAMP

/******************************************************************************/
// 以下不需要变动
#define DOMAIN "iot-as-mqtt.cn-shanghai.aliyuncs.com"

#define MQTT_HOST		PRODUCT_KEY"."DOMAIN
#define MQTT_PORT		1883
#define MQTT_CLIENT_ID	DEVICE_ID"|securemode=3,signmethod=hmacmd5,timestamp=%s|"
#define MQTT_USERNAME	DEVICE_NAME"&"PRODUCT_KEY

//Aliyun Iot topic
#define BASE_TOPIC "/" PRODUCT_KEY "/" DEVICE_NAME
#define REQ_TOPIC BASE_TOPIC "/user/req_dev"
#define RES_TOPIC BASE_TOPIC "/user/res_dev"

#define DEVICE_PROPERTY_TOPIC "/sys" BASE_TOPIC "/thing/event/property/post"
#define WIFI_SCAN_TOPIC "/sys" BASE_TOPIC "/thing/event/WiFiScan/post"
#define WIFI_CONNECT_TOPIC "/sys" BASE_TOPIC "/thing/event/WiFiConnect/post"

#define BUF_SIZE	128

// mqtt struct
typedef struct {
	u8 host[BUF_SIZE];
	u16 port;
	u8 client_id[BUF_SIZE];
	u8 username[BUF_SIZE];
	u8 password[BUF_SIZE];
	u16 keepalive;
} al_mqtt_user_cfg;

#define TIMESTAMP_STR	"789"

/******************************************************************************/
/* interface */
extern void aliyun_mqtt_init(void);
extern void test_hmac_md5(void);

extern al_mqtt_user_cfg cst_al_mqtt;

#endif /* _ALIYUN_MQTT_H_ */
