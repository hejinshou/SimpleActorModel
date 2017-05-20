#include "actor_singlethread.hpp"

//Note: 这个实现的不好的地方是，ctor如果主动地receive，可能会block主线程
//而如果被动的只是实现一个onmsg，就不方便实现顺序的逻辑，比如先发消息，再收消息之类的
//解决方案实现receive的做法
//一种是如果actor没有消息，则开一个额外的线程等消息，防止block主线程
//另一种是使用coroutine
////////////////////////////////////////////////////

class Server : public Actor
{
public:
	Server(Framework &f) :Actor(f)
	{
	}
private:

	virtual void onMsg(QueueItem &msg)
	{
		printf("%s\n", msg.msg.c_str());
		send(msg.sender, msg.msg);

		if (msg.msg == "EXIT")
		{
			return;
		}
	}
};

class Client : public Actor
{
public:
	Client(Framework &f):Actor(f)
	{
	}
private:

	virtual void onMsg(QueueItem &msg)
	{
		printf("Client: Receive %s\n", msg.msg.c_str());
	}
};



void main() {
	Framework frm;
	
	Client c(frm);
	Server s(frm);

	frm.Send(c.addr(), s.addr(), std::string("Hello"));
	frm.Send(c.addr(), s.addr(), std::string("EXIT"));
	
	frm.run();
}
