#include <string>
#include "Mutex.h"

Mutex::Mutex(const char *n, bool r, bool ld, bool bt):
{
    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
    pthread_mutex_init(&_m,&attr);
    pthread_mutexattr_destroy(&attr);
}
void Mutex::Lock(bool no_lockdep)
{
	if (TryLock()) {
		goto out;
	}
	int r;
	r = pthread_mutex_lock(&_m);
out:
	;
}
void Mutex::Unlock() {
	_pre_unlock();
	int r = pthread_mutex_unlock(&_m);
	assert(r == 0);
}
Mutex::~Mutex() {
    assert(nlock == 0);
    pthread_mutex_destroy(&_m);
}
void Mutex::Lock(bool no_lockdep) {
    if (TryLock()) {
        return;
    }
    int r = pthread_mutex_lock(&_m);
    assert(r == 0);
}      

