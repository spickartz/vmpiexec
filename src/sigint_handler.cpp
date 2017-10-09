#include "vmpiexec/sigint_handler.hpp"

#include <fast-lib/log.hpp>

#include <cstdlib>

// inititalize fast-lib log
FASTLIB_LOG_INIT(sigint_handler_log, "sigint_handler")
FASTLIB_LOG_SET_LEVEL_GLOBAL(sigint_handler_log, trace);

// Static member vars
std::atomic<bool> Sigint_handler::notify_flag(false);
std::atomic<bool> Sigint_handler::sigint_flag(false);
std::condition_variable Sigint_handler::notify_cv;
std::mutex Sigint_handler::notify_mutex;

Sigint_handler::Sigint_handler(std::function<void()> cleanup_func) :
	return_status(EXIT_FAILURE),
	cleaning(false),
	cleanup_func(cleanup_func),
	thread_handle(std::async(std::launch::async, &Sigint_handler::thread_func, this))
{
	register_sigint_callback();
}


// TODO: Reregister old callback function
Sigint_handler::~Sigint_handler()
{
	if (cleaning)
		wait_for_notify();
	// No sigint received. Just notify the thread to quit.
	notify_thread();
}

// TODO: Save old callback function
void Sigint_handler::register_sigint_callback()
{
	struct sigaction sa = {};
	sa.sa_handler = sigint_callback;
	sigfillset(&sa.sa_mask);
	sigaction(SIGINT, &sa, nullptr);
}

void Sigint_handler::thread_func()
{
	wait_for_notify();
	// Notification due to sigint?
	if (sigint_flag.load()) {
		cleaning = true;
		FASTLIB_LOG(sigint_handler_log, info) << "sigint received.";
		reset_flags();
		FASTLIB_LOG(sigint_handler_log, info) << "Calling cleanup_func...";
		std::unique_lock<std::mutex> lock(notify_mutex);
		auto cleanup_handle = std::async(std::launch::async, [this]{
			{
				std::lock_guard<std::mutex> lock(notify_mutex);
			}
			cleanup_func();
			notify_thread();
		});
		FASTLIB_LOG(sigint_handler_log, info) << "Waiting for cleanup_func to finish. You can press ctrl+c again to force termination.";
		wait_for_notify(std::move(lock));
		if (sigint_flag.load())
			FASTLIB_LOG(sigint_handler_log, info) << "Forced termination. Calling exit...";
		else
			FASTLIB_LOG(sigint_handler_log, info) << "Everything cleaned up. Calling exit...";
		exit(return_status);
	}
}

void Sigint_handler::set_sigint_flag()
{
	sigint_flag.store(true);
}

void Sigint_handler::notify_thread()
{
	notify_flag.store(true);
	notify_cv.notify_all();
}

void Sigint_handler::wait_for_notify(std::unique_lock<std::mutex> lock)
{
	notify_cv.wait(lock, []{return notify_flag.load();});
}

void Sigint_handler::reset_flags()
{
	notify_flag.store(false);
	sigint_flag.store(false);
}

void Sigint_handler::sigint_callback(int) {
	set_sigint_flag();
	notify_thread();
}
