#ifndef TCPCILENT_H
#define TCPCILENT_H

#include <QAbstractSocket>
#include <QtNetwork/QTcpSocket>
#include <QtNetwork>
#include <memory>

const int HEADSIZE=2;

//继承QObject,否则无法connct
class tcpCilent:
        public QObject
{
    Q_OBJECT
public:
    tcpCilent(QObject *parent = Q_NULLPTR);
    tcpCilent(QString address,int port,QObject *parent = Q_NULLPTR);
    virtual ~tcpCilent();
    void NewConnect(QString address,int port);
    void NewConnect();
    void SendData(QString);
private:
    QTcpSocket *m_tcpSocket;
    QByteArray m_rbuffer;
    QString m_address;
    int m_port;
public slots:
    void slot_RecvData();
    void slot_displayError(QAbstractSocket::SocketError);
signals:
    void signal_TcpCilent_HasRecvData(QString);
};

#endif // TCPCILENT_H
