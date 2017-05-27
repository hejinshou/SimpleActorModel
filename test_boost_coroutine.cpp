/*����coroutine�����
Main: 0
Main: 1
Main: 2
waitDataThread: 3
Main: 4
Main: 5

����������pull���͵�coroutine��create��ʱ�򲢲���ִ�У�ֻ���ڵ�һ��push��ʱ��Ż�ִ��
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
	for(int j=0; j<10; j++)
	{
		printf("waitDataThread: %d\n", i++);
		
		push();
	}
}

void test2(){
	printf("Main: %d\n", i++);
	pull_coro_t  *m_pull = new pull_coro_t(waitDataThread2);
	printf("Main: %d\n", i++);
	for (size_t j = 0; j < 10; j++)
	{
		printf("Main: %d\n", i++);
		(*m_pull)();
		printf("Main: %d\n", i++);
	}
}

void main(){
	test2();
}
