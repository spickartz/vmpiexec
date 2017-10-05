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
#include <csignal>
#include <functional>

/**
 * \brief This class ensures that deconstructors are called on sigint to shutdown gracefully.
 */
class Sigint_handler
{
public:
	/**
	 * The constructor creates a waiting thread and registers a callback on sigint.
	 *
	 * The callback on sigint sets the sigint_flag and notifies the waiting thread.
	 * Also, the destructor may notify the waiting thread.
	 * On notification the waiting thread checks the sigint_flag.
	 * If it is set, the cleanup_func is called and the program terminated, else the thread quits.
	 */
	Sigint_handler(std::function<void()> cleanup_func);
	/**
	 * \brief The destructor notifies the waiting thread.
	 */
	~Sigint_handler();
private:
	void register_sigint_callback();
	void thread_func();

	static void sigint_received();
	static void notify_thread();
	static void sigint_callback(int);

	int return_status;
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
