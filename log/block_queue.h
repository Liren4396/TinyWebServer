#ifndef BLOCKQUEUE_H
#define BLOCKQUEUE_H

#include <iostream>
#include <stdlib.h>
#include <pthread.h>
#include <sys/time.h>
#include "../lock/locker.h"

using namespace std;

/**
 * @brief 阻塞队列类模板
 * 
 * 实现线程安全的阻塞队列，用于生产者-消费者模型
 * 在日志系统中用于异步写入日志
 * @tparam T 队列中存储的数据类型
 */
template <class T>
class block_queue {
public:
    /**
     * @brief 构造函数
     * @param max_size 队列的最大容量，默认为1000
     */
    block_queue(int max_size = 1000) {
        if (max_size < 0) {
            exit(-1);
        }

        m_max_size = max_size;
        m_array = new T[max_size];
        m_size = 0;
        m_front = 0;
        m_back = 0;
    }

    /**
     * @brief 清空队列
     * 
     * 重置队列，不释放内存
     */
    void clear() {
        m_mutex.lock();
        m_size = 0;
        m_front = -1;
        m_back = -1;
        m_mutex.unlock();
    }

    /**
     * @brief 析构函数
     * 
     * 释放队列占用的内存
     */
    ~block_queue() {
        m_mutex.lock();
        if (m_array != NULL) {
            delete[] m_array;
        }
        m_mutex.unlock();
    }

    /**
     * @brief 判断队列是否已满
     * @return 队列满返回true，否则返回false
     */
    bool full() {
        m_mutex.lock();
        if (m_size >= m_max_size) {
            m_mutex.unlock();
            return true;
        }
        m_mutex.unlock();
        return false;
    }

    /**
     * @brief 判断队列是否为空
     * @return 队列空返回true，否则返回false
     */
    bool empty() {
        m_mutex.lock();
        if (0 == m_size) {
            m_mutex.unlock();
            return true;
        }
        m_mutex.unlock();
        return false;
    }

    /**
     * @brief 获取队首元素
     * @param value 用于接收队首元素的引用
     * @return 获取成功返回true，队列为空则返回false
     */
    bool front(T &value) {
        m_mutex.lock();
        if (0 == m_size) {
            m_mutex.unlock();
            return false;
        }
        value = m_array[m_front];
        m_mutex.unlock();
        return true;
    }

    /**
     * @brief 获取队尾元素
     * @param value 用于接收队尾元素的引用
     * @return 获取成功返回true，队列为空则返回false
     */
    bool back(T &value) {
        m_mutex.lock();
        if (0 == m_size) {
            m_mutex.unlock();
            return false;
        }
        value = m_array[m_back];
        m_mutex.unlock();
        return true;
    }

    /**
     * @brief 获取队列当前元素个数
     * @return 队列元素个数
     */
    int size() {
        int tmp = 0;
        m_mutex.lock();
        tmp = m_size;
        m_mutex.unlock();
        return tmp;
    }

    /**
     * @brief 获取队列最大容量
     * @return 队列最大容量
     */
    int max_size() {
        int tmp = 0;
        m_mutex.lock();
        tmp = m_max_size;
        m_mutex.unlock();
        return tmp;
    }

    /**
     * @brief 向队列中添加元素
     * 
     * 生产者调用，往队列尾部添加元素，如果队列已满则通知所有消费者并返回false
     * @param item 要添加的元素
     * @return 添加成功返回true，队列满则返回false
     */
    bool push(const T &item) {
        m_mutex.lock();
        if (m_size >= m_max_size) {
            m_cond.broadcast();
            m_mutex.unlock();
            return false;
        }

        m_back = (m_back + 1) % m_max_size;
        m_array[m_back] = item;

        m_size++;

        m_cond.broadcast();
        m_mutex.unlock();
        return true;
    }

    /**
     * @brief 从队列中取出元素
     * 
     * 消费者调用，从队列头部取元素，如果队列为空则会阻塞等待
     * @param item 用于接收取出元素的引用
     * @return 取出成功返回true，失败返回false
     */
    bool pop(T &item) {
        m_mutex.lock();
        while (m_size <= 0) {
            if (!m_cond.wait(m_mutex.get())) {
                m_mutex.unlock();
                return false;
            }
        }

        m_front = (m_front + 1) % m_max_size;
        item = m_array[m_front];
        m_size--;
        m_mutex.unlock();
        return true;
    }

    /**
     * @brief 带超时的取出元素
     * 
     * 从队列中取出元素，如果队列为空则阻塞等待，但不会永久等待
     * @param item 用于接收取出元素的引用
     * @param ms_timeout 超时时间，单位毫秒
     * @return 取出成功返回true，超时或失败返回false
     */
    bool pop(T& item, int ms_timeout) {
        struct timespec t = {0, 0};
        struct timeval now = {0, 0};
        gettimeofday(&now, NULL);
        m_mutex.lock();
        if (m_size <= 0) {
            t.tv_sec = now.tv_sec + ms_timeout / 1000;
            t.tv_nsec = (ms_timeout % 1000) * 1000;
            if (!m_cond.timewait(m_mutex.get(), t)) {
                m_mutex.unlock();
                return false;
            }
        }

        if (m_size <= 0) {
            m_mutex.unlock();
            return false;
        }

        m_front = (m_front + 1) % m_max_size;
        item = m_array[m_front];
        m_size--;
        m_mutex.unlock();
        return true;
    }
    
private:
    locker m_mutex;    // 互斥锁
    cond m_cond;       // 条件变量

    T *m_array;        // 队列数组
    int m_size;        // 当前队列元素个数
    int m_max_size;    // 队列最大容量
    int m_front;       // 队头位置
    int m_back;        // 队尾位置
};

#endif