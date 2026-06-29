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
// GCOVR_EXCL_START
// It is unspecified which of these functions is called.
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
inline void hxsystem_allocator_scope_init_(hxsystem_allocator_scope* scope,
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
static_assert(hxsystem_allocator_heap == 0, "hxsystem_allocator_heap must be 0 for thread-local default");
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
// Clang emits inline constructors of anonymous namespace types into module
// consumers. Out-of-line constructors prevent this.
class hxmemory_allocator_base {
public:
	hxmemory_allocator_base() : m_label_(hxnull) { }
	hxattr_hot void* allocate(size_t size, hxalignment_t alignment) {
		return on_alloc(size, alignment);
	}

	virtual void begin_allocation_scope(hxsystem_allocator_scope* scope,
		hxsystem_allocator_t new_id) = 0;
	virtual void end_allocation_scope(hxsystem_allocator_scope* scope,
		hxsystem_allocator_t old_id) = 0;
	virtual size_t get_allocation_count(hxsystem_allocator_t id) const = 0;
	virtual size_t get_bytes_allocated(hxsystem_allocator_t id) const = 0;
	virtual size_t get_high_water(hxsystem_allocator_t id) = 0;
	const char* label(void) const { return m_label_; }

protected:
	virtual void* on_alloc(size_t size, hxalignment_t alignment) = 0;
	const char* m_label_;
private:
	void operator=(const hxmemory_allocator_base&) = delete;
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

	void begin_allocation_scope(hxsystem_allocator_scope* scope,
		hxsystem_allocator_t new_id) override {
		(void)scope; (void)new_id;
		hxsystem_allocator_scope_init_(scope, m_allocation_count, m_bytes_allocated);
	}
	void end_allocation_scope(hxsystem_allocator_scope* scope,
		hxsystem_allocator_t old_id) override { (void)scope; (void)old_id; }
	size_t get_allocation_count(hxsystem_allocator_t id) const override {
		(void)id; return m_allocation_count;
	}
	size_t get_bytes_allocated(hxsystem_allocator_t id) const override {
		(void)id; return m_bytes_allocated;
	}
	size_t get_high_water(hxsystem_allocator_t id) override {
		(void)id; return m_high_water;
	}

	hxattr_hot void* on_alloc(size_t size, hxalignment_t alignment) override {
#if HX_USE_STD_ALIGNED_ALLOC
		// C11 aligned_alloc requires alignment ≥ sizeof(void*) on some platforms.
		alignment = hxmax(alignment, hxalignment);
		const size_t alignment_mask = static_cast<size_t>(alignment) - 1u;

		// Round up size to a multiple of alignment as required by aligned_alloc.
		// Treat size 0 as 1 so aligned_alloc never receives size 0 (implementation-defined).
		const size_t rounded = hxmax<size_t>((size + alignment_mask) & ~alignment_mask, alignment);
		hxassertmsg(rounded >= size, "allocation_error size overflow %zu", size);

		void* t = ::aligned_alloc(alignment, rounded);
		hxassert_always(t, "aligned_alloc %zu %zu", static_cast<size_t>(alignment), rounded);
		++m_allocation_count;
		// Bytes are not tracked because there is no allocation header to examine on free.
		return t;
#else
		const size_t alignment_mask = static_cast<size_t>(alignment) - 1u;

		// Place header immediately before aligned allocation. malloc is aligned.
		const size_t total = size + sizeof(hxmemory_allocation_header) + alignment_mask;
		hxassertmsg(total > size, "allocation_error size overflow %zu", size);
		const uintptr_t actual = reinterpret_cast<uintptr_t>(hxmalloc_checked_(total));
		const uintptr_t aligned = (actual + sizeof(hxmemory_allocation_header) + alignment_mask) & ~alignment_mask;
		hxmemory_allocation_header& hdr = reinterpret_cast<hxmemory_allocation_header*>(aligned)[-1];
		hdr.size = size;
		hdr.actual = actual;
		++m_allocation_count;
		m_bytes_allocated += size; // Ignore overhead.
		m_high_water = hxmax(m_high_water, m_bytes_allocated);

		return reinterpret_cast<void*>(aligned);
#endif
	}

	hxattr_hot void on_free_non_virtual(void* ptr) {
#if HX_USE_STD_ALIGNED_ALLOC
		hxassertmsg(m_allocation_count > 0u, "bad_free nothing allocated");
		--m_allocation_count;
		::free(ptr);
#else

		const hxmemory_allocation_header& hdr = reinterpret_cast<hxmemory_allocation_header*>(ptr)[-1];
		hxassertmsg(m_allocation_count > 0u, "bad_free sentinel corrupt");
		hxassertmsg(hdr.size <= m_bytes_allocated, "bad_free sentinel corrupt");
		--m_allocation_count;
		m_bytes_allocated -= hdr.size;

		const uintptr_t actual = hdr.actual;
#if (HX_HARDENING_MODE) == HX_HARDENING_MODE_DEBUG
		::memset(ptr, 0xdd, hdr.size);
#endif
		::free(reinterpret_cast<void*>(actual));
#endif
	}

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

	void begin_allocation_scope(hxsystem_allocator_scope* scope,
		hxsystem_allocator_t new_id) override {
			(void)scope; (void)new_id;
			hxsystem_allocator_scope_init_(scope, m_allocation_count, m_current - m_begin_);
		}
	void end_allocation_scope(hxsystem_allocator_scope* scope,
		hxsystem_allocator_t old_id) override { (void)scope; (void)old_id; }
	bool contains(void* ptr) const {
		const uintptr_t ptr_value = reinterpret_cast<uintptr_t>(ptr);
		return ptr_value >= m_begin_ && ptr_value < m_end_;
	}
	size_t get_allocation_count(hxsystem_allocator_t id) const override {
		(void)id; return m_allocation_count;
	}
	size_t get_bytes_allocated(hxsystem_allocator_t id) const override {
		(void)id; return m_current - m_begin_;
	}
	size_t get_high_water(hxsystem_allocator_t id) override {
		(void)id; return m_current - m_begin_;
	}

	hxattr_cold void* release(void) const {
		// Don't waste space on defensive code.
		return reinterpret_cast<void*>(m_begin_);
	}

	hxattr_hot void* allocate_non_virtual(size_t size, hxalignment_t alignment) {
		const uintptr_t alignment_mask = static_cast<uintptr_t>(alignment - 1u);
		const uintptr_t aligned = (m_current + alignment_mask) & ~alignment_mask;
		// Assume size_t doesn't wrap here.
		if((aligned + size) > m_end_) {
			return hxnull;
		}

		++m_allocation_count;
		m_current = aligned + size;
		return reinterpret_cast<void*>(aligned);
	}

	hxattr_hot void on_free_non_virtual(void* ptr) {
		// Use ≤ because a valid outstanding allocation of size 0 could have
		// been made at m_current.
		const uintptr_t ptr_value = reinterpret_cast<uintptr_t>(ptr);
		hxassertmsg(m_allocation_count > 0 && ptr_value >= m_begin_
			&& ptr_value <= m_current, "bad_free %s", m_label_);
		--m_allocation_count; (void)ptr_value;
	}

protected:
	hxattr_hot void* on_alloc(size_t size, hxalignment_t alignment) override {
		return allocate_non_virtual(size, alignment);
	}

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

	void begin_allocation_scope(hxsystem_allocator_scope* scope,
			hxsystem_allocator_t new_id) override {
#if (HX_HARDENING_MODE) == HX_HARDENING_MODE_DEBUG
		hxassertmsg(m_owner_thread == 0u || m_owner_thread == hxthread_id(),
			"temp_stack cross-thread scope %s", m_label_);
		m_owner_thread = hxthread_id();
		++m_scope_depth;
#endif
		hxmemory_allocator_stack::begin_allocation_scope(scope, new_id);
	}

	hxattr_hot void end_allocation_scope(hxsystem_allocator_scope* scope,
			hxsystem_allocator_t old_id) override {
#if (HX_HARDENING_MODE) == HX_HARDENING_MODE_DEBUG
		hxassertmsg(m_owner_thread == hxthread_id(),
			"temp_stack cross-thread scope %s", m_label_);
		if(--m_scope_depth == 0u) {
			m_owner_thread = 0u;
		}
#endif
		(void)old_id;
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

	hxattr_hot size_t get_high_water(hxsystem_allocator_t id) override {
		(void)id;
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
	hxattr_cold hxmemory_manager_stats utilization(bool stacks_only, bool log);

	hxattr_hot hxsystem_allocator_t begin_allocation_scope(hxsystem_allocator_scope* scope,
		hxsystem_allocator_t new_id);
	hxattr_hot void end_allocation_scope(hxsystem_allocator_scope* scope,
		hxsystem_allocator_t previous_id);

	hxattr_hot hxmemory_allocator_base& get_allocator(hxsystem_allocator_t id) {
		hxassertmsg(id >= 0 && id < (hxsystem_allocator_stack_0 + static_cast<hxsystem_allocator_t>(m_stack_count)),
			"invalid_parameter %d", static_cast<int>(id));
		return *m_memory_allocators[id]; // NOLINT(clang-analyzer-security.ArrayBound)
	}

	hxattr_hot void* allocate(size_t size, hxsystem_allocator_t id, hxalignment_t alignment);
	hxattr_hot void free(void* ptr);

private:
	friend class hxsystem_allocator_scope;

	static constexpr int hxs_allocator_slot_count_ = hxsystem_allocator_stack_0 + HX_MEMORY_MAX_STACKS;

	hxmemory_allocator_base*      m_memory_allocators[hxs_allocator_slot_count_];

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
	m_memory_allocators[hxsystem_allocator_heap] = &m_memory_allocator_heap;

	::new(&m_memory_allocator_heap) hxmemory_allocator_os_heap(); // Set vtable pointer.

	m_memory_allocator_heap.construct("heap");

#if (HX_MEMORY_BUDGET_PERMANENT) != 0
	m_memory_allocators[hxsystem_allocator_permanent] = &m_memory_allocator_permanent;
	::new(&m_memory_allocator_permanent) hxmemory_allocator_stack();
	m_memory_allocator_permanent.construct(hxmalloc_checked_(HX_MEMORY_BUDGET_PERMANENT),
		(HX_MEMORY_BUDGET_PERMANENT), "perm");
#else
	// Without a permanent budget, route permanent allocations to the heap.
	m_memory_allocators[hxsystem_allocator_permanent] = &m_memory_allocator_heap;
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
		hxmemory_allocator_temp_stack& stack = m_memory_allocator_stacks[i]; // NOLINT(clang-analyzer-security.ArrayBound)
		::new(&stack) hxmemory_allocator_temp_stack();
		stack.construct(hxmalloc_checked_(sizes[i]), sizes[i], "temp");
		m_memory_allocators[hxsystem_allocator_stack_0 + i] = &stack;
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

hxmemory_manager_stats hxmemory_manager::utilization(bool stacks_only, bool log) {
	hxmemory_manager_stats stats = { 0u, 0u, m_allocator_overflows };
	HX_MEMORY_MANAGER_LOCK_();
	const size_t first_slot = stacks_only ? static_cast<size_t>(hxsystem_allocator_stack_0) : 0u;
	const size_t slot_count = static_cast<size_t>(hxsystem_allocator_stack_0) + m_stack_count;
	for(size_t i = first_slot; i != slot_count; ++i) {
#if (HX_MEMORY_BUDGET_PERMANENT) == 0
		// The permanent slot aliases the heap when there is no permanent budget.
		if(i == static_cast<size_t>(hxsystem_allocator_permanent)) { continue; }
#endif
		hxmemory_allocator_base& allocator = *m_memory_allocators[i];
		const hxsystem_allocator_t allocator_id = static_cast<hxsystem_allocator_t>(i);
		if(log || allocator.get_allocation_count(allocator_id) != 0u) {
			hxlog("allocator %s count %zu size %zu high_water %zu\n",
				allocator.label(),
				allocator.get_allocation_count(allocator_id),
				allocator.get_bytes_allocated(allocator_id),
				allocator.get_high_water(allocator_id));
		}
		stats.allocations_outstanding += allocator.get_allocation_count(allocator_id);
		stats.bytes_outstanding += allocator.get_bytes_allocated(allocator_id);
	}
	if(log) {
		hxlog("overflows %zu\n", m_allocator_overflows);
	}
	return stats;
}

hxsystem_allocator_t hxmemory_manager::begin_allocation_scope(
		hxsystem_allocator_scope* scope, hxsystem_allocator_t new_id) {
	HX_MEMORY_MANAGER_LOCK_();
	const hxsystem_allocator_t previous_id = hxs_current_memory_allocator;
	hxs_current_memory_allocator = new_id;
	get_allocator(hxs_current_memory_allocator).begin_allocation_scope(
		scope, hxs_current_memory_allocator);
	return previous_id;
}

void hxmemory_manager::end_allocation_scope(
		hxsystem_allocator_scope* scope, hxsystem_allocator_t previous_id) {
	HX_MEMORY_MANAGER_LOCK_();
	get_allocator(hxs_current_memory_allocator).end_allocation_scope(
		scope, hxs_current_memory_allocator);
	hxs_current_memory_allocator = previous_id;
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
	hxassertmsg(size != 0u, "allocation_error Size 0 allocation");

	// Provide an alignment of 1 for strings and unaligned allocations. The
	// following code assumes that "alignment-1" is a valid mask of unused bits
	// and not a mask containing every bit.
	hxassertmsg(alignment != 0u, "alignment_error Allocate with alignment 1 and not 0");
	hxassertmsg(((alignment - 1u) & alignment) == 0u,
		"alignment_error Not pow2 %zu.", static_cast<size_t>(alignment));

	HX_MEMORY_MANAGER_LOCK_();
	void* ptr = get_allocator(id).allocate(size, alignment);

	const uintptr_t alignment_mask = static_cast<uintptr_t>(alignment) - 1u;
	const uintptr_t ptr_value = reinterpret_cast<uintptr_t>(ptr);
	hxassertmsg((ptr_value & alignment_mask) == 0,
		"alignment_error wrong %zx from %d", static_cast<size_t>(ptr_value), id);
	 (void)alignment_mask; (void)ptr_value;

	if(ptr != hxnull) { return ptr; }

	// Will not return null.
	++m_allocator_overflows;
	hxlog_warning("allocation_error overflowing %s size %zu", get_allocator(id).label(), size);
	return m_memory_allocator_heap.allocate(size, alignment);
}

void hxmemory_manager::free(void* ptr) {
	if(ptr == hxnull) {
		return;
	}

	// This path is hard-coded for efficiency.
	HX_MEMORY_MANAGER_LOCK_();

	for(size_t i = 0u; i != m_stack_count; ++i) {
		hxmemory_allocator_temp_stack& stack = m_memory_allocator_stacks[i];
		if(stack.contains(ptr)) {
			stack.on_free_non_virtual(ptr);
			return;
		}
	}

#if (HX_MEMORY_BUDGET_PERMANENT) != 0
	if(m_memory_allocator_permanent.contains(ptr)) {
		hxwarn_msg(hxg_settings.deallocate_permanent, "ERROR: free from permanent");
		m_memory_allocator_permanent.on_free_non_virtual(ptr);
		return;
	}
#endif

	m_memory_allocator_heap.on_free_non_virtual(ptr);
}

// -- hxsystem_allocator_scope -------------------------------------------------
hxattr_noexcept hxsystem_allocator_scope::hxsystem_allocator_scope(hxsystem_allocator_t id) {
	hxinit();
	m_this_allocator_ = id;
	m_initial_allocator_ = hxs_memory_manager.begin_allocation_scope(this, id);
}

hxattr_noexcept hxsystem_allocator_scope::~hxsystem_allocator_scope(void) {
	hxs_memory_manager.end_allocation_scope(this, m_initial_allocator_);
}

size_t hxsystem_allocator_scope::get_current_allocation_count(void) const {
	hxinit();
	return hxs_memory_manager.get_allocator(m_this_allocator_).get_allocation_count(m_this_allocator_);
}

size_t hxsystem_allocator_scope::get_current_bytes_allocated(void) const {
	hxinit();
	return hxs_memory_manager.get_allocator(m_this_allocator_).get_bytes_allocated(m_this_allocator_);
}

void hxmemory_manager_init_(void) {
	// This library is not designed to be reinitialized.
	hxassertmsg(!hxg_init_ver_, "memory_manager Reinitialized");
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
	hxinit();
	hxs_memory_manager.allocate_stacks(sizes_, stack_count_);
}

hxmemory_manager_stats hxmemory_manager_utilization(bool stacks_only, bool log) {
	hxinit();
	return hxs_memory_manager.utilization(stacks_only, log);
}

HX_NS_END_

// -- C API --------------------------------------------------------------------
extern "C"
hxattr_noexcept void* hxmalloc(size_t size) {
	return  hxmalloc_ext(size, hxsystem_allocator_current, hxalignment);
}

extern "C"
hxattr_noexcept void* hxmalloc_ext(size_t size, hxsystem_allocator_t id, hxalignment_t alignment) {
	hxinit();
	void* ptr = HX_NS_PREFIX_ hxs_memory_manager.allocate(size, id, alignment);
#if (HX_HARDENING_MODE) == HX_HARDENING_MODE_DEBUG
		::memset(ptr, 0xcd, size);
#endif
	return ptr;
}

extern "C"
hxattr_noexcept void hxfree(void *ptr) {
	hxinit();

	// Nothing allocated from the OS memory manager can be freed here unless it is
	// wrapped with hxmemory_allocator_os_heap.
	HX_NS_PREFIX_ hxs_memory_manager.free(ptr);
}

// -- Memory manager disabled --------------------------------------------------
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
	hxassertmsg(alignment <= hxalignment, "alignment_error Memory manager disabled: %zu",
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
	const size_t len = strlen(string);
	char* temp = static_cast<char*>(hxmalloc_ext(len + 1, id, 1u));
	::memcpy(temp, string, len + 1);
	return temp;
}
