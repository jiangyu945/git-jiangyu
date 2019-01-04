#include "mythread.h"

myThread::myThread()
{
    time_sec = 1;
}

void myThread::run(){
    while(1){
        sleep(time_sec);
        emit SigTimeout();
    }
}

