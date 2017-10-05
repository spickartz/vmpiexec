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
	cleanup_func(cleanup_func),
	thread_handle(std::async(std::launch::async, &Sigint_handler::thread_func, this))
{
	register_sigint_callback();
}


// TODO: Reregister old callback function
Sigint_handler::~Sigint_handler()
{
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
	std::unique_lock<std::mutex> lock(notify_mutex);
	notify_cv.wait(lock, []{return notify_flag.load();});
	if (sigint_flag.load()) {
		FASTLIB_LOG(sigint_handler_log, info) << "sigint received. Calling cleanup_func...";
		cleanup_func();
		FASTLIB_LOG(sigint_handler_log, info) << "Everything cleaned up. Calling exit...";
		exit(return_status);
	}
}

void Sigint_handler::sigint_received()
{
	sigint_flag.store(true);
}

void Sigint_handler::notify_thread()
{
	notify_flag.store(true);
	notify_cv.notify_all();
}

void Sigint_handler::sigint_callback(int) {
	sigint_received();
	notify_thread();
}
