enum MESSAGETYPE{
	REQUEST_GET_API_VERSION_MSG=1000,//请求获取API版本
	REPLY_AC_GET_API_VERSION_MSG,//回应API版本
	REQUEST_HEARTBEAT_MSG,//请求心跳
	REPLY_HEARTBEAT_MSG,//回应心跳
	REQUEST_GET_CONFIG_MSG,//请求获取配置
	REPLY_GET_CONFIG_MSG,//回应获取配置
	REQUEST_SET_CONFIG_MSG,//请求修改配置
	REPLY_SET_CONFIG_MSG,//回应修改配置
	REQUEST_COMMAND_MSG,//请求一般操作
	REPLY_COMMAND_MSG,//回应一般操作
}NetMsgType;
//一般操作类型
enum CommandType {
	CT_OPEN = 0,      //开门
	CT_CLOSE,     //关门
	CT_FREEZE,    //冻结
	CT_UNFREEZE  //解冻
};
  
  
  
  
  
  
  
  
  