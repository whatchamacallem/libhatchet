#pragma once
// SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
// SPDX-License-Identifier: MIT
// This file is licensed under the MIT license found in the LICENSE.md file.

/// \file
/// C++ wrappers for POSIX pthreads or C11 `threads.h`. Provides
/// `hxthread_local`, `hxmutex`, `hxunique_lock`, `hxcondition_variable`, and
/// `hxthread`. For atomics consider `<stdatomic.h>`.

#include "libhatchet.h"

// HX_USE_MACROS_WITH_MODULE allows including macros alongside the module.
#if HX_USE_MACROS_WITH_MODULE
#error Header does not provide macros alone.
#endif
#if (HX_USE_THREADS) != 0 && (HX_USE_THREADS) != 1 && (HX_USE_THREADS) != 11
#error HX_USE_THREADS must be 0, 1 or 11. 11 is for using <threads.h>.
#endif

#if (HX_USE_THREADS) == 11
#include <threads.h>
#elif HX_USE_THREADS
#include <pthread.h>
#endif

#include "hxutility.h"

HX_NS_BEGIN_

/// `hxthread_local` - Provides thread-local storage for an integer or a
/// pointer. The default value must be zero or null. This class is available for
/// compatibility when threading is off. The design has been simplified to avoid
/// heap allocations and callbacks into the memory manager after the main thread
/// exits.
template<typename T_>
class hxthread_local {
	static_assert(sizeof(T_) <= sizeof(void*), "hxthread_local: sizeof(T) must be <= sizeof(void*)");

public:
	/// Construct to 0 or null.
	explicit hxthread_local(void) {
#if (HX_USE_THREADS) == 11
		const int code_ = ::tss_create(&m_key_, 0);
		hxassert_always(code_ == thrd_success, "tss_create %d", code_); (void)code_;
#elif HX_USE_THREADS
		const int code_ = ::pthread_key_create(&m_key_, 0);
		hxassert_always(code_ == 0, "pthread_key_create %s", ::strerror(code_)); (void)code_;
#else
		m_value_ = T_();
#endif
	}

	/// Frees resources.
	~hxthread_local() {
#if (HX_USE_THREADS) == 11
		::tss_delete(m_key_);
#elif HX_USE_THREADS
		::pthread_key_delete(m_key_);
#endif
	}

	/// Returns the thread-local value.
	operator T_(void) {
#if (HX_USE_THREADS) == 11
		return (T_)(intptr_t)::tss_get(m_key_); // NOLINT(google-readability-casting)
#elif HX_USE_THREADS
		return (T_)(intptr_t)::pthread_getspecific(m_key_); // NOLINT(google-readability-casting)
#else
		return m_value_;
#endif
	}

	/// Sets the thread-local value. This is a form of "mutable when const."
	void operator=(T_ local_) {
#if (HX_USE_THREADS) == 11
		const int code_ = ::tss_set(m_key_, (void*)(intptr_t)local_); // NOLINT(google-readability-casting)
		hxassert_hard(code_ == thrd_success, "tss_set %d", code_); (void)code_;
#elif HX_USE_THREADS
		const int code_ = ::pthread_setspecific(m_key_, (void*)(intptr_t)local_); // NOLINT(google-readability-casting)
		hxassert_hard(code_ == 0, "pthread_setspecific %s", ::strerror(code_)); (void)code_;
#else
		m_value_ = local_;
#endif
	}

private:
	explicit hxthread_local(const hxthread_local&) = delete;
	hxthread_local& operator=(const hxthread_local&) = delete;

	// No allocation is being made with which to return a pointer to.
	void operator&(void) const = delete;
	void operator&(void) = delete;

#if (HX_USE_THREADS) == 11
	::tss_t m_key_;
#elif HX_USE_THREADS
	::pthread_key_t m_key_;
#else
	T_ m_value_;
#endif
};

/// `hxthread_id` - Returns the current thread ID. Returns `0` when threads are
/// disabled. This is used by the profiler and so it tries to be efficient.
inline size_t hxthread_id(void) {
#if (HX_USE_THREADS) == 11
#if defined(_WIN32)
	return static_cast<size_t>(::thrd_current()._Tid);
#else
	return static_cast<size_t>(::thrd_current());
#endif
#elif HX_USE_THREADS
	return static_cast<size_t>(::pthread_self());
#else
	return 0; // Single threaded.
#endif
}

// The remaining classes are only available when threading is enabled. Emulating
// pthreads is a little too nutty because it has a range of valid implementations.
#if HX_USE_THREADS

/// `hxmutex` - `std::mutex` style wrapper for the configured threading backend.
/// Default behavior is non-recursive and no translation layer.
class hxmutex {
public:
	/// Constructs a mutex and initializes it. May not return if the mutex can't
	/// be initialized correctly. Something is very wrong if this fails.
	inline hxmutex(void) {
#if (HX_USE_THREADS) == 11
		const int code_ = ::mtx_init(&m_mutex_, mtx_plain);
		hxassert_always(code_ == thrd_success, "mtx_init %d", code_); (void)code_;
#else
		const int code_ = ::pthread_mutex_init(&m_mutex_, 0);
		hxassert_always(code_ == 0, "pthread_mutex_init %s", ::strerror(code_)); (void)code_;
#endif
	}

	/// Destroys the mutex.
	~hxmutex(void) {
#if (HX_USE_THREADS) == 11
		::mtx_destroy(&m_mutex_);
#else
		const int code_ = ::pthread_mutex_destroy(&m_mutex_);
		hxassertmsg(code_ == 0, "pthread_mutex_destroy %s", ::strerror(code_)); (void)code_;
#endif
	}

	/// Locks the mutex. Returns true on success, asserts on invalid arguments,
	/// and returns false on failure. Callers must check the return value and
	/// avoid ignoring lock failures.
	hxattr_nodiscard bool lock(void) {
#if (HX_USE_THREADS) == 11
		const int code_ = ::mtx_lock(&m_mutex_);
		hxassertmsg(code_ == thrd_success, "mtx_lock %d", code_);
		return code_ == thrd_success;
#else
		const int code_ = ::pthread_mutex_lock(&m_mutex_);
		hxassertmsg(code_ == 0 || code_ == EAGAIN, "pthread_mutex_lock %s", ::strerror(code_));
		return code_ == 0;
#endif
	}

	/// Unlocks the mutex. Returns true on success and false otherwise. It is
	/// undefined to unlock a mutex that you have not locked, and such an
	/// operation may succeed.
	bool unlock(void) {
#if (HX_USE_THREADS) == 11
		const int code_ = ::mtx_unlock(&m_mutex_);
		hxassertmsg(code_ == thrd_success, "mtx_unlock %d", code_);
		return code_ == thrd_success;
#else
		const int code_ = ::pthread_mutex_unlock(&m_mutex_);
		hxassertmsg(code_ == 0, "pthread_mutex_unlock %s", ::strerror(code_));
		return code_ == 0;
#endif
	}

	/// Returns a pointer to the native mutex handle.
#if (HX_USE_THREADS) == 11
	::mtx_t* native_handle(void) { return &m_mutex_; }
#else
	::pthread_mutex_t* native_handle(void) { return &m_mutex_; }
#endif

private:
	hxmutex(const hxmutex&) = delete;
	hxmutex& operator=(const hxmutex&) = delete;

#if (HX_USE_THREADS) == 11
	::mtx_t m_mutex_;
#else
	::pthread_mutex_t m_mutex_;
#endif
};

/// `hxunique_lock` - `std::unique_lock` style RAII-style unique lock for `hxmutex`.
/// Locks the mutex on construction and unlocks on destruction.
class hxunique_lock {
public:
	/// Constructs with an option to defer locking.
	/// - `defer_lock` : If true, does not lock the mutex immediately.
	hxunique_lock(hxmutex& mtx_, bool defer_lock_=false)
			: m_mutex_(mtx_), m_owns_(false) {
		if(!defer_lock_) {
			this->lock();
		}
	}
	/// Unlocks the mutex if owned.
	~hxunique_lock(void) {
		if(m_owns_) {
			this->unlock();
		}
	}
	/// Locks the mutex if not already locked.
	void lock(void) {
		if(!m_owns_) {
			m_owns_ = m_mutex_.lock();
		}
	}
	/// Unlocks the mutex if owned.
	void unlock(void) {
		if(m_owns_) {
			m_mutex_.unlock();
			m_owns_ = false;
		}
	}

	/// Returns true if the lock owns the mutex.
	bool owns_lock(void) const { return m_owns_; }

	/// Returns a reference to the associated mutex.
	hxmutex& mutex(void) { return m_mutex_; }

private:
	hxunique_lock(const hxunique_lock&) = delete;
	hxunique_lock& operator=(const hxunique_lock&) = delete;
	hxmutex& m_mutex_;
	bool m_owns_;
};

/// `hxcondition_variable` - `std::condition_variable` style condition variable
/// wrapper for the configured thread backend. Allows threads to wait for
/// notifications.
class hxcondition_variable {
public:
	/// Constructs and initializes the condition variable.
	hxcondition_variable(void) {
#if (HX_USE_THREADS) == 11
		const int code_ = ::cnd_init(&m_cond_);
		hxassert_always(code_ == thrd_success, "cnd_init %d", code_); (void)code_;
#else
		const int code_ = ::pthread_cond_init(&m_cond_, 0);
		hxassert_always(code_ == 0, "pthread_cond_init %s", ::strerror(code_)); (void)code_;
#endif
	}

	/// Destroys the condition variable if valid.
	~hxcondition_variable(void) {
#if (HX_USE_THREADS) == 11
		::cnd_destroy(&m_cond_);
#else
		const int code_ = ::pthread_cond_destroy(&m_cond_);
		hxassertmsg(code_ == 0, "pthread_cond_destroy %s", ::strerror(code_)); (void)code_;
#endif
	}

	/// Waits for the condition variable to be notified. Returns true on success,
	/// false otherwise. Callers must check the return value to confirm the wait
	/// succeeded.
	/// - `mutex` : The mutex to use for waiting.
	hxattr_nodiscard bool wait(hxmutex& mutex_) {
#if (HX_USE_THREADS) == 11
		const int code_ = ::cnd_wait(&m_cond_, mutex_.native_handle());
		hxassertmsg(code_ == thrd_success, "cnd_wait %d", code_);
		return code_ == thrd_success;
#else
		const int code_ = ::pthread_cond_wait(&m_cond_, mutex_.native_handle());
		hxassertmsg(code_ == 0, "pthread_cond_wait %s", ::strerror(code_));
		return code_ == 0;
#endif
	}

	/// Overload: Waits using a `hxunique_lock`. Returns true on success, false
	/// otherwise. Callers must check the return value to confirm the wait
	/// succeeded.
	/// - `lock` : The unique lock to use for waiting.
	hxattr_nodiscard bool wait(hxunique_lock& lock_) {
		return this->wait(lock_.mutex());
	}

	/// Waits until the predicate returns true.
	/// - `lock` : The unique lock to use for waiting.
	/// - `callable` : Predicate function to check.
	template<typename callable_t_>
	void wait(hxunique_lock& lock_, callable_t_&& callable_) {
		while(!callable_()) {
			// Failure is undefined as per the standard.
			const bool wait_result_ = this->wait(lock_);
			hxassertmsg(wait_result_, "wait"); (void)wait_result_;
		}
	}

	/// Notifies one waiting thread. Returns true on success, false otherwise.
	bool notify_one(void) {
#if (HX_USE_THREADS) == 11
		const int code_ = ::cnd_signal(&m_cond_);
		hxassertmsg(code_ == thrd_success, "cnd_signal %d", code_);
		return code_ == thrd_success;
#else
		const int code_ = ::pthread_cond_signal(&m_cond_);
		hxassertmsg(code_ == 0, "pthread_cond_signal %s", ::strerror(code_));
		return code_ == 0;
#endif
	}

	/// Notifies all waiting threads. Returns true on success, false otherwise.
	bool notify_all(void) {
#if (HX_USE_THREADS) == 11
		const int code_ = ::cnd_broadcast(&m_cond_);
		hxassertmsg(code_ == thrd_success, "cnd_broadcast %d", code_);
		return code_ == thrd_success;
#else
		const int code_ = ::pthread_cond_broadcast(&m_cond_);
		hxassertmsg(code_ == 0, "pthread_cond_broadcast %s", ::strerror(code_));
		return code_ == 0;
#endif
	}

	/// Returns a pointer to the native condition variable handle.
#if (HX_USE_THREADS) == 11
	::cnd_t* native_handle(void) { return &m_cond_; }
#else
	::pthread_cond_t* native_handle(void) { return &m_cond_; }
#endif

private:
	hxcondition_variable(const hxcondition_variable&) = delete;
	hxcondition_variable& operator=(const hxcondition_variable&) = delete;

#if (HX_USE_THREADS) == 11
	::cnd_t m_cond_;
#else
	::pthread_cond_t m_cond_;
#endif
};

/// `hxthread` - `std::thread` style thread wrapper for the configured backend.
/// Provides thread creation and joining.
class hxthread {
public:
#if (HX_USE_THREADS) == 11
	/// The return type of a thread entry point as required by the configured
	/// backend.
	using return_t = int;
#else
	using return_t = void*;
#endif

	/// Default constructor. Thread is not started.
	hxthread(void) : m_thread_(), m_joinable_(false) { }

	/// Constructs and starts a thread with the given function and argument. Does
	/// not free the argument. Any function that takes a single pointer and
	/// returns `return_t` should work. The return value is ignored but may be
	/// unsafe to cast to a function with a different return type.
	/// - `entry_point` : Function pointer of type: `entry_point(T*)`.
	/// - `parameter` : `T*` to pass to the function.
	template<typename parameter_t_>
	explicit hxthread(return_t (*entry_point_)(parameter_t_*), parameter_t_* parameter_)
			: hxthread() {
		this->start(entry_point_, parameter_);
	}

	/// Destructor. Asserts that the thread was stopped correctly.
	~hxthread(void) {
		hxassert_hard(!this->joinable(), "thread_still_running");
	}

	/// Starts a thread with the given function and argument. Does not free the
	/// argument. Any function that takes a single `T` pointer and returns
	/// `return_t` should work. The return value is ignored but is required by
	/// the native calling convention.
	/// - `entry_point` : Function pointer of type: `entry_point(T*)`.
	/// - `parameter` : `T*` to pass to the function.
	template<typename parameter_t_>
	void start(return_t (*entry_point_)(parameter_t_*), parameter_t_* parameter_) {
		hxassertmsg(!this->joinable(), "thread_still_running");

		// Stay on the right side of the C++ standard by avoiding assumptions
		// about pointer representations. The parameter is being reinterpreted
		// twice instead of cast once and reinterpreted back.
		static_assert(sizeof(void*) == sizeof(parameter_t_*), "Incompatible pointer sizes");

		void* reinterpreted_parameter_ = hxnull;
		::memcpy(&reinterpreted_parameter_, &parameter_, sizeof(void*)); // NOLINT(bugprone-bitwise-pointer-cast)
#if (HX_USE_THREADS) == 11
		const int code_ = ::thrd_create(&m_thread_,
			reinterpret_cast<entry_point_function_t_>(entry_point_), reinterpreted_parameter_);
		hxassert_always(code_ == thrd_success, "thrd_create %d", code_); (void)code_;
#else
		const int code_ = ::pthread_create(&m_thread_, 0,
			reinterpret_cast<entry_point_function_t_>(entry_point_), reinterpreted_parameter_);
		hxassert_always(code_ == 0, "pthread_create %s", ::strerror(code_)); (void)code_;
#endif
		m_joinable_ = true;
	}

	/// Returns true if the thread has been started and not yet joined. Callers
	/// must check the return value before acting on the thread state.
	hxattr_nodiscard bool joinable(void) const { return m_joinable_; }

	/// Joins the thread. Blocks until the thread finishes.
	void join(void) {
		hxassertmsg(this->joinable(), "thread_not_running");
#if (HX_USE_THREADS) == 11
		const int code_ = ::thrd_join(m_thread_, hxnull);
		hxassert_always(code_ == thrd_success, "thrd_join %d", code_);
		(void)code_;
#else
		const int code_ = ::pthread_join(m_thread_, 0);
		hxassert_always(code_ == 0, "pthread_join %s", ::strerror(code_));
		(void)code_;
#endif
		m_joinable_ = false;
	}

private:
	using entry_point_function_t_ = return_t (*)(void*);

	hxthread(const hxthread&) = delete;
	hxthread& operator=(const hxthread&) = delete;

#if (HX_USE_THREADS) == 11
	::thrd_t m_thread_;
#else
	::pthread_t m_thread_;
#endif
	bool m_joinable_;
};

#endif // HX_USE_THREADS
HX_NS_END_
