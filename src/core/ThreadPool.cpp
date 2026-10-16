#include "core/ThreadPool.hpp"
#include <cassert>

namespace mcc {

ThreadPool::ThreadPool(std::size_t n) {
	assert(n > 0 && "ThreadPool::ThreadPool | Number of threads must be at least 1");

	for (std::size_t t = 0; t < n; t++) {
		m_workers.emplace_back([this] { WorkerLoop(); });
	}
}

ThreadPool::~ThreadPool() {
	{
		std::lock_guard<std::mutex> lock(m_mutex);

		// Clear the queue before setting m_stopping = true, preventing pending tasks from
		// stalling the game on shutdown, since all pending tasks have to be first completed
		// This is a move assignment, which frees the underlying queue, hence no memory leak
		m_tasks = std::queue<std::function<void(void)>>();

		m_stopping = true;
	}

	// Unblock all threads currently waiting on the condition variable
	m_cv.notify_all();
	for (auto& worker : m_workers) {
		worker.join();
	}
}

void ThreadPool::WorkerLoop() {
	while (true) {
		std::function<void(void)> job;
		{
			std::unique_lock<std::mutex> lock(m_mutex);

			// Wait until either the thread pool is stopping or there is a task
			m_cv.wait(lock, [this] { return m_stopping || !m_tasks.empty(); });

			// If thread pool is stopping
			// Queue is cleared beforehand, so no need to finish the tasks
			if (m_stopping) return;

			// Assign the job
			job = std::move(m_tasks.front());
			m_tasks.pop();
		}

		// Call the job after we free the mutex to allow other threads to pick up the tasks
		job();
	}
}

}
