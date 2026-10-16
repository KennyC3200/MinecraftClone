#pragma once

#include <cstddef>
#include <condition_variable>
#include <mutex>
#include <thread>
#include <queue>
#include <vector>
#include <functional>
#include <future>

namespace mcc {

class ThreadPool final {
public:
	explicit ThreadPool(std::size_t n);
	~ThreadPool();

	template <class FunctionType>
	auto Submit (FunctionType f) -> std::future<decltype(f())>;

private:
	void WorkerLoop();

	bool m_stopping = false;

	// Queue of tasks
	std::queue<std::function<void(void)>> m_tasks;
	std::mutex m_mutex;
	std::condition_variable m_cv;

	// Needs to be instantiated LAST so it is destroyed FIRST
	// This is because in the WorkerLoop, for threads to clean up, the mutex gets locked repeatedly
	// So the mutex and cv need to be destroyed AFTER the worker threads have been destroyed
	std::vector<std::thread> m_workers;
};


template <class FunctionType>
auto ThreadPool::Submit (FunctionType f) -> std::future<decltype(f())> {
	using ReturnType = decltype(f());

	// Need type ReturnType() in packaged_task because we call (*task)() later down the line
	auto task = std::make_shared<std::packaged_task<ReturnType()>>(std::move(f));
	std::future<ReturnType> result = task->get_future();

	// Lock the mutex so we can push to the queue
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		m_tasks.push([task] { (*task)(); });
	}

	m_cv.notify_one();

	return result;
}

}