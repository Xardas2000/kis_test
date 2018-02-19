//
// Created by KuznetsovAA on 16.02.2018.
//

#ifndef KIS_CONQUEUE_H
#define KIS_CONQUEUE_H

#include <queue>
//#include <thread>
//#include <mutex>
//#include <condition_variable>
#include <iostream>

#include <windows.h>


template <typename T>
class conQueue
{
public:
    conQueue()
    {
        pushLock = CreateEvent(nullptr, FALSE, FALSE, nullptr);
        popLock = CreateEvent(nullptr, FALSE, FALSE, nullptr);
        if(nullptr == pushLock && nullptr == popLock)
        {
            std::cout << "pushLock or popLock is null" << std::endl;
        }
        InitializeCriticalSection(&m_lock);
    }
    ~conQueue()
    {
        CloseHandle(pushLock);
        CloseHandle(popLock);
        DeleteCriticalSection(&m_lock);
    }

public:
    T pop()
    {
        //std::unique_lock<std::mutex> m_lock(mutex_);
        //WaitForSingleObject(hMutex, INFINITE);
        EnterCriticalSection(&m_lock);
        while(queue_.empty())
        {
            LeaveCriticalSection(&m_lock);
            if(WaitForSingleObject(popLock, INFINITE) != WAIT_OBJECT_0)
            {

            }
            EnterCriticalSection(&m_lock);
            //cond_.wait(m_lock);
        }
        auto item = queue_.front();
        queue_.pop();
        SetEvent(pushLock);
        LeaveCriticalSection(&m_lock);
        return item;
    }

    void pop(T& item)
    {
        //std::unique_lock<std::mutex> m_lock(mutex_);
        EnterCriticalSection(&m_lock);
        while(queue_.empty())
        {
            LeaveCriticalSection(&m_lock);
            if(WaitForSingleObject(popLock, INFINITE) != WAIT_OBJECT_0)
            {

            }
            EnterCriticalSection(&m_lock);
            //cond_.wait(m_lock);
        }
        item = queue_.front();
        queue_.pop();
        SetEvent(pushLock);
        LeaveCriticalSection(&m_lock);
    }

    void push(const T& item)
    {
        //std::unique_lock<std::mutex> m_lock(mutex_);
        EnterCriticalSection(&m_lock);
        queue_.push(std::move(item));
        SetEvent(popLock);
        LeaveCriticalSection(&m_lock);
        //m_lock.unlock();
        //cond_.notify_one();
    }

    void push(T&& item)
    {
        //std::unique_lock<std::mutex> m_lock(mutex_);
        EnterCriticalSection(&m_lock);
        queue_.push(std::move(item));
        SetEvent(popLock);
        LeaveCriticalSection(&m_lock);
        //m_lock.unlock();
        //cond_.notify_one();
    }

    unsigned long size()
    {
        //std::unique_lock<std::mutex> m_lock(mutex_);
        unsigned long size = queue_.size();
        //m_lock.unlock();
        //cond_.notify_one();

        return size;
    }

private:
    std::queue<T> queue_;
    HANDLE pushLock;
    HANDLE popLock;
//    std::mutex mutex_;
//    std::condition_variable cond_;
    CRITICAL_SECTION m_lock;
};


#endif //KIS_CONQUEUE_H
