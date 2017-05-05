#include <mutex>
#include <condition_variable>
#include <deque>

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

class Actor;

typedef blocking_queue MailBox;

class ActorSystem;

class Actor
{
	MailBox m_mb;
	ActorSystem &m_as;
public:
	Actor(ActorSystem &as):m_as(as)
	{}
	
	void send(Actor *other, std::string &msg);

	void receive(Actor **other, std::string *msg);
	
	virtual void run()=0;
	virtual ~Actor()=0{}
};

class ActorImpl:public Actor
{
};

class ThreadPool
{
public:
	ThreadPool(int n);
	void push(std::function<void>);
};

class ActorSystem
{
	ThreadPool &m_tp;
	std::vector<std::shared_ptr<Actor>> m_actors;
	std::vector<std::shared_ptr<Actor>> m_busy_actors;
public:
	ActorSystem(ThreadPool &tp):m_tp(tp)
	{}
	
	std::shared_ptr<Actor> spawn(std::function<void(Actor*)> f)
	{
		std::shared_ptr<Actor> act(new ActorImpl(f));
		m_actors.push_back(act);
		return act;
	}
	
};

void Actor::send(std::shared_ptr<Actor> other, std::string &msg)
{
	other->m_mb.push(std::make_pair(other, msg));
	m_busy_actors.push_back(
}

void Actor::receive(Actor **other, std::string *msg)
{
	auto t(m_mb.pop());
	*other = t.first;
	*msg = t.second;
}

void main()
{
	ActorSystem as;
	std::shared_ptr<Actor> p1 = as.spawn([](Actor *This){
		std::string msg;
		Actor *other =0£»
		while(1)
		{
			This->receive(&other, &msg);
			if(msg=="exit")
			{
				std::cout << "p1 Quiting";
				return;
			}
			for(int i=0; i<msg.length();i++)
				msg[i]+=1;
			This->send(other, msg);
		}
	});

	actor_spawn([p1](Actor *This){
		This->send(p1, "Hello");

		std::string msg;
		Actor *other =0£»
		This->receive(&other, &msg);
		std::cout << msg;

		This->send(p1, "exit");
		std::cout << "p2 Quiting";
	});
}
