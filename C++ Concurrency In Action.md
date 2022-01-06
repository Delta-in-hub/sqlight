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





---



## 线程间共享数据

### mutex

#### `std::lock_guard vs std::scoped_lock vs std::unique_lock`

The `scoped_lock` is a strictly superior version of `lock_guard` that locks an arbitrary number of mutexes all at once.

A **unique_lock can be created without immediately locking**, can unlock at any point in its existence, and can transfer ownership of the lock from one instance to another.

####  lock a  std::mutex twice from the same thread without unlocking in between, you get undefined behavior.

> https://stackoverflow.com/questions/11173532/why-is-locking-a-stdmutex-twice-undefined-behaviour

#### std::lock()

> https://stackoverflow.com/questions/18520983/is-stdlock-ill-defined-unimplementable-or-useless

All arguments are locked via a sequence of calls to `lock()`, `try_lock()`, or `unlock()` on each argument. The sequence of calls shall not result in deadlock, but is otherwise unspecifed. 

可能的实现是 back-off-and-retry 

Consider the call `std::lock (a, b, c, d, e, f)` in which `f` was the only lockable that was already locked. In the first lock attempt, that call would lock `a` through `e` then "fail" on `f`.

Following the back-off (unlocking `a` through `e`), the list to lock would be changed to `f, a, b, c, d, e` so that subsequent iterations would be less likely to unnecessarily lock. That's not fool-proof since other resources may be locked or unlocked between iterations, but it *tends* towards success.



#### 防范死锁

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



### 在初始化过程中保护共享数据

`std::once_flag` 于`std::call_once()` 配合

C++11之后,static变量初始化仅仅会在某一线程上发生.

-----

[**double-checked locking pattern** 的问题](https://stackoverflow.com/questions/945232/whats-wrong-with-this-fix-for-double-checked-locking?rq=1)

The problem is that a second thread may see instance is non-null and try to return it before the first-thread has constructed it. 



### 共享锁 shared_mutex

读锁`std::shared_lock<std::shared_mutex>` ,写锁`std::lock_guard<std::shared_mutex> ...etc` 

`std::shared_mutex_timed`

## 并发操作的同步

###  `std::condition_variable`

`std::condition_variable`仅限于`std::mutex`配合使用.

`std::condition_variable_any`可以和充当mutex的任何类型使用.

#### condition_variable::wait()

若当前线程未锁定 [lock.mutex()](https://zh.cppreference.com/w/cpp/thread/unique_lock/mutex) ，则调用此函数是未定义行为。

若 [lock.mutex()](https://zh.cppreference.com/w/cpp/thread/unique_lock/mutex) 与所有其他当前等待在同一条件变量上的线程所用的互斥不相同，则调用此函数是未定义行为。

#### 构建线程安全队列

`mutable`

1. If you have a **const** reference or pointer to an object, you cannot modify that object in any way **except** when and how it is marked `mutable`.Because,using `const_cast` to modify a part of a `const` object yields undefined behaviour. 

2. Allowing the variable to be modified by a **const function**.

3. Since c++11 `mutable` can be used on a lambda to denote that things captured by value are modifiable (they aren't by default):

```c++
int x = 0;
auto f1 = [=]() mutable {x = 42;};  // OK
auto f2 = [=]()         {x = 42;};  // Error: a by-value capture cannot be modified in a non-mutable lambda
```





```c++
#include <mutex>
#include <condition_variable>
#include <queue>
#include <memory>

template <typename T>
class threadsafe_queue
{
private:
    mutable std::mutex mut;
    std::queue<T> data_queue;
    std::condition_variable data_cond;

public:
    threadsafe_queue()
    {
    }
    threadsafe_queue(threadsafe_queue const &other)
    {
        std::lock_guard<std::mutex> lk(other.mut);
        data_queue = other.data_queue;
    }

    void push(T new_value)
    {
        std::lock_guard<std::mutex> lk(mut);
        data_queue.push(new_value);
        data_cond.notify_one();
    }

    void wait_and_pop(T &value)
    {
        std::unique_lock<std::mutex> lk(mut);
        data_cond.wait(lk, [this]
                       { return !data_queue.empty(); });
        value = data_queue.front();
        data_queue.pop();
    }

    std::shared_ptr<T> wait_and_pop()
    {
        std::unique_lock<std::mutex> lk(mut);
        data_cond.wait(lk, [this]()
                       { return !data_queue.empty(); });
        std::shared_ptr<T> res(std::make_shared<T>(data_queue.front()));
        data_queue.pop();
        return res;
    }

    bool try_pop(T &value)
    {
        std::lock_guard<std::mutex> lk(mut);
        if (data_queue.empty())
            return false;
        value = data_queue.front();
        data_queue.pop();
        return true;
    }

    std::shared_ptr<T> try_pop()
    {
        std::lock_guard<std::mutex> lk(mut);
        if (data_queue.empty())
            return std::shared_ptr<T>();
        std::shared_ptr<T> res(std::make_shared<T>(data_queue.front()));
        data_queue.pop();
        return res;
    }

    bool empty() const
    {
        std::lock_guard<std::mutex> lk(mut);
        return data_queue.empty();
    }
};
```

在`notify_one()`之前,释放掉锁是一个常见的优化.

> The notifying thread does not need to hold the lock on the same  mutex as the one held by the waiting thread(s); in fact doing so is a **pessimization**, since the notified thread would immediately block again, waiting for the notifying thread to release the lock.

`notify_one()`与`notify_all()` : `notify_one()`不存在锁竞争



### `std::future`

使用`future`一个线程依赖于另一个线程的计算结果,却不必显式的访问共享数据.



使用`future`等待一次性事件发生

创建`future`的三种方法:

1. `std::async()`

   ​	`std::launch::deferred` : 没有创建新线程,仅为惰性求值.

   ​	`std::launch::async` : 创建新线程,异步求值.

2. `std::packaged_task<>`
   它连接了`future`对象与函数,`std::packaged_task<>`执行时会调用关联的函数,并将返回值保存为`future`的内部数据,并令`future`准备就绪.
   `std::packaged_task<>` + `std::thread`就成了`std::async`
   
3. `std::promise`
   可以实现如下机制: 等待数据的线程在`future`上阻塞,而提供数据的线程的线程利用相配的`promise`的`set_value()`设定关联的值,使`future`准备就绪.

   作为1,2的底层

`std::future`也可以保存异常,在`get()`抛出保存在其中的异常.



### `std::shared_future`

`std::future`问题在于只允许一个线程等待结果,`get()`仅能被有效调用唯一一次,因其会进行移动操作.(若为 `future<T&>` 模板特化,则返回左值引用).`std::future`对象仅能被移动.



`std::shared_future::get()`默认返回const引用,但本身不保证线程安全.
若每个线程通过其自身的 `shared_future` 对象副本访问，则从多个线程访问同一共享状态是安全的。



>  So we know that calls to `global_sf.get()` in different threads are **potentially concurrent** unless you accompany them with additional synchronization (e.g. a mutex). But we also know that calls to `global_sf.get()` in different threads do **not conflict**, because it is a `const` method and hence forbidden from modifying objects accessible from multiple threads, including `*this`. So the definition of a data race (unsequenced, potentially concurrent conflicting actions) is not satisfied, the program does not contain a data race.  https://stackoverflow.com/a/41416686/11803107

```c++
void worker_thread()
{
    // The most correct way is as followed.
    auto local_sf = global_sf; // <-- unsynchronized access of global_sf here
    const bigdata *ptr = local_sf.get();     // thread safe access to shared state
}
```



### 限时等待



`_for`为后缀

`_until`为后缀



#### 时间库 `std::chrono`

**The Unix Time**

从[UTC](https://zh.wikipedia.org/wiki/協調世界時)1970年1月1日0时0分0秒起至现在的总秒数，不考虑[闰秒](https://zh.wikipedia.org/wiki/閏秒).

故`epoch`一般指1970.1.1 00:00

C++20前，并没有对这个时间点的起始做出规定，从C++20开始，这个时间被确定为Unix Time。不过在20之前，大部分编译器的实现都使用Unix时间（Unix Time）作为这个时钟源的纪元（epoch）起点，因此一般来说可以将其视为Unix时间戳进行使用（对于20版本以前能确认一下当然也是更好的）。





**`std::ratio`**

常用来表示一个tick表示多少秒,即tick period.

```c++
  template <intmax_t _Num, intmax_t _Den = 1> //即分母默认为1
  struct ratio{}
```



`ratio<x,y>`表示 $\frac{x}{y}$ .

==SI prefixes==

<img src="C:\Users\Administrator\AppData\Roaming\Typora\typora-user-images\image-20220105201924353.png" alt="image-20220105201924353" style="zoom: 80%;" />

```c++
  typedef ratio<1, 1000000000000000000> atto;
  typedef ratio<1, 1000000000000000> femto;
  typedef ratio<1, 1000000000000> pico; //ps 皮秒
  typedef ratio<1, 1000000000> nano; //ns（纳秒，十亿分之一秒）
  typedef ratio<1, 1000000> micro; //μs（微秒，百万分之一秒）
  typedef ratio<1, 1000> milli; //ms（毫秒，千分之一秒）
  typedef ratio<1, 100> centi;
  typedef ratio<1, 10> deci;
  typedef ratio<10, 1> deca;
  typedef ratio<100, 1> hecto;
  typedef ratio<1000, 1> kilo;
  typedef ratio<1000000, 1> mega;
  typedef ratio<1000000000, 1> giga;
  typedef ratio<1000000000000, 1> tera;
  typedef ratio<1000000000000000, 1> peta;
  typedef ratio<1000000000000000000, 1> exa;
```

**时长类`std::chrono::duration<>`**

模板接受两个参数,第一个表示计时单元数量的类型,第二个表示一个计时单元代表多少秒.

```c++
#define _GLIBCXX_CHRONO_INT64_T int64_t

/// nanoseconds
using nanoseconds = duration<_GLIBCXX_CHRONO_INT64_T, nano>;

/// microseconds
using microseconds = duration<_GLIBCXX_CHRONO_INT64_T, micro>;

/// milliseconds
using milliseconds = duration<_GLIBCXX_CHRONO_INT64_T, milli>;

/// seconds
using seconds = duration<_GLIBCXX_CHRONO_INT64_T>;

/// minutes
using minutes = duration<_GLIBCXX_CHRONO_INT64_T, ratio<60>>;

/// hours
using hours = duration<_GLIBCXX_CHRONO_INT64_T, ratio<3600>>;

/// days
using days = duration<_GLIBCXX_CHRONO_INT64_T, ratio<86400>>;

/// weeks
using weeks = duration<_GLIBCXX_CHRONO_INT64_T, ratio<604800>>;

/// years
using years = duration<_GLIBCXX_CHRONO_INT64_T, ratio<31556952>>;

/// months
using months = duration<_GLIBCXX_CHRONO_INT64_T, ratio<2629746>>;
```

时间段显式转化`std::duration_case<>()`



**时间点`std::chrono::time_point<>`**

模板第一个参数指明参考的时钟,第二个参数表明计时单元(`std::chrono::duration<>`) 

时间点是一个时间跨度,开始于epoch,终止于当前时间点.跨度的值表明某具体时长的倍数.

```c++
template<
    class Clock,
    class Duration = typename Clock::duration
>class time_point;

```

`time_point_cast<>()`进行转换

**时钟类**

时钟提供几类关键信息:

1. 是否为steady clock(计时速率恒定且无法调整)
   `system_clock`不是恒稳时钟,它可以被调整,两次调用`now()`可能后者返回值早于前一个.
2. 计时单元的长度( tick period )
3. 时间值的类型
4. 获取当前时间

```c++
struct system_clock
{
    typedef chrono::nanoseconds duration;  //计时单元为 nanoseconds
    typedef duration::rep rep;
    typedef duration::period period;
    typedef chrono::time_point<system_clock, duration> time_point; //时间值的类型
    static constexpr bool is_steady = false;  //是否为stead_clock
    //...
}
```

成员函数`from_time_t` `to_time_t`将时钟的时间点类型与`time_t`类型相互转化.



> https://zhuanlan.zhihu.com/p/414702182

时间的定义:

1. 人们对时间最直观的感受就是太阳的东升西落，这就引出了世界时（UT1）简单理解这个就是基于地球自转所定义的时间。即基于天文观测确定的世界时.
2. 原子时（TAI），以铯原子跃迁周期作为基准定义时间长度.
   *问题就在于地球自转的周期实际上并没有那么稳定，如果使用  TAI，就会出现太阳在12点零几秒才到达了当天的最高点，对于日常使用实际是非常不便的。这种误差还会随着时间推移不断积累。*
3. 协调世界时，即UTC时间。UTC时间以原子时作为计时基础，同时通过添加或减少闰秒（leap second）的方式与UT进行同步。我们日常使用的时间，实际就是UTC时间。

chrono库中`utc_clock`提供的就是UTC时间(not supported by gcc)，而`system_clock`提供的时间并不包含闰秒。因此在打印秒数的时候，二者会有一定的差距.



### 同步操作简化代码

#### 使用`future`进行函数式编程

串行/并行的快速排序

并行版本反而更慢了.

```c++
#include <bits/stdc++.h>

using namespace std;

template <typename T>
list<T> sequential_quick_sort(list<T> input)
{
    if (input.empty())
        return input;
    list<T> result;
    result.splice(result.begin(), input, input.begin());
    auto &&pivot = result.front();
    auto divide_point = partition(input.begin(), input.end(), [&pivot](const T &t)
                                  { return t < pivot; });
    list<T> lower_part;
    lower_part.splice(lower_part.end(), input, input.begin(), divide_point);
    auto new_lower = sequential_quick_sort(move(lower_part));
    auto new_higher = sequential_quick_sort(move(input));
    result.splice(result.begin(), new_lower);
    result.splice(result.end(), new_higher);
    return result;
}

template <typename T>
list<T> parallel_quick_sort(list<T> input)
{
    if (input.empty())
        return input;
    list<T> result;
    result.splice(result.begin(), input, input.begin());
    auto &&pivot = result.front();
    auto divide_point = partition(input.begin(), input.end(), [&pivot](const T &t)
                                  { return t < pivot; });
    list<T> lower_part;
    lower_part.splice(lower_part.end(), input, input.begin(), divide_point);

    auto new_lower = sequential_quick_sort(move(lower_part));
    //auto new_higher = sequential_quick_sort(move(input));
    auto new_higher = async(&parallel_quick_sort<T>, move(input));
    result.splice(result.begin(), new_lower);
    result.splice(result.end(), new_higher.get());
    return result;
}

signed main(void)
{
    srand(time(0));
    list<int> l;
    for (int i = 0; i < 200000; i++)
    {
        l.emplace_back(rand());
    }
    auto start = chrono::high_resolution_clock::now();
    auto r = sequential_quick_sort(l);
    auto end = chrono::high_resolution_clock::now();
    cout << (end - start).count() << endl;
    int pre = -1;
    for (auto &&i : r)
    {
        assert(pre <= i);
        pre = i;
    }

    start = chrono::high_resolution_clock::now();
    r = parallel_quick_sort(l);
    end = chrono::high_resolution_clock::now();
    cout << (end - start).count() << endl;
    pre = -1;
    for (auto &&i : r)
    {
        assert(pre <= i);
        pre = i;
    }
    return 0;
}
```



## 内存模型与原子操作



