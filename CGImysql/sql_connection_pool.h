#ifndef SQLCONNECTION_H
#define SQLCONNECTION_H

#include <stdio.h>
#include <list>
#include <mysql/mysql.h>
#include <error.h>
#include <string.h>
#include <iostream>
#include <string>
#include "../lock/locker.h"
#include "../log/log.h"

using namespace std;

/**
 * @brief 数据库连接池类
 * 
 * 实现MySQL数据库连接池，管理连接的创建和释放
 * 单例模式确保全局唯一
 */
class connection_pool {
public:
    /**
     * @brief 获取一个数据库连接
     * 
     * 从连接池中取出一个可用连接
     * @return MYSQL* 数据库连接指针
     */
    MYSQL *GetConnection();
    
    /**
     * @brief 释放一个数据库连接
     * 
     * 将使用完的连接放回连接池
     * @param conn 数据库连接指针
     * @return 释放是否成功
     */
    bool ReleaseConnection(MYSQL *conn);
    
    /**
     * @brief 获取空闲连接数量
     * @return 当前空闲连接数
     */
    int GetFreeConn();
    
    /**
     * @brief 销毁连接池
     * 
     * 关闭所有连接并清空连接池
     */
    void DestroyPool();

    /**
     * @brief 获取连接池单例实例
     * 
     * 单例模式，确保全局唯一的连接池
     * @return 连接池单例指针
     */
    static connection_pool *GetInstance();
    
    /**
     * @brief 初始化连接池
     * 
     * @param url 数据库主机地址
     * @param User 数据库用户名
     * @param passWord 数据库密码
     * @param DataBaseName 数据库名
     * @param Port 数据库端口
     * @param MaxConn 最大连接数
     * @param close_log 是否关闭日志
     */
    void init(string url, string User, string passWord, string DataBaseName, int Port, int MaxConn, int close_log);
    
private:
    /**
     * @brief 私有构造函数
     * 
     * 实现单例模式，防止外部直接创建实例
     */
    connection_pool();
    
    /**
     * @brief 私有析构函数
     */
    ~connection_pool();

    int m_MaxConn;   // 最大连接数
    int m_CurConn;   // 当前已使用的连接数
    int m_FreeConn;  // 当前空闲的连接数
    locker lock;     // 互斥锁，保护连接池
    list<MYSQL *> connList;  // 连接池
    sem reserve;     // 信号量，表示可用连接数
    
public:
    string m_url;          // 主机地址
    string m_Port;         // 数据库端口
    string m_User;         // 数据库用户名
    string m_PassWord;     // 数据库密码
    string m_DatabaseName; // 数据库名
    int m_close_log;       // 日志开关
};

/**
 * @brief 数据库连接的RAII封装类
 * 
 * 使用RAII技术，确保数据库连接的自动获取和释放
 */
class connectionRAII {
public:
    /**
     * @brief 构造函数
     * 
     * 自动从连接池获取一个连接
     * @param con 数据库连接的二级指针
     * @param connPool 连接池指针
     */
    connectionRAII(MYSQL **con, connection_pool *connPool);
    
    /**
     * @brief 析构函数
     * 
     * 自动释放数据库连接回连接池
     */
    ~connectionRAII();
    
private:
    MYSQL *conRAII;           // 持有的数据库连接
    connection_pool *poolRAII; // 数据库连接池的指针
};

#endif