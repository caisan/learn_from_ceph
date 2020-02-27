#include <iostream>
#include "ThreadPool.h"
#include <unistd.h> 
#include <boost/function.hpp>
using namespace std;

class Context {
  Context(const Context &other);
  const Context& operator=(const Context& other);

protected:
  virtual void finish(int r) = 0;
public:
  Context() {}
  virtual void complete(int r) {
    finish(r);
    delete this;
  }
};

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

class ContextWQ : public ThreadPool::WorkQueue<Context> {
public:
    ContextWQ(const string &name, ThreadPool *tp) : ThreadPool::WorkQueue<Context>(name, tp)
    {
      tp->add_work_queue(this);
    }
    void queue(Context *ctx)
    {
       ThreadPool::WorkQueue<Context>::queue(ctx);
    }
    void process(Context *ctx) {
      int r;
      ctx->complete(r);
    }
    void _clear() {};
    bool _empty() {};
    bool _enqueue(Context *item) {};
    //void _dequeue(Context *);
    Context *_dequeue() 
    {
       if (m_items.empty()) {return NULL;}
       Context *item = m_items.front();
       m_items.pop_front();
       return item;
    };
    void _dequeue(Context *) {};
   void _process_finish(Context *) {};
   void _process(Context *t) 
   { 
     process(t);
   };
};
class FunctionContext : public Context {
public:
  FunctionContext(boost::function<void(int)> &&callback)
    : m_callback(std::move(callback))
  {
  }

  void finish(int r) override {
    m_callback(r);
  }
private:
  boost::function<void(int)> m_callback;
};
int main()
{
        ThreadPool *thread_pool = nullptr;
        ContextWQ *work_queue = nullptr;
	thread_pool = new ThreadPool("test", 2);
        thread_pool->start();
	TestCallback *test_callback = new TestCallback;
        Context *ctx = test_callback->pre_set_fn();

        FunctionContext *fc_ctx = new FunctionContext([](int r) {
          std::cout<<"this is printed by FunctionContext callback"<<std::endl;
        });

        work_queue = new ContextWQ("contextWQ", thread_pool);
        /*Choose Context *ctx or FunctionContext *fc_ctx*/
        work_queue->queue(fc_ctx);
//        thread_pool->stop();
        sleep(10);
        return 0;
}
