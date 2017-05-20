// 1 actor per thread implementation

// 单线程实现的actor框架


#include <deque>
#include <memory>
#include <map>

template <typename T>
class blocking_queue
{
private:
    std::deque<T>           d_queue;
public:
    void push(T const& value) {
        d_queue.push_front(value);
    }
	bool hasMsg()
	{
		return !d_queue.empty();
	}
    T pop() {
        T rc(std::move(this->d_queue.back()));
        this->d_queue.pop_back();
        return std::move(rc);
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

	friend class Framework;
public:
	Actor(Framework &);

	void send(Address other, std::string &msg);
	virtual void onMsg(QueueItem &m) = 0;
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
	void run();
};

////////////////////////////////////////////////////
Actor::Actor(Framework &frm):m_frm(frm)
{
	m_addr.addr = frm.g_nmaxId++;
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

void Framework::run()
{
	bool bHasMsg = true;
	while(bHasMsg)
	{
		bHasMsg = false;
		
		for(auto it:g_actors)
		{
			if(it.second->m_mailBox.hasMsg())
			{
				bHasMsg = true;
				
				it.second->onMsg(it.second->m_mailBox.pop());
			}
		}
	}
}
