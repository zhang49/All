#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDialog>
#include<QtNetwork/QTcpSocket>

namespace Ui {
class MainWindow;
}


class TcpMgr;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    void makeJson(QJsonArray &root,char *type,int dataSize,...);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    TcpMgr *tcpClient;
    QByteArray m_rbuffer;
private slots:
    void slot_readMessage(QString);
    void slot_connectbtn_clicked();
    void slot_sendbtn_clicked();
    void slot_closebtn_clicked();
};

#endif // MAINWINDOW_H
