#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <list>
#include <cstdio>
#include <exception>
#include <pthread.h>
#include "../lock/locker.h"
#include "../CGImysql/sql_connection_pool.h"
  
/**
 * @brief 线程池类模板
 * 
 * 线程池用于管理工作线程，提高服务器并发处理能力
 * 实现了Reactor和Proactor两种并发模型
 * @tparam T 任务类型，通常是HTTP连接类
 */
template <typename T>
class threadpool {
public:
    /**
     * @brief 构造函数
     * @param actor_model 并发模型选择：0-Proactor模式，1-Reactor模式
     * @param connPool 数据库连接池指针
     * @param thread_number 线程数量
     * @param max_request 最大请求数量
     */
    threadpool(int actor_model, connection_pool *connPool, int thread_number = 8, int max_request = 10000);
    
    /**
     * @brief 析构函数
     */
    ~threadpool();
    
    /**
     * @brief 向请求队列添加任务（reactor模式）
     * @param request 任务请求
     * @param state 状态（读/写）
     * @return 添加是否成功
     */
    bool append(T *request, int state);
    
    /**
     * @brief 向请求队列添加任务（proactor模式）
     * @param request 任务请求
     * @return 添加是否成功
     */
    bool append_p(T *request);

private:
    /**
     * @brief 工作线程函数
     * @param arg 线程参数
     * @return 线程返回值
     */
    static void *worker(void *arg);
    
    /**
     * @brief 运行函数 - 线程池中的所有线程都调用这个函数
     */
    void run();

private:
    int m_thread_number;       // 线程池中的线程数
    int m_max_requests;        // 请求队列中允许的最大请求数
    pthread_t *m_threads;      // 线程池数组，大小为m_thread_number
    std::list<T *> m_workqueue;// 请求队列
    locker m_queuelocker;      // 互斥锁，保护请求队列
    sem m_queuestat;           // 信号量，表示是否有任务需要处理
    connection_pool *m_connPool; // 数据库连接池
    int m_actor_model;         // 模型切换（reactor/proactor）
};

template <typename T>
threadpool<T>::threadpool(int actor_model, connection_pool *connPool, int thread_number, int max_requests)
: m_actor_model(actor_model), m_thread_number(thread_number), m_max_requests(max_requests), 
m_threads(NULL), m_connPool(connPool) {
    if (thread_number <= 0 || max_requests <= 0) {
        throw std::exception(); 
    }    
    // 创建线程池数组
    m_threads = new pthread_t[m_thread_number];
    if (!m_threads)
        throw std::exception();

    // 创建thread_number个线程，并将它们设置为分离状态
    for (int i = 0; i < thread_number; ++i) {
        // 创建线程，绑定worker函数
        if (pthread_create(m_threads + i, NULL, worker, this) != 0) {
            delete[] m_threads;
            throw std::exception();
        }
        // 设置为分离状态，线程结束后自动回收资源
        if (pthread_detach(m_threads[i])) {
            delete[] m_threads;
            throw std::exception();
        }
    }
}

template <typename T>
threadpool<T>::~threadpool() {
    delete[] m_threads;
}

// Reactor模式下的任务添加函数
template <typename T>
bool threadpool<T>::append(T *request, int state) {
    m_queuelocker.lock();
    // 如果工作队列已满，则拒绝添加
    if (m_workqueue.size() >= m_max_requests) {
        m_queuelocker.unlock();
        return false;
    }
    // 设置任务状态并添加到工作队列
    request->m_state = state;
    m_workqueue.push_back(request);
    m_queuelocker.unlock();
    // 增加信号量，通知有任务要处理
    m_queuestat.post();
    return true;
}

// Proactor模式下的任务添加函数
template <typename T>
bool threadpool<T>::append_p(T *request) {
    m_queuelocker.lock();
    if (m_workqueue.size() >= m_max_requests) {
        m_queuelocker.unlock();
        return false;
    }
    m_workqueue.push_back(request);
    m_queuelocker.unlock();
    m_queuestat.post();
    return true;
}

// 线程工作函数（静态成员函数），将this指针作为参数传入
template <typename T>
void* threadpool<T>::worker(void* arg) {
    threadpool *pool = (threadpool *) arg;
    pool->run();
    return pool;
}

// 工作线程运行的函数，不断从工作队列中取出任务并执行
template <typename T>
void threadpool<T>::run() {
    while (true) {
        // 等待信号量，有任务时才会继续执行
        m_queuestat.wait();
        m_queuelocker.lock();
        if (m_workqueue.empty()) {
            m_queuelocker.unlock();
            continue;
        }
        
        // 从请求队列中取出第一个任务
        T *request = m_workqueue.front();
        m_workqueue.pop_front();
        m_queuelocker.unlock();
        
        if (!request) continue;
        
        // Reactor模式
        if (1 == m_actor_model) {
            // 读事件
            if (0 == request->m_state) {
                if (request->read_once()) {
                    request->improv = 1;
                    // 创建数据库连接
                    connectionRAII mysqlcon(&request->mysql, m_connPool);
                    // 处理请求
                    request->process();
                }
                else {
                    request->improv = 1;
                    request->timer_flag = 1;
                }
            } 
            // 写事件
            else {
                if (request->write()) {
                    request->improv = 1;
                } else {
                    request->improv = 1;
                    request->timer_flag = 1;
                }
            }
        } 
        // Proactor模式
        else {
            // 创建数据库连接
            connectionRAII mysqlcon(&request->mysql, m_connPool);
            // 直接处理业务逻辑
            request->process();
        }
    }
}
#endif