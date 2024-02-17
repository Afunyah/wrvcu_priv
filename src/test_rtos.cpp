#include "arduino_freertos.h"
#include "rtos/rtos.hpp"

using namespace wrvcu;

void task1() {
    while (true) {
        digitalWrite(LED_BUILTIN, HIGH);
        Task::delay(500);

        digitalWrite(LED_BUILTIN, LOW);
        Task::delay(500);
    }
}

int a = 0;
Mutex a_mutex;

void task2() {
    while (true) {
        // Serial.println("TICK");
        // Task::delay(1000);

        // Serial.println("TOCK");
        // Task::delay(1000);

        a_mutex.take();
        printf("a count: %d\n", a);
        a_mutex.give();
        Task::delay(1000);
    }
}

void task3() {
    while (true) {
        a_mutex.take();
        a += 1;
        a_mutex.give();
        Task::delay(200);
    }
}

void task4() {
    while (true) {
        a_mutex.take();
        a += 1;
        a_mutex.give();
        Task::delay(200);
    }
}

Semaphore sem(32, 0);

void task5() {
    while (true) {
        sem.post();
        Task::delay(200);
    }
}

void task6() {
    while (true) {
        printf("Semaphore count: %d\n", (int)sem.get_count());
        while (sem.wait(0))
            ;
        Task::delay(1000);
    }
}

Queue<int> queue(32);

void task7() {
    int i = 0;
    while (true) {
        queue.enqueue(i, TIMEOUT_MAX);
        i++;
        Task::delay(500);
    }
}

void task8() {
    while (true) {
        printf("Queue size: %d\n", (int)queue.size());
        while (queue.size() > 0) {
            int i = queue.dequeue(0);
            printf("item: %d\n", i);
        }
        Task::delay(1000);
    }
}

void test_rtos() {
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, LOW);

    Task(task1, "task1");
    Task(task2, "task2");
    Task(task3, "task3");
    Task(task4, "task4");
    Task(task5, "task5");
    Task(task6, "task6");
    Task(task7, "task7");
    Task(task8, "task8");

    startScheduler();
}