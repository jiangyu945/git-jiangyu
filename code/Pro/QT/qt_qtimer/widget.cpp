#include "widget.h"
#include "ui_widget.h"

Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget)
{
    ui->setupUi(this);
    index = 0;
    mythread = new myThread();

    connect(mythread,SIGNAL(SigTimeout()),this,SLOT(doProcessTimeout()));
}

Widget::~Widget()
{
    delete mythread;
    delete ui;

}


void Widget::on_pushButton_start_clicked()
{
    mythread->start();
}

void Widget::doProcessTimeout(){
    index++;
    if(index == 11){
        index = 0;
    }
    ui->lcdNumber->display(index);
}

void Widget::on_pushButton_stop_clicked()
{
    mythread->terminate();
}


