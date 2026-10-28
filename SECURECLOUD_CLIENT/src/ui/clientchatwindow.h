#ifndef CLIENTCHATWINDOW_H
#define CLIENTCHATWINDOW_H


#include <QMainWindow>
#include <string>

namespace Ui {
class ClientChatWindow;
}

class ClientChatWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit ClientChatWindow(QWidget *parent = nullptr);
    ~ClientChatWindow() override;

    void logInfo(const QString& msg);

    void logInfo(const std::string& msg);


private:
    Ui::ClientChatWindow *ui;
};

#endif // CLIENTCHATWINDOW_H
