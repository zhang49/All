#ifndef TCPMGR_H
#define TCPMGR_H

#include <QAbstractSocket>
#include <QtNetwork/QTcpSocket>
#include <QtNetwork>

const QString ADDRESS="192.168.20.104";
const int PORT=7777;
const int HEADSIZE=2;

//继承QObject,否则无法connct
class TcpMgr :
        public QObject
{
    Q_OBJECT
public:
    TcpMgr(QObject *parent = Q_NULLPTR);
    virtual ~TcpMgr();
    void NewConnect();
    void SendData(QString);
private:
    QTcpSocket *tcpSocket;
    QByteArray m_rbuffer;
public slots:
    void slot_RecvData();
    void slot_displayError(QAbstractSocket::SocketError);
signals:
    void signal_tcpHasRecvData(QString);
};

#endif // TCPMGR_H
