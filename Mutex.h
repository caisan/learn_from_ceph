#ifndef MUTEX_H
#define MUTEX_H

#include <pthread.h>

class Mutex
{
private:
	const char* name;
	int id;
	bool backtrace;
	
	pthread_mutex_t _m;
	int nlock;
	pthread_t locked_by;
	
	Mutex(const Mutex &M);
public:

	Mutex(const char *n, bool r=false, bool ld=true, bool bt=false);
	~Mutex();

	bool is_locked() const {
		return (nlock > 0);
	}
	bool TryLock()
	{
		int r = pthread_mutex_trylock(&_m);
		return r == 0;
	}
	bool is_locked_by_me() const {
		return nlock > 0 && locked_by == pthread_self();
	}

	void Lock();
	void Unlock();
};

