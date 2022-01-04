# C++ Concurrency In Action

> https://www.bookstack.cn/read/CPP-Concurrency-In-Action-2ed-2019/README.md

每个线程都需要一个独立的堆栈空间

## 什么时候不使用并发

知道何时**不使用**并发与知道何时**使用**它一样重要。基本上，不使用并发的唯一原因就是收益比不上成本。使用并发的代码在很多情况下难以理解，因此编写和维护的多线程代码就会直接产生脑力成本，同时额外的复杂性也可能引起更多的错误。除非潜在的性能增益足够大或关注点分离地足够清晰，能抵消所需的额外的开发时间以及与维护多线程代码相关的额外成本(代码正确的前提下)；否则，勿用并发。

## 什么时候使用并发

既然你已经看到了这里，那无论是为了性能、关注点分离，亦或是因为*多线程星期一*(multithreading Monday)(译者：可能是学习多线程的意思)，你应该确定要在应用中使用并发了。

## 锁

保护共享数据结构的最基本的方式，是使用C++标准库提供的互斥量。

### Let's Talk Locks!

> https://www.youtube.com/watch?v=7OpCf6f_BAM
>
> https://www.infoq.com/presentations/go-locks/

#### Build a lock!

```c++
int flag = 0;

// Thread1 / Thread2
while(1)
{
    if(not flag)
    {
        flag++;
        doSomeThing();
        flag--;
        return;
    }
}
```

1. `flag++` is non-atomic. One thread change it , but before it write to memory, the old value was read by another thread.
2. memory access reordering



The **atomic CAS(Compare and Swap)** is the quintessence of any lock implementation.

### C++ Memory Order 与 Atomic

> https://zhuanlan.zhihu.com/p/31386431



### `std::call_once` `static`
在C++11标准中：初始化及定义完全在一个线程中发生，并且没有其他线程可在初始化完成前对其进行处理，条件竞争终止于初始化阶段，这样比在之后再去处理好的多。



## 线程间共享数据 (mutex)

### `std::lock_guard vs std::scoped_lock vs std::unique_lock`

The `scoped_lock` is a strictly superior version of `lock_guard` that locks an arbitrary number of mutexes all at once.

A **unique_lock can be created without immediately locking**, can unlock at any point in its existence, and can transfer ownership of the lock from one instance to another.

###  lock a  std::mutex twice from the same thread without unlocking in between, you get undefined behavior.

> https://stackoverflow.com/questions/11173532/why-is-locking-a-stdmutex-twice-undefined-behaviour

### std::lock()

> https://stackoverflow.com/questions/18520983/is-stdlock-ill-defined-unimplementable-or-useless

All arguments are locked via a sequence of calls to `lock()`, `try_lock()`, or `unlock()` on each argument. The sequence of calls shall not result in deadlock, but is otherwise unspecifed. 

可能的实现是 back-off-and-retry 

Consider the call `std::lock (a, b, c, d, e, f)` in which `f` was the only lockable that was already locked. In the first lock attempt, that call would lock `a` through `e` then "fail" on `f`.

Following the back-off (unlocking `a` through `e`), the list to lock would be changed to `f, a, b, c, d, e` so that subsequent iterations would be less likely to unnecessarily lock. That's not fool-proof since other resources may be locked or unlocked between iterations, but it *tends* towards success.



### 防范死锁

1. 每个线程最好只持有一个锁，

2. 若需要多个锁，使用`std::lock()`一步操作全部获取，

3. 若无法一步操作全部获取，则依从固定顺序获取锁。

4. 按照层级加锁.
   在某层级的mutex上加锁时,之后只能由相对低层级的metux获取锁.  p56.

   ```c++
   #include <mutex>
   #include <stdexcept>
   #include <climits>
   class hierarchical_mutex
   {
       std::mutex internal_mutex;
       unsigned long const hierarchy_value; //constant after construction
       unsigned long previous_hierarchy_value;
       static thread_local unsigned long this_thread_hierarchy_value; //当前线程最后一次加锁的层级值,初始化为MAX
   
       void check_for_hierarchy_violation()
       {
           if(this_thread_hierarchy_value <= hierarchy_value)
           {
               throw std::logic_error("mutex hierarchy violated");
           }
       }
       void update_hierarchy_value()
       {
           previous_hierarchy_value=this_thread_hierarchy_value;
           this_thread_hierarchy_value=hierarchy_value;
       }
   public:
       explicit hierarchical_mutex(unsigned long value):
           hierarchy_value(value),
           previous_hierarchy_value(0)
       {}
       void lock()
       {
           check_for_hierarchy_violation();
           internal_mutex.lock();
           update_hierarchy_value();
       }
       void unlock()
       {
           this_thread_hierarchy_value=previous_hierarchy_value;
           internal_mutex.unlock();
       }
       bool try_lock()
       {
           check_for_hierarchy_violation();
           if(!internal_mutex.try_lock())
               return false;
           update_hierarchy_value();
           return true;
       }
   };
   thread_local unsigned long
       hierarchical_mutex::this_thread_hierarchy_value(ULONG_MAX);       
   
   int main()
   {
       hierarchical_mutex m1(42);
       hierarchical_mutex m2(2000);
       
   }
   ```

   **thread_local**

   > https://murphypei.github.io/blog/2020/02/thread-local

   在同一线程内,thread_local 变量表现的就和静态变量一样，但是不同线程中是不同的.**`static thread_local` 和 `thread_local` 声明是等价的**.

   与`static`区别,`static`所有线程共享一个,`thread_local`每个线程内共享同一个,不同线程不同.

   作为类成员时,必须显式为`static thread_local`,且类外初始化时,`thread_local` 以外的存储类说明符都不能[显式特化](https://zh.cppreference.com/w/cpp/language/template_specialization)及[显式实例化](https://zh.cppreference.com/w/cpp/language/class_template#.E6.98.BE.E5.BC.8F.E5.AE.9E.E4.BE.8B.E5.8C.96)中使用.

   `thread_local`变量若未被使用,则无法保证被构造,且他们的析构顺序与构造顺序相反,但无法确定具体顺序.

   `thread_local`变量地址因线程而异,可以对其进行取址,在另一线程中操作.

5. 任何同步机制导致循环等待都会导致死锁出现



## 线程间共享数据 (其他方式)







