# Door Control通信接口

## 0、版本记录

| 版本    | 修订人  | 修订时间       | 修订内容 |
| ----- | ---- | ---------- | ---- |
| 1.0.0 | 周红伟  | 2018-12-17 | 初始版本 |
|       |      |            |      |
|       |      |            |      |

## 1、概述

* 此文档定义自动门控制相关的所有接口

* 采用Tcp的方式通信，监听端口7641

* 协议采用“2字节长度“+“2字节类型“+“数据”的格式

  | 0~1    | 2~3  | 4~... |
  | ------ | ---- | ----- |
  | length | type | data  |

* length: 整型，type+data的长度，不包含length本身，单个消息最大长度2M字节

* type: 整型，消息类型，每一个请求类型对应一个回应类型

* data: 二进制，消息结构体

  * 格式采用json

  * 所有的"请求"或"回应"都包含"type","data"
    ```
    //通用子项定义
    {
    	"type" : ""
    	"data" : ""
    }
    或者
    {
    	"type" : ""
    	"data" : {
          "key" : "value"
    	}
    }
    ```


## 2、获取API版本

- 1）请求

```
//获取API版本信息
{
	"type" : "Request_GetApiVersion",
	"data":""
}
```
- 2）回应


```
//获取API版本信息回应
{
	"type" : "Reply_GetApiVersion",
	"data" : {
    	"api_version" : "2",  //为客户端API版本信息  每一次涉及到接口更新，版本号都加1，数字从0到9，9以后从0开始，比如1.0.0，1.0.1，1.0.2... 1.0.9，1.1.0等
		"string app_version" : "3"  //为客户端版本信息，包含build序号，比如1.0.0.2349
	}
}
```
## 3、心跳

- 1）请求

  ```
  //心跳请求，客户端发送
  {
  	"type" ："Request_Hearbet"
  	"data" : {
     	"time_tick" : "2";   //时间戳 
  	}
  }
  ```

- 2）回应

  ```
  //当前状态
  enum RunState{
    uint8_t RS_READY = 0;            //上电就绪状态
    uint8_t RS_STUDY_STEP_1 = 1;     //学习第一步（开门）
    uint8_t RS_STUDY_STEP_2 = 2;     //学习第二步（关门）
    uint8_t RS_STUDY_STEP_3 = 3;     //学习第三步（开门）
    uint8_t RS_CLOSED = 4;           //关门，没落锁
    uint8_t RS_LOCKED = 5;           //上锁
    uint8_t RS_UNLOCKING = 6;        //解锁中（解锁后会自动进入开门流程）
    uint8_t RS_OPENING = 7;          //开门中
    uint8_t RS_OPENED = 8;           //已开门
    uint8_t RS_CLOSING = 9;          //关门中
    uint8_t RS_ALWAYS_OPENED = 10;   //常开
    uint8_t RS_FREEZED = 11;         //冻结（按遥控的锁定进入冻结状态，和关门和锁定状态可以进入冻结状态）
    uint8_t RS_ERROR = 12;            //错误(比如学习失败)
};
  //心跳回应，服务端回应
  
  {
  	"type" : "Reply_Heartbeat",
  	"data" : {
  		"time_tick" : "2",   //时间戳，把请求包里的time_tick值传回
  		"RunState" : "RunState Num";    //硬件当前状态
  }
  ```
## 4、获取配置

- 1）请求

  ```
  //同步配置请求
  {
  	"type" : "Request_Getconfig",
  	"data" : ""
  }
  ```

- 2）回应


  ```
  //运行模式
  enum RunMode{
  	uint8_t RM_SINGLE = 0;     //单门
  	uint8_t RM_DOUBLE = 1;     //双门
  };

  //同步配置回应
  {
  	"type" : "Reply_Getconfig"
    "data" : {
      "RunMode" : "0",           //运行模式
      "has_lock" : "1",         //是否带锁：1：带锁，0：不带
      "open_stay_time" : "3",   //开门停留时间，单位ms
      "lock_delay_time" : "4"  //上锁前、开锁后的等待时间，单位ms  	  
    }
  }
  ```

## 5、修改ESP8266模块配置

- 1）请求

```
  //修改ESP8266模块配置请求
  {
  	"type" : "Request_ESP8266SetConfig",
  	"data" : {
  		"mode" : "ap",				//配置某模式的信息
  		"ssid" : "ssid",           //SSID名称
  		"psw" : "psw",         //PSW密码
  	}
  }
```

- 2）回应

```
  //修改ESP8266模块配置回应
  {
  	"type" : "Reply_ESP8266SetConfig",
  	"data" : "success"  //or failed
  }
```

## 6、还原ESP8266模块配置

- 1）请求

```
  //还原ESP8266模块配置请求
  {
  	"type" : "Request_ESP8266SetRestore",
  	"data" : ""
  }
```

- 2）回应

```
  //还原ESP8266模块配置回应
  {
  	"type" : "Reply_ESP8266SetRestore",
  	"data" : ""  //or failed
  }
```

## 7、修改自动门配置

- 1）请求

```
  //同步配置请求
  {
  	"type" : "Request_SetConfig",
  	"data" : {
  		"RunMode" : "0",           //运行模式
  		"has_lock" : "1",         //是否带锁：1：带锁，0：不带
  		"open_stay_time" : "4",   //开门停留时间，单位ms
  		"lock_delay_time" : "5";  //上锁前、开锁后的等待时间，单位ms  
  	}
  }
```

- 2）回应

```
  //同步配置回应
  {
  	"type" : "Reply_SetConfig",
  	"data" : ""
  }
```

## 8、一般操作

- 1）请求

  ```
  //一般操作类型
  enum CommandType {
  	uint8_t CT_OPEN = 0;      //开门
  	uint8_t CT_CLOSE = 1;     //关门
  	uint8_t CT_FREEZE = 2;    //冻结
  	uint8_t CT_UNFREEZE = 3;  //解冻
  };

  //平台一般操作
  {
  	"type" : "Request_Command",
  	"data" : {
    	"CommandType" : "2";   //操作类型  	  
  	}
  }
  ```

- 2）回应


  ```
  //平台一般操作
  {
  	"type" : "Reply_Command",
  	"data" : ""
  }
  ```

