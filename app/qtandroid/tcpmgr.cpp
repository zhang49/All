#include "tcpmgr.h"
#include "tcpcilent.h"
#include "qjson.h"
#include <QAbstractSocket>
#include <QtNetwork/QTcpSocket>
#include <QtNetwork>
#include <QTimer>
#include <QJsonObject>
#include <memory>

TcpMgr * TcpMgr::CTcpMgr=nullptr;
TcpMgr::TcpMgr(QObject *parent)
{

}
TcpMgr *TcpMgr::getInstance()
{
    if(CTcpMgr==nullptr)CTcpMgr=new TcpMgr();
    return CTcpMgr;
}
int TcpMgr::NewTcpClientConnect(QString address,int port)
{
    m_tcpCilent=new tcpCilent();
    QObject::connect(m_tcpCilent,SIGNAL(signal_TcpCilent_HasRecvData(QString)),this,SLOT(slot_TcpClient_ReadMessage(QString)));
    m_tcpCilent->NewConnect(address,port);
    return 1;
}

void TcpMgr::CloseTcpClientConnect()
{
    m_tcpCilent=nullptr;
}
void TcpMgr::tcpCilent_SendMsg(QString msg)
{
    m_tcpCilent->SendData(msg);
}
void TcpMgr::Request_Hearbet(uint32_t timestamp)
{
    QJsonObject root;
    QJson::json_add(root,"type","Request_Hearbet");
    QJson::json_add(root,"data.time_tick",QString::number(timestamp));
    QJsonDocument jdoc=QJsonDocument(root);
    QString out(jdoc.toJson(QJsonDocument::Compact));
    qDebug()<<out<<endl;
    tcpCilent_SendMsg(out);
}
void TcpMgr::Request_GetApiVersion()
{
    QJsonObject root;
    QJson::json_add(root,"type","Request_GetApiVersion");
    QJson::json_add(root,"data","");
    QJsonDocument jdoc=QJsonDocument(root);
    QString out(jdoc.toJson(QJsonDocument::Compact));
    qDebug()<<out<<endl;
    tcpCilent_SendMsg(out);
}
void TcpMgr::Request_Getconfig()
{
    QJsonObject root;
    QJson::json_add(root,"type","Request_Getconfig");
    QJson::json_add(root,"data","");
    QJsonDocument jdoc=QJsonDocument(root);
    QString out(jdoc.toJson(QJsonDocument::Compact));
    qDebug()<<out<<endl;
    tcpCilent_SendMsg(out);

}
void TcpMgr::Request_ESP8266SetConfig(QString ssid,QString psw)
{
    QJsonObject root;
    QJson::json_add(root,"type","Request_ESP8266SetConfig");
    QJson::json_add(root,"data.mode","ap");
    QJson::json_add(root,"data.ssid",ssid);
    QJson::json_add(root,"data.psw",psw);
    QJsonDocument jdoc=QJsonDocument(root);
    QString out(jdoc.toJson(QJsonDocument::Compact));
    qDebug()<<out<<endl;
    tcpCilent_SendMsg(out);
}
void TcpMgr::Request_ESP8266SetRestore()
{
    QJsonObject root;
    QJson::json_add(root,"type","Request_ESP8266SetConfig");
    QJson::json_add(root,"data","");
    QJsonDocument jdoc=QJsonDocument(root);
    QString out(jdoc.toJson(QJsonDocument::Compact));
    qDebug()<<out<<endl;
    tcpCilent_SendMsg(out);
}
void TcpMgr::Request_SetConfig(uint8_t RunMode,uint8_t has_lock,uint32_t open_stay_time,uint32_t lock_delay_time)
{
    QJsonObject root;
    QJson::json_add(root,"type","Request_SetConfig");
    QJson::json_add(root,"data.RunMode",QString::number(RunMode));
    QJson::json_add(root,"data.has_lock",QString::number(has_lock));
    QJson::json_add(root,"data.open_stay_time",QString::number(open_stay_time));
    QJson::json_add(root,"data.lock_delay_time",QString::number(lock_delay_time));
    QJsonDocument jdoc=QJsonDocument(root);
    QString out(jdoc.toJson(QJsonDocument::Compact));
    qDebug()<<out<<endl;
    tcpCilent_SendMsg(out);

}
void TcpMgr::Request_Command(uint8_t CommandType)
{
    QJsonObject root;
    QJson::json_add(root,"type","Request_Command");
    QJson::json_add(root,"data.CommandType",QString::number(CommandType));
    QJsonDocument jdoc=QJsonDocument(root);
    QString out(jdoc.toJson(QJsonDocument::Compact));
    qDebug()<<out<<endl;
    tcpCilent_SendMsg(out);
}
void TcpMgr::slot_TcpClient_ReadMessage(QString recvdata)
{
    QJsonDocument document=QJsonDocument::fromJson(recvdata.toLocal8Bit());
    QJsonObject root = document.object();
    QJsonDocument out;
    out.setObject(root);
    QByteArray bytea= out.toJson(QJsonDocument::Compact);

    QJsonValue findret;
    findret=QJson::json_find(root,"type");
    if(findret!=NULL)
    {
        if(findret.isString())
                qDebug()<<"type is:"<<findret.toString()<<endl;
        if(findret=="Reply_GetApiVersion")
        {
            QString api_version=QJson::json_find(root,"data.app_version").toString();
            QString app_version=QJson::json_find(root,"data.app_version").toString();
            qDebug()<<"api_version is:"<<api_version<<endl;
            qDebug()<<"app_version is:"<<app_version<<endl;
        }else if(findret=="Reply_Heartbeat")
        {
            QString time_tick=QJson::json_find(root,"data.time_tick").toString();
            QString RunState=QJson::json_find(root,"data.RunState").toString();
            qDebug()<<"time_tick is:"<<time_tick<<endl;
            qDebug()<<"RunState is:"<<RunState<<endl;
        }else if(findret=="Reply_Getconfig")
        {
            QString RunMode=QJson::json_find(root,"data.RunMode").toString();
            QString has_lock=QJson::json_find(root,"data.has_lock").toString();
            QString open_stay_time=QJson::json_find(root,"data.open_stay_time").toString();
            QString lock_delay_time=QJson::json_find(root,"data.lock_delay_time").toString();
            qDebug()<<"RunMode is:"<<RunMode<<endl;
            qDebug()<<"has_lock is:"<<has_lock<<endl;
            qDebug()<<"open_stay_time is:"<<open_stay_time<<endl;
            qDebug()<<"lock_delay_time is:"<<lock_delay_time<<endl;
        }else if(findret=="Reply_ESP8266SetConfig")
        {
            QString data=QJson::json_find(root,"data").toString();
            qDebug()<<"data is:"<<data<<endl;

        }else if(findret=="Reply_SetConfig")
        {
            QString data=QJson::json_find(root,"data").toString();
            qDebug()<<"data is:"<<data<<endl;

        }else if(findret=="Reply_Command")
        {
            QString data=QJson::json_find(root,"data").toString();
            qDebug()<<"data is:"<<data<<endl;
        }
    }
}
TcpMgr::~TcpMgr()
{
    delete m_tcpCilent;
}
