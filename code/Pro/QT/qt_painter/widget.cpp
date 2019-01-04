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

void Widget::paintEvent(QPaintEvent *)
{
    QPainter mypainter(this);
//    //QImage加载图片速度快，但显示较慢
//    QImage img;
//    img.load(":/img/Sharingan.jpg");
//    mypainter.drawImage(0,0,img);

//    //QPixmap加载图片慢，但显示速度快
//    QPixmap img;
//    img.load(":/img/Sharingan.jpg");
//    mypainter.drawPixmap(0,0,this->width(),this->height(),img);

    //加速显示技巧：先用QImage加载，再转为QPixmap显示（结合两者优势，当图片较大时能有效加速）
    QString filename(":/img/player.png");
    QImage img;
    if(!img.load(filename)){
        qDebug() << "open image failed!" <<endl;
    }

    QPixmap imageToPixmap = QPixmap::fromImage( img.scaled(size(), Qt::KeepAspectRatio) );
    mypainter.drawPixmap(80,80,320,240,imageToPixmap);

}

