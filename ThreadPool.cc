#include <assert.h>
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
    stacksize &= _PAGE_MASK;
    if (stacksize) {
        thread_attr = (pthread_attr_t *)malloc(sizeof(pthread_attr_t));
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
