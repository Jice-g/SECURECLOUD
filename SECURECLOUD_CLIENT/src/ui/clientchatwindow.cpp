#include "clientchatwindow.h"
#include "ui_clientchatwindow.h"

ClientChatWindow::ClientChatWindow(QWidget *parent)
    : QMainWindow(parent) , ui(new Ui::ClientChatWindow)
    {
    ui->setupUi(this);
    }

ClientChatWindow::~ClientChatWindow()
    {
    delete ui;
    }


    void ClientChatWindow::logInfo(const QString& msg)
    {
        ui->dataCtrlBox->append(QString(msg));
    }

