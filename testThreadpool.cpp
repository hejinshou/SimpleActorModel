#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <memory>
#include <boost/foreach.hpp>
#include <boost/bind.hpp>
#include <boost\range.hpp>
#include <boost/range/irange.hpp>

//https://stackoverflow.com/questions/19500404/how-to-create-a-thread-pool-using-boost-in-c

namespace bamthread
{
    typedef std::unique_ptr<boost::asio::io_service::work> asio_worker;

    struct ThreadPool {
        ThreadPool(size_t threads) :service(), working(new asio_worker::element_type(service)) {
            while(threads--)
            {
                auto worker = boost::bind(&boost::asio::io_service::run, &(this->service));
                g.add_thread(new boost::thread(worker));
            }
        }

        template<class F>
            void enqueue(F f){
                service.post(f);
            }

        ~ThreadPool() {
            working.reset(); //allow run() to exit
            g.join_all();
            service.stop();
        }

        private:
        boost::asio::io_service service; //< the io_service we are wrapping
        asio_worker working;
        boost::thread_group g; //< need to keep track of threads so we can join them
    };
}

void main()
{
	int n_threads{ 1 };
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
