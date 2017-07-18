#include "Thread.h"
#include "Mutex.h"
#include <stdio.h>

class Test {
    class Worker : public Thread {
        Mutex lock;
    public:
        Worker() : lock("TestWorker") {}
        void *entry();
        void stop();
    };
    Worker *TestWorker;
public:
    Test() : TestWorker(NULL) {}
    ~Test() {
        stop_processor();
    }
    void start_processor();
    void stop_processor();
};

void Test::start_processor()
{
    TestWorker = new Worker();
    TestWorker->create();
}
void Test::stop_processor()
{
    if (TestWorker) {
        TestWorker->stop();
        TestWorker->join();
    }
    delete TestWorker;
    TestWorker = NULL;
}
int run()
{
    int a = 10;
    int b = 20;
    return a+b;
}
void *Test::Worker::entry() {
    int r = run();
    printf("SUM:%d\n", r);
    return NULL;
}
void Test::Worker::stop()
{
    Mutex::Locker l(lock);
}
int main()
{
    Test *test_thread  = new Test();
    test_thread->start_processor();
    test_thread->stop_processor();
    return 0;
}

