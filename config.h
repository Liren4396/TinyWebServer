#include "webserver.h"

using namespace std;

/**
 * @brief 配置类 - 管理服务器配置参数
 * 
 * 负责解析命令行参数，设置服务器各项配置的默认值和自定义值
 */
class Config {
public:
    /**
     * @brief 构造函数 - 设置默认配置值
     */
    Config();
    
    /**
     * @brief 析构函数
     */
    ~Config() {};
    
    /**
     * @brief 解析命令行参数
     * @param argc 参数数量
     * @param argv 参数数组
     */
    void parse_arg(int argc, char* argv[]);
    
    int PORT;              // 服务器端口号，默认9006
    int LOGWrite;          // 日志写入方式，0:同步写入，1:异步写入
    int TRIGMode;          // 触发组合模式，0:LT+LT，1:LT+ET，2:ET+LT，3:ET+ET
    int LISTENTrigmode;    // 监听socket的触发模式，0:LT，1:ET
    int CONNTrigmode;      // 连接socket的触发模式，0:LT，1:ET
    int OPT_LINGER;        // 优雅关闭连接，0:不使用，1:使用
    int sql_num;           // 数据库连接池数量，默认8
    int thread_num;        // 线程池内线程数量，默认8
    int close_log;         // 是否关闭日志，0:不关闭，1:关闭
    int actor_model;       // 并发模型选择，0:Proactor，1:Reactor
};