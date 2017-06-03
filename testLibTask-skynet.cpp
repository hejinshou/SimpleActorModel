#include "libtask.hpp"
#include <vector>

typedef Channel<std::uint64_t>	channel_type;

int nTask = 0;

void skynet(Task *t, channel_type c, std::size_t num, std::size_t size, std::size_t div) {
	if (1 == size) {
		c.push(num);
	}
	else {
		channel_type rc;
		for (std::size_t i = 0; i < div; ++i) {
			auto sub_num = num + i * size / div;
			taskCreate([=](Task *t){
				nTask++;
				skynet(t, rc, sub_num, size / div, div);
			});
		}
		
		std::uint64_t sum{ 0 };
		for (std::size_t i = 0; i < div; ++i) {
			sum += rc.pop(t);
		}
		c.push(sum);
	}
}

void taskMain(Task *t) {
	channel_type rc;
	std::size_t size{ 100000 };
	std::size_t div{ 10 };

	nTask++;

	taskCreate([=](Task *t) {
		nTask++;
		skynet(t, rc, 0, size, div);
	});
	
	std::uint64_t result{ 0 };
	result = rc.pop(t);
	printf("%lld\n", result);
	printf("nTask %d\n", nTask);
	system("pause");
}

void t1() {
	taskCreate(taskMain);

	taskRun();
}

void main() {
	t1();
}
