#include "actor_coroutine.hpp"

class Node : public Actor
{
	int m_id;
	Address m_next;
public:
	Node(Framework &f, int id) :Actor(f), m_id(id)
	{
		RegisterHandler(std::bind(&Node::run, this));
	}

	void setNext(Address addr)
	{
		m_next = addr;
	}

	void run()
	{
		printf("Node %d, running\n", m_id);
		auto msg=receive();
		printf("Node %d, Receive %s\n", m_id, msg.msg.c_str());
		
		char s[100];
		sprintf(s, "%d", m_id + 1);
		send(m_next, std::string(s));

		sleep(1000);
		printf("Node %d, Sleep ok\n", m_id);
	}
};

void main() {
	Framework frm;
	
	const int N = 10;

	Node *c[N];
	
	for (size_t i = 0; i < N; i++)
	{
		c[i] = new Node(frm, i);
	}

	for (size_t i = 0; i < N-1; i++)
	{
		c[i]->setNext(c[i + 1]->addr());
	}

	frm.Send(c[N-1]->addr(), c[0]->addr(), std::string("Hello"));
	
	frm.run();
}
