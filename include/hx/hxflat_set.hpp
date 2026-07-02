#pragma once
// SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
// SPDX-License-Identifier: MIT
// This file is licensed under the MIT license found in the LICENSE.md file.

/// \file
/// A sorted flat set and multiset backed by an array of keys.

#include "libhatchet.h"

// HX_USE_MACROS_WITH_MODULE allows including macros alongside the module.
#if HX_USE_MACROS_WITH_MODULE
#error Header does not provide macros alone.
#endif

#include "hxallocator.hpp"
#include "hxkey.hpp"
#include "hxsort.hpp"

HX_NS_BEGIN_

/// `hxflat_set` - A sorted associative container that stores keys in a single
/// array. Lookup is Θ(log n) via binary search. Insert and erase are Θ(n)
/// because elements are shifted to maintain order. This design keeps keys
/// cache-friendly and avoids heap overhead per element.
///
/// When `multi_t` is `false` duplicate keys are rejected and `insert` returns a
/// pointer to the existing element. When `multi_t` is `true` duplicate keys are
/// always inserted. `compare_t` must be a callable with signature `bool(const
/// key_t_&, const key_t_&)` returning true when the first argument is ordered
/// before the second. It defaults to `hxkey_less_t`.
///
/// When `capacity` is `hxallocator_dynamic_capacity` storage must be allocated
/// by calling `reserve` before inserting elements. Otherwise the array is
/// statically sized to `capacity` elements and `reserve` may only be called
/// with exactly that value. `extract()` is not provided, just use `data()`.
///
/// E.g.:
/// ```
/// // A static flat set of 64 integers.
/// hxflat_set<int, hxkey_less_t<int>, false, 64> lookup;
/// ```
/// - `key_t` : Key type.
/// - `compare_t` : Callable implementing a strict weak order on `key_t`.
/// - `multi_t` : When `true` duplicate keys are allowed.
/// - `capacity` : Fixed element count or `hxallocator_dynamic_capacity`.
template<typename key_t_,
	typename compare_t_=hxkey_less_t<key_t_>,
	bool multi_t_=true,
	hxsize_t capacity_=hxallocator_dynamic_capacity>
class hxflat_set : private hxallocator<key_t_, capacity_> {
public:
	using key_t = key_t_;
	using compare_t = compare_t_;

	/// `iterator` - Random access iterator over keys.
	using iterator = const key_t_*;

	/// `const_iterator` - Const random access iterator over keys.
	using const_iterator = const key_t_*;

	/// Constructs an empty set. Requires `reserve` before inserting when
	/// `capacity` is `hxallocator_dynamic_capacity`.
	explicit hxflat_set(void);

	/// Copy constructs from another `hxflat_set`. Requires `x.size()` ≤
	/// `capacity()`.
	/// - `x` : A non-temporary `hxflat_set<key_t, compare_t, multi_t, capacity>`.
	hxflat_set(const hxflat_set& x_) noexcept;

	/// Move constructs from a temporary `hxflat_set`. Requires
	/// `hxallocator_dynamic_capacity`.
	/// - `x` : A temporary `hxflat_set<key_t, compare_t, multi_t, hxallocator_dynamic_capacity>`.
	hxflat_set(hxflat_set&& x_) noexcept;

	/// Destructs the set and destroys all keys.
	~hxflat_set(void) noexcept;

	/// Assigns the contents of `x` to this set. Clears this set then copies all
	/// elements from `x`. Requires `x.size()` ≤ `capacity()`.
	/// - `x` : The set to copy from.
	void operator=(const hxflat_set& x_) noexcept;

	/// Cross-capacity copy assignment. Assigns the contents of `x` to this
	/// set. Requires `x.size()` ≤ `capacity()`.
	/// - `x` : The set to copy from.
	template<hxsize_t capacity_x_>
	void operator=(const hxflat_set<key_t_, compare_t_, multi_t_, capacity_x_>& x_) noexcept;

	/// Move assigns from a temporary set using `swap`. Requires
	/// `hxallocator_dynamic_capacity`.
	/// - `x` : A temporary set to move from.
	void operator=(hxflat_set&& x_) noexcept;

	/// Returns a const pointer to the element at `index`. Requires
	/// `index < size()`.
	/// - `index` : The 0-based position of the element.
	hxattr_nodiscard const key_t_* operator[](hxsize_t index_) const;

	/// Returns a const pointer to the first element.
	const key_t_* begin(void) const { return this->data(); }

	/// Returns the capacity of the set or 0 if unallocated.
	hxattr_nodiscard hxsize_t capacity(void) const;

	/// Returns a const pointer to the first element (alias for `begin`).
	const key_t_* cbegin(void) const { return this->data(); }

	/// Returns a const pointer past the last element.
	const key_t_* cend(void) const { return m_end_; }

	/// Removes all elements and destroys their keys.
	void clear(void) noexcept;

	/// Returns the number of elements with the given key.
	/// - `key` : The key to count.
	hxattr_nodiscard hxsize_t count(const key_t_& key_) const;

	/// Returns a pointer to a potentially uninitialized array of `key_t`.
	using hxallocator<key_t_, capacity_>::data;

	/// Checks if the set contains no elements.
	hxattr_nodiscard bool empty(void) const { return m_end_ == this->data(); }

	/// Returns a const pointer past the last element.
	const key_t_* end(void) const { return m_end_; }

	/// Returns `true` if this set and `x` contain the same keys in the same
	/// order using `hxkey_equal`.
	/// - `x` : The set to compare against.
	template<hxsize_t capacity_x_>
	hxattr_nodiscard bool equal(const hxflat_set<key_t_, compare_t_, multi_t_, capacity_x_>& x_) const;

	/// Removes all elements with the given key and destroys their keys. Returns
	/// the number of elements removed.
	/// - `key` : The key to search for and remove.
	hxsize_t erase(const key_t_& key_) noexcept;

	/// Removes the element at the pointer position and destroys the key.
	/// Returns a const pointer to the element that followed the erased one.
	/// - `pos` : A const pointer to an element in this set.
	const key_t_* erase(const key_t_* pos_) noexcept;

	/// Returns a const pointer to the key if found, or `hxnull` if not present.
	/// When `multi_t` is `true` the first match is returned.
	/// - `key` : The key to search for.
	hxattr_nodiscard const key_t_* find(const key_t_& key_) const;

	/// Returns `true` if the set has reached its capacity.
	hxattr_nodiscard bool full(void) const { return m_end_ == this->data() + this->capacity(); }

	/// Inserts a key. When `multi_t` is `false` and a matching key already
	/// exists, returns a const pointer to the existing element without
	/// inserting. Otherwise inserts in sorted order and returns a const pointer
	/// to the new element.
	/// - `key` : The key to insert.
	const key_t_* insert(const key_t_& key_) noexcept;

	/// Move overload of `insert`.
	/// - `key` : The key forwarded into storage.
	const key_t_* insert(key_t_&& key_) noexcept;

	/// Returns `true` if this set compares less than `x` lexicographically,
	/// using `hxkey_equal` and `hxkey_less` on keys.
	/// - `x` : The set to compare against.
	template<hxsize_t capacity_x_>
	hxattr_nodiscard bool less(const hxflat_set<key_t_, compare_t_, multi_t_, capacity_x_>& x_) const;

	/// Returns a const pointer to the first element whose key is not ordered
	/// before `key`. Returns `end` if no such element exists.
	/// - `key` : The key to search for.
	hxattr_nodiscard const key_t_* lower_bound(const key_t_& key_) const;

	/// Returns the capacity of the set.
	hxattr_nodiscard hxsize_t max_size(void) const { return this->capacity(); }

	/// Allocates storage for `cap` keys. When `capacity` is fixed, `cap` must
	/// equal `capacity`. Reallocation is not allowed.
	/// - `cap` : The number of elements to allocate storage for.
	/// - `allocator` : The memory manager ID to use for allocation (default: `hxsystem_allocator_current`)
	/// - `alignment` : The alignment for the allocation. (default: `hxalignment`)
	void reserve(hxsize_t cap_,
			hxsystem_allocator_t allocator_=hxsystem_allocator_current,
			hxalignment_t alignment_=hxalignment);

	/// Returns the number of elements in the set.
	hxattr_nodiscard hxsize_t size(void) const { return m_end_ - this->data(); }

	/// Swaps the contents with `x`. Requires `hxallocator_dynamic_capacity`.
	/// - `x` : The set to swap with.
	void swap(hxflat_set& x_) noexcept;

	/// Returns a const pointer to the first element whose key is ordered after
	/// `key`. Returns `end` if no such element exists.
	/// - `key` : The key to search for.
	hxattr_nodiscard const key_t_* upper_bound(const key_t_& key_) const;

private:
	/// \cond HIDDEN
	template<typename, typename, bool, hxsize_t> friend class hxflat_set;

	template<typename key_u_>
	const key_t_* insert_at_(key_t_* pos_, key_u_&& key_) noexcept;

	key_t_* m_end_;
	/// \endcond
};

// Without the "requires" keyword these end up being ambiguous.
#if HX_CPLUSPLUS >= 202002L
/// `bool hxkey_equal(hxflat_set<K>& x, hxflat_set<K>& y)` - Compares the
/// contents of `x` and `y` for equivalence.
template<typename key_t_, typename compare_t_, bool multi_t_,
	hxsize_t capacity_x_, hxsize_t capacity_y_> bool hxkey_equal(
		const hxflat_set<key_t_, compare_t_, multi_t_, capacity_x_>& x_,
		const hxflat_set<key_t_, compare_t_, multi_t_, capacity_y_>& y_) {
	return x_.equal(y_);
}

/// `bool hxkey_less(hxflat_set<K>& x, hxflat_set<K>& y)` - Compares the
/// contents of `x` and `y` lexicographically using `hxkey_equal` and
/// `hxkey_less` on each element.
template<typename key_t_, typename compare_t_, bool multi_t_,
	hxsize_t capacity_x_, hxsize_t capacity_y_> bool hxkey_less(
		const hxflat_set<key_t_, compare_t_, multi_t_, capacity_x_>& x_,
		const hxflat_set<key_t_, compare_t_, multi_t_, capacity_y_>& y_) {
	return x_.less(y_);
}

/// `void hxswap(hxflat_set<K>& x, hxflat_set<K>& y)` - Exchanges the contents
/// of `x` and `y`. Only works with `hxallocator_dynamic_capacity`. Dynamically
/// allocated arrays are swapped with very little overhead.
template<typename key_t_, typename compare_t_, bool multi_t_> void hxswap(
		hxflat_set<key_t_, compare_t_, multi_t_, hxallocator_dynamic_capacity>& x_,
		hxflat_set<key_t_, compare_t_, multi_t_, hxallocator_dynamic_capacity>& y_) noexcept {
	x_.swap(y_);
}

#endif // HX_CPLUSPLUS >= 202002L
#include "detail/hxflat_set.inl"
HX_NS_END_
