#include "./ui/clientchatwindow.h"
#include "domain/model/User.h"
#include "domain/model/Message.h"

#include <QApplication>

namespace dom=domain;

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    ClientChatWindow w;
    w.show();

    return QApplication::exec();
}
