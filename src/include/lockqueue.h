#pragma once
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>

// 异步写日志的日志队列
template<typename T>
class LockQueue{
public:
    // 模版代码只能在此处实现
    // 多个worker线程都会写日志queue
    void push(const T &data){
        std::lock_guard<std::mutex> lock(m_mutex);
        m_queue.push(data);
        m_cond.notify_one();
    }

    // 只会有一个线程读日志queue，写日志文件
    T pop(){
        std::unique_lock<std::mutex> locker(m_mutex);
        // 使用while防止虚假唤醒
        while(m_queue.empty()){
            // 日志队列为空，线程进入wait状态不要让他抢锁，把锁释放掉
            m_cond.wait(locker);
        }
        T data = m_queue.front();
        m_queue.pop();
        return data;
    }

private:
    std::queue<T> m_queue;
    std::mutex m_mutex;
    std::condition_variable m_cond;
};

