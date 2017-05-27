#include "actor_coroutine.hpp"

class Node : public Actor
{
public:
	Node(Framework &f) :Actor(f)
	{
		RegisterHandler(std::bind(&Node::run, this));
	}

	void run()
	{
		for (int i = 0; i < 2; i++)
		{
			printf("Node %d i %d\n", addr().addr, i);
			yield();
		}
		
	}
};

void main() {
	Framework frm;
	
	const int N = 1;

	Node *c[N];
	
	for (size_t i = 0; i < N; i++)
	{
		c[i] = new Node(frm);
	}

	frm.run();
}
