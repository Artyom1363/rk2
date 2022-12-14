#include <iostream>
#include <atomic>
#include <thread>
#include <semaphore>
#include <condition_variable>

std::counting_semaphore<1> first_is_run(0);

class PingPong
{
public:
	static constexpr std::size_t MAX = 3;

	void ping()
	{
    	std::unique_lock<std::mutex> lock(m_);
        first_is_run.release();
        // std::cout << "ping" << std::endl;
    	while (count_ < MAX)
    	{
        	std::cout << "Ping" << std::endl;
        	count_++;
            wait = true;
        	cv_.notify_all();
            while (wait) {
        	    cv_.wait(lock);
                wait = false;
            }
    	}
        // std::cout << "ping out" << std::endl;
 	}

	void pong()
	{
        first_is_run.acquire();
    	std::unique_lock<std::mutex> lock(m_);
        // std::cout << "pong" << std::endl;
        // std::cout << count_ << std::endl;
    	while (count_ < MAX)
    	{
        	std::cout << "Pong" << std::endl;
        	count_++;
            wait = true;
        	cv_.notify_all();
            while (wait) {
                cv_.wait(lock);
                wait = false;
            }
    	}
        // std::cout << "pong out" << std::endl;
	}

private:
    std::atomic<bool> wait = false;; 
	std::atomic<std::size_t> count_ = 0;
	std::mutex m_;
	std::condition_variable cv_;
};

int main()
{
	PingPong p;
	std::thread pingThread(&PingPong::ping, &p);
    // first_is_run.release();
	std::thread pongThread(&PingPong::pong, &p);

	pingThread.join();
	pongThread.join();
}
