#include "tcpmgr.h"

#include <QAbstractSocket>
#include <QtNetwork/QTcpSocket>
#include <QtNetwork>
#include <QTimer>
#include <QJsonObject>

TcpMgr::TcpMgr(QObject *parent)
{
    tcpSocket=new QTcpSocket(parent);
    connect(tcpSocket,SIGNAL(readyRead()),this,SLOT(slot_RecvData()));
    connect(tcpSocket,SIGNAL(error(QAbstractSocket::SocketError)),this,SLOT(slot_displayError(QAbstractSocket::SocketError)));
}
void TcpMgr::NewConnect()
{
    //QHostAddress::LocalHost本地地址
    //tcpSocket->abort();
    m_rbuffer.clear();
    tcpSocket->connectToHost(ADDRESS,PORT);
}

void TcpMgr::SendData(QString parameter)
{
    QByteArray data=parameter.toLocal8Bit();
    uint16_t len=strlen(data);
    char ch[2];

    ch[0]=(uint8_t)len>>8;
    ch[1]=(uint8_t)len&0x00ff;

    if((tcpSocket->write(ch,2)==-1))
    {
        qDebug()<<"Write Data Error";
    }
    if(tcpSocket->write(data,strlen(data))==-1)
    {
        qDebug()<<"Write Data Error";
    }
    tcpSocket->flush();
}

void TcpMgr::slot_RecvData()
{
    qDebug()<<"get message"<<endl;
    m_rbuffer.append(tcpSocket->readAll());

    uint16_t bodylen;
    for(int i=0;i<HEADSIZE;i++)
    {
        bodylen<<=8;
        bodylen |= (uint8_t)m_rbuffer[i];
    }
    qDebug()<<"body length is "<<bodylen<<endl;
    while((m_rbuffer.length())>=bodylen+HEADSIZE)
    {
        //m_rbuffer长度大于bodylength , 读取数据
        QString recvdata=QString(m_rbuffer.mid(HEADSIZE,bodylen));
        //接收到数据
        emit signal_tcpHasRecvData(recvdata);
        //删除已成功读取的数据
        m_rbuffer=m_rbuffer.mid(HEADSIZE+bodylen);
        if(m_rbuffer.length()>HEADSIZE)
        {
             bodylen = m_rbuffer.mid(0,HEADSIZE).toInt();
        }
        else
        {
            break;
        }
    }
}

void TcpMgr::slot_displayError(QAbstractSocket::SocketError)
{


}
TcpMgr::~TcpMgr()
{
    delete tcpSocket;
}
