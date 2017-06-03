#include <deque>
#include <functional>
#include <memory>
#include <boost/coroutine/all.hpp>
//#include <boost/any.hpp>

typedef boost::coroutines::coroutine< void >::pull_type pull_coro_t;
typedef boost::coroutines::coroutine< void >::push_type push_coro_t;

using namespace std::placeholders;  // for _1, _2, _3...
/*
每个task就绪时，放在就绪队列末尾。 
准备运行时，从就绪队列取出。 
yield时，放在就绪队列末尾。 
等待channel时，如果channel为空，则将自己放在channel的等待队列末尾

注意：一个task可能在的几个地方
就绪队列，某个channel的等待队列，什么队列都不在
这几个地方同时只能选其一
*/
struct Task: std::enable_shared_from_this<Task> {

	std::shared_ptr<Task> getptr() {
		return shared_from_this();
	}

	std::shared_ptr<push_coro_t> _push;
	pull_coro_t *_pull;

	Task() :_pull(0) {}

	void yield();
};

template <class T>
struct ChannelImpl {
	std::deque<std::shared_ptr<Task>> _waitingQueue;	//Tasks waiting this ChannelImpls
	std::deque<T> _data;

	T pop(Task *currTsk);
	void push(T const&);
};

template <class T>
struct Channel {
	std::shared_ptr<ChannelImpl<T>> impl;

	Channel():impl(std::make_shared<ChannelImpl<T>>()){}

	T pop(Task *currTsk=0) const {
		return impl->pop(currTsk);
	}
	void push(T const& t) const {
		impl->push(t);
	}
};

//std::deque<std::shared_ptr<ChannelImpl<T>>> _channels;
std::deque<std::shared_ptr<Task>>	_readyQueue;
std::shared_ptr<Task>	_activeTask;
std::size_t	_nCreatedTasks = 0;

/**Task**/
void Task::yield() {
	for (auto it: _readyQueue)
	{
		if (it.get()==this)
		{
			assert(0);
		}
	}
	assert(this == _activeTask.get());
	_readyQueue.push_back(getptr());
	(*_pull)();
}

/**ChannelImpl**/
template <class T>
T ChannelImpl<T>::pop(Task *currTsk) {
	if (currTsk)
	{
		assert(currTsk == _activeTask.get());
	}
	else {
		currTsk = _activeTask.get();
	}

	while (_data.empty())
	{
		currTsk->yield();
	}
	
	T t(std::move(_data.front()));
	_data.pop_front();
	return std::move(t);
}

template <class T>
void ChannelImpl<T>::push(T const& t) {
	_data.push_back(t);

	bool bFinish = false;
	while (!_waitingQueue.empty() && !bFinish)
	{
		auto waittsk = _waitingQueue.front();
		if (waittsk) {
			_readyQueue.push_back(waittsk);
			bFinish = true;
		}
		_waitingQueue.pop_front();
	}
}

void taskFn(pull_coro_t &pull, std::function<void(Task*)> f, Task *task) {
	task->_pull = &pull;

	pull();

	assert(task == _activeTask.get());
	f(task);
}

void taskCreate(std::function<void(Task*)> f){
	_nCreatedTasks++;
	std::shared_ptr<Task> task(std::make_shared<Task>());
	
	assert(task == task->getptr());

	task->_push = std::make_shared<push_coro_t>(std::bind(taskFn, _1, f, task.get()));
	(*task->_push)();	// Trigger the setup of task*

	_readyQueue.push_back(task);
}

void taskCreate2(std::function<void()> f) {
	std::function<void(Task* t)> f2 = [=](Task* t) {
		assert(t == _activeTask.get());
		f();
	};

	taskCreate(f2);
}

void taskYield() {
	_activeTask->yield();
}

/*bug
没有处理等待的channel
*/
void  taskRun() {
	
	int nLoop = 0;
	while (!_readyQueue.empty()) {
		if(nLoop++%10==0)
			printf("\rnCreatedTasks %zd readyQueueSize %zd", _nCreatedTasks, _readyQueue.size());
		auto task = _readyQueue.front();
		_readyQueue.pop_front();
		
		_activeTask = task;
		(*task->_push)();
	}
}
