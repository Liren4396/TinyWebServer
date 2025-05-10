#include <iostream>
#include <string>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "http_conn.h"

// 测试 HTTP 请求解析
void test_http_parsing() {
    // 创建 http_conn 对象
    http_conn conn;
    
    // 初始化连接
    sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(9006);
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    
    // 初始化连接
    conn.init(0, addr, nullptr, 0, 0, "root", "123456", "webdb");
    
    // 测试 GET 请求
    std::cout << "\nTesting GET request parsing..." << std::endl;
    const char* get_request = "GET /index.html HTTP/1.1\r\n"
                             "Host: localhost:9006\r\n"
                             "Connection: keep-alive\r\n"
                             "\r\n";
    
    // 测试 POST 请求
    std::cout << "\nTesting POST request parsing..." << std::endl;
    const char* post_request = "POST /2 HTTP/1.1\r\n"
                              "Host: localhost:9006\r\n"
                              "Content-Type: application/x-www-form-urlencoded\r\n"
                              "Content-Length: 23\r\n"
                              "Connection: keep-alive\r\n"
                              "\r\n"
                              "username=admin&password=123";
    
    // 测试错误请求
    std::cout << "\nTesting bad request parsing..." << std::endl;
    const char* bad_request = "INVALID REQUEST LINE\r\n"
                             "Host: localhost:9006\r\n"
                             "\r\n";
    
    // 测试请求处理
    std::cout << "\nTesting request processing..." << std::endl;
    conn.process();
    
    // 测试连接关闭
    std::cout << "\nTesting connection closing..." << std::endl;
    conn.close_conn();
}

int main() {
    std::cout << "HTTP Connection Class Test Program" << std::endl;
    std::cout << "----------------------------------------" << std::endl;
    
    test_http_parsing();
    
    std::cout << "\nTest completed" << std::endl;
    return 0;
} 