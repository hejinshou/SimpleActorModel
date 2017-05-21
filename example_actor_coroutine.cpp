#include "actor_coroutine.hpp"


class Server : public Actor
{
public:
	Server(Framework &f) :Actor(f)
	{
		RegisterHandler(std::bind(&Server::run, this));
	}

	void run()
	{
		while (1)
		{
			
			auto msg=receive();
			printf("%s\n", msg.msg.c_str());
			send(msg.sender, msg.msg);

			if (msg.msg == "EXIT")
			{
				return;
			}
		}
	}
};

class Client : public Actor
{
public:
	Client(Framework &f):Actor(f)
	{
		RegisterHandler(std::bind(&Client::run, this));
	}
private:

	void run()
	{
		while (1)
		{
			printf("Client: Wait msg\n");
			auto msg=receive();
			printf("Client: Receive %s\n", msg.msg.c_str());

			if (msg.msg == "EXIT")
			{
				return;
			}
		}
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
