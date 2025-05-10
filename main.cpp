#include "config.h"

int main(int argc, char *argv[]) {
    // 数据库用户名、密码和数据库名
    string user = "root";
    string passwd = "123456";
    string databasename = "yourdb";

    // 创建配置对象
    Config config;
    // 解析命令行参数
    config.parse_arg(argc, argv);

    // 创建Web服务器对象
    WebServer server;

    // 初始化服务器
    // 参数包括：端口、数据库用户名/密码/数据库名、日志写入方式、优雅关闭连接选项
    // 触发模式、数据库连接数、线程数、是否关闭日志、并发模型选择
    server.init(config.PORT, user, passwd, databasename, config.LOGWrite,
                config.OPT_LINGER, config.TRIGMode, config.sql_num, config.thread_num,
                config.close_log, config.actor_model);

    // 初始化日志系统
    server.log_write();

    // 初始化数据库连接池
    server.sql_pool();

    // 初始化线程池
    server.thread_pool();

    // 设置触发模式（LT/ET）
    server.trig_mode();

    // 开始监听连接请求
    server.eventListen();

    // 启动事件循环处理
    server.eventLoop();
}