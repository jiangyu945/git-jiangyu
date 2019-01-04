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

void Widget::on_pushButton_clicked()
{
    QMessageBox msg;
    //msg.about(this,"Warning!!!","jiangyu88");
    msg.information(this,"information","I am jiangyu88",QMessageBox::Yes|QMessageBox::No|QMessageBox::Close);
}
