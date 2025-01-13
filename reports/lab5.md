## 问答作业
1. 在我们的多线程实现中，当主线程 (即 0 号线程) 退出时，视为整个进程退出， 此时需要结束该进程管理的所有线程并回收其资源。

    需要回收的资源有哪些？
    主线程的所有子线程+他们的内核资源，其中有：
    分配的页表
    lock、semaphore与条件变量
    堆内存和局部存储
    开着的文件
    ...

    其他线程的 struct thread 可能在哪些位置被引用，分别是否需要回收，为什么？
    当时，其他线程的struct thread可能被引用在lock或 sem或条件变量等待队列中。本实验实现的调度算法进队runnable线程进行调度，因此不回收也不会造成影响，可以不用回收。

2. 对比以下两种 mutex_unlock 中阻塞锁的实现，二者有什么区别？这些区别可能会导致什么问题？ （假设无论哪种实现，对应的 mutex_lock 均正确处理了 m->locked）

```
 1void mutex_unlock_v1(struct mutex *m)
 2{
 3   if (m->blocking) {
 4      m->locked = 0;
 5      struct thread *t = id_to_task(pop_queue(&m->wait_queue));
 6      if (t != NULL) {
 7         t->state = RUNNABLE;
 8         add_task(t);
 9      }
10   } else ...
11}
12
13void mutex_unlock_v2(struct mutex *m)
14{
15   if (m->blocking) {
16      struct thread *t = id_to_task(pop_queue(&m->wait_queue));
17      if (t == NULL) {
18         m->locked = 0;
19      } else {
20         t->state = RUNNABLE;
21         add_task(t);
22      }
23   } else ...
24}
```
第一种实现方法会先释放出来锁，然后进行dequeue。
第二种实现方法先dequeue一个线程，检查队列是否有线程：若没有那么释放锁，若有那么把锁给刚刚选择的线程来执行它的任务。那么主要区别在于他们所实现的队列类型：第一种实现方法可以作为一个抢占式的队列，而第二种方法是一个FIFO队列。这样，第一种实现方法可能会导致抢占锁的问题。