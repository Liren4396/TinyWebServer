#ifndef TIMER_H
#define TIMER_H

#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <assert.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <stdarg.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/uio.h>

#include <sys/time.h>
#include "../log/log.h"

class util_timer;

/**
 * @brief 客户端数据结构
 * 
 * 保存客户端的地址信息、socket描述符和对应的定时器
 */
struct client_data {
    sockaddr_in address;  // 客户端socket地址
    int sockfd;           // 客户端socket文件描述符
    util_timer *timer;    // 指向对应的定时器
};

/**
 * @brief 定时器类
 * 
 * 实现定时器功能，采用双向链表结构
 */
class util_timer {
public:
    util_timer() : prev(NULL), next(NULL) {}
public:
    time_t expire;                      // 定时器到期时间
    void (* cb_func)(client_data *);    // 回调函数，处理定时器到期事件
    client_data *user_data;             // 用户数据，指向对应的客户端数据
    util_timer *prev;                   // 前向指针，指向前一个定时器
    util_timer *next;                   // 后向指针，指向后一个定时器
};

/**
 * @brief 定时器链表类
 * 
 * 管理定时器的链表，按到期时间升序排列
 */
class sort_timer_lst {
public:
    /**
     * @brief 构造函数，初始化头尾指针
     */
    sort_timer_lst();
    
    /**
     * @brief 析构函数，删除所有定时器
     */
    ~sort_timer_lst();

    /**
     * @brief 添加定时器到链表
     * @param timer 要添加的定时器
     */
    void add_timer(util_timer *timer);
    
    /**
     * @brief 调整定时器在链表中的位置
     * @param timer 需要调整的定时器
     */
    void adjust_timer(util_timer *timer);
    
    /**
     * @brief 从链表中删除定时器
     * @param timer 要删除的定时器
     */
    void del_timer(util_timer *timer);
    
    /**
     * @brief 定时器任务处理函数
     * 
     * 处理链表上到期的定时器，执行回调函数
     */
    void tick();
    
private:
    /**
     * @brief 添加定时器到链表中的指定位置
     * @param timer 要添加的定时器
     * @param lst_head 链表头指针
     */
    void add_timer(util_timer *timer, util_timer *lst_head);
    
    util_timer *head;  // 链表头指针
    util_timer *tail;  // 链表尾指针
};

/**
 * @brief 工具类
 * 
 * 提供一系列工具函数，如信号处理、设置非阻塞等
 */
class Utils {
public:
    Utils() {}
    ~Utils() {}
    
    /**
     * @brief 初始化工具类
     * @param timeslot 时间槽大小
     */
    void init(int timeslot);

    /**
     * @brief 设置文件描述符为非阻塞
     * @param fd 文件描述符
     * @return 旧的文件状态标志
     */
    int setnonblocking(int fd);

    /**
     * @brief 向epoll注册文件描述符
     * @param epollfd epoll文件描述符
     * @param fd 要注册的文件描述符
     * @param one_shot 是否设置EPOLLONESHOT
     * @param TRIGMode 触发模式，0:LT，1:ET
     */
    void addfd(int epollfd, int fd, bool one_shot, int TRIGMode);

    /**
     * @brief 信号处理函数
     * @param sig 信号值
     */
    static void sig_handler(int sig);

    /**
     * @brief 设置信号处理函数
     * @param sig 信号值
     * @param handler 处理函数
     * @param restart 是否重启系统调用
     */
    void addsig(int sig, void(handler)(int), bool restart = true);

    /**
     * @brief 定时处理任务
     * 
     * 处理到期的定时器，并重新定时
     */
    void timer_handler();

    /**
     * @brief 显示错误信息
     * @param connfd 连接文件描述符
     * @param info 错误信息
     */
    void show_error(int connfd, const char *info);
    
public:
    static int *u_pipefd;            // 管道文件描述符
    sort_timer_lst m_timer_lst;      // 定时器链表
    static int u_epollfd;            // epoll文件描述符
    int m_TIMESLOT;                  // 时间槽
};

/**
 * @brief 定时器回调函数
 * 
 * 当定时器到期时执行的函数，关闭对应的客户端连接
 * @param user_data 用户数据，指向客户端数据
 */
void cb_func(client_data *user_data);

#endif