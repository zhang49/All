enum MESSAGETYPE{
	REQUEST_GET_API_VERSION_MSG=1000,//�����ȡAPI�汾
	REPLY_AC_GET_API_VERSION_MSG,//��ӦAPI�汾
	REQUEST_HEARTBEAT_MSG,//��������
	REPLY_HEARTBEAT_MSG,//��Ӧ����
	REQUEST_GET_CONFIG_MSG,//�����ȡ����
	REPLY_GET_CONFIG_MSG,//��Ӧ��ȡ����
	REQUEST_SET_CONFIG_MSG,//�����޸�����
	REPLY_SET_CONFIG_MSG,//��Ӧ�޸�����
	REQUEST_COMMAND_MSG,//����һ�����
	REPLY_COMMAND_MSG,//��Ӧһ�����
}NetMsgType;
//һ���������
enum CommandType {
	CT_OPEN = 0,      //����
	CT_CLOSE,     //����
	CT_FREEZE,    //����
	CT_UNFREEZE  //�ⶳ
};
  
  
  
  
  
  
  
  
  