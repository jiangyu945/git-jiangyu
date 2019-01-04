#ifndef MYTHREAD_H
#define MYTHREAD_H

#include <QThread>
#include <QObject>


class myThread : public QThread
{
    Q_OBJECT

public:
    myThread();

signals:
    void SigTimeout();

protected:
    void run();

private:
    int time_sec;
};

#endif // MYTHREAD_H
