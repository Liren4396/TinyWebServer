#include "log.h"
#include <iostream>
#include <unistd.h>
#include <pthread.h>
#include <string>

// 添加全局变量来访问 m_close_log
extern int m_close_log;

// 测试同步日志
void* test_sync_log(void* arg) {
    int thread_id = *(int*)arg;
    for(int i = 0; i < 5; i++) {
        Log::get_instance()->write_log(1, "Thread %d: This is a sync log message %d", thread_id, i);
        Log::get_instance()->flush();
        usleep(100000);  // 休眠100ms
    }
    return NULL;
}

// 测试异步日志
void* test_async_log(void* arg) {
    int thread_id = *(int*)arg;
    for(int i = 0; i < 5; i++) {
        Log::get_instance()->write_log(1, "Thread %d: This is an async log message %d", thread_id, i);
        Log::get_instance()->flush();
        usleep(100000);  // 休眠100ms
    }
    return NULL;
}

// 测试不同日志级别
void test_log_levels() {
    std::cout << "Testing different log levels..." << std::endl;
    
    Log::get_instance()->write_log(0, "This is a debug message");
    Log::get_instance()->flush();
    Log::get_instance()->write_log(1, "This is an info message");
    Log::get_instance()->flush();
    Log::get_instance()->write_log(2, "This is a warning message");
    Log::get_instance()->flush();
    Log::get_instance()->write_log(3, "This is an error message");
    Log::get_instance()->flush();
    
    std::cout << "Log levels test completed" << std::endl;
}

// 测试日志文件分割
void test_log_split() {
    std::cout << "Testing log file split..." << std::endl;
    
    // 写入一些日志
    for(int i = 0; i < 10; i++) {
        Log::get_instance()->write_log(1, "Testing log file split message %d", i);
        Log::get_instance()->flush();
    }
    
    std::cout << "Log split test completed" << std::endl;
}

int main() {
    // 初始化日志系统
    if(!Log::get_instance()->init("test_log", 0, 8192, 5000000, 800)) {
        std::cout << "Log init failed!" << std::endl;
        return -1;
    }
    
    std::cout << "=== Testing Log Levels ===" << std::endl;
    test_log_levels();
    
    std::cout << "\n=== Testing Sync Log ===" << std::endl;
    pthread_t sync_threads[2];
    int sync_ids[2] = {1, 2};
    
    // 创建两个同步日志线程
    pthread_create(&sync_threads[0], NULL, test_sync_log, &sync_ids[0]);
    pthread_create(&sync_threads[1], NULL, test_sync_log, &sync_ids[1]);
    
    pthread_join(sync_threads[0], NULL);
    pthread_join(sync_threads[1], NULL);
    
    std::cout << "\n=== Testing Async Log ===" << std::endl;
    pthread_t async_threads[2];
    int async_ids[2] = {3, 4};
    
    // 创建两个异步日志线程
    pthread_create(&async_threads[0], NULL, test_async_log, &async_ids[0]);
    pthread_create(&async_threads[1], NULL, test_async_log, &async_ids[1]);
    
    pthread_join(async_threads[0], NULL);
    pthread_join(async_threads[1], NULL);
    
    std::cout << "\n=== Testing Log Split ===" << std::endl;
    test_log_split();
    
    std::cout << "\nAll tests completed!" << std::endl;
    return 0;
} 