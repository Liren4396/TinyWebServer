#ifndef LOCKER_H
#define LOCKER_H

#include <exception>
#include <pthread.h>
#include <semaphore.h>

/**
 * @brief 信号量类
 * 
 * 封装了POSIX信号量，用于线程间同步
 */
class sem {
public:
    /**
     * @brief 默认构造函数，创建值为0的信号量
     */
    sem() {
        if (sem_init(&m_sem, 0, 0) != 0) {
            throw std::exception();
        }
    }
    
    /**
     * @brief 带初始值的构造函数
     * @param num 信号量的初始值
     */
    sem(int num) {
        if (sem_init(&m_sem, 0, num) != 0) {
            throw std::exception();
        }
    }
    
    /**
     * @brief 析构函数，销毁信号量
     */
    ~sem() {
        sem_destroy(&m_sem);
    }
    
    /**
     * @brief 等待信号量
     * 
     * 信号量减1，如果信号量为0则阻塞
     * @return 操作是否成功
     */
    bool wait() {
        return sem_wait(&m_sem) == 0;
    }
    
    /**
     * @brief 增加信号量
     * 
     * 信号量加1，可能唤醒阻塞在wait上的线程
     * @return 操作是否成功
     */
    bool post() {
        return sem_post(&m_sem) == 0;
    }
    
private:
    sem_t m_sem;  // POSIX信号量
};

/**
 * @brief 互斥锁类
 * 
 * 封装了POSIX互斥锁，用于保护共享资源
 */
class locker {
public:
    /**
     * @brief 构造函数，初始化互斥锁
     */
    locker() {
        if (pthread_mutex_init(&m_mutex, NULL) != 0) {
            throw std::exception();
        }
    }
    
    /**
     * @brief 析构函数，销毁互斥锁
     */
    ~locker() {
        pthread_mutex_destroy(&m_mutex);
    }
    
    /**
     * @brief 获取互斥锁
     * 
     * 如果锁已被占用，则阻塞
     * @return 操作是否成功
     */
    bool lock() {
        return pthread_mutex_lock(&m_mutex) == 0;
    }
    
    /**
     * @brief 释放互斥锁
     * @return 操作是否成功
     */
    bool unlock() {
        return pthread_mutex_unlock(&m_mutex) == 0;
    }
    
    /**
     * @brief 获取互斥锁指针
     * @return 互斥锁指针
     */
    pthread_mutex_t *get() {
        return &m_mutex;
    }
    
private:
    pthread_mutex_t m_mutex;  // POSIX互斥锁
};

/**
 * @brief 条件变量类
 * 
 * 封装了POSIX条件变量，用于线程间同步
 */
class cond {
public:
    /**
     * @brief 构造函数，初始化条件变量
     */
    cond() {
        if (pthread_cond_init(&m_cond, NULL) != 0) {
            throw std::exception();
        }
    }
    
    /**
     * @brief 析构函数，销毁条件变量
     */
    ~cond() {
        pthread_cond_destroy(&m_cond);
    }
    
    /**
     * @brief 等待条件变量
     * 
     * 阻塞当前线程直到条件变量被唤醒
     * @param m_mutex 互斥锁指针
     * @return 操作是否成功
     */
    bool wait(pthread_mutex_t *m_mutex) {
        int ret = 0;
        ret = pthread_cond_wait(&m_cond, m_mutex);
        return ret == 0;
    }
    
    /**
     * @brief 带超时的等待条件变量
     * 
     * 阻塞当前线程直到条件变量被唤醒或超时
     * @param m_mutex 互斥锁指针
     * @param t 超时时间
     * @return 操作是否成功
     */
    bool timewait(pthread_mutex_t *m_mutex, struct timespec t) {
        int ret = 0;
        ret = pthread_cond_timedwait(&m_cond, m_mutex, &t);
        return ret == 0;
    }
    
    /**
     * @brief 唤醒一个等待条件变量的线程
     * @return 操作是否成功
     */
    bool signal() {
        return pthread_cond_signal(&m_cond) == 0;
    }
    
    /**
     * @brief 唤醒所有等待条件变量的线程
     * @return 操作是否成功
     */
    bool broadcast() {
        return pthread_cond_broadcast(&m_cond) == 0;
    }
    
private:
    pthread_cond_t m_cond;  // POSIX条件变量
};

#endif  // LOCKER_H