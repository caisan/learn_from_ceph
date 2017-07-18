#include <dirent.h>
#include <assert.h>
#include <errno.h>
#include <iostream>
#include <pthread.h>
#include <signal.h>
#include <sstream>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <unistd.h>
#include "Thread.h"

#ifndef IOPRIO_WHO_PROCESS
#define IOPRIO_WHO_PROCESS 1
#endif

#ifndef IOPRIO_PRIO_VALUE
# define IOPRIO_CLASS_SHIFT 13
# define IOPRIO_PRIO_VALUE(class, data) \
                    (((class) << IOPRIO_CLASS_SHIFT) | (data))
#endif

pid_t get_tid(void)
{
#ifdef __linux__
    return syscall(SYS_gettid);
#else
    return -ENOSYS;
#endif
}
int ioprio_set(int whence, int who, int ioprio)
{
#ifdef __linux__
    return syscall(SYS_ioprio_set, whence, who, ioprio);
#else
    return -ENOSYS;
#endif
}

Thread::Thread()
    : thread_id(0),
    pid(0),
    ioprio_class(-1),
    ioprio_priority(-1)
{

}
Thread::~Thread()
{

}
void *Thread::_entry_func(void *arg) {
    void *r = ((Thread*)arg)->entry_wrapper();
    return r;

}
void *Thread::entry_wrapper()
{
    int p = get_tid();
    if (p>0)
        pid = p;
    if (ioprio_class >=0 &&
            ioprio_priority >= 0) {
        ioprio_set(IOPRIO_WHO_PROCESS,
                   pid,
                   IOPRIO_PRIO_VALUE(ioprio_class, ioprio_priority));
    }
    return entry();
}
const pthread_t &Thread::get_thread_id()
{
    return thread_id;
}
bool Thread::is_started()
{
    return thread_id !=0;
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
#define PAGE_MASK _page_mask

int Thread::try_create(size_t stacksize)
{
    unsigned _page_size = sysconf(_SC_PAGESIZE);
    unsigned long _page_mask = ~(unsigned long)(_page_size - 1);
    pthread_attr_t *thread_attr = NULL;
    stacksize &= PAGE_MASK;
    if (stacksize) {
        thread_attr = (pthread_attr_t *)malloc(sizeof(pthread_attr_t));
        if (!thread_attr)
            return -ENOMEM;
        pthread_attr_init(thread_attr);
        pthread_attr_setstacksize(thread_attr, stacksize);
    }

    int r;
    sigset_t old_sigset;
    r = pthread_create(&thread_id, thread_attr, _entry_func, (void*)this);
    if (thread_attr)
        free(thread_attr);
    return r;
}
void Thread::create(size_t stacksize)
{
    int ret = try_create(stacksize);
    if (ret != 0)
    {
        char buf[256];
        snprintf(buf, sizeof(buf), "Thread::try_create():pthread_create failed with error %d", ret);
        assert(ret == 0);
    }
}
int Thread::join(void **prval)
{
    if (thread_id == 0){
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
int Thread::set_ioprio(int cls, int prio)
{
    ioprio_class = cls;
    ioprio_priority = prio;
    if (pid && cls >= 0 && prio >= 0)
        return ioprio_set(IOPRIO_WHO_PROCESS,
                          pid,
                          IOPRIO_PRIO_VALUE(cls, prio));
    return 0;
}
