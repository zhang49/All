# Door Control通信接口

## 0、版本记录

| 版本  | 修订人 | 修订时间   | 修订内容 |
| ----- | ------ | ---------- | -------- |
| 1.0.0 | 周红伟 | 2018-12-17 | 初始版本 |
|       |        |            |          |
|       |        |            |          |

## 1、概述

* 此文档定义自动门控制相关的所有接口

* 采用Tcp的方式通信，监听端口7641

* 协议采用“4字节长度“+“2字节类型“+“数据”的格式

  | 0~3    | 4~5  | 6~... |
  | ------ | ---- | ----- |
  | length | type | data  |

* length: 整型，type+data的长度，不包含length本身，单个消息最大长度2M字节

* type: 整型，消息类型，每一个请求类型对应一个回应类型

* data: 二进制，消息结构体

  * 格式采用protobuf，版本v2

  * 所有的“请求“都包含一个request成员，request定义如下（具体接口不再说明）：
    ```
    //请求包通用子项定义
    message Request
    {
    	optional int32 serial_num = 1;    //请求序列号，用于标识请求，可以从1开始每发送一次请求就累加1
    }
    ```

  * 所有的“回应”都包含一个reply成员，reply定义如下（具体接口不再说明）：
    ```
    //回应包通用子项定义
    message Reply
    {
    	optional int32 error_code = 1;    //0:成功，<0: 错误，>0: 提示或警告(1：执行中)
    	optional string error_str = 2;    //error_code为非0时的失败详细信息
    	optional int32 serial_num = 3;    //返回序列号，把请求包里的serial_num值传回
    }
    ```

## 2、获取API版本

- 1）请求
  - type: REQUEST_GET_API_VERSION_MSG，值1000
  - data:

```
//获取API版本信息
message Request_GetApiVersionMsg
{
	optional Request request = 1;
}
```
- 2）回应

  - type: REPLY_AC_GET_API_VERSION_MSG，值1001
  - data:

```
//获取API版本信息回应
message Reply_GetApiVersionMsg
{
	optional Reply reply = 1;
	optional string api_version = 2;  //为客户端API版本信息  每一次涉及到接口更新，版本号都加1，数字从0到9，9以后从0开始，比如1.0.0，1.0.1，1.0.2... 1.0.9，1.1.0等
	optional string app_version = 3;  //为客户端版本信息，包含build序号，比如1.0.0.2349
}
```
## 3、心跳

- 1）请求
    - type：REQUEST_HEARTBEAT_MSG，值1002

    - data：

  ```
  //心跳请求，客户端发送
  message Request_HeartbeatMsg
  {
  	optional Request request = 1;
  	optional int32 time_tick = 2;   //时间戳
  }
  ```

- 2）回应

    - type: REPLY_HEARTBEAT_MSG，值1003
    - data:
  ```
  //当前状态
  enum RunState{
    RS_READY = 0;            //上电就绪状态
    RS_STUDY_STEP_1 = 1;     //学习第一步（开门）
    RS_STUDY_STEP_2 = 2;     //学习第二步（关门）
    RS_STUDY_STEP_3 = 3;     //学习第三步（开门）
    RS_CLOSED = 4;           //关门，没落锁
    RS_LOCKED = 5;           //上锁
    RS_UNLOCKING = 6;        //解锁中（解锁后会自动进入开门流程）
    RS_OPENING = 7;          //开门中
    RS_OPENED = 8;           //已开门
    RS_CLOSING = 9;          //关门中
    RS_ALWAYS_OPENED = 10;   //常开
    RS_FREEZED = 11;         //冻结（按遥控的锁定进入冻结状态，和关门和锁定状态可以进入冻结状态）
    RS_ERROR = 12;            //错误(比如学习失败)
};
  //心跳回应，服务端回应
  message Reply_HeartbeatMsg
  {
  	optional Reply reply = 1;
  	optional int32 time_tick = 2;   //时间戳，把请求包里的time_tick值传回
  	optional RunState state = 3;    //硬件当前状态
  }
  ```
## 4、获取配置

- 1）请求
    - type: REQUEST_GET_CONFIG_MSG，值1004

    - data:

  ```
  //同步配置请求
  message Request_GetConfigMsg
  {
  	optional Request request = 1;  	
  }
  ```

- 2）回应

    - type: REPLY_GET_CONFIG_MSG，值1005

    - data:

  ```
  //运行模式
  enum RunMode{
  	RM_SINGLE = 0;     //单门
  	RM_DOUBLE = 1;     //双门
  };

  //同步配置回应
  message Reply_GetConfigMsg
  {
  	optional Reply reply = 1;  	
  	optional RunMode mode = 2;           //运行模式
  	optional int32 has_lock = 3;         //是否带锁：1：带锁，0：不带
  	optional int32 open_stay_time = 4;   //开门停留时间，单位ms
  	optional int32 lock_delay_time = 5;  //上锁前、开锁后的等待时间，单位ms  	
  }
  ```

## 5、修改配置

- 1）请求
    - type: REQUEST_SET_CONFIG_MSG，值1006

    - data:

  ```
  //同步配置请求
  message Request_SetConfigMsg
  {
  	optional Request request = 1;
  	optional RunMode mode = 2;           //运行模式
  	optional int32 has_lock = 3;         //是否带锁：1：带锁，0：不带
  	optional int32 open_stay_time = 4;   //开门停留时间，单位ms
  	optional int32 lock_delay_time = 5;  //上锁前、开锁后的等待时间，单位ms
  }
  ```

- 2）回应

    - type: REPLY_SET_CONFIG_MSG，值1007

    - data:

  ```
  //同步配置回应
  message Reply_SetConfigMsg
  {
  	optional Reply reply = 1;
  }
  ```


## 6、一般操作

- 1）请求
    - type: REQUEST_COMMAND_MSG，值1008

    - data:

  ```
  //一般操作类型
  enum CommandType {
  	CT_OPEN = 0;      //开门
  	CT_CLOSE = 1;     //关门
  	CT_FREEZE = 2;    //冻结
  	CT_UNFREEZE = 3;  //解冻
  };

  //平台一般操作
  message Request_CommandMsg
  {
  	optional Request request = 1;
  	optional CommandType command = 2;   //操作类型  	
  }
  ```

- 2）回应

    - type: REPLY_COMMAND_MSG，值1009

    - data:

  ```
  //平台一般操作
  message Reply_CommandMsg
  {
  	optional Reply reply = 1;
  }
  ```

