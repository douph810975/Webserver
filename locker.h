#ifndef LOCKER_H
#define LOCKER_H

#include <pthread.h>

class  locker
{
private:
    pthread_mutex_t m_mutex;
public:
     locker(/* args */);
    ~ locker();
};

 locker:: locker(/* args */)
{
}

 locker::~ locker()
{
}
