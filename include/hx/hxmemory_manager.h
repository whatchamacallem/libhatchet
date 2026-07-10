#pragma once
// SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
// SPDX-License-Identifier: MIT
// This file is licensed under the MIT license found in the LICENSE.md file.

/// \file
/// Memory Manager C/C++ API. Memory allocators are selected using an ID. These
/// are the large system-wide allocators, not the per-container `hxallocator`
/// which allocates from here. Temporary stacks are allocated at runtime with
/// `hxmemory_manager_allocate_stacks` and a sophisticated streaming design can
/// select between them using `hxsystem_allocator_stack_0` + index.
///
/// General purpose memory allocators are inefficient and unsafe to use. The
/// problem is that long running code requires a lot of extra space to make sure
/// it doesn't fragment and unexpectedly fail to make a large allocation.
/// (Hardware support for virtual memory can help defrag the program heap, but
/// that requires processor support and even more expensive system call
/// overhead.) For code that uses a lot of temporary intermediate allocations
/// 1/3 of your memory and 1/3 of your processor time could get eaten by the
/// heap allocator. The `hxsystem_allocator_stack_0` is provided as a
/// replacement for that use case.
///
/// There are also a category of allocations that are expected to last for the
/// lifetime of the application. They can be allocated with 0 overhead using
/// `hxsystem_allocator_permanent`.
///
/// WARNING: The current allocator ID is a thread local attribute that is
/// managed by the `hxsystem_allocator_scope` RAII class. This provides a non-
/// intrusive way to move swaths of code to different allocators.
///
/// Alignment must be a power of two. (It always is.)
///
/// `HX_HARDENING_MODE == HX_HARDENING_MODE_DEBUG` debug memory byte patterns:
///
/// | Hex    | Pattern Description              |
/// | ------ | -------------------------------- |
/// | `0xab` | Static `hxallocator` instance.   |
/// | `0xcd` | Allocated.                       |
/// | `0xdd` | Deallocated.                     |
///
/// Global new and delete are provided when `HX_USE_LIBCXX==0`. This is a
/// requirement for running as a stand alone C++ runtime. Otherwise they are not
/// interfered with. Those default versions do not use the memory manager's
/// current allocator because it may not be safe to do so. `hxnew` and
/// `hxdelete` are available as recommended substitutes.
///
/// It should be possible to implement a triple buffered streaming strategy
/// for DMA by allocating three temp stacks and selecting between them with
/// `hxsystem_allocator_stack_0 + index`.

#if !LIBHATCHET_VER
#error #include <hx/libhatchet.h> instead.
#endif

#if HX_CPLUSPLUS
#if HX_USE_LIBCXX
#include <new>
#endif

extern "C" {
#endif

/// `hxalignment_t` - A positive integer power of 2 for aligning allocations.
typedef unsigned int hxalignment_t;

#if HX_CPLUSPLUS >= 202002L
/// `hxalignment` - The default alignment. Allows for storing things like
/// pointers. This alignment should work for most types.
inline constexpr hxalignment_t hxalignment = static_cast<hxalignment_t>(alignof(max_align_t ));
#elif HX_CPLUSPLUS
#define hxalignment static_cast<hxalignment_t>(alignof(max_align_t ))
#else
#define hxalignment (hxalignment_t)_Alignof(max_align_t )
#endif

/// `hxsystem_allocator_t` - This is extendable by the application.
typedef int hxsystem_allocator_t;

enum {
	/// `hxsystem_allocator_current` - Use current allocation scope. Not a real
	/// allocator slot.
	hxsystem_allocator_current = -1,
	/// `hxsystem_allocator_heap` - OS heap with alignment.
	hxsystem_allocator_heap,
	/// `hxsystem_allocator_permanent` - Contiguous allocations that must not be
	/// freed.
	hxsystem_allocator_permanent,
	/// `hxsystem_allocator_stack_0` - Temporary stacks. Reset to previous depth
	/// at scope closure. Stack index n is `hxsystem_allocator_stack_0 + n`.
	hxsystem_allocator_stack_0
};

/// `hxfree` - Frees memory previously allocated with `hxmalloc` or
/// `hxmalloc_ext`. Freeing null pointers is allowed.
/// - `ptr` : Pointer to the memory to free.
void hxfree(void* ptr_) hxattr_noexcept hxattr_hot;

/// `hxmalloc` - Allocates memory of the specified size using the default memory
/// manager. A C++ overload optionally provides the same arguments as
/// `hxmalloc_ext`. Will not return on failure. WARNING: It is undefined
/// behavior to compare pointers to different allocations. This is consistent
/// with the C++ standard. Allocations of size 0 may or may not return the same
/// pointer as previous allocations. Returns a pointer that must be released
/// with `hxfree`.
/// - `size` : The size of the memory to allocate.
/// - `allocator`(C++ only): The memory manager ID to use for allocation. (Default is
///   `hxsystem_allocator_current`.)
/// - `alignment`(C++ only): The alignment for the allocation. (Default
///   is `hxalignment`.)
void* hxmalloc(size_t size_) hxattr_allocator(hxfree) hxattr_noexcept hxattr_hot;

/// `hxmalloc_ext` - Allocates memory of the specified size with a specific
/// memory manager and alignment. Will not return on failure.
/// Returns a pointer that must be released with `hxfree`.
/// - `size` : The size of the memory to allocate.
/// - `allocator` : The memory manager ID to use for allocation. (Default is `hxsystem_allocator_current`.)
/// - `alignment` : The alignment for the allocation. (Default is `hxalignment`.)
void* hxmalloc_ext(size_t size_, hxsystem_allocator_t allocator_,
	hxalignment_t alignment_/*=hxalignment*/) hxattr_noexcept hxattr_allocator(hxfree) hxattr_hot;

/// `hxstring_duplicate` - Allocates a copy of a string using the specified
/// memory manager. Returns a pointer to the duplicated string.
/// Returns a pointer that must be released with `hxfree`.
/// - `string` : Non-null string to duplicate.
/// - `allocator` : The memory manager ID to use for allocation. Defaults to
///   `hxsystem_allocator_current` in C++.
char* hxstring_duplicate(const char* string_,
	hxsystem_allocator_t allocator_ /*=hxsystem_allocator_current*/)
		 hxattr_noexcept hxattr_allocator(hxfree) hxattr_nonnull(1) hxattr_hot;

#if HX_CPLUSPLUS
} // extern "C"

/// `hxmalloc` - Add `hxmalloc_ext` args to `hxmalloc` C interface. Allocates
/// memory with a specific memory manager and alignment. NOTE: This is not in
/// the libhatchet namespace.
inline void* hxmalloc( size_t size_, hxsystem_allocator_t allocator_, hxalignment_t alignment_=hxalignment) {
	return hxmalloc_ext(size_, allocator_, alignment_);
}

/// `hxstring_duplicate` - Add default args to C interface. The allocator is
/// `hxsystem_allocator_current`. Duplicates a string using the default memory
/// manager. NOTE: This is not in the libhatchet namespace.
inline char* hxstring_duplicate(const char* s_) {
	return hxstring_duplicate(s_, hxsystem_allocator_current);
}

// Memory Manager C++ API

#if !(HX_USE_LIBCXX)
// Declare placement new. These are not built into the compiler.
// HX_PROVIDE_NEW_DELETE is concerned with the other versions of new and delete.
#if HX_CPLUSPLUS >= 202002L
constexpr void* operator new(size_t, void* ptr_) noexcept { return ptr_; }
constexpr void* operator new[](size_t, void* ptr_) noexcept { return ptr_; }
#else
inline void* operator new(size_t, void* ptr_) noexcept { return ptr_; }
inline void* operator new[](size_t, void* ptr_) noexcept { return ptr_; }
#endif

#if (HX_PROVIDE_NEW_DELETE) != -1
// Provide these for constant evaluation/consteval. Link errors may still result.
// Use HX_PROVIDE_NEW_DELETE=-1 when compiling a module to prevent declaration.
void* operator new(size_t size_) hxattr_nodiscard;
void* operator new[](size_t size_) hxattr_nodiscard;
void operator delete(void* ptr_) noexcept;
void operator delete[](void* ptr_) noexcept;
#endif
#endif

HX_NS_BEGIN_

/// `hxsystem_allocator_scope` - An RAII class to set the current memory manager
/// allocator for the current scope. It automatically restores the previous
/// allocator when the scope ends. It also resets stack allocators to their
/// initial offsets thereby freeing any allocations made during the lifetime of
/// this object. WARNING: Two threads cannot share a stack allocator using
/// this mechanism without due caution. Wait for worker tasks to complete before
/// freeing their temporary allocations. The closest thing in the standard is
/// `std::scoped_allocator_adaptor` and it is a template nightmare.
class hxsystem_allocator_scope
{
public:
	/// Constructor: Sets the current memory allocator to the specified ID.
	/// - `allocator` : The memory manager ID to set for this scope. May modify
	///   new allocator in a way that cannot be shared between threads.
	hxsystem_allocator_scope(hxsystem_allocator_t allocator_) hxattr_noexcept;

	/// Destructor restores the stored previous memory manager allocator ID. May
	/// modify current allocator in a way that cannot be shared between threads.
	~hxsystem_allocator_scope(void) hxattr_noexcept;

	/// Gets the total number of allocations outstanding for this memory
	/// allocator. There should be no system overhead. Allocations made directly
	/// to `new`, `delete`, `malloc` and `free` are not tracked by
	/// hxsystem_allocator_heap.
	hxattr_nodiscard size_t get_current_allocation_count(void) const;

	/// Gets the total number of bytes allocated outstanding for this memory
	/// allocator. Freed bytes might not be reused until the scope closes. There
	/// may also be significant system overhead that is not being reported.
	hxattr_nodiscard size_t get_current_bytes_allocated(void) const;

	/// Gets the number of allocations made when this scope was entered.
	hxattr_nodiscard size_t get_initial_allocation_count(void) const { return m_initial_allocation_count_; }

	/// Gets the number of bytes allocated when this scope was entered.
	hxattr_nodiscard size_t get_initial_bytes_allocated(void) const { return m_initial_bytes_allocated_; }

private:
	// The hxsystem_allocator_* classes are responsible for setting
	// m_initial_allocation_count_ and m_initial_bytes_allocated_.
	// This avoids a number of potential cache misses.
	friend inline void hxsystem_allocator_scope_init_(hxsystem_allocator_scope* scope_,
		size_t allocation_count_, size_t bytes_allocated_);

	// Deleted copy constructor to prevent copying.
	hxsystem_allocator_scope(const hxsystem_allocator_scope&) = delete;

	// Deleted assignment operator to prevent copying.
	void operator=(const hxsystem_allocator_scope&) = delete;

	hxsystem_allocator_t m_this_allocator_;
	hxsystem_allocator_t m_initial_allocator_;
	size_t m_initial_allocation_count_;
	size_t m_initial_bytes_allocated_;
};

/// `hxnew<T, allocator, align>(...)` - Allocates and constructs an object of
/// type `T` using an optional memory allocator and alignment. Returns a pointer
/// to the newly constructed object. Will not return on failure.
/// - `allocator` : The memory manager ID to use for allocation. Defaults to `hxsystem_allocator_current`.
/// - `align` : Alignment to use when allocating new pointers. Defaults to `hxalignment`.
template <typename T_, hxsystem_allocator_t allocator_=hxsystem_allocator_current, hxalignment_t align_=hxalignment, typename... Args_>
T_* hxnew(Args_&&... args_) noexcept {
	// Implements hxforward.
	return ::new(hxmalloc_ext(sizeof(T_), allocator_, align_)) T_(static_cast<Args_&&>(args_)...);
}

/// `hxdelete` - Deletes an object of type `T` and frees its memory using the
/// memory manager.
/// - `t` : Pointer to the object to delete.
template <typename T_>
void hxdelete(T_* t_) noexcept {
	if(t_) {
		t_->~T_();
		hxfree(t_);
	}
}

/// `hxdefault_delete` - A callable that deletes objects of type `T` using
/// `hxdelete`. Used by containers to implement the destruction of their contents
/// according to a template parameter. Implements `std::default_delete`.
class hxdefault_delete {
public:
	/// Deletes the object using `hxdelete`.
	template <typename T_>
	void operator()(T_* t_) const { hxdelete(t_); }

	/// Always returns true, indicating the deleter is valid.
	operator bool(void) const { return true; }
};

/// `hxdo_not_delete` - A version of `hxdefault_delete` that does not delete the
/// object. Allows removing object destruction from container destructors that
/// handle static allocations or don't own their contents for another reason.
class hxdo_not_delete {
public:
	/// Does not delete the object.
	template <typename T_>
	void operator()(T_*) const { }

	/// Always returns false, indicating the deleter should not be called.
	operator bool(void) const { return false; }
};

#if HX_CPLUSPLUS >= 201402L
/// `hxconsteval_delete` - A `constexpr`-compatible deleter that uses `::delete`.
/// Required for `consteval` contexts where `hxdefault_delete` cannot be used
/// because `hxdelete` calls `hxfree` which is not `constexpr`.
class hxconsteval_delete {
public:
	// GCOVR_EXCL_START. No coverage at compile time.
	/// Deletes the object using `::delete`.
	template <typename T_>
	constexpr void operator()(T_* t_) const { ::delete t_; }

	/// Always returns true, indicating the deleter is valid.
	constexpr operator bool(void) const { return true; }
	// GCOVR_EXCL_STOP
};
#endif
/// \cond HIDDEN

// `hxmemory_manager_init_` - WARNING: Not intended for direct use. This is
// called by hxinit(). Initializes the memory manager. Must be called before
// using any memory manager functions.
void hxmemory_manager_init_(void) hxattr_cold;

// `hxmemory_manager_shut_down_` - WARNING: Not intended for direct use. Shuts
// down the memory manager. Frees any remaining resources.
void hxmemory_manager_shut_down_(void) hxattr_cold;
/// \endcond

/// `hxmemory_manager_allocate_stacks` - Allocates the runtime temporary stacks.
/// Stack n is addressed as `hxsystem_allocator_stack_0 + n`. The memory
/// manager does not allocate any temporary stacks at init. Do not call twice.
/// - `stack_count` : The number of temporary stacks to allocate. Must not
///   exceed `HX_MEMORY_MAX_STACKS`.
/// - `sizes` : An array of `stack_count` byte budgets, one per stack.
void hxmemory_manager_allocate_stacks(const size_t* sizes_, size_t stack_count_) hxattr_cold;

/// A safer `hxmemory_manager_allocate_stacks`.
template<size_t stack_count_>
void hxmemory_manager_allocate_stacks(const size_t(&list_)[stack_count_]) {
	hxmemory_manager_allocate_stacks(list_, stack_count_);
}

/// `hxmemory_manager_stats` - The utilization statistics reported by
/// `hxmemory_manager_utilization`.
class hxmemory_manager_stats {
public:
	size_t allocations_outstanding;
	size_t bytes_outstanding;
	size_t allocator_overflows;
};

/// `hxmemory_manager_utilization` - Returns the utilization statistics of the
/// memory manager.
/// - `stacks_only` : Only report temporary stack utilization.
/// - `log` : Log stats when logging is enabled.
hxmemory_manager_stats hxmemory_manager_utilization(bool stacks_only_, bool log_) hxattr_cold;

HX_NS_END_
#endif // HX_CPLUSPLUS
