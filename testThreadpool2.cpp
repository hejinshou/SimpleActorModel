#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <memory>
#include <functional>
#include <boost/foreach.hpp>
#include <boost/bind.hpp>
#include <boost\range.hpp>
#include <boost/range/irange.hpp>
#include <boost/lockfree/queue.hpp>

namespace bamthread
{
    struct ThreadPool {
		typedef std::function<void()>	job_type;

		ThreadPool(size_t threads):_q(128) {
            while(threads--)
            {
				auto worker = [this]() {this->run();};
                g.add_thread(new boost::thread(worker));
            }
        }

        void enqueue(job_type f){
			_q.push(new job_type(f));
        }

        ~ThreadPool() {
            g.join_all();
        }

        private:
			boost::lockfree::queue<job_type*>	_q;
			boost::thread_group g; //< need to keep track of threads so we can join them

		void run() {
			job_type *f = nullptr;
			while (_q.pop(f))
			{
				(*f)();
				delete f;
				f = nullptr;
			}
		}
    };
}

void main()
{
	int n_threads{ 4 };
	int starty{ 0 };
	int endy{ 3 };
	int step{ 1 };
	int startx{ 0 };
	int endx{ 3 };
	
    bamthread::ThreadPool tp(n_threads);
    for(int y: boost::irange(starty, endy, step)){
        for(int x: boost::irange(startx, endx, step)){
			tp.enqueue([=]() {
				printf("%d %d\n", x, y); 
			});
           
        }
       
    }
}
