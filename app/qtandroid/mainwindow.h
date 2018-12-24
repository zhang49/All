#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDialog>
#include<QtNetwork/QTcpSocket>
#include "tcpmgr.h"
#include <memory>

namespace Ui {
class MainWindow;
}


class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    void send_Json(QJsonObject root);
    void start();
    ~MainWindow();

private:
    Ui::MainWindow *ui;

public slots:
    void slot_connectbtn_clicked();
    void slot_sendbtn_clicked();
    void slot_closebtn_clicked();
    void slot_modifyapbtn_clicked();
    void slot_restorebtn_clicked();
};

#endif // MAINWINDOW_H
