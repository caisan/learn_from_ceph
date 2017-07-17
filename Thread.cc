#include "Thread.h"
#include <errno.h>
#include <pthread.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <sstream>
#include <dirent.h>
#include <iostream>

Thread::Thread():thread_id(0), pid(0), ioprio_class(-1), ioprio_priority(-1)
{
}

Thread::~Thread()
{
}

void *Thread::_entry_func(void *arg){
	void *r = ((Thread*)arg)->entry_wrapper();
	return r;
}

void *Thread::entry_wrapper()
{
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
	if (stacksize) {
		thread_attr = (pthread_attr_t*)malloc(sizeof(pthread_attr_t));
		if (!thread_attr)
			return -ENOMEM;
		pthread_attr_init(thread_attr);
		pthread_attr_setstacksize(thread_attr, stacksize);
	}
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
		snprintf(buf, sizeof(buf), "Thread::try_create(): pthread_create failed with error %d", ret);
	}
}
int Thread::join(void **prval)
{
	if (pthread_id == 0) {
		assert("join on thread that was never started" == 0);
		return -EINVAL;
	}
	int status = pthread_join(thread_id, prval);
	assert(status == 0);
	return status;
}
int Thread::detach()
{
	return pthread_detach(thread_id);
}

