// WhiteCore is a C++ chess engine
// Copyright (c) 2023 Balázs Szilágyi
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
//

#include <vector>
#include <queue>
#include <functional>
#include <atomic>
#include <condition_variable>
#include <thread>

class ThreadPool {
public:

    ThreadPool();
    ~ThreadPool();

    void stop_workers();
    void allocate_threads(unsigned int num_threads);

    template<typename F>
    void enqueue(F f);

    void wait();

    unsigned int get_thread_count();

private:
    std::vector<std::thread> workers;
    std::queue<std::function<void()>> tasks;
    std::mutex task_mutex;
    std::atomic<unsigned int> active_tasks;
    std::condition_variable tasks_finished;
    std::condition_variable cond_var;
    std::atomic<bool> stop;
};

ThreadPool::ThreadPool() : active_tasks(0), stop(false) {}

ThreadPool::~ThreadPool() {
    stop_workers();
}

void ThreadPool::stop_workers() {
    stop = true;
    cond_var.notify_all();
    for (std::thread &worker : workers) {
        if (worker.joinable())
            worker.join();
    }
    workers.clear();
}

void ThreadPool::allocate_threads(unsigned int num_threads) {
    stop = false;
    for (unsigned int i = 0; i < num_threads; i++) {
        workers.emplace_back([this]{
            while (true) {
                std::function<void()> task;
                {
                    std::unique_lock<std::mutex> lock(task_mutex);
                    cond_var.wait(lock, [this]{
                        return this->stop.load() || !this->tasks.empty();
                    });

                    if(this->stop.load() && this->tasks.empty())
                        return;

                    task = std::move(this->tasks.front());
                    this->tasks.pop();
                }
                active_tasks++;
                task();
                active_tasks--;
                tasks_finished.notify_all();
            }
        });
    }
}

template<typename F>
void ThreadPool::enqueue(F f) {
    {
        std::unique_lock<std::mutex> lock(task_mutex);
        tasks.emplace(f);
    }
    cond_var.notify_one();
}

void ThreadPool::wait() {
    std::unique_lock<std::mutex> lock(task_mutex);
    tasks_finished.wait(lock, [this] { return this->active_tasks == 0 && this->tasks.empty(); });
}

unsigned int ThreadPool::get_thread_count() {
    return workers.size();
}
