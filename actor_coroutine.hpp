#include <deque>
#include <functional>
#include <memory>
#include <map>
#include <boost/coroutine/all.hpp>

typedef boost::coroutines::coroutine< void >::pull_type pull_coro_t;
typedef boost::coroutines::coroutine< void >::push_type push_coro_t;

using namespace std::placeholders;  // for _1, _2, _3...

template <typename T>
class blocking_queue
{
private:
    std::deque<T>           d_queue;
public:

	blocking_queue()
	{
	}
	
	void push(T const& value) {
		d_queue.push_front(value);
	}

    bool empty()
	{
		return d_queue.empty();
	}
	
    T pop() {
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

struct QueueItem
{
	std::string msg;
	Address sender;
};

class Actor
{
public:
	
private:
	Framework &m_frm;
	Address	m_addr;
	blocking_queue<QueueItem> m_mailBox;
	push_coro_t *m_push;
	pull_coro_t *ppull;
	std::function<void()> m_f;
	bool	m_isWaitMsg;
	
	void waitDataThread(pull_coro_t &pull) {
		ppull = &pull;

		if (m_f)
			m_f();
	}

	friend class Framework;
public:
	Actor(Framework &);

	void RegisterHandler(std::function<void()> f)
	{
		m_f = f;
		m_push = new push_coro_t(std::bind(&Actor::waitDataThread, this, _1));
	}
	
	void send(Address other, std::string &msg);

	QueueItem receive();

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
	
	bool receive(Address from, QueueItem &i);
	void run();
};

////////////////////////////////////////////////////
Actor::Actor(Framework &frm)
	:m_frm(frm)
	, m_push(0)
	, ppull (0)
{
	m_addr.addr = frm.g_nmaxId++;
	m_isWaitMsg = true;
	frm.g_actors[m_addr.addr] = this;
}

Actor::~Actor()
{
	m_frm.g_actors.erase(m_addr.addr);
}

void Actor::send(Address other, std::string &msg)
{
	m_frm.Send(m_addr, other, msg);
}

QueueItem Actor::receive()
{
	m_isWaitMsg = true;
	QueueItem i;
	while (m_mailBox.empty())
	{
		assert(ppull);
		(*ppull)();
	}
	i = m_mailBox.pop();
	m_isWaitMsg = false;
	return std::move(i);
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

	QueueItem i{ msg, from };
	d->second->m_mailBox.push(i);
}

bool Framework::receive(Address from, QueueItem &i)
{
	auto s = g_actors.find(from.addr);
	
	if (s==g_actors.end())
	{
		return false;
	}
	
	i = std::move(s->second->receive());
	return true;
}

void Framework::run()
{
	bool bHasMsg = true;
	while (bHasMsg)
	{
		bHasMsg = false;

		for (auto it : g_actors)
		{
			if (it.second->m_isWaitMsg)
			{
				bHasMsg = true;

				if((*it.second->m_push))
					(*it.second->m_push)();
			}
		}
	}
}
