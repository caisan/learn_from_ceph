#ifndef MUTEX_H
#define MUTEX_H

#include <pthread.h>
#include <assert.h>

enum {
    l_mutex_first = 999082,
    l_mutex_wait,
    l_mutex_last
};
class Mutex {
    private:
        const char *name;
        int id;
        bool recursive;
        bool lockdep;
        pthread_mutex_t _m;
        int nlock;
        pthread_t locked_by;
        void operator=(Mutex &M);
        Mutex(const Mutex &M);
    public:
        Mutex(const char *n, bool r = false, bool ld=true, bool bt=false);
        ~Mutex();
        bool is_locked() const {
            return (nlock > 0);
        }
        bool is_locked_by_me() const {
            return nlock > 0 && locked_by == pthread_self();
        }
        bool TryLock() {
            int r = pthread_mutex_trylock(&_m);
            return r==0;
        }
        void Lock(bool no_lockdep = false);
        void Unlock() {
            int r = pthread_mutex_unlock(&_m);
            assert(r==0);
        }
    public:
        class Locker{
            Mutex &mutex;
        public:
            Locker(Mutex& m):mutex(m) {
                mutex.Lock();
            }
            ~Locker() {
                mutex.Unlock();
            }

        };
};
#endif
