# learn_from_ceph
some code snippets from ceph

### 1. simulation_crush_algorithm.cc

这是从Ceph中提取出来的crush straw算法.
对增加/减少OSD的情景进行了模拟.

hash函数,表查找:http://burtleburtle.net/bob/hash/evahash.html

### 2. ThreadPool.cc/ThreadPool.h

工作队列和线程池
将任务推入工作队列,而线程中的线程负责从工作队列中取出任务进行处理.    
工作队列和线程池的关系:    
工作队列中保存任务,线程池从工作队列中领取任务并完成.正是因为有了任务,才需要雇佣线程来完成任务.
没有了任务,线程也就失去了意义.两者紧密相关,也正是因为关系密切,在ceph中,两者的代码都位于WorkQueue.cc和WorkQueue.h,    
此处是将ceph线程池以及工作队列的代码提取出来    
```tp_test.cc```是线程池的试验测试代码

To build tp_test.cc:
```g++ ThreadPool.cc ThreadPool_test.cc -o tp_test -lpthread -std=c++11```

With boost::function (install boost lib firstly)
```g++ ThreadPool.cc ThreadPool_test.cc -o tp_test -I /usr/local/include -L /usr/local/lib -lpthread -std=c++11```

### 3. callback_class_function.cc

一个demo用于研究ceph中的回调机制。
