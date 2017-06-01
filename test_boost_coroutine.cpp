/*测试coroutine，结果
Main: 0
Main: 1
Main: 2
waitDataThread: 3
Main: 4
Main: 5

分析：对于pull类型的coroutine (即函数参数为pull_coro_t&)，create的时候并不会执行，只有在第一次push的时候才会执行
*/

#include <boost/coroutine/all.hpp>

typedef boost::coroutines::coroutine< void >::pull_type pull_coro_t;
typedef boost::coroutines::coroutine< void >::push_type push_coro_t;

int i=0;

void waitDataThread(pull_coro_t &pull)
{
	for(int j=0; j<10; j++)
	{
		printf("waitDataThread: %d\n", i++);
		
		pull();
	}
}

void test1(){
	printf("Main: %d\n", i++);
	push_coro_t  *m_push = new push_coro_t(waitDataThread);
	printf("Main: %d\n", i++);
	for (size_t j = 0; j < 10; j++)
	{
		printf("Main: %d\n", i++);
		(*m_push)();
		printf("Main: %d\n", i++);
	}
}

void waitDataThread2(push_coro_t &push)
{
	for(int j=0; j<3; j++)
	{
		printf("waitDataThread: %d\n", i++);
		
		push();
	}
}

/*
Main: 0
waitDataThread: 1
Main: 2
Main: 3
waitDataThread: 4
Main: 5
Main: 6
waitDataThread: 7
Main: 8
Main: 9
Main: 10
对于push类型的coroutine，在创建的时候coroutine会马上运行，直到第一个push语句才会暂停该coroutine，返回caller
*/
void test2(){
	printf("Main: %d\n", i++);
	pull_coro_t  *m_pull = new pull_coro_t(waitDataThread2);
	printf("Main: %d\n", i++);
	for (size_t j = 0; j < 3; j++)
	{
		printf("Main: %d\n", i++);
		(*m_pull)();
		printf("Main: %d\n", i++);
	}
}

void main(){
	test2();
}
