#include "./ui/clientchatwindow.h"


#include <QApplication>

namespace dom=domain;

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    ClientChatWindow w;
    w.show();

    return QApplication::exec();
}
