// 1 actor per thread implementation

// Q
// 1. Actor������������ι���
// A: Actor�������Լ���loop����ȷ����ʱ�˳�
// ������Ϣͳһʹ�õ�ַ��������Actorָ�롣������Ϣ��ͨ�����ʵ��
// 2. ��δ����ղ�����Ϣ�������
// A: receive��������ʱ

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
