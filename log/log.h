#ifndef LOG_H
#define LOG_H

#include <stdio.h>
#include <iostream>
#include <string>
#include <stdarg.h>
#include <pthread.h>
#include "block_queue.h"

using namespace std;

/**
 * @brief 日志类
 * 
 * 单例模式实现的日志类，提供同步/异步写入日志功能
 */
class Log
{
public:
    /**
     * @brief 获取日志单例实例
     * 
     * C++11后，使用局部静态变量实现线程安全的单例模式
     * @return 日志类的单例指针
     */
    static Log *get_instance()
    {
        static Log instance;
        return &instance;
    }

    /**
     * @brief 异步写日志线程的入口函数
     * 
     * 静态方法作为线程函数，调用实例的async_write_log方法
     * @param args 线程参数，未使用
     * @return 线程返回值
     */
    static void *flush_log_thread(void *args)
    {
        Log::get_instance()->async_write_log();
        return NULL;
    }
    
    /**
     * @brief 初始化日志系统
     * 
     * @param file_name 日志文件名
     * @param close_log 是否关闭日志
     * @param log_buf_size 日志缓冲区大小
     * @param split_lines 单个日志文件最大行数
     * @param max_queue_size 异步写入时的队列大小，为0表示同步写入
     * @return 初始化是否成功
     */
    bool init(const char *file_name, int close_log, int log_buf_size = 8192, int split_lines = 5000000, int max_queue_size = 0);

    /**
     * @brief 写入日志
     * 
     * 根据日志级别和格式化字符串写入日志内容
     * @param level 日志级别
     * @param format 格式化字符串
     * @param ... 可变参数
     */
    void write_log(int level, const char *format, ...);

    /**
     * @brief 刷新日志缓冲
     * 
     * 强制将缓冲区内容写入文件
     */
    void flush(void);

private:
    /**
     * @brief 构造函数
     * 
     * 私有构造函数，确保单例模式
     */
    Log();
    
    /**
     * @brief 析构函数
     */
    virtual ~Log();
    
    /**
     * @brief 异步写日志方法
     * 
     * 从阻塞队列中取出日志字符串，写入文件
     * @return 线程返回值
     */
    void *async_write_log()
    {
        string single_log;
        //从阻塞队列中取出一个日志string，写入文件
        while (m_log_queue->pop(single_log))
        {
            m_mutex.lock();
            fputs(single_log.c_str(), m_fp);
            m_mutex.unlock();
        }
        return NULL;
    }

private:
    char dir_name[128]; //路径名
    char log_name[128]; //log文件名
    int m_split_lines;  //日志最大行数
    int m_log_buf_size; //日志缓冲区大小
    long long m_count;  //日志行数记录
    int m_today;        //因为按天分类,记录当前时间是那一天
    FILE *m_fp;         //打开log的文件指针
    char *m_buf;        //日志缓冲区
    block_queue<string> *m_log_queue; //阻塞队列
    bool m_is_async;                  //是否同步标志位
    locker m_mutex;                   //互斥锁
    int m_close_log;                  //关闭日志标志
};

/**
 * @brief 日志宏定义 - DEBUG级别
 * 
 * 只有在日志未关闭时才写入DEBUG级别日志
 */
#define LOG_DEBUG(format, ...) if(0 == m_close_log) {Log::get_instance()->write_log(0, format, ##__VA_ARGS__); Log::get_instance()->flush();}

/**
 * @brief 日志宏定义 - INFO级别
 * 
 * 只有在日志未关闭时才写入INFO级别日志
 */
#define LOG_INFO(format, ...) if(0 == m_close_log) {Log::get_instance()->write_log(1, format, ##__VA_ARGS__); Log::get_instance()->flush();}

/**
 * @brief 日志宏定义 - WARN级别
 * 
 * 只有在日志未关闭时才写入WARN级别日志
 */
#define LOG_WARN(format, ...) if(0 == m_close_log) {Log::get_instance()->write_log(2, format, ##__VA_ARGS__); Log::get_instance()->flush();}

/**
 * @brief 日志宏定义 - ERROR级别
 * 
 * 只有在日志未关闭时才写入ERROR级别日志
 */
#define LOG_ERROR(format, ...) if(0 == m_close_log) {Log::get_instance()->write_log(3, format, ##__VA_ARGS__); Log::get_instance()->flush();}
#endif
