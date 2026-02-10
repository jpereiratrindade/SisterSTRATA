#pragma once

#include <queue>
#include <functional>
#include <mutex>

namespace Infrastructure::Threading {

class CommandQueue {
public:
    using Command = std::function<void()>;

    void push(Command cmd) {
        std::lock_guard<std::mutex> lock(mutex_);
        queue_.push(std::move(cmd));
    }

    [[nodiscard]] bool hasPending() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return !queue_.empty();
    }

    void processAll() {
        std::queue<Command> localQueue;
        {
            std::lock_guard<std::mutex> lock(mutex_);
            if (queue_.empty()) return;
            std::swap(localQueue, queue_);
        }

        while (!localQueue.empty()) {
            localQueue.front()();
            localQueue.pop();
        }
    }

    void clear() {
        std::lock_guard<std::mutex> lock(mutex_);
        std::queue<Command> empty;
        std::swap(queue_, empty);
    }

private:
    std::queue<Command> queue_;
    mutable std::mutex mutex_;
};

} // namespace Infrastructure::Threading
