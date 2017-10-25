#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include <pthread.h>

/* pthread() wrapper*/

class Thread {
    private:
        pthread_t thread_id;
        pid_t pid;
        int ioprio_class, ioprio_priority;

        void *entry_wrapper();

    public:
        Thread(const Thread& other);
        const Thread& operator=(const Thread& other);

        Thread();
        virtual ~Thread();

    protected:
        virtual void *entry() = 0;

    private:
        static void *_entry_func(void *arg);

    public:
        const pthread_t &get_thread_id();
        bool is_started();
        bool am_self();
        int kill(int signal);
        int try_create(size_t stacksize);
        void create(size_t stacksize = 0);
        int join(void **prval = 0);
        int detach();
        int set_ioprio(int cls, int prio);
};

/* thread pool */
class ThreadPool {
	string name;
	bool _stop;
	int _pause;
	int _draining;

public:
private:

	struct WorkQueue_ {
		string name;
		time_t timeout_interval, suicide_interval;
		WorkQueue_(string n, time_t ti, time_t sti)
			: name(n), timeout_interval(ti), suicide_interval(sti)
		{  }
		virtual ~WorkQueue_() {}
		virtual void _clear() = 0;
		virtual bool _empty() = 0;
		virtual void *_void_dequeue() = 0;
		virtual void _void_process_finish(void *) = 0;
	};

	//跟踪线程池中线程数量的变化
	unsigned _num_threads;
	string _thread_num_option;
	const char **_conf_keys;

	const char **get_tracked_conf_keys() const {
		return _conf_keys;
	}
	//void handle_conf_change(const struct )
public:
	template<class T>
	class WorkQueue : public WorkQueue_ {
		ThreadPool *pool;

		virtual bool _enqueue(T *) = 0;
		virtual void _dequeue(T *) = 0;
		virtual T *_dequeue() = 0;
		virtual void _process(T *t) { assert(0); }
		virtual void _process(T *t, TPHandle &) {
			_process(t);
		}
		virtual void _process_finish(T *) {}

		void *_void_dequeue() {
			return (void*)_dequeue();
		}
		void _void_process(void *p, TPHandle &handle) {
			_process(static_cast<T *>(p), handle);
		}
		void _void_process_finish(void *p) {
			_process_finish(static_cast<T *>(p));
		}

	public:
		WorkQueue(string n, time_t ti, time_t sti, ThreadPool *p) : WorkQueue_(n, ti, sti), pool(p) {
			pool->add_work_queue(this);
		}
		~WorkQueue() {
			pool->remove_work_queue(this);
		}
		bool queue(T *item) {
			pool->_lock.Lock();
			bool r = _enqueue(item);
			pool->_lock.Unlock();
			return r;
		}
		void dequeue(T *item) {
			pool->_lock.Lock();
			_dequeue(item);
			pool->_lock.Unlock();
		}
		void clear() {
			pool->_lock.Lock();
			_clear();
			pool->_lock.Unlock();
		}
		void lock() {
			pool->lock();
		}
		void Unlock() {
			pool->unlock();
		}
	};

private:
	vector<WorkQueue_ *> work_queues;
	int last_work_queue;

	//Threads
	struct WorkThread : public Thread {
		ThreadPool *pool;
		WorkThread(ThreadPool *p) : pool(p) {}
		void *entry() {
			pool->worker(this);
			return 0;
		}
	};

	set<WorkThread*> _threads;
	list<WorkThread*> _old_threads;

	int processing;

	void start_threads();
	void join_old_threads();
	void worker(WorkThread *wt);

public:
	ThreadPool(string nm, int n, const char *option = NULL);
	~ThreadPool();

	int get_num_threads() {
		return _num_threads;
	}

	void add_work_queue(WorkQueue_* wq) {
		work_queues.push_back(wq);
	}

	void remove_work_queue(WorkQueue_* wq) {
		unsigned i = 0;
		while (work_queues[i] != wq)
			i++;
		for (i++; i < work_queues.size(); i++)
			work_queues[i-1] = work_queues[i];
		assert(i == work_queues.size());
		work_queues.resize(i-1);
	}

	void start();
	void stop(bool clear_after = true);
	void pause();
	void pause_new();
	void unpause();

};
#endif

