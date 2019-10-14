#ifndef TREAD_POOL_THREAD_SAFE_QUEUE_H
#define TREAD_POOL_THREAD_SAFE_QUEUE_H

#include <queue>
#include <mutex>
#include <condition_variable>

template<typename T>
class thread_safe_queue
    {
private:
    mutable std::mutex mut;
    std::queue<std::shared_ptr<T>> data_queue;
    std::condition_variable data_cond;
public:
    thread_safe_queue () = default;

    void push (T new_value)
    {
        std::shared_ptr<T> data(std::make_shared<T>(std::move(new_value)));
        std::lock_guard<std::mutex> lk(mut);
        data_queue.push(data);
        data_cond.notify_one();
    }


    void double_push (T first, T second)
    {
        std::shared_ptr<T> f(std::make_shared<T>(std::move(first)));
        std::shared_ptr<T> s(std::make_shared<T>(std::move(second)));
        std::lock_guard<std::mutex> lk(mut);
        data_queue.push(f);
        data_queue.push(s);
        data_cond.notify_one();
    }


    void wait_and_pop (T &value)
    {
        std::unique_lock<std::mutex> lk(mut);
        data_cond.wait(lk, [this] { return !data_queue.empty(); });
        value = std::move(*data_queue.front());
        data_queue.pop();
    }


    std::shared_ptr<T> wait_and_pop ()
    {
        std::unique_lock<std::mutex> lk(mut);
        data_cond.wait(lk, [this] { return !data_queue.empty(); });
        std::shared_ptr<T> res = data_queue.front();
        data_queue.pop();
        return res;
    }


    bool try_pop (T &value)
    {
        std::lock_guard<std::mutex> lk(mut);
        if (data_queue.empty())
            return false;
        value = std::move(*data_queue.front());
        data_queue.pop();
        return true;
    }


    std::shared_ptr<T> try_pop ()
    {
        std::lock_guard<std::mutex> lk(mut);
        if (data_queue.empty())
            return std::shared_ptr<T>();
        std::shared_ptr<T> res = data_queue.front();
        data_queue.pop();
        return res;
    }


    std::pair<T, T> double_pop ()
    {
        std::unique_lock<std::mutex> lk(mut);
        data_cond.wait(lk, [this] { return data_queue.size() >= 2; });
        auto first = std::move(*data_queue.front());
        data_queue.pop();
        auto second = std::move(*data_queue.front());
        data_queue.pop();

        return std::make_pair(first, second);
    }


    unsigned size () const
    {
        std::lock_guard<std::mutex> lk(mut);
        return data_queue.size();
    }


    bool empty () const
    {
        std::lock_guard<std::mutex> lk(mut);
        return data_queue.empty();
    }

    T get_result ()
    {
        std::unique_lock<std::mutex> lk(mut);
        data_cond.wait(lk, [this] () { return data_queue.size() == 2; });
        auto res = double_pop();
        return (res.first.empty()) ? res.second : res.first;
    }

    };


#endif //TREAD_POOL_THREAD_SAFE_QUEUE_H