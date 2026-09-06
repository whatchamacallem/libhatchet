#pragma once
// SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
// SPDX-License-Identifier: MIT
// This file is licensed under the MIT license found in the LICENSE.md file.

/// \file
/// Implements a fixed-capacity free list allocator backed by `hxallocator`.

#include "libhatchet.h"

// HX_USE_MACROS_WITH_MODULE allows including macros alongside the module.
#if HX_USE_MACROS_WITH_MODULE
#error Header does not provide macros alone.
#endif

#include "hxallocator.hpp"
#include "hxptr.hpp"

HX_NS_BEGIN_

#if HX_CPLUSPLUS >= 202002L
/// \cond HIDDEN
template<typename T_>
concept hxfree_list_concept_ = requires(T_& x_) {
	sizeof(T_);
	x_.~T_();
};
/// \endcond
#else
#define hxfree_list_concept_ typename
#endif

/// `hxfree_list` - A fixed-capacity pool of `T` managed by a singly linked free
/// list. Once capacity is established (statically or via `reserve`),
/// `allocate` constructs a `T` and returns an `hxptr` owning it. `release`
/// accepts either the `hxptr` returned by `allocate` or a raw pointer,
/// destructs `*p`, and returns its slot to the list.
///
/// When `capacity` is `hxallocator_dynamic_capacity`, storage must be allocated
/// by calling `reserve` before use.
/// - `T` : Element type stored by the free list.
/// - `capacity` : Pool size or `hxallocator_dynamic_capacity`.
template<hxfree_list_concept_ T_, hxsize_t capacity_=hxallocator_dynamic_capacity>
class hxfree_list : private hxallocator<T_, capacity_> {
public:
	/// `deleter_t` - A callable deleter that releases a `T` back to this
	/// `hxfree_list`.
	class deleter_t {
	public:
		/// Constructs a deleter associated with `owner`.
		/// - `owner` : The `hxfree_list` that allocated the pointer.
		explicit deleter_t(hxfree_list& owner_) noexcept : m_owner_(&owner_) { }

		/// Destructs `*p` and returns its slot to the owning `hxfree_list`.
		/// WARNING: Made non-const to indicate deletion is not thread-safe.
		/// - `p` : A pointer previously returned by `allocate` or `hxnull`.
		void operator()(T_* ptr_) noexcept { if(ptr_ != hxnull) { m_owner_->release(ptr_); } }

		/// Always returns true, indicating the deleter is valid.
		operator bool(void) const { return true; }

	private:
		hxfree_list* m_owner_;
	};

	/// `value_t` - Publishes the value type.
	using value_t = T_;

	/// `ptr_t` - Publish the hxptr type.
	using ptr_t = hxptr<T_, deleter_t>;

	/// When `capacity` is `hxallocator_dynamic_capacity` storage must be
	/// allocated via `reserve` before use.
	explicit hxfree_list(void) noexcept;

	/// Move constructs from a temporary `hxfree_list`. Requires
	/// `hxallocator_dynamic_capacity`.
	/// - `x` : A temporary `hxfree_list<T, hxallocator_dynamic_capacity>`.
	hxfree_list(hxfree_list&& x_) noexcept;

	/// All `T` must have been released before destruction.
	~hxfree_list(void) noexcept;

	/// Constructs a `T` in a free slot and returns an `hxptr` owning it. Use
	/// `release()` to transfer to a different hxptr type.
	/// - `args` : Arguments forwarded to `T`'s constructor.
	template<typename... args_t_>
	hxattr_nodiscard hxptr<T_, deleter_t> allocate(args_t_&&... args_) noexcept;

	/// Returns the capacity of the pool or 0 if unallocated.
	hxattr_nodiscard hxsize_t capacity(void) const;

	/// Returns a `deleter_t` associated with this `hxfree_list`.
	deleter_t deleter(void) noexcept { return deleter_t(*this); }

	/// Returns true if no unallocated `T` remains.
	hxattr_nodiscard bool empty(void) const { return m_free_head_ == hxnull; }

	/// Returns true if `p` points into this free list's storage.
	/// - `p` : The pointer to test. May be null.
	hxattr_nodiscard bool is_allocator(const T_* ptr_) const noexcept;

	/// Returns true if `ptr` points into this free list's storage.
	/// - `ptr` : The `hxptr` to test.
	template<typename deleter_t_>
	hxattr_nodiscard bool is_allocator(const hxptr<T_, deleter_t_>& ptr_) const noexcept;

	/// Returns the capacity of the pool.
	hxattr_nodiscard hxsize_t max_size(void) const { return this->capacity(); }

	/// - `ptr` : An `T*` referencing an object previously returned by `allocate`.
	/// - `p` : A non-null pointer previously returned by `allocate`.
	void release(T_* ptr_) noexcept hxattr_nonnull(2);

	/// Destructs `*p` and returns its slot to the free list.
	/// - `ptr` : An `hxptr` referencing an object previously returned by `allocate`.
	template<typename deleter_t_>
	void release(hxptr<T_, deleter_t_>&& ptr_) noexcept;

	/// Allocates storage for `size` slots and enqueues them. Only usable when
	/// `capacity` is `hxallocator_dynamic_capacity`. The current capacity
	/// becomes `size` slots. Reallocation is not allowed.
	/// - `size` : The number of slots to allocate.
	/// - `allocator` : The memory manager ID to use for allocation.
	/// - `alignment` : The alignment to use for the allocation.
	void reserve(hxsize_t size_,
			hxsystem_allocator_t allocator_=hxsystem_allocator_current,
			hxalignment_t alignment_=hxalignment) noexcept;

	/// Returns the number of unallocated `T` available.
	hxattr_nodiscard hxsize_t size(void) const { return m_size_; }

	/// Constructs a `T` in a free slot and returns an `hxptr` owning it.
	/// Returns an empty `hxptr` when no slots remain.
	/// - `args` : Arguments forwarded to `T`'s constructor.
	template<typename... args_t_>
	hxattr_nodiscard hxptr<T_, deleter_t> try_allocate(args_t_&&... args_) noexcept;

private:
	hxfree_list(const hxfree_list&) = delete;
	void operator=(const hxfree_list&) = delete;

	union slot_ {
		slot_* next_;
		T_ value_;
		slot_(void) { }
		~slot_(void) { }
	};

	void enqueue_all_(hxsize_t count_) noexcept;

	slot_* m_free_head_;
	hxsize_t m_size_;
};

#include "detail/hxfree_list.inl"
HX_NS_END_
