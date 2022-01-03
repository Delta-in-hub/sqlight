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

<img src="/home/delta/.config/Typora/typora-user-images/image-20211225142036348.png" alt="image-20211225142036348" style="zoom:50%;" />



The **atomic CAS(Compare and Swap)** is the quintessence of any lock implementation.

### C++ Memory Order 与 Atomic

> https://zhuanlan.zhihu.com/p/31386431







### `std::call_once` `static`
在C++11标准中：初始化及定义完全在一个线程中发生，并且没有其他线程可在初始化完成前对其进行处理，条件竞争终止于初始化阶段，这样比在之后再去处理好的多。
