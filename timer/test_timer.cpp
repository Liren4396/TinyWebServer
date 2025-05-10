#include "lst_timer.h"
#include <iostream>
#include <unistd.h>
#include <pthread.h>
#include <string>

client_data* create_client_data(int sockfd) {
    client_data* client = new client_data();
    client->sockfd = sockfd;
    client->timer = nullptr;
    return client;
}

void timer_callback(client_data* user_data) {
    std::cout << "Timer expired for socket: " << user_data->sockfd << std::endl;
    delete user_data;
}

void test_add_timer(sort_timer_lst& timer_lst) {
    std::cout << "\n=== Testing Add Timer ===" << std::endl;
    client_data* client1 = create_client_data(1);
    client_data* client2 = create_client_data(2);
    client_data* client3 = create_client_data(3);

    util_timer* timer1 = new util_timer();
    util_timer* timer2 = new util_timer();
    util_timer* timer3 = new util_timer();

    timer1->expire = time(NULL) + 5;
    timer2->expire = time(NULL) + 10;
    timer3->expire = time(NULL) + 15;

    timer1->cb_func = timer_callback;
    timer2->cb_func = timer_callback;
    timer3->cb_func = timer_callback;

    timer1->user_data = client1;
    timer2->user_data = client2;
    timer3->user_data = client3;

    timer_lst.add_timer(timer1);
    timer_lst.add_timer(timer2);
    timer_lst.add_timer(timer3);

    std::cout << "Added 3 timers with different expiration times" << std::endl;
}

void test_adjust_timer(sort_timer_lst& timer_lst) {
    std::cout << "\n=== Testing Adjust Timer ===" << std::endl;
    client_data* client = create_client_data(4);

    util_timer* timer = new util_timer();
    timer->expire = time(NULL) + 5;
    timer->cb_func = timer_callback;
    timer->user_data = client;

    timer_lst.add_timer(timer);

    timer->expire = time(NULL) + 20;
    timer_lst.adjust_timer(timer);

    std::cout << "Adjust timer expiration time" << std::endl;
}

void test_del_timer(sort_timer_lst& timer_lst) {
    std::cout << "\n=== Testing Delete Timer ===" << std::endl;
    client_data* client = create_client_data(5);

    util_timer* timer = new util_timer();
    timer->expire = time(NULL) + 5;
    timer->cb_func = timer_callback;
    timer->user_data = client;

    timer_lst.add_timer(timer);

    timer_lst.del_timer(timer);

    std::cout << "Deleted timer" << std::endl;
}

void test_timer_expiration(sort_timer_lst& timer_lst) {
    std::cout << "\n=== Testing Timer Expiration ===" << std::endl;

    client_data* client = create_client_data(6);

    util_timer* timer = new util_timer();
    timer->expire = time(NULL) + 3;
    timer->cb_func = timer_callback;
    timer->user_data = client;

    timer_lst.add_timer(timer);

    std::cout << "Added timer that will expire in 3 seconds" << std::endl;

    sleep(4);

    timer_lst.tick();
}

int main() {
    sort_timer_lst timer_lst;
    test_add_timer(timer_lst);
    test_adjust_timer(timer_lst);
    test_del_timer(timer_lst);
    test_timer_expiration(timer_lst);

    std::cout << "\nAll tests completed!" << std::endl;
    return 0;
}