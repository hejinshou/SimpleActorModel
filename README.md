# SimpleActorModel
A simple actor model implementation

里面有几个例子

- example_actor_singlethread.cpp	单线程actor框架
- example_actor_thread.cpp		每个actor一个线程
- example_lift.cpp				电梯调度程序，使用每个actor一个线程
- example_actor_coroutine.cpp		使用coroutine做单线程调度的actor框架
- example_actor_coroutine_ring.cpp 环形队列测试
- actor_threadpool.cpp	基于线程池的actor框架
- example_actor_coroutine_yield.cpp	yield测试
- example_lift.cpp	电梯调度例子（注：未做详细测试）
- example_task.cpp	利用actor model实现的task框架
- testLibTask-channel.cpp	task框架channel测试，该框架支持coroutine+基于work sharing的多线程调度
- testLibTask-channel2.cpp task框架channel测试
- testLibTask-skynet.cpp	boost::fiber里的skynet测试
- testLibTask-sleep.cpp	sleep测试
