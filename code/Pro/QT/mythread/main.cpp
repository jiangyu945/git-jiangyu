#include "widget.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    qDebug() << "main thread id: " << QThread::currentThreadId();

    MyThread obj;
    obj.start();

    return a.exec();
}
