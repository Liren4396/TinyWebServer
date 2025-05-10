#include "config.h"

/**
 * @brief 构造函数 - 设置服务器默认配置
 * 
 * 初始化各项服务器配置的默认值
 */
Config::Config() {
    PORT = 9006;           // 默认端口号为9006
    LOGWrite = 0;          // 默认同步写入日志
    TRIGMode = 0;          // 默认使用LT+LT模式
    LISTENTrigmode = 0;    // 监听socket使用LT模式
    CONNTrigmode = 0;      // 连接socket使用LT模式
    OPT_LINGER = 0;        // 默认不使用优雅关闭连接
    sql_num = 8;           // 默认数据库连接池数量为8
    thread_num = 8;        // 默认线程池线程数量为8
    close_log = 0;         // 默认不关闭日志
    actor_model = 0;       // 默认使用Proactor模型
}

/**
 * @brief 解析命令行参数
 * 
 * 从命令行获取用户指定的服务器配置参数
 * 
 * @param argc 参数数量
 * @param argv 参数数组
 */
void Config::parse_arg(int argc, char* argv[]) {
    int opt;
    // 定义命令行选项字符串，冒号表示该选项后跟参数
    const char *str = "p:l:m:o:s:t:c:a:";
    
    // 使用getopt解析命令行参数
    while ((opt = getopt(argc, argv, str)) != -1) {
        switch (opt) {
        case 'p': // 端口号
        {
            PORT = atoi(optarg);
            break;
        }
        case 'l': // 日志写入方式
        {
            LOGWrite = atoi(optarg);
            break;
        }
        case 'm': // 触发模式
        {
            TRIGMode = atoi(optarg);
            break;
        }
        case 'o': // 优雅关闭选项
        {
            OPT_LINGER = atoi(optarg);
            break;
        }
        case 's': // 数据库连接数量
        {
            sql_num = atoi(optarg);
            break;
        }
        case 't': // 线程数量
        {
            thread_num = atoi(optarg);
            break;
        }
        case 'c': // 日志开关
        {
            close_log = atoi(optarg);
            break;
        }
        case 'a': // 并发模型选择
        {
            actor_model = atoi(optarg);
            break;
        }
        default:
            break;
        }
    }
}