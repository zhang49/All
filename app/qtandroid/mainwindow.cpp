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
void MainWindow::json_test()
{
    QJsonArray root;
    QJsonObject obj;
    QJsonArray dataAarray;
    //root.push_back(QJsonObject({QPair<QString,QJsonValue>("type","REQUEST")}));
    //root.push_back(QJsonObject({QPair<QString,QJsonValue>("type","REQUEST")}));
    dataAarray.push_back(QJsonObject({QPair<QString,QJsonValue>("id","#1")}));
    dataAarray.push_back(QJsonObject({QPair<QString,QJsonValue>("thing","none")}));

    json_add(root,"type","REQUEST");
    root.push_back(QJsonObject({QPair<QString,QJsonValue>("data",dataAarray)}));
    //json_add(root,"data.id","#1");
    //json_add(root,"data.per","cat");
    //json_add(root,"next.id","#3");
    //json_add(root,"next.per","dog");

    QJsonDocument jdoc=QJsonDocument(root);
    QString out(jdoc.toJson(QJsonDocument::Compact));
    qDebug()<<out<<endl;
    //json_find(root,"data.thing",NULL);
}

/*
 * Json最多两层
 */
void MainWindow::json_add(QJsonArray &root,QString key,QString value)
{
        if(key.contains('.'))
        {
            QString frontkey=key.mid(0,key.indexOf('.'));
            QString backdkey=key.mid(key.indexOf('.')+1);
            QJsonObject tempobj;
            int i=0;
            for(;i<root.size();i++)
            {
                if(root[i].toObject().contains(frontkey))
                {
                   //qDebug()<<"found, insert backdkey:"<<backdkey<<endl;
                   tempobj=root[i].toObject();
                   tempobj=tempobj[frontkey].toObject();
                   root.removeAt(i);
                   break;
                }
            }
            tempobj.insert(backdkey,value);
            //qDebug()<<"not found, insert backdkey & frontkey:"<<backdkey<<endl;
            root.push_back(QJsonObject({QPair<QString,QJsonValue>(frontkey,tempobj)}));

            return;
        }
        root.push_back(QJsonObject({QPair<QString,QJsonValue>(key,value)}));
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
//find Json data
QJsonValue MainWindow::json_find(QJsonValue root,QString key)
{
    if(root.isArray() && root.toArray().size()==0)return NULL;
    if(root.isObject() && root.toObject().size()==0)return NULL;
    if(root.isObject())
    {
        QString fkey;
        if(key.contains('.'))
            fkey=key.mid(0,key.indexOf('.'));
        else
            fkey=key;
        for(QString k : root.toObject().keys())
        {
            if(k==fkey)
            {
                if(fkey==key)//查找到的是，参数key的最后子键
                {
                    //qDebug()<<root.toObject()[k].toString()<<endl;
                    return root.toObject()[k];
                }
                else
                {
                    return json_find(root.toObject()[k],key.mid(key.indexOf('.')+1));
                }
            }
        }
        return 0;
    }
    else if(root.isArray())
    {
        for(int i=0;i<root.toArray().size();i++)
        return json_find(root.toArray().at(i),key);
    }
    return NULL;
}

void MainWindow::send_Json(QJsonArray root)
{
    QJsonDocument jdoc=QJsonDocument(root);
    QString out(jdoc.toJson(QJsonDocument::Compact));
    out.length();
    tcpClient->SendData(out);
}

void MainWindow::slot_sendbtn_clicked()
{
    tcpClient->SendData("1");
}
void MainWindow::slot_closebtn_clicked()
{

}
void MainWindow::slot_readMessage(QString recvdata)
{
    QJsonDocument document=QJsonDocument::fromJson(recvdata.toLocal8Bit());
    QJsonObject object = document.object();

    QJsonDocument dd;
    dd.setObject(object);
    QByteArray bytea= dd.toJson(QJsonDocument::Compact);
    qDebug()<<"Exchange is :"<<QString(bytea)<<endl;

    QJsonValue findret;
    findret=json_find(object,"type");
    if(findret!=NULL)
    {
        if(findret.isString())
                qDebug()<<"return is:"<<findret.toString()<<endl;
    }
    if(findret=="request")
    {

    }

    //ui->rdTextEdit->append(recvdata);
}


MainWindow::~MainWindow()
{
    delete ui;
    delete tcpClient;
}

