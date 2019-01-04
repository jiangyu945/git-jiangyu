#include "widget.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    qDebug()<<"main thread:"<<QThread::currentThreadId();


    Widget w;
    w.show();

    QThread thread;
    myObject obj;
    Dummy dummy;
    obj.moveToThread(&thread);
    QObject::connect(&dummy, SIGNAL(sig()), &obj, SLOT(slot()));
    thread.start();

    dummy.emitsig();

    return a.exec();
}
