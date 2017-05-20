#include "actor_singlethread.hpp"

//Note: ���ʵ�ֵĲ��õĵط��ǣ�ctor���������receive�����ܻ�block���߳�
//�����������ֻ��ʵ��һ��onmsg���Ͳ�����ʵ��˳����߼��������ȷ���Ϣ��������Ϣ֮���
//�������ʵ��receive������
//һ�������actorû����Ϣ����һ��������̵߳���Ϣ����ֹblock���߳�
//��һ����ʹ��coroutine
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
