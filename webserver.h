#ifndef WEBSERVER_H
#define WEBSERVER_H

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <cassert>
#include <sys/epoll.h>
#include "./threadpool/threadpool.h"
#include "./http/http_conn.h"

// 最大文件描述符数量
const int MAX_FD = 65536;
// 最大事件数
const int MAX_EVENT_NUMBER = 10000;
// 定时器时间槽
const int TIMESLOT = 5;

/**
 * @brief WebServer类 - 整个服务器的核心类
 * 负责服务器的初始化、配置、运行和管理各模块之间的协作
 */
class WebServer {
public:
    // 构造和析构函数
    WebServer();
    ~WebServer();

    /**
     * @brief 初始化服务器配置
     * @param port 端口号
     * @param user 数据库用户名
     * @param passWord 数据库密码
     * @param dataBaseName 数据库名
     * @param log_write 日志写入方式
     * @param opt_linger 是否开启socket的linger选项
     * @param trigmode 触发模式选择
     * @param sql_num 数据库连接池数量
     * @param thread_num 线程池中的线程数量
     * @param close_log 是否关闭日志
     * @param actor_model reactor/proactor模式选择
     */
    void init(int port, string user, string passWord, string dataBaseName,
              int log_write, int opt_linger, int trigmode, int sql_num,
              int thread_num, int close_log, int actor_model);
    
    // 各个模块的初始化函数
    void thread_pool();    // 初始化线程池
    void sql_pool();       // 初始化数据库连接池
    void log_write();      // 初始化日志系统
    void trig_mode();      // 设置触发模式
    void eventListen();    // 开始监听
    void eventLoop();      // 事件循环处理
    
    // 定时器相关函数
    void timer(int connfd, struct sockaddr_in client_address);  // 创建定时器
    void adjust_timer(util_timer *timer);                      // 调整定时器
    void deal_timer(util_timer *timer, int sockfd);            // 处理定时器事件
    
    // 客户端连接处理函数
    bool dealclientdata();  // 处理客户端连接
    bool dealwithsignal(bool& timeout, bool& stop_server);  // 处理信号
    void dealwithread(int sockfd);   // 处理读事件
    void dealwithwrite(int sockfd);  // 处理写事件

public:
    int m_port;           // 服务器端口
    char *m_root;         // 网站根目录
    int m_log_write;      // 日志写入方式
    int m_close_log;      // 是否关闭日志
    int m_actormodel;     // 模型选择（reactor/proactor）

    int m_pipefd[2];      // 管道文件描述符
    int m_epollfd;        // epoll文件描述符
    http_conn *users;     // HTTP连接数组

    // 数据库相关
    connection_pool *m_connPool;  // 数据库连接池
    string m_user;               // 数据库用户名
    string m_passWord;           // 数据库密码
    string m_databaseName;       // 数据库名
    int m_sql_num;               // 数据库连接数量

    // 线程池相关
    threadpool<http_conn> *m_pool;  // 线程池
    int m_thread_num;               // 线程数量

    epoll_event events[MAX_EVENT_NUMBER];  // epoll事件数组

    int m_listenfd;         // 监听的文件描述符
    int m_OPT_LINGER;       // 是否优雅关闭连接
    int m_TRIGMode;         // 触发组合模式
    int m_LISTENTrigmode;   // 监听的触发模式
    int m_CONNTrigmode;     // 连接的触发模式

    client_data *users_timer;  // 定时器客户端数据
    Utils utils;               // 工具类
};

#endif