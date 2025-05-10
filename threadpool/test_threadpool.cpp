#include <iostream>
#include <unistd.h>
#include <pthread.h>
#include <list>
#include <cstdio>
#include <exception>
#include "../lock/locker.h"

// 简单的任务类
class Task {
public:
    Task(int id) : m_id(id) {}
    
    void process() {
        std::cout << "Task " << m_id << " is being processed by thread " << pthread_self() << std::endl;
        sleep(1); // 模拟任务执行时间
        std::cout << "Task " << m_id << " completed" << std::endl;
    }

    int m_state = 0;  // 线程池需要的状态变量
    int m_id;         // 任务ID
    bool improv = 0;  // 线程池需要的变量
    bool timer_flag = 0;  // 线程池需要的变量
};

// 简化的线程池类
template <typename T>
class threadpool {
public:
    threadpool(int thread_number = 8, int max_requests = 10000);
    ~threadpool();
    bool append(T *request, int state);

private:
    static void *worker(void *arg);
    void run();

private:
    int m_thread_number;
    int m_max_requests;
    pthread_t *m_threads;
    std::list<T *> m_workqueue;
    locker m_queuelocker;
    sem m_queuestat;
};

template <typename T>
threadpool<T>::threadpool(int thread_number, int max_requests) 
    : m_thread_number(thread_number), m_max_requests(max_requests), m_threads(NULL) {
    if (thread_number <= 0 || max_requests <= 0)
        throw std::exception();
    m_threads = new pthread_t[m_thread_number];
    if (!m_threads)
        throw std::exception();
    for (int i = 0; i < thread_number; ++i) {
        if (pthread_create(m_threads + i, NULL, worker, this) != 0) {
            delete[] m_threads;
            throw std::exception();
        }
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

template <typename T>
bool threadpool<T>::append(T *request, int state) {
    m_queuelocker.lock();
    if (m_workqueue.size() >= m_max_requests) {
        m_queuelocker.unlock();
        return false;
    }
    request->m_state = state;
    m_workqueue.push_back(request);
    m_queuelocker.unlock();
    m_queuestat.post();
    return true;
}

template <typename T>
void *threadpool<T>::worker(void *arg) {
    threadpool *pool = (threadpool *)arg;
    pool->run();
    return pool;
}

template <typename T>
void threadpool<T>::run() {
    while (true) {
        m_queuestat.wait();
        m_queuelocker.lock();
        if (m_workqueue.empty()) {
            m_queuelocker.unlock();
            continue;
        }
        T *request = m_workqueue.front();
        m_workqueue.pop_front();
        m_queuelocker.unlock();
        if (!request)
            continue;
        request->process();
    }
}

int main() {
    // 创建线程池，8个线程，最大10000个请求
    threadpool<Task> *pool = new threadpool<Task>(8, 10000);
    
    // 创建20个任务
    for(int i = 0; i < 20; i++) {
        Task *task = new Task(i);
        pool->append(task, 0);
    }
    
    // 等待所有任务完成
    sleep(25);
    
    delete pool;
    return 0;
} 