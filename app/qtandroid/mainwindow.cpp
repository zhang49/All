#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "tcpmgr.h"
#include "qjson.h"
#include <stdarg.h>
#include <QAbstractSocket>
#include <QtNetwork/QTcpSocket>
#include <QtNetwork>
#include <QTimer>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>
#include <memory>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    connect(ui->connectbtn,SIGNAL(clicked(bool)),this,SLOT(slot_connectbtn_clicked()));
    connect(ui->sendbtn,SIGNAL(clicked(bool)),this,SLOT(slot_sendbtn_clicked()));
    connect(ui->clostbtn,SIGNAL(clicked(bool)),this,SLOT(slot_closebtn_clicked()));
    connect(ui->modifyapbtn,SIGNAL(clicked(bool)),this,SLOT(slot_modifyapbtn_clicked()));
    connect(ui->restorebtn,SIGNAL(clicked(bool)),this,SLOT(slot_restorebtn_clicked()));
    start();
}
void MainWindow::start()
{
    TcpMgr::getInstance()->NewTcpClientConnect(TCPCLIENT_ADDRESS,TCPCLIENT_PORT);
}
void MainWindow::slot_connectbtn_clicked()
{
    //TcpMgr::getInstance()->NewTcpClientConnect(TCPCLIENT_ADDRESS,TCPCLIENT_PORT);
}
void MainWindow::send_Json(QJsonObject root)
{
    QJsonDocument jdoc=QJsonDocument(root);
    QString out(jdoc.toJson(QJsonDocument::Compact));
    TcpMgr::getInstance()->tcpCilent_SendMsg(out);
}

void MainWindow::slot_sendbtn_clicked()
{
    TcpMgr::getInstance()->Request_SetConfig(0,1,250,500);
}
void MainWindow::slot_closebtn_clicked()
{
    TcpMgr::getInstance()->CloseTcpClientConnect();
}

void MainWindow::slot_modifyapbtn_clicked()
{
    TcpMgr::getInstance()->Request_ESP8266SetConfig("QtModifySSID","12345678");
}
void MainWindow::slot_restorebtn_clicked()
{
    TcpMgr::getInstance()->Request_ESP8266SetRestore();
}
MainWindow::~MainWindow()
{
    delete ui;
}

