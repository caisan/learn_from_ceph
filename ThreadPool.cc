#include <assert.h>
#include <stdio.h>
#include <dirent.h>
#include <unistd.h>
#include <errno.h>
#include <iostream>
#include <pthread.h>
#include <signal.h>
#include <sstream>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/syscall.h> /* For SYS_xxx definitions */
#include "ThreadPool.h"
//#include "page.h"

using namespace std;

Thread::Thread()
    : thread_id(0),
      pid(0)
{
}

Thread::~Thread()
{
}
pid_t _gettid(void)
{
#ifdef __linux__
    return syscall(SYS_gettid);
#else
    return -ENOSYS;
#endif
}
void *Thread::entry_wrapper()
{
    int p = _gettid();
    if ( p > 0 )
        pid = p;
    //TODO:set io prioority
    return entry();
}
const pthread_t &Thread::get_thread_id()
{
    return thread_id;
}

bool Thread::is_started()
{
    return thread_id != 0;
}

bool Thread::am_self()
{
    return (pthread_self() == thread_id);
}

int Thread::kill(int signal)
{
    if (thread_id)
        return pthread_kill(thread_id, signal);
    else
        return -EINVAL;
}
int Thread::try_create(size_t stacksize)
{
    pthread_attr_t *thread_attr = NULL;
    /*
    stacksize &= _PAGE_MASK;
    if (stacksize) {
        thread_attr = (pthread_attr_t *)malloc(sizeof(pthread_attr_t));
        if (!thread_attr)
            return -ENOMEM;
        pthread_attr_init(thread_attr);
        pthread_attr_setstacksize(thread_attr, stacksize);
    }
*/
    int r;
    r = pthread_create(&thread_id, thread_attr, _entry_func, (void*)this);

    if (thread_attr)
        free(thread_attr);
    return r;
}
void Thread::create(size_t stacksize)
{
    int ret = try_create(stacksize);
    if (ret != 0) {
        char buf[256];
        snprintf(buf, sizeof(buf), "Thread::try_create() : pthread_create "
                "failed with error %d", ret);
        assert(ret == 0);
    }
}
int Thread::join(void **prval)
{
    if (thread_id == 0)
    {
        assert("join on thread that was never started" == 0);
        return -EINVAL;
    }
    int status = pthread_join(thread_id, prval);
    assert(status == 0);
    thread_id = 0;
    return status;
}
int Thread::detach()
{
    return pthread_detach(thread_id);
}
void *Thread::_entry_func(void *arg) {
    void *r = ((Thread*)arg)->entry_wrapper();
    return r;
}
/*
void *Thread::entry_wrapper()
{
    return entry();
}*/
//Thread Pool impl
ThreadPool::ThreadPool(string nm, int n)
	: name(nm),
	_stop(false),
	_pause(0),
	_num_threads(n),
	last_work_queue(0),
	processing(0)
{
}

ThreadPool::~ThreadPool()
{
	assert(_threads.empty());
}

void ThreadPool::start()
{
	std::cout<<"start"<<std::endl;
	start_threads();
	std::cout<<"started"<<std::endl;
}

void ThreadPool::start_threads()
{
	while (_threads.size() < _num_threads) {
		WorkThread *wt = new WorkThread(this);
		std::cout<<"start_threads createing and statring"<<std::endl;
		_threads.insert(wt);

		wt->create(20);
	}
}

void ThreadPool::worker(WorkThread *wt)
{
	std::cout<<"worker start"<<std::endl;
	std::stringstream ss;
	ss << name << " thread " << (void *)pthread_self();
	while (!_stop) {
		if (!_pause && !work_queues.empty()) {
			WorkQueue_* wq;
			int tries = work_queues.size();
			bool did = false;
			while(tries--) {
				last_work_queue++;
				last_work_queue %= work_queues.size();
				wq = work_queues[last_work_queue];

				void *item = wq->_void_dequeue();
				if (item) {
					processing++;
					std::cout<<"worker wq:"<< wq->name << " start processing "<<std::endl;
					wq->_void_process(item);
					//wq->_void_process_finish(item);
					processing--;
					did = true;
				}
			}
			if (did)
				continue;
		}
	}
	std::cout<<"worker stopping"<<std::endl;
}

void ThreadPool::stop(bool clear_after)
{
  std::cout<<"worker thread stop"<<std::endl;
  _stop = true;
  for(set<WorkThread*>::iterator p = _threads.begin();p!=_threads.end(); ++p){
    (*p)->join();
    delete *p;
  }
  _stop = false;
}
