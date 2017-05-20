// 1 actor per thread implementation

// Q
// 1. Actor的生命周期如何管理？
// A: Actor本身在自己的loop里面确定何时退出
// 发送消息统一使用地址，而不是Actor指针。发送消息都通过框架实现
// 2. 如何处理收不到消息的情况？
// A: receive函数允许超时

#include "actor_thread.hpp"


////////////////////////////////////////////////////

class Server : public Actor
{
public:

	Server(Framework &frm) :Actor(frm)
	{
		RegisterHandler(std::bind(&Server::Print, this));
	}

private:

	// Handler for messages of type std::string. 
	void Print()
	{
		while (true)
		{
			QueueItem i = receive();
			printf("%s\n", i.msg.c_str());
			send(i.sender, i.msg);

			if (i.msg == "EXIT")
			{
				return;
			}
		}
	}
};

class Client : public Actor
{
public:

	Client(Framework &frm) :Actor(frm)
	{
		RegisterHandler(std::bind(&Client::run, this));
	}

	void sendAndWait(Address to, std::string s)
	{
		printf("Client: Send %s\n", s.c_str());
		send(to, std::string(s));
		auto i = receive();
		printf("Client: Receive %s\n", i.msg.c_str());
	}
private:

	// Handler for messages of type std::string. 
	void run()
	{
	}
};



void main() {
	Framework frm;
	
	Client c(frm);
	Server s(frm);

	c.sendAndWait(s.addr(), std::string("Hello"));
	c.sendAndWait(s.addr(), std::string("EXIT"));
}
