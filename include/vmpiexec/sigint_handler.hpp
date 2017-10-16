/**
 * vmpiexec
 *
 * Copyright 2017 by ACS RWTH Aachen University
 * Simon Pickartz     <spickartz@eonerc.rwth-aachen.de>
 *
 * Licensed under GNU General Public License 2.0 or later.
 * Some rights reserved. See LICENSE
 */

#ifndef sigint_handler_hpp
#define sigint_handler_hpp

#include <mutex>
#include <condition_variable>
#include <atomic>
#include <future>
#include <functional>
#include <csignal>

/**
 * \brief This class ensures that cleanup_func is called on first sigint to shutdown gracefully.
 *
 * If sigint is received, cleanup_func is called. After that exit() is called..
 * If another sigint is received while cleanup_func is executed, exit() is called immediately.
 */
class Sigint_handler
{
public:
	/**
	 * The constructor registers a callback on sigint and creates a thread waiting on notification by the callback.
	 *
	 * On notification due to sigint, the waiting thread calls cleanup_func and exits afterwards
	 * (or immediately on a second sigint).
	 * Also, the destructor may notify without sigint received to signal termination of the waiting thread.
	 */
	Sigint_handler(std::function<void()> cleanup_func);
	/**
	 * \brief The destructor notifies the waiting thread.
	 */
	~Sigint_handler();
private:
	void register_sigint_callback();
	void thread_func();

	static void set_sigint_flag();
	static void notify_thread();
	static void wait_for_notify(std::unique_lock<std::mutex> lock = std::unique_lock<std::mutex>(notify_mutex));
	static void reset_flags();
	static void sigint_callback(int);

	bool cleaning;
	std::function<void()> cleanup_func;
	// Handle for the waiting thread
	std::future<void> thread_handle;

	// Condition variable to notify the waiting thread
	static std::condition_variable notify_cv;
	static std::mutex notify_mutex;
	static std::atomic<bool> notify_flag;
	// Signals that sigint was recived.
	static std::atomic<bool> sigint_flag;
};

#endif
