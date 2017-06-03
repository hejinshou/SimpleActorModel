#include "libtask.hpp"

void taskMain() {
	Channel<int> c;
	for (int i = 0; i < 5; i++) {
		taskCreate2([=]() {
			int n = c.pop();
			printf("t%d %d\n", i, n);
			taskYield();
			printf("t%d\n", i);
		});
	}
	
	for (int i = 0; i < 5; i++)
		c.push(100*i);
	printf("s\n");
}
void t1() {
	taskCreate2(taskMain);
	taskRun();
}

void main() {
	t1();
}
