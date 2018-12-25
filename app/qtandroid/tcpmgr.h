#ifndef TCPMGR_H
#define TCPMGR_H

#include <QAbstractSocket>
#include <QtNetwork/QTcpSocket>
#include <QtNetwork>
#include <memory>
#include "tcpcilent.h"
//192.168.20.105 deviceServerIP
static const QString TCPCLIENT_ADDRESS="192.168.4.1";
static const int TCPCLIENT_PORT=7641;

//继承QObject,否则无法connct
class TcpMgr:
        public QObject
{
    Q_OBJECT

public:
    static TcpMgr *getInstance();
    virtual ~TcpMgr();
    void tcpCilent_SendMsg(QString msg);
    int NewTcpClientConnect(QString address,int port);
    void CloseTcpClientConnect();
    void Request_GetApiVersion();
    void Request_Hearbet(uint32_t timestamp);
    void Request_Getconfig();
    void Request_ESP8266SetConfig(QString ssid,QString psw);
    void Request_ESP8266SetRestore();
    void Request_SetConfig(uint8_t RunMode,uint8_t has_lock,uint32_t open_stay_time,uint32_t lock_delay_time);
    void Request_Command(uint8_t CommandType);
private:
    static TcpMgr *CTcpMgr;
    TcpMgr(QObject *parent=Q_NULLPTR);
    tcpCilent *m_tcpCilent;
public slots:
    void slot_TcpClient_ReadMessage(QString recvdata);
};

#endif // TCPMGR_H
