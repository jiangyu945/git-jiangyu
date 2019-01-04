#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QThread>
#include <QDebug>
#include <QTimer>

class MyThread : public QThread
{
    Q_OBJECT
public:
    explicit MyThread(QObject *parent = 0);
protected:
    void run();

public slots:
    void slotFunc();

private:
    QTimer *timer;
    quint8 i=1;
};

#endif // WIDGET_H
