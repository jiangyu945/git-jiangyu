#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QThread>
#include <QDebug>
#include <QPushButton>
#include <QVBoxLayout>
#include <QProgressBar>

namespace Ui {
class Widget;
}


class WorkerThread : public QThread
{
    Q_OBJECT

public:
    explicit WorkerThread(QObject *parent = 0)
        : QThread(parent)
    {
        qDebug() << "Worker Thread : " << QThread::currentThreadId();
    }

protected:
    virtual void run(){
        qDebug() << "Worker Run Thread : " << QThread::currentThreadId();
        int nValue = 0;
        while (nValue < 100)
        {
            // 休眠50毫秒
            msleep(50);
            ++nValue;

            // 准备更新
            emit resultReady(nValue);
        }
    }
signals:
    void resultReady(int value);
};


class Widget : public QWidget
{
    Q_OBJECT

    public:
        explicit Widget(QWidget *parent = 0)
            : QWidget(parent)
        {
            qDebug() << "Main Thread : " << QThread::currentThreadId();

            // 创建开始按钮、进度条
            QPushButton *pStartButton = new QPushButton(this);
            m_pProgressBar = new QProgressBar(this);

            //设置文本、进度条取值范围
            pStartButton->setText(QString::fromLocal8Bit("开始"));
            m_pProgressBar->setFixedHeight(25);
            m_pProgressBar->setRange(0, 100);
            m_pProgressBar->setValue(0);

            QVBoxLayout *pLayout = new QVBoxLayout();
            pLayout->addWidget(pStartButton, 0, Qt::AlignHCenter);
            pLayout->addWidget(m_pProgressBar);
            pLayout->setSpacing(50);
            pLayout->setContentsMargins(10, 10, 10, 10);
            setLayout(pLayout);

            // 连接信号槽
            connect(pStartButton, SIGNAL(clicked(bool)), this, SLOT(startThread()));

        }
        ~Widget(){}

private slots:
    // 更新进度
    void handleResults(int value)
    {
        qDebug() << "Handle Thread : " << QThread::currentThreadId();
        m_pProgressBar->setValue(value);
    }

    // 开启线程
    void startThread()
    {
        WorkerThread *workerThread = new WorkerThread(this);
        connect(workerThread, SIGNAL(resultReady(int)), this, SLOT(handleResults(int)));
        // 线程结束后，自动销毁
        connect(workerThread, SIGNAL(finished()), workerThread, SLOT(deleteLater()));
        workerThread->start();
    }

private:
    QProgressBar *m_pProgressBar;
    WorkerThread m_workerThread;
};





#endif // WIDGET_H
