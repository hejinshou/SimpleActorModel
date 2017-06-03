#include "libtask.hpp"

void taskMain(Task *t) {
	t->sleep(10000);
	printf("XXXXXX\n");
}

void t1() {
	taskCreate(taskMain);
	taskRun();
}

void main() {
	t1();
}
