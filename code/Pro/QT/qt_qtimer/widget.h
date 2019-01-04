#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QDebug>
#include "mythread.h"

namespace Ui {
class Widget;
}

class Widget : public QWidget
{
    Q_OBJECT

public:
    explicit Widget(QWidget *parent = 0);
    ~Widget();

private slots:
    void on_pushButton_start_clicked();

    void doProcessTimeout();

    void on_pushButton_stop_clicked();

private:
    Ui::Widget *ui;
    int index;
    myThread* mythread;
};

#endif // WIDGET_H
