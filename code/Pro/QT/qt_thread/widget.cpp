#include "widget.h"
#include "ui_widget.h"

Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget)
{
    ui->setupUi(this);
}

Widget::~Widget()
{
    delete ui;
}

void Dummy::emitsig()
{
    emit sig();
}

void myObject::slot()
{
    qDebug()<<"the slot come from thread:" <<QThread::currentThreadId();
    qDebug()<< "i = " << i++ <<endl;
}
