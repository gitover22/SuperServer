#include <iostream>
#include <memory>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <functional>

struct Pool{
    std::mutex mtx;
    std::condition_variable cond;
    bool isClosed;
    std::queue<std::function<void()>> tasks;
    // 构造函数初始化 isClosed 为 false
    // Pool() : isClosed(false) {}
};

int main() {
    // 创建一个 Pool 实例的 shared_ptr
    std::shared_ptr<Pool> pool_ = std::make_shared<Pool>();

    // 通过智能指针访问 Pool 的成员
    // 比如，我们可以修改 isClosed 状态
    pool_->isClosed = false; // 假设我们现在关闭 pool

    // 也可以向 tasks 队列添加任务
    pool_->tasks.push([]{ std::cout << "Hello from the task!" << std::endl; });
    pool_->tasks.push([]{ std::cout << "Hello from the task!" << std::endl; });
    pool_->tasks.push([]{ std::cout << "Hello from the task!" << std::endl; });
    pool_->tasks.push([]{ std::cout << "Hello from the task!" << std::endl; });
    pool_->tasks.push([]{ std::cout << "Hello from the task!" << std::endl; });
    pool_->tasks.push([]{ std::cout << "Hello from the task!" << std::endl; });
    pool_->tasks.push([]{ std::cout << "Hello from the task!" << std::endl; });

    // 检查并执行一个任务（简化示例，实际中需要更复杂的线程同步逻辑）
    while (!pool_->tasks.empty() && !pool_->isClosed) {
        auto task = std::move(pool_->tasks.front());
        pool_->tasks.pop();
        task(); // 执行任务
    }

    return 0;
}
