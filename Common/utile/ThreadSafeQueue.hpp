#ifndef COMMON_UTILE_THREADSAFEQUEUE_HPP
#define COMMON_UTILE_THREADSAFEQUEUE_HPP

#include <mutex>
#include <thread>
#include <queue>
#include <utility>
#include <string>
#include <vector>
#include <functional>
#include <chrono>
#include <optional>

#include "Logger.hpp"

namespace common
{
    namespace utile
    {
        template<typename T>
        class ThreadSafeQueue
        {
        protected:
            std::mutex mutexQueue_;
            std::queue<T> queue_;
            LOGGER("TSQ");
        public:
            ThreadSafeQueue() = default;
            ThreadSafeQueue(const ThreadSafeQueue&) = delete;
            virtual ~ThreadSafeQueue() { clear(); }

            std::optional<T> pop()
            {
                if (queue_.empty())
                {
                    LOG_WARN << "Tried to extract data from queue when it was empty";
                    return {};
                }
                std::scoped_lock lock(mutexQueue_);
                auto t = std::move(queue_.front());
                queue_.pop();
                return t;
            }

            void push(const T& operation)
            {
                std::scoped_lock lock(mutexQueue_);
                queue_.push(std::move(operation));
            }

            const bool empty()
            {
                std::scoped_lock lock(mutexQueue_);
                return queue_.empty();
            }

            const size_t size()
            {
                std::scoped_lock lock(mutexQueue_);
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
#endif // #COMMON_UTILE_THREADSAFEQUEUE_HPP