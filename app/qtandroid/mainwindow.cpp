#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "tcpmgr.h"
#include <stdarg.h>

#include <QAbstractSocket>
#include <QtNetwork/QTcpSocket>
#include <QtNetwork>
#include <QTimer>
#include <memory>
#include <QJsonObject>



MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    tcpClient = new TcpMgr(this);
    connect(ui->connectbtn,SIGNAL(clicked(bool)),this,SLOT(slot_connectbtn_clicked()));
    connect(ui->sendbtn,SIGNAL(clicked(bool)),this,SLOT(slot_sendbtn_clicked()));
    connect(ui->clostbtn,SIGNAL(clicked(bool)),this,SLOT(slot_closebtn_clicked()));
    connect(tcpClient,SIGNAL(signal_tcpHasRecvData(QString)),this,SLOT(slot_readMessage(QString)));
}

MainWindow::~MainWindow()
{
    delete ui;
    delete tcpClient;
}

void MainWindow::slot_connectbtn_clicked()
{
    tcpClient->NewConnect();
}
void makeJson(QJsonArray &root,char *type,int dataSize,...)
{
    const char *dname;
    const char *dvalue;
    QJsonObject jsontype;
    QJsonArray jsondataArray;

    va_list arg_ptr;
    va_start(arg_ptr, dataSize);
    while(dataSize)
    {
        dname = va_arg(arg_ptr, char *);
        dvalue = va_arg(arg_ptr, char *);
        jsondataArray.insert(0,QJsonObject({QPair<QString, QJsonValue>( QString(dname),dvalue)}));
        dataSize-=1;
    }
    va_end(arg_ptr);
    jsontype.insert("type", QString(type));
    // QJsonObject ()  std::initializer_list<QPair<QString, QJsonValue>>
    root.insert(0,jsontype);
    root.insert(1,QJsonObject({QPair<QString, QJsonValue>( QString("data"),jsondataArray)}));
}
//get
int json_find(QJsonValue root,QString dname)
{
    if(root.isArray() && root.toArray().size()==0)return 0;
    if(root.isObject() && root.toObject().size()!=0)
    {
        //if(root.toObject().value(dname)!=NULL)
        {
            return 1;
        }
    }
    else if(root.isArray())
    {
        for(int i=0;i<root.toArray().size();i++)
        if(json_find(root.toArray().at(i),dname)==1)
        {

        }

    }
    return 0;
}


void MainWindow::slot_sendbtn_clicked()
{
    //tcpClient->SendData(ui->wdTextEdit->text());
    //QTimer::singleShot(1,this,&MainWindow::function);
    QJsonArray root;
    //json_add(root,"type","Request");

    //json_find(root,"type");
    //makeJson(&root,"REQUESTID",1,"id","#1");


    //tcpClient->SendData();
}
void MainWindow::slot_closebtn_clicked()
{

}
void MainWindow::slot_readMessage(QString recvdata)
{
      ui->rdTextEdit->append(recvdata);
}

