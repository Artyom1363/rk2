#include <vector>
#include <functional>
#include <fstream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>

#include <chrono>
#include <iostream>
#include <string>
#include <sstream>
#include <queue>

using namespace std;


void solve();

template <typename T>
class threadsafe_queue
{
    std::queue<T> _data_queue;

public:
    threadsafe_queue() {}

    threadsafe_queue(threadsafe_queue const& other) {
        _data_queue = other._data_queue;
    }

    void push(T new_value) {
        _data_queue.push(std::move(new_value));
    }

    void wait_and_pop(T& value) {
        value = std::move(_data_queue.front());
        
        _data_queue.pop();
    }

    bool try_pop(T& value) {
        if (_data_queue.empty())
            return false;
    
        value = std::move(_data_queue.front());
        _data_queue.pop();
        return true;
    }

    bool empty() const {
        return _data_queue.empty();
    }

    size_t size() const {
        return _data_queue.size();
    }
};



threadsafe_queue<std::function<void()>> work_queue;

class thread_pool{
    int                      thread_quantity;
    std::atomic_bool         done;
    std::condition_variable  cond;
    std::mutex               cond_mutex;
    std::vector<std::thread> threads;
    std::string              filename;

    void worker_thread() {
        while(!done) {
            if (work_queue.empty()) {
                std::unique_lock<std::mutex> lk(cond_mutex);
                cond.wait(lk, [this]{ return (!work_queue.empty()) || done; });
            }

            std::function<void()> task;
            if(work_queue.try_pop(task)) {
                task();
            }
                

        }
    }

public:
    thread_pool(int thread_quantity, string file_name) : 
        thread_quantity(thread_quantity), 
        done(false),
        filename(file_name) {}

    ~thread_pool() {
        done = true;
        cond.notify_one();
    }

    void start() {
        if (threads.empty()) {
            try {
                for(int i=0; i < thread_quantity; ++i) {
                    threads.push_back(std::thread(&thread_pool::worker_thread,this));
                }
                ifstream fin;
                fin.open(filename);
                std::string str;
                while (std::getline(fin, str)) {
                    work_queue.push(std::function<void()>(solve));
                }
            }
            catch(...) {
                done = true;
                cond.notify_all();
                throw;
            }
        }

        cond.notify_all();
    }

};

void solve() {

}

int main(int argc, char *argv[]) {
    string file_name = argv[1];
    int k = atoi(argv[2]);
    thread_pool pool(k, file_name);
    // ifstream fin;
    // fin.open(file_name);

    // cout << file_name << endl << k << endl;
}