#include "actor_coroutine.hpp"

class Node;

struct task_id{
	Node *node;
	
	task_id():node(0);
};

class Node : public Actor
{
	std::function<void(void)> m_callback;
	int m_nInputs;
	std::vector<task_id> m_outputs;
public:
	Node(Framework &f, std::function<void(void)> callback, std::vector<task_id> const & depends):Actor(f), m_callback(callback), m_nInputs(0)
	{
		m_nInputs = depends.size();
		for(auto i in depends)
		{
			i.node->m_outputs.push_back(i);
		}
		
		RegisterHandler(std::bind(&Server::run, this));
	}

	void run()
	{
		for (int i = 0; i < m_nInputs; i++)
		{
			receive();
		}
		m_callback();
		for(auto i in m_outputs)
		{
			send(i.addr, "");
		}
	}
};

task_id creatTask(Framework &frm, std::function<void(void)> f, std::vector<task_id> const & depends)
{
	task_id i;
	i.node = new Node(frm, f,depends);
	return i;
}

void main() {
	Framework frm;
	
	const int N = 1;

	task_id c[N];
	
	for (size_t i = 0; i < N; i++)
	{
		c[i] = creatTask(frm, [i](){printf("Task %d\n", i);}, i>0?);
	}

	frm.run();
}
