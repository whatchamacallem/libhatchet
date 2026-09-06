// SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
// SPDX-License-Identifier: MIT
// This file is licensed under the MIT license found in the LICENSE.md file.

#include "../include/hx/libhatchet.h"
#include "../include/hx/hxthread.hpp"
#include "../include/hx/hxutility.h"

// Switches heap to using allocation tracking headers in debug.
#ifdef _MSC_VER
#define HX_USE_STD_ALIGNED_ALLOC 0
#else
#define HX_USE_STD_ALIGNED_ALLOC (HX_CPLUSPLUS >= 201703L && (HX_HARDENING_MODE) != HX_HARDENING_MODE_DEBUG)
#endif

// hxmalloc_checked_ always checks malloc and halts on failure. It enforces the
// overall policy against allocation failure handling routines and the static
// analysis contract described by hxattr_allocator.
hxattr_allocator(free) hxattr_hot hxattr_noexcept static void* hxmalloc_checked_(size_t size) {
	void* t = ::malloc(size);
	hxassert_hard(t, "malloc %zu", size);
#if (HX_HARDENING_MODE) == HX_HARDENING_MODE_NONE
	if(!t) {
		hxlog_handler(hxlog_level_assert, "malloc %zu", size);
		hxexit(EXIT_FAILURE);
	}
#endif
	return t;
}

#if (HX_PROVIDE_NEW_DELETE) == 1
// Forward declare for C++11.
hxattr_hot void operator delete(void* ptr, size_t) noexcept;
hxattr_hot void operator delete[](void* ptr, size_t) noexcept;

hxattr_hot void* operator new(size_t size) {
	return hxmalloc_checked_(size);
}
hxattr_hot void* operator new[](size_t size) {
	return hxmalloc_checked_(size);
}
// GCOVR_EXCL_START. It is unspecified which of these functions is called.
hxattr_hot void operator delete(void* ptr) noexcept {
	::free(ptr);
}
hxattr_hot void operator delete(void* ptr, size_t) noexcept {
	::free(ptr);
}
hxattr_hot void operator delete[](void* ptr) noexcept {
	::free(ptr);
}
hxattr_hot void operator delete[](void* ptr, size_t) noexcept {
	::free(ptr);
}
// GCOVR_EXCL_STOP
#endif

// HX_USE_MEMORY_MANAGER. See hxsettings.h.
#if HX_USE_MEMORY_MANAGER

HX_NS_BEGIN_

// Friend function that restricts direct access to these variables to this file.
hxinline void hxsystem_allocator_scope_init_(hxsystem_allocator_scope* scope,
		size_t allocation_count, size_t bytes_allocated) {
	scope->m_initial_allocation_count_ = allocation_count;
	scope->m_initial_bytes_allocated_ = bytes_allocated;
}

// The heap allocator is thread-safe. The permanent allocator is supposed to be
// single-threaded. And the temp stack requires scopes to be opened and closed
// on the same thread (asserted in debug). In theory locking could be removed
// if the leak tracking was ripped out too.
#if HX_USE_THREADS
static hxmutex hxs_memory_manager_mutex;
#define HX_MEMORY_MANAGER_LOCK_() const hxunique_lock memory_manager_lock_(hxs_memory_manager_mutex)
#else
#define HX_MEMORY_MANAGER_LOCK_() (void)0
#endif

// The current allocator is a thread-local attribute. hxthread_local is
// zero-initialized, which must equal hxsystem_allocator_heap for non-init
// threads to default safely to the heap.
static_assert(hxsystem_allocator_heap == 0,
	"hxsystem_allocator_heap must be 0 for thread-local default");
static hxthread_local<hxsystem_allocator_t> hxs_current_memory_allocator;

// -- hxmemory_allocation_header -----------------------------------------------
// Used until C++17.
#if !HX_USE_STD_ALIGNED_ALLOC
class hxmemory_allocation_header {
public:
	size_t size;
	uintptr_t actual; // Address actually returned by malloc.
};
#endif

// -- hxmemory_allocator_base --------------------------------------------------
// There are no virtual methods. The memory manager dispatches on the allocator
// id with a single compare instead of a vtable.
class hxmemory_allocator_base {
public:
	const char* label(void) const { return m_label_; }

protected:
	const char* m_label_;
};

// -- hxmemory_allocator_os_heap -----------------------------------------------
// This just calls aligned_alloc when HX_HARDENING_MODE !=
// HX_HARDENING_MODE_DEBUG. In debug and in C++98 mode this code wraps heap
// allocations with a header and adds padding to obtain required alignment. This
// allows tracking bytes allocated in debug.
class hxmemory_allocator_os_heap : public hxmemory_allocator_base {
public:
	hxattr_cold void construct(const char* label) {
		m_label_ = label;
		m_allocation_count = 0u;
		m_bytes_allocated = 0u;
		m_high_water = 0u;
	}

	hxattr_hot void* allocate(size_t size, hxalignment_t alignment) {
#if HX_USE_STD_ALIGNED_ALLOC
		// C11 aligned_alloc requires alignment ≥ sizeof(void*) on some platforms.
		alignment = hxmax(alignment, hxalignment);
		const size_t alignment_mask = static_cast<size_t>(alignment) - 1u;

		// Round up size to a multiple of alignment as required by
		// aligned_alloc. Treat size 0 as 1 so aligned_alloc never receives size
		// 0 (implementation-defined). Integer overflow due to trying to
		// allocate max memory is UB.
		const size_t rounded = hxmax<size_t>((size + alignment_mask) & ~alignment_mask, alignment);
		hxassertmsg(rounded >= size, "no_memory size %zu", size);

		void* t = ::aligned_alloc(alignment, rounded);
		hxassert_always(t, "aligned_alloc %zu %zu", static_cast<size_t>(alignment), rounded);
		++m_allocation_count;
		// Bytes are not tracked because there is no allocation header to examine on free.
		return t;
#else
		const size_t alignment_mask = static_cast<size_t>(alignment) - 1u;

		// Place header immediately before aligned allocation. malloc is aligned.
		// Integer overflow due to trying to allocate max memory is UB.
		const size_t total = size + sizeof(hxmemory_allocation_header) + alignment_mask;
		hxassertmsg(total > size, "no_memory size %zu", size);
		const uintptr_t actual = reinterpret_cast<uintptr_t>(hxmalloc_checked_(total));
		const uintptr_t aligned = (actual + sizeof(hxmemory_allocation_header)
			+ alignment_mask) & ~alignment_mask;
		hxmemory_allocation_header& hdr = reinterpret_cast<hxmemory_allocation_header*>(aligned)[-1];
		hdr.size = size;
		hdr.actual = actual;
		++m_allocation_count;
		m_bytes_allocated += size; // Ignore overhead.
		m_high_water = hxmax(m_high_water, m_bytes_allocated);

		return reinterpret_cast<void*>(aligned);
#endif
	}

	hxattr_hot void deallocate(void* ptr) {
#if HX_USE_STD_ALIGNED_ALLOC
		hxassertmsg(m_allocation_count > 0u, "bad_free nothing allocated");
		--m_allocation_count;
		::free(ptr);
#else

		const hxmemory_allocation_header& hdr = reinterpret_cast<hxmemory_allocation_header*>(ptr)[-1];
		hxassertmsg(m_allocation_count > 0u, "bad_free sentinel corrupt");
		hxassertmsg(hdr.size <= m_bytes_allocated,
			"bad_free sentinel corrupt %zu %zu", hdr.size, m_bytes_allocated);
		--m_allocation_count;
		m_bytes_allocated -= hdr.size;

		const uintptr_t actual = hdr.actual;
#if (HX_HARDENING_MODE) == HX_HARDENING_MODE_DEBUG
		::memset(ptr, 0xdd, hdr.size);
#endif
		::free(reinterpret_cast<void*>(actual));
#endif
	}

	size_t get_allocation_count(void) const { return m_allocation_count; }
	size_t get_bytes_allocated(void) const { return m_bytes_allocated; }
	size_t get_high_water(void) const { return m_high_water; }

private:
	size_t m_allocation_count;
	size_t m_bytes_allocated;
	size_t m_high_water;
};

// -- hxmemory_allocator_stack ------------------------------------------------
// Nothing can be freed.
class hxmemory_allocator_stack : public hxmemory_allocator_base {
public:
	hxattr_cold void construct(void* ptr, size_t size, const char* label) {
		m_label_ = label;

		m_allocation_count = 0u;
		m_begin_ = reinterpret_cast<uintptr_t>(ptr);
		m_end_ = reinterpret_cast<uintptr_t>(ptr) + size;
		m_current = reinterpret_cast<uintptr_t>(ptr);
	}

	hxattr_cold void* release(void) const {
		// Don't waste space on defensive code.
		return reinterpret_cast<void*>(m_begin_);
	}

	hxattr_hot void* allocate(size_t size, hxalignment_t alignment) {
		const uintptr_t alignment_mask = static_cast<uintptr_t>(alignment - 1u);
		const uintptr_t aligned = (m_current + alignment_mask) & ~alignment_mask;
		// Integer overflow due to trying to allocate max memory is UB.
		if((aligned + size) > m_end_) {
			return hxnull;
		}

		++m_allocation_count;
		m_current = aligned + size;
		return reinterpret_cast<void*>(aligned);
	}

	bool contains(void* ptr) const {
		const uintptr_t ptr_value = reinterpret_cast<uintptr_t>(ptr);
		return ptr_value >= m_begin_ && ptr_value < m_end_;
	}

	hxattr_hot void deallocate(void* ptr) {
		// Use ≤ because an outstanding allocation of size 0 could have been
		// made at m_current.
		const uintptr_t ptr_value = reinterpret_cast<uintptr_t>(ptr);
		hxassertmsg(m_allocation_count > 0 && ptr_value >= m_begin_
			&& ptr_value <= m_current, "bad_free %s %zx", m_label_, static_cast<size_t>(ptr_value));
		(void)ptr_value;
		--m_allocation_count;
	}

	size_t get_allocation_count(void) const { return m_allocation_count; }
	size_t get_bytes_allocated(void) const { return m_current - m_begin_; }
	size_t get_high_water(void) const { return m_current - m_begin_; }

protected:
	uintptr_t m_begin_;
	uintptr_t m_end_;
	uintptr_t m_current;
	size_t m_allocation_count;
};

// -- hxmemory_allocator_temp_stack -------------------------------------------
// Resets after a scope closes.
class hxmemory_allocator_temp_stack : public hxmemory_allocator_stack {
public:
	hxattr_cold void construct(void* ptr, size_t size, const char* label) {
		hxmemory_allocator_stack::construct(ptr, size, label);
		m_high_water = 0u;
#if (HX_HARDENING_MODE) == HX_HARDENING_MODE_DEBUG
		m_owner_thread = 0u;
		m_scope_depth = 0u;
#endif
	}

	hxattr_hot void begin_allocation_scope(hxsystem_allocator_scope* scope) {
#if (HX_HARDENING_MODE) == HX_HARDENING_MODE_DEBUG
		hxassertmsg(m_owner_thread == 0u || m_owner_thread == hxthread_id(),
			"temp_stack cross-thread scope %s", m_label_);
		m_owner_thread = hxthread_id();
		++m_scope_depth;
#endif
		hxsystem_allocator_scope_init_(scope, m_allocation_count, m_current - m_begin_);
	}

	hxattr_hot void end_allocation_scope(hxsystem_allocator_scope* scope) {
#if (HX_HARDENING_MODE) == HX_HARDENING_MODE_DEBUG
		hxassertmsg(m_owner_thread == hxthread_id(), "temp_stack cross-thread scope %s", m_label_);
		if(--m_scope_depth == 0u) {
			m_owner_thread = 0u;
		}
#endif
		hxassertmsg(m_allocation_count <= scope->get_initial_allocation_count(),
			"memory_leak scope %s allocations %zu", m_label_,
			m_allocation_count - scope->get_initial_allocation_count());

		m_high_water = hxmax(m_high_water, m_current);

		// Do not reset m_allocation_count = scope->get_initial_allocation_count()
		// because that would break leak tracking.

		const uintptr_t previous_current = m_begin_ + scope->get_initial_bytes_allocated();
#if (HX_HARDENING_MODE) == HX_HARDENING_MODE_DEBUG
		::memset(reinterpret_cast<void*>(previous_current), 0xdd,
			static_cast<size_t>(m_current - previous_current));
#endif
		m_current = previous_current;
	}

	hxattr_hot size_t get_high_water(void) {
		m_high_water = hxmax(m_high_water, m_current);
		return m_high_water - m_begin_;
	}

protected:
	uintptr_t m_high_water;
#if (HX_HARDENING_MODE) == HX_HARDENING_MODE_DEBUG
	size_t m_owner_thread;
	size_t m_scope_depth;
#endif
};

// -- hxmemory_manager ---------------------------------------------------------

class hxmemory_manager {
public:
	hxattr_cold void construct(void);
	hxattr_cold void allocate_stacks(const size_t* sizes, size_t stack_count);
	hxattr_cold void destruct(void);
	hxattr_hot void* allocate(size_t size, hxsystem_allocator_t id, hxalignment_t alignment);
	hxattr_hot void free(void* ptr);
	hxattr_hot hxsystem_allocator_t begin_allocation_scope(hxsystem_allocator_scope* scope,
		hxsystem_allocator_t new_id);
	hxattr_hot void end_allocation_scope(hxsystem_allocator_scope* scope,
		hxsystem_allocator_t previous_id);
	size_t get_allocation_count(hxsystem_allocator_t id);
	size_t get_bytes_allocated(hxsystem_allocator_t id);
	hxattr_cold hxmemory_manager_stats utilization(bool stacks_only, bool log);

private:
	hxmemory_allocator_os_heap	  m_memory_allocator_heap;
#if (HX_MEMORY_BUDGET_PERMANENT) != 0
	hxmemory_allocator_stack	  m_memory_allocator_permanent;
#endif
	hxmemory_allocator_temp_stack m_memory_allocator_stacks[HX_MEMORY_MAX_STACKS];
	size_t                        m_stack_count;
	size_t                        m_allocator_overflows;
};

// NOTE: Using static instead of an anonymous namespace because of a linker issue.
static hxmemory_manager hxs_memory_manager;

void hxmemory_manager::construct(void) {
	m_memory_allocator_heap.construct("heap");

#if (HX_MEMORY_BUDGET_PERMANENT) != 0
	m_memory_allocator_permanent.construct(hxmalloc_checked_(HX_MEMORY_BUDGET_PERMANENT),
		(HX_MEMORY_BUDGET_PERMANENT), "perm");
#endif

	// Temporary stacks are not allocated until hxmemory_manager_allocate_stacks.
	m_stack_count = 0u;
	m_allocator_overflows = 0u;

	// Safe default.
	hxs_current_memory_allocator = hxsystem_allocator_heap;
}

void hxmemory_manager::allocate_stacks(const size_t* sizes, size_t stack_count) {
	hxassert_always(m_stack_count == 0u, "memory_manager stacks reinitialized");
	hxassert_always(stack_count <= HX_MEMORY_MAX_STACKS,
		"memory_manager too many stacks %zu", stack_count);

	HX_MEMORY_MANAGER_LOCK_();
	for(size_t i = 0u; i != stack_count; ++i) {
		hxmemory_allocator_temp_stack& stack = m_memory_allocator_stacks[i];
		stack.construct(hxmalloc_checked_(sizes[i]), sizes[i], "temp");
	}
	m_stack_count = stack_count;
}

void hxmemory_manager::destruct(void) {
#if (HX_MEMORY_BUDGET_PERMANENT) != 0
	::free(m_memory_allocator_permanent.release());
#endif
	for(size_t i = 0u; i != m_stack_count; ++i) {
		::free(m_memory_allocator_stacks[i].release());
	}
	m_stack_count = 0u;
}

// WARNING: It is undefined behavior to compare pointers to different
// allocations. This is consistent with the C++ standard. Allocations of size 0
// may or may not return the same pointer as a previous allocation. Compiler
// optimizations may still assume allocated pointers do not alias in that case
// because they do not refer to any memory. Comparing pointers to size 0
// allocations is a bad idea.
void* hxmemory_manager::allocate(size_t size, hxsystem_allocator_t id, hxalignment_t alignment) {
	if(id == hxsystem_allocator_current) {
		// This involves a call to pthreads.
		id = hxs_current_memory_allocator;
	}

	// The result of a size 0 allocation is UB and the pointer may or may not
	// equal a prior result.
	hxassertmsg(size != 0u, "bad_alloc size 0");

	// Provide an alignment of 1 for strings and unaligned allocations. The
	// following code assumes that "alignment-1" is a valid mask of unused bits
	// and not a mask containing every bit.
	hxassertmsg(alignment != 0u, "bad_align use 1 not 0");
	hxassertmsg(((alignment - 1u) & alignment) == 0u,
		"bad_align not pow2 %#zx", static_cast<size_t>(alignment));

	HX_MEMORY_MANAGER_LOCK_();
	hxassertmsg(id >= 0 && id < (hxsystem_allocator_stack_0
		+ static_cast<hxsystem_allocator_t>(m_stack_count)),
		"bad_arg %d", static_cast<int>(id));

#if (HX_MEMORY_BUDGET_PERMANENT) != 0
	// The permanent allocator is the first fixed size allocator.
	const hxsystem_allocator_t first_fixed_id = hxsystem_allocator_permanent;
#else
	// Without a permanent budget the permanent allocator aliases the heap.
	const hxsystem_allocator_t first_fixed_id = hxsystem_allocator_stack_0;
#endif

	void* ptr = hxnull;
	if(id >= hxsystem_allocator_stack_0) {
		ptr = m_memory_allocator_stacks[id - hxsystem_allocator_stack_0]
			.allocate(size, alignment);
	}
#if (HX_MEMORY_BUDGET_PERMANENT) != 0
	else if(id == hxsystem_allocator_permanent) {
		ptr = m_memory_allocator_permanent.allocate(size, alignment);
	}
#endif

	if(ptr == hxnull) {
		// Only the fixed size allocators overflow. The heap halts instead of
		// returning null.
		if(id >= first_fixed_id) {
			++m_allocator_overflows;
			hxlog_warning("no_memory overflowing %d size %zu", static_cast<int>(id), size);
		}
		ptr = m_memory_allocator_heap.allocate(size, alignment);
	}

	const uintptr_t alignment_mask = static_cast<uintptr_t>(alignment) - 1u;
	const uintptr_t ptr_value = reinterpret_cast<uintptr_t>(ptr);
	hxassertmsg((ptr_value & alignment_mask) == 0,
		"bad_align wrong %zx from %d", static_cast<size_t>(ptr_value), id);
	 (void)alignment_mask; (void)ptr_value;

	return ptr;
}

void hxmemory_manager::free(void* ptr) {
	if(ptr == hxnull) {
		return;
	}

	// This path is hard-coded for efficiency.
	HX_MEMORY_MANAGER_LOCK_();

	hxmemory_allocator_temp_stack* hxrestrict stack = m_memory_allocator_stacks;
	for(hxmemory_allocator_temp_stack* const stacks_end = stack + m_stack_count;
			stack != stacks_end; ++stack) {
		if(stack->contains(ptr)) {
			stack->deallocate(ptr);
			return;
		}
	}

#if (HX_MEMORY_BUDGET_PERMANENT) != 0
	if(m_memory_allocator_permanent.contains(ptr)) {
		hxwarn_msg(hxg_settings.deallocate_permanent, "ERROR: free from permanent");
		m_memory_allocator_permanent.deallocate(ptr);
		return;
	}
#endif

	m_memory_allocator_heap.deallocate(ptr);
}

hxsystem_allocator_t hxmemory_manager::begin_allocation_scope(
		hxsystem_allocator_scope* scope, hxsystem_allocator_t new_id) {
	HX_MEMORY_MANAGER_LOCK_();
	hxassertmsg(new_id >= 0 && new_id < (hxsystem_allocator_stack_0
		+ static_cast<hxsystem_allocator_t>(m_stack_count)),
		"bad_arg %d", static_cast<int>(new_id));

	const hxsystem_allocator_t previous_id = hxs_current_memory_allocator;
	hxs_current_memory_allocator = new_id;
	if(new_id >= hxsystem_allocator_stack_0) {
		m_memory_allocator_stacks[new_id - hxsystem_allocator_stack_0].begin_allocation_scope(scope);
	}
	else {
		hxsystem_allocator_scope_init_(scope, get_allocation_count(new_id),
			get_bytes_allocated(new_id));
	}
	return previous_id;
}

void hxmemory_manager::end_allocation_scope(
		hxsystem_allocator_scope* scope, hxsystem_allocator_t previous_id) {
	HX_MEMORY_MANAGER_LOCK_();
	const hxsystem_allocator_t current_id = hxs_current_memory_allocator;
	if(current_id >= hxsystem_allocator_stack_0) {
		hxassertmsg(current_id < (hxsystem_allocator_stack_0
			+ static_cast<hxsystem_allocator_t>(m_stack_count)),
			"bad_arg %d", static_cast<int>(current_id));
		m_memory_allocator_stacks[current_id - hxsystem_allocator_stack_0].end_allocation_scope(scope);
	}
	hxs_current_memory_allocator = previous_id;
}

size_t hxmemory_manager::get_allocation_count(hxsystem_allocator_t id) {
	hxassertmsg(id >= 0 && id < (hxsystem_allocator_stack_0
		+ static_cast<hxsystem_allocator_t>(m_stack_count)),
		"bad_arg %d", static_cast<int>(id));
	if(id >= hxsystem_allocator_stack_0) {
		return m_memory_allocator_stacks[id - hxsystem_allocator_stack_0].get_allocation_count();
	}
#if (HX_MEMORY_BUDGET_PERMANENT) != 0
	if(id == hxsystem_allocator_permanent) {
		return m_memory_allocator_permanent.get_allocation_count();
	}
#endif
	return m_memory_allocator_heap.get_allocation_count();
}

size_t hxmemory_manager::get_bytes_allocated(hxsystem_allocator_t id) {
	hxassertmsg(id >= 0 && id < (hxsystem_allocator_stack_0
		+ static_cast<hxsystem_allocator_t>(m_stack_count)),
		"bad_arg %d", static_cast<int>(id));
	if(id >= hxsystem_allocator_stack_0) {
		return m_memory_allocator_stacks[id - hxsystem_allocator_stack_0].get_bytes_allocated();
	}
#if (HX_MEMORY_BUDGET_PERMANENT) != 0
	if(id == hxsystem_allocator_permanent) {
		return m_memory_allocator_permanent.get_bytes_allocated();
	}
#endif
	return m_memory_allocator_heap.get_bytes_allocated();
}

template<typename allocator_t_>
hxattr_cold static void hxmemory_allocator_utilization(allocator_t_& allocator,
		hxmemory_manager_stats& stats, bool log) {
	const size_t allocation_count = allocator.get_allocation_count();
	const size_t bytes_allocated = allocator.get_bytes_allocated();
	if(log || allocation_count != 0u) {
		hxlog("allocator %s count %zu size %zu high_water %zu\n",
			allocator.label(), allocation_count, bytes_allocated,
			allocator.get_high_water());
	}
	stats.allocations_outstanding += allocation_count;
	stats.bytes_outstanding += bytes_allocated;
}

hxmemory_manager_stats hxmemory_manager::utilization(bool stacks_only, bool log) {
	hxmemory_manager_stats stats = { 0u, 0u, m_allocator_overflows };
	HX_MEMORY_MANAGER_LOCK_();
	if(!stacks_only) {
		hxmemory_allocator_utilization(m_memory_allocator_heap, stats, log);
#if (HX_MEMORY_BUDGET_PERMANENT) != 0
		hxmemory_allocator_utilization(m_memory_allocator_permanent, stats, log);
#endif
	}
	for(size_t i = 0u; i != m_stack_count; ++i) {
		hxmemory_allocator_utilization(m_memory_allocator_stacks[i], stats, log);
	}
	if(log) {
		hxlog("overflows %zu\n", m_allocator_overflows);
	}
	return stats;
}

// -- hxsystem_allocator_scope -------------------------------------------------
hxattr_noexcept hxsystem_allocator_scope::hxsystem_allocator_scope(hxsystem_allocator_t id) {
	hxinit(); // GCOVR_EXCL_LINE
	m_this_allocator_ = id;
	m_initial_allocator_ = hxs_memory_manager.begin_allocation_scope(this, id);
}

hxattr_noexcept hxsystem_allocator_scope::~hxsystem_allocator_scope(void) {
	hxs_memory_manager.end_allocation_scope(this, m_initial_allocator_);
}

size_t hxsystem_allocator_scope::get_current_allocation_count(void) const {
	hxinit(); // GCOVR_EXCL_LINE
	return hxs_memory_manager.get_allocation_count(m_this_allocator_);
}

size_t hxsystem_allocator_scope::get_current_bytes_allocated(void) const {
	hxinit(); // GCOVR_EXCL_LINE
	return hxs_memory_manager.get_bytes_allocated(m_this_allocator_);
}

void hxmemory_manager_init_(void) {
	// This library is not designed to be reinitialized.
	hxassertmsg(!hxg_init_ver_, "memory_manager reinitialized");
	hxs_memory_manager.construct();
}

void hxmemory_manager_shut_down_(void) {
	// Any allocations made while active will crash when freed. If these are not
	// fixed you will hit a leak sanitizer elsewhere.
	const size_t leaks = hxs_memory_manager.utilization(false, false).allocations_outstanding;
	hxassert_hard(leaks == 0u, "memory_leak at shutdown %zu", leaks); (void)leaks;

	// Return everything to the system allocator.
	hxs_memory_manager.destruct();
}

void hxmemory_manager_allocate_stacks(const size_t* sizes_, size_t stack_count_) {
	hxinit(); // GCOVR_EXCL_LINE
	hxs_memory_manager.allocate_stacks(sizes_, stack_count_);
}

hxmemory_manager_stats hxmemory_manager_utilization(bool stacks_only, bool log) {
	hxinit(); // GCOVR_EXCL_LINE
	return hxs_memory_manager.utilization(stacks_only, log);
}

HX_NS_END_

// -- C API --------------------------------------------------------------------
extern "C"
hxattr_noexcept void* hxmalloc(size_t size) {
	return hxmalloc_ext(size, hxsystem_allocator_current, hxalignment);
}

extern "C"
hxattr_noexcept void* hxmalloc_ext(size_t size, hxsystem_allocator_t id, hxalignment_t alignment) {
	hxinit(); // GCOVR_EXCL_LINE
	void* ptr = HX_NS_PREFIX_ hxs_memory_manager.allocate(size, id, alignment);
#if (HX_HARDENING_MODE) == HX_HARDENING_MODE_DEBUG
	::memset(ptr, 0xcd, size);
#endif
	return ptr;
}

extern "C"
hxattr_noexcept void hxfree(void *ptr) {
	hxinit(); // GCOVR_EXCL_LINE

	// Nothing allocated from the OS memory manager can be freed here unless it is
	// wrapped with hxmemory_allocator_os_heap.
	HX_NS_PREFIX_ hxs_memory_manager.free(ptr);
}

// -- Memory Manager disabled --------------------------------------------------
#else // !HX_USE_MEMORY_MANAGER

HX_NS_BEGIN_

hxattr_noexcept hxsystem_allocator_scope::hxsystem_allocator_scope(hxsystem_allocator_t) { }

hxattr_noexcept hxsystem_allocator_scope::~hxsystem_allocator_scope(void) { }

size_t hxsystem_allocator_scope::get_current_allocation_count(void) const { return 0; }

size_t hxsystem_allocator_scope::get_current_bytes_allocated(void) const { return 0; }

void hxmemory_manager_init_(void) { }

void hxmemory_manager_shut_down_(void) { }

void hxmemory_manager_allocate_stacks(const size_t*, size_t) { }

hxmemory_manager_stats hxmemory_manager_utilization(bool, bool) { return { }; }

HX_NS_END_

extern "C"
hxattr_noexcept void* hxmalloc(size_t size) {
	return hxmalloc_checked_(size);
}

// No support for special alignments when disabled. This is enough for WASM.
extern "C"
hxattr_noexcept void* hxmalloc_ext(size_t size, hxsystem_allocator_t id, hxalignment_t alignment) {
	(void)id; (void)alignment;
	hxassertmsg(alignment <= hxalignment, "bad_align alignment disabled %#zx",
		static_cast<size_t>(alignment));
	return hxmalloc_checked_(size);
}

extern "C"
hxattr_noexcept void hxfree(void *ptr) {
	::free(ptr);
}

#endif // !HX_USE_MEMORY_MANAGER

extern "C"
hxattr_noexcept char* hxstring_duplicate(const char* string, hxsystem_allocator_t id) {
	const size_t len = ::strlen(string);
	char* temp = static_cast<char*>(hxmalloc_ext(len + 1, id, 1u));
	::memcpy(temp, string, len + 1);
	return temp;
}
