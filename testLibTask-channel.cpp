#include "libtask.hpp"

void taskMain(Task *t) {
	Channel<int> c;
	for (int i = 0; i < 5; i++) {
		taskCreate([=](Task *t) {
			int n = c.pop(t);
			printf("t%d %d\n", i, n);
			t->yield();
			printf("t%d\n", i);
		});
	}
	
	for (int i = 0; i < 5; i++)
		c.push(100*i);
	printf("XXXXXX\n");
}
void t1() {
	taskCreate(taskMain);
	taskRun();
}

void main() {
	t1();
}
