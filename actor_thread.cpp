// 1 actor per thread implementation

// Q
// 1. Actor的生命周期如何管理？
// A: Actor本身在自己的loop里面确定何时退出
// 发送消息统一使用地址，而不是Actor指针。发送消息都通过框架实现
// 2. 如何处理收不到消息的情况？
// A: receive函数允许超时

#include <mutex>
#include <condition_variable>
#include <deque>
#include <functional>
#include <thread>
#include <memory>
#include <map>

using namespace std::placeholders;  // for _1, _2, _3...

//http://stackoverflow.com/questions/12805041/c-equivalent-to-javas-blockingqueue
template <typename T>
class blocking_queue
{
private:
    std::mutex              d_mutex;
    std::condition_variable d_condition;
    std::deque<T>           d_queue;
public:
    void push(T const& value) {
        {
            std::unique_lock<std::mutex> lock(this->d_mutex);
            d_queue.push_front(value);
        }
        this->d_condition.notify_one();
    }
    T pop() {
        std::unique_lock<std::mutex> lock(this->d_mutex);
        this->d_condition.wait(lock, [=]{ return !this->d_queue.empty(); });
        T rc(std::move(this->d_queue.back()));
        this->d_queue.pop_back();
        return rc;
    }
};

struct Address
{
	int addr;

	Address():addr(0)
	{}
};

class Framework;

class Actor
{
public:
	struct QueueItem
	{
		std::string msg;
		Address sender;
	};

private:
	Framework &m_frm;
	Address	m_addr;
	blocking_queue<QueueItem> m_mailBox;

	std::function<void()> m_f;

	std::thread m_t1;
	
	friend class Framework;
public:
	Actor(Framework &);

	void RegisterHandler(std::function<void()> f)
	{
		m_f = f;
		m_t1 = std::thread(m_f);
	}
	
	void send(Address other, std::string &msg);

	QueueItem receive()
	{
		return std::move(m_mailBox.pop());
	}

	virtual ~Actor();
	Address addr() {
		return m_addr;
	}

};

class Framework {
	int g_nmaxId;
	std::map<int, Actor* > g_actors;

	friend class Actor;
public:
	Framework():g_nmaxId(0)
	{}

	void Send(Address from, Address to, std::string const &msg);
};

////////////////////////////////////////////////////
Actor::Actor(Framework &frm):m_frm(frm)
{
	m_addr.addr = frm.g_nmaxId++;
	frm.g_actors[m_addr.addr] = this;
}

Actor::~Actor()
{
	m_t1.join();
	m_frm.g_actors.erase(m_addr.addr);
}

void Actor::send(Address other, std::string &msg)
{
	m_frm.Send(m_addr, other, msg);
}

////////////////////////////////////////////////////
void Framework::Send(Address from, Address to, std::string const &msg)
{
	auto s = g_actors.find(from.addr);
	auto d = g_actors.find(to.addr);

	if (s==g_actors.end() || d == g_actors.end())
	{
		return;
	}

	Actor::QueueItem i{ msg, from };
	d->second->m_mailBox.push(i);
}

////////////////////////////////////////////////////

class Server : public Actor
{
public:

	Server(Framework &frm):Actor(frm)
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

			if (i.msg=="EXIT")
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
