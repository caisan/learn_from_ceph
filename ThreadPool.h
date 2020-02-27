#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include <pthread.h>
#include <string>
#include <list>
#include <vector>
#include <set>
using namespace std;
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
	int _num_threads;
	int _pause;
	int _draining;
public:
private:

	struct WorkQueue_ {
		string name;
		WorkQueue_(string n)
			: name(n)
		{  }
		virtual ~WorkQueue_() {}
		virtual void _clear() = 0;
		virtual bool _empty() = 0;
		virtual void *_void_dequeue() = 0;
                virtual void _void_process(void *) = 0;
		virtual void _void_process_finish(void *) = 0;
	};

public:
	template<class T>
	class WorkQueue : public WorkQueue_ {
        public:
		ThreadPool *pool;
                std::list<T *> m_items;
        private:
		virtual bool _enqueue(T *) = 0;
		virtual void _dequeue(T *) = 0;
		virtual T *_dequeue() = 0;
                
		virtual void process(T *t) = 0;
                /* {
			_process(t);
		}*/
		virtual void _process_finish(T *) =0;/*{}*/
                
		void *_void_dequeue() {
			return (void*)_dequeue();
		}
		void _void_process(void *p) {
			process(static_cast<T *>(p));
		}
		void _void_process_finish(void *p) {
			_process_finish(static_cast<T *>(p));
		}

	public:
		WorkQueue(string n, ThreadPool *p) : WorkQueue_(n), pool(p) {
			//pool->add_work_queue(this);
		}
		~WorkQueue() {
			pool->remove_work_queue(this);
		}
		bool queue(T *item) {
			//bool r = _enqueue(item);
                        m_items.push_back(item);
		}
		void dequeue(T *item) {
			_dequeue(item);
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
	ThreadPool(string nm, int n);
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
		//assert(i == work_queues.size());
		work_queues.resize(i-1);
	}

	void start();
	void stop(bool clear_after = true);
	void pause();
	void pause_new();
	void unpause();

};
#endif
