#include "locker.h"
#include <iostream>
#include <unistd.h>
#include <pthread.h>

int shared_data = 0;
sem semaphore(1);
locker mutex;
cond condition;

void* test_semaphore(void* arg) {
    int thread_id = *(int*)arg;
    std::cout << "Thread " << thread_id << " trying to acquire semaphore" << std::endl;

    if (semaphore.wait()) {
        std::cout << "Thread " << thread_id << " acquired semaphore" << std::endl;
        sleep(1);
        std::cout << "Thread " << thread_id << " releasing semaphore" << std::endl;
        semaphore.post();
    }

    return NULL;
}

void* test_mutex(void* arg) {
    int thread_id = *(int*) arg;

    if (mutex.lock()) {
        std::cout << "Thread " << thread_id << " acquired mutex" << std::endl;
        shared_data++;
        sleep(1);
        std::cout << "Thread " << thread_id << " acquired mutex" << std::endl;
        mutex.unlock();
    }
    
    return NULL;
}

void* test_condition_producer(void* arg) {
    sleep(1);
    if (mutex.lock()) {
        std::cout << "Producer: acquired mutex" << std::endl;
        shared_data = 42;
        std::cout << "Producer: data ready" << std::endl;
        condition.signal();
        mutex.unlock();
    }

    return NULL;
}

void* test_condition_consumer(void* arg) {
    if (mutex.lock()) {
        std::cout << "Consumer: waiting for data" << std::endl;
        while (shared_data == 0) {
            condition.wait(mutex.get());
        }
        std::cout << "Consumer: received data = " << shared_data << std::endl;
        mutex.unlock();
    }

    return NULL;
}

int main() {
    pthread_t threads[4];
    int thread_ids[4] = {1, 2, 3, 4};
    
    std::cout << "=== Testing Semaphore ===" << std::endl;
    pthread_create(&threads[0], NULL, test_semaphore, &thread_ids[0]);
    pthread_create(&threads[1], NULL, test_semaphore, &thread_ids[1]);
    pthread_join(threads[0], NULL);
    pthread_join(threads[1], NULL);

    std::cout << "\n=== Testing Mutex ===" << std::endl;
    pthread_create(&threads[2], NULL, test_mutex, &thread_ids[2]);
    pthread_create(&threads[3], NULL, test_mutex, &thread_ids[3]);
    pthread_join(threads[2], NULL);
    pthread_join(threads[3], NULL);

    std::cout << "\n=== Testing Condition Variable ===" << std::endl;
    pthread_t producer, consumer;
    pthread_create(&producer, NULL, test_condition_producer, NULL);
    pthread_create(&consumer, NULL, test_condition_consumer, NULL);
    pthread_join(producer, NULL);
    pthread_join(consumer, NULL);

    std::cout << "\nAll tests completed!" << std::endl;
    return 0;
}