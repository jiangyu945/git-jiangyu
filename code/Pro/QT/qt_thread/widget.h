#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QThread>
#include <QDebug>
#include <QPainter>


namespace Ui {
class Widget;
}

class Widget : public QWidget
{
    Q_OBJECT

public:
    explicit Widget(QWidget *parent = 0);
    ~Widget();

private:
    Ui::Widget *ui;
};


class Dummy:public QObject{
    Q_OBJECT

public:
    Dummy(QObject* parent=0):QObject(parent){}
    void emitsig();

signals:
    void sig();

};

class myObject:public QObject{
    Q_OBJECT

public:
    myObject(){}

public slots:
    void slot();

private :
    int i=0;
};


#endif // WIDGET_H
