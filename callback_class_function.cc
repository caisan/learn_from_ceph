// =====================================================================================
// 
//       Filename:  callback_class_function.cc
// 
//    Description:  callback class and callback function, code snippet from ceph
// 
//        Version:  1.0
//        Created:  2020/02/23 11时54分35秒
//       Revision:  none
//       Compiler:  g++/gcc
// =====================================================================================
#include <iostream>
#include <string>
#include <errno.h>
#include <iostream>
#include <sstream>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <assert.h>
#include <signal.h>
#include <pthread.h>
#include <sys/types.h>
#include <vector>
#include <list>
using namespace std;

//Base class and function
class Context {
  Context(const Context& other);
  const Context& operator=(const Context& other);

  protected:
   virtual void finish(int r) = 0;

  public:
   Context() {}
   virtual ~Context() {}
   virtual void complete(int r) {
     finish(r);
     delete this;
   }
};

class Thread {
private:
    pthread_t thread_id;
    const char* thread_name;
    pid_t pid;
    
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
    pid_t get_pid() const { return pid; }
    bool is_started() const;
    bool am_self();
    int kill(int signal);
    int try_create(size_t stacksize);
    void create(size_t stacksize = 0);
    void create(const char* name, size_t stacksize = 0);
    int join(void **prval = 0);
    int detach();
};
Thread::~Thread()
{
}

Thread::Thread():thread_id(0),pid(0)
{
}

void *Thread::_entry_func(void *arg) {
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
bool Thread::is_started() const
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
#define PAGE_MASK 100
int Thread::try_create(size_t stacksize)
{
    pthread_attr_t *thread_attr = NULL;
    pthread_attr_t thread_attr_loc;

    stacksize &= PAGE_MASK;

    if (stacksize) {
        thread_attr = &thread_attr_loc;
        pthread_attr_init(thread_attr);
        pthread_attr_setstacksize(thread_attr, stacksize);
    }
    int r;
    r = pthread_create(&thread_id, thread_attr, _entry_func, (void*)this);
    if (thread_attr) {
        pthread_attr_destroy(thread_attr); 
    }
    return r;
}
void Thread::create(size_t stacksize)
{
    int ret = try_create(stacksize);
    if (ret != 0) {
      char buf[256];
      snprintf(buf, sizeof(buf), "Thread::try_create(): pthread_create "
                                              "failed with error %d", ret);
      std::cout<<buf<<std::endl;
      assert(ret == 0);
   }
}
void Thread::create(const char* name, size_t stacksize)
{
    assert(strlen(name) < 16);
    thread_name = name;
    int ret = try_create(stacksize);
    if(ret!=0)
    {
        char buf[256];
        snprintf(buf, sizeof(buf), "Thread::try_create(): pthread_create failed wi  th %d", ret);
        assert(ret==0);
    }
}
int Thread::join(void **prval)
{
    if (thread_id == 0) {
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

class ThreadPool
{
    string name;
    string thread_name;
public:
private:
  struct WorkQueue_ {
    string name;
    time_t timeout_interval, suicide_interval;
    WorkQueue_(string n, time_t ti, time_t sti):
      : name(std::move(n)), timeout_interval(ti), suicide_interval(sti)
    { }
    virtual ~WorkQueue_() {}
    virtual void _clear() = 0;
    virtual bool _empty() = 0;
    virtual void *_void_dequeue() = 0;
    virtual void _void_process(void *item, TPHandle &handle) = 0;
    virtual void _void_process_finish(void *) = 0;
  };
  // track thread pool size changes
  unsigned _num_threads;
  
  template<class T>
  class PointerWQ : public WorkQueue_ {
    public:
      ~PointerWQ() override {
        m_pool->remove_work_queue(this);
        assert(m_processing == 0);
      }
      void drain() {
        {
          if (m_processing == 0 && m_items.empty()) {return ;}
        }
        m_pool->drain(this);
      }
      void queue(T *item) {
        m_items.push_back(item);
      }
      bool empty() {
        return _empty();
      }
    protected:
      PointerWQ(string n, time_t ti, time_t sti, ThreadPool* p):
        WorkQueue_(std::move(n), ti, sti), m_pool(p), m_processing(0) {
        
      }
      void _void_process(void *item, ThreadPool::TPHandle &handle) override {
        process(reinterpret_cast<T *>(item));
      }
      void _void_process_finish(void *item) override {
        --m_processing;
      }
      virtual void process(T *item) = 0;
      void process_finish() {
        _void_process_finish(nullptr);
      }
    private:
      ThreadPool *m_pool;
      std::list<T *> m_items;
      uint32_t m_processing;
  };
private:
  vector<WorkQueue_*> work_queues;
  int next_work_queue = 0;
  // threads
  struct WorkThread : public Thread {
    ThreadPool *pool;
    WorkThread(ThreadPool *p) : pool(p) {}
    void *entry() override {
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
  ThreadPool(string nm, string tn, int n, const char *option = NULL);
  ~ThreadPool() override;
  void start(); 
  void stop();

};

ThreadPool::ThreadPool(string nm, string tn, int n)
  : name(std::move(nm)), thread_name(std::move(tn)),_num_threads(n),processing(0)
{}

void ThreadPool::start()
{
  start_threads();
}
void ThreadPool::start_threads()
{
  while (_threads.size() < _num_threads)
  {
    WorkThread *wt = new WorkThread(this);
    _threads.insert(wt);
    wt->create(thread_name.c_str());
  }
}
void ThreadPool::worker(WorkerThread *wt) 
{
  while(!_stop)
  {
    if (!_pause && !work_queues.empty()) 
    {
      WorkQueue_* wq;
      int tries = work_queues.size();
      bool did = false;
      while (tries--)
      {
        next_work_queue %= work_queues.size();
        wq = work_queues[next_work_queue++];
        void *item = wq->_void_dequeue();
        if (item)
        {
          processing++;
          wq->_void_process(item, tp_handle);
          wq->_void_process_finish(item);
          processing--;
          did = true;
          break;
        }
      }
      if (did)
        continue;
    }
  }
}

class ContextWQ : public ThreadPool::PointerWQ<Context> {
    public:
      ContextWQ(const string &name, time_t ti, ThreadPool *tp)
        : ThreadPool::PointerWQ<Context>(name, ti, tp) {
        this->register_work_queue();
      }
      void queue(Context *ctx, int result = 0) {
        ThreadPool::PointerWQ<Context>::queue(ctx);
      }
    private:
      unordered_map<Context*, int> m_context_results;
    protected:
      void process(Context *ctx) override {
        int result = 0;
        {
          unordered_map<Context *, int>::iterator it = m_context_results.find(ctx);
          if (it != m_context_results.end()) {
            result = it->second;
            m_context_results.erase(it);
          }
        }
        ctx->complete(result);
      }
};
//-------------------------
/* 
class Finisher {
  vector<Context*> finisher_queue;

  string thread_name;

  list<pair<Context*, int> > finisher_queue_rval;

  void *finisher_thread_entry();

  struct FinisherThread : public Thread {
    Finisher *fin;
    explicit FinisherThread(Finisher *f) : fin(f) {}
    void* entry() override { return (void*)fin->finisher_thread_entry(); }
  } finisher_thread;

 public:

  void queue(Context *c, int r = 0) {
    if (r) {
      finisher_queue_rval.push_back(pair<Context*, int>(c, r));
      finisher_queue.push_back(NULL);
    } else
      finisher_queue.push_back(c);
    }

  // Start the worker thread
  void start();

  void stop();
}

void Finisher::start()
{
  finisher_thread.create(thread_name.c_str());
}

void Finisher::stop()
{
  finisher_stop = true;
  finisher_thread.join();
}

void Finisher::finisher_thread_entry()
{
  utime_t start;
  uint64_t count = 0;

  while (!finisher_stop) {
    while (!finisher_queue.empty()) {
      vector<Context*> ls;
      list<pair<Context*, int> > ls_rval;
      ls.swap(finisher_queue);
      ls_rval.swap(finisher_queue_rval);
      finisher_running = true;

      for (vector<Context*>::iterator p = ls.begin(); p != ls.end(); ++p) {
        if (*p) {
          (*p)->complete(0);
        } else {
          Context *c = ls_rval.front().first;
          c->complete(ls_rval.front().second);
          ls_rval.pop_front();
        }
      }
    }
  }
  return 0;
}
*/
//------------------------------------------

template <typename T, void (T::*MF)(int)>
class C_CallbackAdapter: public Context {
  T *obj;
public:
  C_CallbackAdapter(T *obj) : obj(obj) {
  }
protected:
  void finish(int r) override {
    (obj->*MF)(r);
  }
};


template <typename T, void(T::*MF)(int) = &T::complete>
Context *create_context_callback(T *obj) {
    return new C_CallbackAdapter<T, MF>(obj);
}

class TestCallback {

  public:
  protected:
  public:
    void start_test();
    Context *pre_set_fn();
    void handle_test_callback(int r);
  private:
};

Context *TestCallback::pre_set_fn() {
  Context *ctx = create_context_callback<
    TestCallback, &TestCallback::handle_test_callback>(this);

  return ctx;

}
void TestCallback::handle_test_callback(int r)
{
  std::cout<<"This printed by <handle_test_callback>"<<std::endl;
}
int main()
{
  TestCallback *test_callback = new TestCallback;
  Context *ctx = test_callback->pre_set_fn();
  int r = 0;
  ctx->complete(r);
//  ctx->finish(r);

  return 0;
}
