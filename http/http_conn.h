#ifndef HTTPCONN_H
#define HTTPCONN_H

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
#include <string.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <stdarg.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/uio.h>
#include <map>

#include "../lock/locker.h"
#include "../CGImysql/sql_connection_pool.h"
#include "../timer/lst_timer.h"
#include "../log/log.h"

/**
 * @brief HTTP连接处理类
 * 
 * 处理HTTP请求的解析、响应生成和发送
 * 支持GET和POST请求，实现了HTTP协议的主要功能
 */
class http_conn {
public:
    // 文件名最大长度
    static const int FILENAME_LEN = 200;
    // 读缓冲区大小
    static const int READ_BUFFER_SIZE = 2048;
    // 写缓冲区大小
    static const int WRITE_BUFFER_SIZE = 1024;
    
    // HTTP请求方法枚举
    enum METHOD {
        GET = 0,
        POST,
        HEAD,
        PUT,
        DELETE,
        TRACE,
        OPTIONS,
        PATH
    };
    
    // 解析客户端请求时，主状态机的状态
    enum CHECK_STATE {
        CHECK_STATE_REQUESTLINE = 0,  // 正在解析请求行
        CHECK_STATE_HEADER,           // 正在解析头部字段
        CHECK_STATE_CONTENT           // 正在解析请求体
    };
    
    // 服务器处理HTTP请求的可能结果
    enum HTTP_CODE {
        NO_REQUEST,         // 请求不完整，需要继续读取客户数据
        GET_REQUEST,        // 获得了完整的HTTP请求
        BAD_REQUEST,        // HTTP请求有语法错误
        NO_RESOURCE,        // 没有对应的资源
        FORBIDDEN_REQUEST,  // 客户对资源没有足够的访问权限
        FILE_REQUEST,       // 文件请求
        INTERNAL_ERROR,     // 服务器内部错误
        CLOSED_CONNECTION   // 客户端已关闭连接
    };
    
    // 行的读取状态
    enum LINE_STATUS {
        LINE_OK = 0,  // 读取到完整行
        LINE_BAD,     // 行出错
        LINE_OPEN     // 行数据不完整
    };
public:
    http_conn() {}
    ~http_conn() {}
public:
    /**
     * @brief 初始化连接
     * @param sockfd 客户端套接字
     * @param addr 客户端地址
     * @param root 网站根目录
     * @param trigmode 触发模式
     * @param close_log 是否关闭日志
     * @param user 数据库用户名
     * @param passwd 数据库密码
     * @param sqlname 数据库名
     */
    void init(int sockfd, const sockaddr_in &addr, char *, int, int, string user, string passwd, string sqlname);
    
    /**
     * @brief 关闭连接
     * @param real_close 是否真正关闭连接
     */
    void close_conn(bool real_close = true);
    
    /**
     * @brief 处理客户端请求
     * 解析HTTP请求内容并生成响应
     */
    void process();
    
    /**
     * @brief 非阻塞读操作
     * @return 读取是否成功
     */
    bool read_once();
    
    /**
     * @brief 非阻塞写操作
     * @return 写入是否成功
     */
    bool write();
    
    /**
     * @brief 获取客户端地址
     * @return 客户端地址指针
     */
    sockaddr_in *get_address() {
        return &m_address;
    }
    
    /**
     * @brief 初始化数据库连接
     * @param connPool 连接池指针
     */
    void initmysql_result(connection_pool *connPool);
    
    // 定时器相关标志
    int timer_flag;
    int improv;

private:
    /**
     * @brief 初始化连接
     */
    void init();
    
    /**
     * @brief 解析HTTP请求
     * @return 请求的处理结果
     */
    HTTP_CODE process_read();
    
    /**
     * @brief 填充HTTP响应
     * @param ret 响应状态
     * @return 填充是否成功
     */
    bool process_write(HTTP_CODE ret);
    
    /**
     * @brief 解析HTTP请求行
     * @param text 请求行文本
     * @return 解析结果
     */
    HTTP_CODE parse_request_line(char *text);
    
    /**
     * @brief 解析HTTP请求头
     * @param text 请求头文本
     * @return 解析结果
     */
    HTTP_CODE parse_headers(char *text);
    
    /**
     * @brief 解析HTTP请求体
     * @param text 请求体文本
     * @return 解析结果
     */
    HTTP_CODE parse_content(char *text);
    
    /**
     * @brief 处理HTTP请求
     * @return 处理结果
     */
    HTTP_CODE do_request();
    
    /**
     * @brief 获取一行数据
     * @return 行数据的起始位置
     */
    char *get_line() { return m_read_buf + m_start_line; }
    
    /**
     * @brief 解析一行数据
     * @return 行的状态
     */
    LINE_STATUS parse_line();
    
    /**
     * @brief 解除内存映射
     */
    void unmap();
    
    /**
     * @brief 向写缓冲中添加响应
     * @param format 格式化字符串
     * @param ... 可变参数
     * @return 添加是否成功
     */
    bool add_response(const char *format, ...);
    bool add_content(const char *content);
    bool add_status_line(int status, const char *title);
    bool add_headers(int m_content_length);
    bool add_content_type();
    bool add_content_length(int m_content_length);
    bool add_linger();
    bool add_blank_line();

public:
    static int m_epollfd;      // 所有socket上的事件都被注册到同一个epoll内核事件中
    static int m_user_count;   // 统计用户数量
    MYSQL *mysql;              // 数据库连接
    int m_state;               // 读为0，写为1

private:
    int m_sockfd;              // 该HTTP连接的socket
    sockaddr_in m_address;     // 通信的socket地址
    
    char m_read_buf[READ_BUFFER_SIZE];  // 读缓冲区
    long m_read_idx;           // 标识读缓冲区中已经读入的客户端数据的最后一个字节的下一个位置
    long m_checked_idx;        // 当前正在分析的字符在读缓冲区中的位置
    int m_start_line;          // 当前正在解析的行的起始位置
    
    char m_write_buf[WRITE_BUFFER_SIZE];  // 写缓冲区
    int m_write_idx;           // 写缓冲区中待发送的字节数
    
    CHECK_STATE m_check_state; // 主状态机当前所处的状态
    METHOD m_method;           // 请求方法
    
    char m_real_file[FILENAME_LEN];  // 客户请求的目标文件的完整路径
    char *m_url;               // 客户请求的目标文件的文件名
    char *m_version;           // HTTP协议版本号
    char *m_host;              // 主机名
    long m_content_length;     // HTTP请求的消息总长度
    bool m_linger;             // 是否保持连接
    
    char *m_file_address;      // 客户请求的目标文件被mmap到内存中的起始位置
    struct stat m_file_stat;   // 目标文件的状态
    struct iovec m_iv[2];      // 采用writev来执行写操作
    int m_iv_count;            // 被写内存块的数量
    
    int cgi;                   // 是否启用POST
    char *m_string;            // 存储请求体数据
    
    int bytes_to_send;         // 剩余发送字节数
    int bytes_have_send;       // 已发送字节数
    char *doc_root;            // 网站根目录

    map<string, string> m_users;  // 用户名和密码的映射表
    int m_TRIGMode;            // 触发模式
    int m_close_log;           // 是否关闭日志

    char sql_user[100];        // 数据库用户名
    char sql_passwd[100];      // 数据库密码
    char sql_name[100];        // 数据库名
};

#endif