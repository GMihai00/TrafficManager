#ifndef COMMON_UTILE_THREADSAFEPRIORITYQUEUE_HPP
#define COMMON_UTILE_THREADSAFEPRIORITYQUEUE_HPP

#include <mutex>
#include <thread>
#include <queue>
#include <utility>
#include <string>
#include <vector>
#include <functional>
#include <chrono>
#include <optional>
#include <shared_mutex>

#include "Logger.hpp"

namespace common
{
    namespace utile
    {
        template<typename T>
        struct Compare
        {
            bool operator() (std::pair<T, bool>& operation1, std::pair<T, bool>& operation2)
            {
                if(operation1.second == false && operation2.second == true)
                {
                    return true;   
                }
                return false;
            }
        };

        template<typename T>
        class ThreadSafePriorityQueue
        {
        protected:
            std::shared_mutex mutexQueue_;
            std::priority_queue<std::pair<T, bool>, std::vector<std::pair<T, bool>>, Compare<T>> queue_;
            LOGGER("TSPQ");
        public:
            ThreadSafePriorityQueue() = default;
            ThreadSafePriorityQueue(const ThreadSafePriorityQueue&) = delete;
            virtual ~ThreadSafePriorityQueue() { clear(); }
    
            std::optional<std::pair<T, bool>> pop()
            {
                if (queue_.empty())
                {
                    LOG_WARN << "Tried to extract data from queue when it was empty";
                    return {};
                }
                std::scoped_lock lock(mutexQueue_);
                auto t = std::move(queue_.top());
                queue_.pop();
                return t;
            }
    
            void push(const std::pair<T, bool>& operation)
            {
                std::scoped_lock lock(mutexQueue_);
                queue_.push(std::move(operation));
            }
    
            const bool empty()
            {
                std::shared_lock lock(mutexQueue_);
                return queue_.empty();
            }
    
            const size_t size()
            {
                std::shared_lock lock(mutexQueue_);
                return queue_.size();
            }
    
            void clear()
            {
                std::scoped_lock lock(mutexQueue_);
                while (!queue_.empty())
                {
                    queue_.pop();
                }
            }
        };
    } // namespace utile
} // namespace common
#endif // #COMMON_UTILE_THREADSAFEPRIORITYQUEUE_HPP
