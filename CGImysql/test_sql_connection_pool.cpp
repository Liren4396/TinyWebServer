#include <iostream>
#include <unistd.h>
#include <pthread.h>
#include <mysql/mysql.h>
#include "sql_connection_pool.h"

// 模拟多个线程同时使用连接池
void* thread_func(void* arg) {
    connection_pool* pool = (connection_pool*)arg;
    
    // 获取连接
    MYSQL* conn = pool->GetConnection();
    if (conn == NULL) {
        std::cout << "Thread " << pthread_self() << " failed to get connection" << std::endl;
        return NULL;
    }
    
    // 打印当前线程ID和连接信息
    std::cout << "Thread " << pthread_self() << " got connection" << std::endl;
    std::cout << "Free connections: " << pool->GetFreeConn() << std::endl;
    
    // 模拟使用连接
    sleep(1);
    
    // 释放连接
    if (pool->ReleaseConnection(conn)) {
        std::cout << "Thread " << pthread_self() << " released connection" << std::endl;
        std::cout << "Free connections: " << pool->GetFreeConn() << std::endl;
    } else {
        std::cout << "Thread " << pthread_self() << " failed to release connection" << std::endl;
    }
    
    return NULL;
}

int main() {
    try {
        // 创建连接池
        connection_pool* pool = connection_pool::GetInstance();
        
        // 初始化连接池
        // 注意：需要根据你的实际数据库配置修改这些参数
        pool->init("localhost", "root", "123456", "yourdb", 3306, 10, 0);
        
        std::cout << "Connection pool initialized with 10 connections" << std::endl;
        std::cout << "Initial free connections: " << pool->GetFreeConn() << std::endl;
        
        // 创建多个线程来测试连接池
        const int THREAD_NUM = 15;  // 创建15个线程，超过连接池大小
        pthread_t threads[THREAD_NUM];
        int thread_count = 0;
        
        // 创建线程
        for (int i = 0; i < THREAD_NUM; ++i) {
            if (pthread_create(&threads[i], NULL, thread_func, pool) != 0) {
                std::cout << "Failed to create thread " << i << std::endl;
                continue;
            }
            thread_count++;
        }
        
        // 等待所有线程完成
        for (int i = 0; i < thread_count; ++i) {
            pthread_join(threads[i], NULL);
        }
        
        // 等待一段时间确保所有连接都被释放
        sleep(2);
        
        // 检查是否所有连接都已释放
        if (pool->GetFreeConn() != 10) {
            std::cout << "Warning: Not all connections were released properly" << std::endl;
        }
        
        // 销毁连接池
        pool->DestroyPool();
        std::cout << "Connection pool destroyed" << std::endl;
        
    } catch (const std::exception& e) {
        std::cout << "Exception: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cout << "Unknown exception" << std::endl;
        return 1;
    }
    
    return 0;
} 