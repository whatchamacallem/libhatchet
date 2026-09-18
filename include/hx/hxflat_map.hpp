#pragma once
// SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
// SPDX-License-Identifier: MIT
// This file is licensed under the MIT license found in the LICENSE.md file.

/// \file
/// A sorted flat map and multimap backed by two parallel arrays of keys and
/// values.

#include "libhatchet.h"

// HX_USE_MACROS_WITH_MODULE allows including macros alongside the module.
#if HX_USE_MACROS_WITH_MODULE
#error Header does not provide macros alone.
#endif

#include "hxallocator.hpp"
#include "hxrange.hpp"
#include "hxinitializer_list.hpp"
#include "hxkey.hpp"
#include "hxpair.hpp"
#include "hxsort.hpp"

HX_NS_BEGIN_

#if HX_CPLUSPLUS >= 202002L
/// \cond HIDDEN
template<typename T_>
concept hxflat_map_concept_ = requires(T_& x_) {
	sizeof(T_);
	x_.~T_();
};
/// \endcond
#else
#define hxflat_map_concept_ typename
#endif

/// `hxflat_map_const_value_t` - A proxy referencing the key and mapped value at
/// a fixed position. The key is field `a` and the mapped value is field `b`.
template<typename key_t_, typename mapped_t_>
using hxflat_map_const_value_t = hxpair<const key_t_&, const mapped_t_&>;

/// `hxflat_map_value_t` - A proxy referencing the key and mapped value at a
/// fixed position. The key is field `a` and the mapped value is field `b`.
template<typename key_t_, typename mapped_t_>
using hxflat_map_value_t = hxpair<const key_t_&, mapped_t_&>;

/// `hxflat_map` - A sorted associative container that stores keys and mapped
/// values in two parallel arrays. Lookup is O(log n) via binary search. Insert
/// and erase are O(n) because elements are shifted to maintain order. This
/// design keeps keys and values cache-friendly and avoids heap overhead per
/// element.
///
/// When `traits & hxtrait_multi` is unset duplicate keys are rejected and
/// `insert` returns an iterator to the existing element. When set, duplicate
/// keys are always inserted. `compare_t` defaults to `hxkey_three_way_t`, a
/// callable with signature returning a value less than, equal to, or greater
/// than zero, and `traits` defaults to `hxtrait_three_way`. Passing
/// `hxkey_less_t` and clearing `hxtrait_three_way` in `traits` selects a strict
/// weak order `compare_t` instead, with signature `bool(const key_t_&, const
/// key_t_&)` returning true when the first argument is ordered before the
/// second.
///
/// When `capacity` is `hxallocator_dynamic_capacity` storage must be allocated
/// by calling `reserve` before inserting elements. Otherwise the arrays are
/// statically sized to `capacity` elements and `reserve` may only be called
/// with exactly that value.
///
/// E.g.:
/// ```
/// // A static flat map of 64 integer keys to string pointers.
/// hxflat_map<int, const char*, 64> lookup;
/// ```
/// - `key_t` : Key type.
/// - `T` : Mapped value type.
/// - `capacity` : Fixed element count or `hxallocator_dynamic_capacity`.
/// - `compare_t` : Callable implementing a strict weak order or a three-way
///   comparison on `key_t`, depending on `hxtrait_three_way` in `traits`.
/// - `traits` : A bitmask of `hxtrait_multi` and `hxtrait_three_way`.
template<hxflat_map_concept_ key_t_,
	hxflat_map_concept_ mapped_t_,
	hxsize_t capacity_=hxallocator_dynamic_capacity,
	typename compare_t_=hxkey_three_way_t<key_t_>,
	int traits_=hxtrait_three_way>
class hxflat_map {
public:
	using key_t = key_t_;
	using mapped_t = mapped_t_;
	using compare_t = compare_t_;

	/// `const_iterator` - A random-access iterator over key-value pairs.
	/// Iterators are invalidated by any insert or erase operation.
	class const_iterator {
	public:
		/// Constructs an end iterator.
		const_iterator(void) : m_map_(hxnull), m_index_(0) { }

		/// Returns a `hxflat_map_const_value_t` proxy referencing the key and
		/// value at the current position.
		hxflat_map_const_value_t<key_t_, mapped_t_> operator*(void) const {
			return hxflat_map_const_value_t<key_t_, mapped_t_>{this->key(), this->value()};
		}

		/// Advances the iterator by `n` positions.
		/// - `n` : The number of positions to advance.
		const_iterator& operator+=(hxsize_t n_);

		/// Moves the iterator back by `n` positions.
		/// - `n` : The number of positions to retreat.
		const_iterator& operator-=(hxsize_t n_) { return operator+=(-n_); }

		/// Returns an iterator advanced by `n` positions.
		/// - `n` : The number of positions to advance.
		const_iterator operator+(hxsize_t n_) const { const_iterator t_(*this); t_ += n_; return t_; }

		/// Returns an iterator retreated by `n` positions.
		/// - `n` : The number of positions to retreat.
		const_iterator operator-(hxsize_t n_) const { const_iterator t_(*this); t_ -= n_; return t_; }

		/// Returns the signed distance between two iterators.
		/// - `x` : The iterator to subtract.
		hxsize_t operator-(const const_iterator& x_) const;

		/// Returns a `hxflat_map_const_value_t` proxy referencing the key and
		/// value `n` positions ahead of this one.
		/// - `n` : The offset from this position.
		hxflat_map_const_value_t<key_t_, mapped_t_> operator[](hxsize_t n_) const { return *operator+(n_); }

		/// Advances the iterator to the next element.
		const_iterator& operator++(void) { return operator+=(1); }

		/// Advances the iterator to the next element (post-increment).
		const_iterator operator++(int) { const_iterator t_(*this); operator++(); return t_; }

		/// Retreats the iterator to the previous element.
		const_iterator& operator--(void) { return operator+=(-1); }

		/// Retreats the iterator to the previous element (post-decrement).
		const_iterator operator--(int) { const_iterator t_(*this); operator--(); return t_; }

		/// Compares two iterators for equality.
		/// - `a` : An iterator.
		/// - `b` : The iterator to compare against.
		friend bool operator==(const const_iterator& a_, const const_iterator& b_) {
			hxassertf(a_.m_map_ != hxnull && a_.m_map_ == b_.m_map_, "bad_iter");
			return a_.m_index_ == b_.m_index_;
		}

		/// Returns true when `a` is ordered before `b`.
		/// - `a` : An iterator.
		/// - `b` : The iterator to compare against.
		friend bool operator<(const const_iterator& a_, const const_iterator& b_) {
			hxassertf(a_.m_map_ != hxnull && a_.m_map_ == b_.m_map_, "bad_iter");
			return a_.m_index_ < b_.m_index_;
		}

		/// Returns true when this iterator is ordered after `x`.
		/// - `x` : The iterator to compare against.
		bool operator>(const const_iterator& x_) const { return x_ < *this; }

		/// Returns true when this iterator is not ordered after `x`.
		/// - `x` : The iterator to compare against.
		bool operator<=(const const_iterator& x_) const { return !(x_ < *this); }

		/// Returns true when this iterator is not ordered before `x`.
		/// - `x` : The iterator to compare against.
		bool operator>=(const const_iterator& x_) const { return !(*this < x_); }

#if HX_CPLUSPLUS < 202002L
		/// Compares two iterators for inequality.
		/// - `x` : The iterator to compare against.
		hxattr_nodiscard bool operator!=(const const_iterator& x_) const {
			return !(*this == x_);
		}
#endif

		/// Returns the key at the current position. This is simple so the
		/// compiler errors are simple.
		const key_t_& key(void) const;

		/// Returns the mapped value at the current position. This is simple so
 		/// the compiler errors are simple.
 		const mapped_t_& value(void) const;

	protected:
		/// \cond HIDDEN
		friend class hxflat_map;
		const_iterator(const hxflat_map* map_, hxsize_t index_)
			: m_map_(map_), m_index_(index_) { }
		const hxflat_map* m_map_;
		hxsize_t m_index_;
		/// \endcond
	};

	/// `iterator` - A mutable random-access iterator that allows modifying the
	/// mapped value.
	class iterator : public const_iterator {
	public:
		/// Constructs an end iterator.
		iterator(void) { }

		/// Returns a `hxflat_map_value_t` proxy referencing the key and value
		/// at the current position, allowing mutation of the value.
		hxflat_map_value_t<key_t_, mapped_t_> operator*(void) const {
			return hxflat_map_value_t<key_t_, mapped_t_>{this->key(), this->value()};
		}

		/// Advances the iterator by `n` positions.
		/// - `n` : The number of positions to advance.
		iterator& operator+=(hxsize_t n_) { const_iterator::operator+=(n_); return *this; }

		/// Moves the iterator back by `n` positions.
		/// - `n` : The number of positions to retreat.
		iterator& operator-=(hxsize_t n_) { return operator+=(-n_); }

		/// Returns an iterator advanced by `n` positions.
		/// - `n` : The number of positions to advance.
		iterator operator+(hxsize_t n_) const { iterator t_(*this); t_ += n_; return t_; }

		/// Returns an iterator retreated by `n` positions.
		/// - `n` : The number of positions to retreat.
		iterator operator-(hxsize_t n_) const { iterator t_(*this); t_ -= n_; return t_; }

		/// Returns the signed distance from `x` to this position.
		/// - `x` : The iterator to subtract.
		hxsize_t operator-(const const_iterator& x_) const { return const_iterator::operator-(x_); }

		/// Returns a `hxflat_map_value_t` proxy referencing the key and value
		/// `n` positions ahead of this one.
		/// - `n` : The offset from this position.
		hxflat_map_value_t<key_t_, mapped_t_> operator[](hxsize_t n_) const { return *operator+(n_); }

		/// Advances the iterator to the next element.
		iterator& operator++(void) { const_iterator::operator++(); return *this; }

		/// Advances the iterator to the next element (post-increment).
		iterator operator++(int) { iterator t_(*this); const_iterator::operator++(); return t_; }

		/// Retreats the iterator to the previous element.
		iterator& operator--(void) { const_iterator::operator--(); return *this; }

		/// Retreats the iterator to the previous element (post-decrement).
		iterator operator--(int) { iterator t_(*this); const_iterator::operator--(); return t_; }

		/// Returns the mapped value at the current position, allowing mutation.
		/// This is simple so the compiler errors are simple.
		mapped_t_& value(void) const;

	protected:
		/// \cond HIDDEN
		friend class hxflat_map;
		iterator(hxflat_map* map_, hxsize_t index_) : const_iterator(map_, index_) { }
		iterator(const const_iterator& x_) : const_iterator(x_) { }
		/// \endcond
	};

	/// Constructs an empty map. Requires `reserve` before inserting when
	/// `capacity` is `hxallocator_dynamic_capacity`.
	explicit hxflat_map(void);

	/// Copy constructs from another `hxflat_map`. Requires `x.size()` ≤
	/// `capacity()`.
	/// - `x` : A non-temporary `hxflat_map<key_t, mapped_t, capacity,
	///   compare_t, traits>`.
	hxflat_map(const hxflat_map& x_) noexcept;

	/// Copy constructs from an `hxflat_map` of a different capacity. Requires
	/// `x.size()` <= `capacity()`.
	/// - `x` : A non-temporary `hxflat_map<key_t, mapped_t, capacity_x,
	///   compare_t, traits>`.
	template<hxsize_t capacity_x_>
	hxflat_map(const hxflat_map<key_t_, mapped_t_, capacity_x_, compare_t_,
		traits_>& x_) noexcept;

	/// Move constructs from a temporary `hxflat_map`. Requires
	/// `hxallocator_dynamic_capacity`.
	/// - `x` : A temporary `hxflat_map<key_t, mapped_t,
	///   hxallocator_dynamic_capacity, compare_t, traits>`.
	hxflat_map(hxflat_map&& x_) noexcept;

	/// Constructs a map by appending every key-value pair from `x` with
	/// `add_range`. Requires `x` to be sorted by key and requires `x.size()`
	/// <= `capacity` when `capacity` is fixed. Presort data that may be
	/// unordered, e.g. by copying it into a `hxarray` and calling `sort`.
	/// - `x` : A sorted `std::initializer_list<hxpair<key_t, mapped_t>>`.
	hxflat_map(std::initializer_list<hxpair<key_t_, mapped_t_> > x_) noexcept;

	/// Destructs the map and destroys all key-value pairs.
	~hxflat_map(void) noexcept;

#if HX_CPLUSPLUS >= 202302L
	/// Returns the result of calling `callable` with the mapped value for the
	/// first key matching `key`, otherwise returns `hxnil`. Use `and_then` to
	/// return `hxptr`, `hxref` or `hxexpected`.
	/// - `key` : The key to search for.
	/// - `callable` : The function to call with the found mapped value.
	template<typename self_t_, typename callable_t_>
	hxattr_nodiscard auto and_then(
		this self_t_&& self_, const key_t_& key_, callable_t_&& callable_)
		-> hxremove_cvref_t<decltype(
			hxforward<callable_t_>(callable_)(
				hxforward_like<self_t_>(hxdeclval<mapped_t_&>())))>;

	/// Iterator overload of `and_then`.
	/// - `it` : A const iterator to a mapped value or `end()`.
	/// - `callable` : The function to call with the mapped value.
	template<typename self_t_, typename callable_t_>
	hxattr_nodiscard auto and_then(
		this self_t_&& self_, const_iterator it_, callable_t_&& callable_)
		-> hxremove_cvref_t<decltype(
			hxforward<callable_t_>(callable_)(hxforward_like<self_t_>(
				hxdeclval<const mapped_t_&>())))>;
#endif // HX_CPLUSPLUS >= 202302L

	/// Assigns the contents of `x` to this map. Clears this map then copies all
	/// elements from `x`. Requires `x.size()` ≤ `capacity()`.
	/// - `x` : The map to copy from.
	void operator=(const hxflat_map& x_) noexcept;

	/// Cross-capacity copy assignment. Assigns the contents of `x` to this map.
	/// Requires `x.size()` ≤ `capacity()`.
	/// - `x` : The map to copy from.
	template<hxsize_t capacity_x_>
	void operator=(const hxflat_map<key_t_, mapped_t_, capacity_x_, compare_t_,
		traits_>& x_) noexcept;

	/// Move assigns from a temporary map using `swap`. Requires
	/// `hxallocator_dynamic_capacity`.
	/// - `x` : A temporary map to move from.
	void operator=(hxflat_map&& x_) noexcept;

	/// Returns a const iterator to the element at `index`. Requires `index <
	/// size()`.
	/// - `index` : The 0-based position of the element.
	hxattr_nodiscard const_iterator operator[](hxsize_t index_) const;

	hxattr_nodiscard iterator operator[](hxsize_t index_);

	/// Returns `true` if `a` and `b` contain the same key-value pairs in
	/// the same order using `hxkey_equal` on both keys and values.
	/// - `a` : A map.
	/// - `b` : The map to compare against.
	template<hxsize_t capacity_x_>
	hxattr_nodiscard friend bool operator==(const hxflat_map& a_,
			const hxflat_map<key_t_, mapped_t_, capacity_x_, compare_t_, traits_>& b_) {
		return hxequal_range(a_, b_);
	}

#if HX_CPLUSPLUS < 202002L
	/// Returns `true` if this map and `x` differ in size or in any key-value
	/// pair, using `hxkey_equal` on both keys and values.
	/// - `x` : The map to compare against.
	template<hxsize_t capacity_x_>
	hxattr_nodiscard bool operator!=(const hxflat_map<key_t_, mapped_t_, capacity_x_,
			compare_t_, traits_>& x_) const {
		return !(*this == x_);
	}
#endif

	/// Returns `true` if `a` compares less than `b` lexicographically,
	/// using `hxkey_equal` and `hxkey_less` on keys and values.
	/// - `a` : A map.
	/// - `b` : The map to compare against.
	template<hxsize_t capacity_x_>
	hxattr_nodiscard friend bool operator<(const hxflat_map& a_,
			const hxflat_map<key_t_, mapped_t_, capacity_x_, compare_t_, traits_>& b_) {
		return hxless_range(a_, b_);
	}

	/// Appends every key-value pair from `range` to the end of the arrays and
	/// sorts the arrays afterward. Equivalent to `add_range(false, range)`.
	/// - `range` : The range to move key-value pairs from.
	template<hxrange_concept_ range_t_>
	void add_range(range_t_&& range_) noexcept;

	/// Appends every key-value pair from a range to the end of the arrays
	/// without searching for an insertion point or shifting existing pairs.
	/// When `is_correct` is true, requires the appended keys to be sorted and
	/// ordered after the last key of the map, and, when `traits &
	/// hxtrait_multi` is unset, to contain no keys equal to the last key of
	/// the map or to each other, asserting `bad_ordering` otherwise. When
	/// `is_correct` is false the arrays are sorted after appending and
	/// equal keys are not rejected.
	/// - `is_correct` : True when `range` is sorted, ordered after the map,
	///   and, unless `traits & hxtrait_multi` is set, free of keys equal to
	///   the last key of the map or to each other.
	/// - `range` : The range to move key-value pairs from.
	template<hxrange_concept_ range_t_>
	void add_range(bool is_correct_, range_t_&& range_) noexcept;

	/// Returns a const iterator pointing to the first element.
	const_iterator begin(void) const { return const_iterator(this, 0); }

	/// Returns an iterator pointing to the first element.
	iterator begin(void) { return iterator(this, 0); }

	/// Returns the number of elements the map can hold.
	hxattr_nodiscard hxsize_t capacity(void) const { return m_keys_.capacity(); }

	/// Returns a const iterator pointing to the first element (alias for
	/// `begin`).
	const_iterator cbegin(void) const { return const_iterator(this, 0); }

	/// Returns a const iterator pointing past the last element.
	const_iterator cend(void) const { return const_iterator(this, m_size_); }

	/// Removes all elements and destroys their key-value pairs.
	void clear(void) noexcept;

	/// Returns the number of elements with the given key.
	/// - `key` : The key to count.
	hxattr_nodiscard hxsize_t count(const key_t_& key_) const;

	/// Constructs and inserts a mapped value from `args`, returning an iterator
	/// to the new or existing element.
	/// - `key` : The key of the new element.
	/// - `args` : Arguments forwarded to the mapped value constructor.
	template<typename... args_t_>
	iterator emplace(const key_t_& key_, args_t_&&... args_) noexcept;

	/// Constructs a mapped value from `args` and appends it with `key` when
	/// `key` is ordered after the last key in the map, without searching for
	/// an insertion point or shifting existing pairs. Otherwise calls
	/// `insert`. Returns an iterator to the new or existing element.
	/// - `key` : The key of the new element.
	/// - `args` : Arguments forwarded to the mapped value constructor.
	template<typename... args_t_>
	iterator emplace_back(const key_t_& key_, args_t_&&... args_) noexcept;

	/// Checks if the map contains no elements.
	hxattr_nodiscard bool empty(void) const { return m_size_ == 0; }

	/// Returns a const iterator pointing past the last element.
	const_iterator end(void) const { return const_iterator(this, m_size_); }

	/// Returns an iterator pointing past the last element.
	iterator end(void) { return iterator(this, m_size_); }

	/// Removes all elements with the given key and destroys their key-value
	/// pairs. Returns the number of elements removed.
	/// - `key` : The key to search for and remove.
	hxsize_t erase(const key_t_& key_) noexcept;

	/// Removes the element at the iterator position and destroys the key-value
	/// pair. Returns an iterator to the element that followed the erased one.
	/// - `it` : An iterator to an element in this map.
	iterator erase(const_iterator it_) noexcept;

	/// Returns a const iterator to `key`, or `end()` if the key is not found.
	/// When `traits & hxtrait_multi` is set the first match is returned.
	/// - `key` : The key to search for.
	hxattr_nodiscard const_iterator find(const key_t_& key_) const;

	hxattr_nodiscard iterator find(const key_t_& key_);

	/// Returns `true` if the map has reached its capacity.
	hxattr_nodiscard bool full(void) const { return m_size_ == m_keys_.capacity(); }

	/// Returns `true` if an element matching `key` exists.
	/// - `key` : The key to search for.
	hxattr_nodiscard bool has_value(const key_t_& key_) const;

	/// Inserts a key-value pair. When `traits & hxtrait_multi` is unset and a
	/// matching key already exists, returns an iterator to the existing element
	/// without inserting. Otherwise inserts in sorted order and returns an
	/// iterator to the new element. Both arguments are forwarded into storage.
	/// - `key` : The key of the new element.
	/// - `value` : The mapped value of the new element.
	template<typename key_u_, typename mapped_u_>
	iterator insert(key_u_&& key_, mapped_u_&& mapped_) noexcept;

	/// Returns a const pointer to the underlying key storage. Only the first
	/// `size()` elements are constructed.
	hxattr_nodiscard const key_t_* keys(void) const noexcept;

	/// Returns an iterator to the first element whose key is not ordered before
	/// `key`. Returns `end` if no such element exists.
	/// - `key` : The key to search for.
	hxattr_nodiscard const_iterator lower_bound(const key_t_& key_) const;

	hxattr_nodiscard iterator lower_bound(const key_t_& key_);

	/// Returns the capacity of the map.
	hxattr_nodiscard hxsize_t max_size(void) const { return this->capacity(); }

#if HX_CPLUSPLUS >= 202302L
	/// Returns an iterator to the first element matching `key`, or the result
	/// of calling `callable` when lookup fails.
	/// - `key` : The key to search for.
	/// - `callable` : The function to call when lookup fails. Must return an
	///   iterator.
	template<typename self_t_, typename callable_t_>
	hxattr_nodiscard auto or_else(
		this self_t_&& self_, const key_t_& key_, callable_t_&& callable_)
		-> decltype(self_.end());

	/// Iterator overload of `or_else`.
	/// - `it` : An iterator to an element or `end()`.
	/// - `callable` : The function to call for `end()`. Must return an
	///   iterator.
	template<typename self_t_, typename callable_t_>
	hxattr_nodiscard auto or_else(
		this self_t_&& self_, const_iterator it_, callable_t_&& callable_)
		-> decltype(self_.end());
#endif // HX_CPLUSPLUS >= 202302L

	/// Appends a key-value pair when `key` is ordered after the last key in
	/// the map, without searching for an insertion point or shifting existing
	/// pairs. Otherwise calls `insert`. Both arguments are forwarded into
	/// storage. Returns an iterator to the new or existing element.
	/// - `key` : The key of the new element.
	/// - `value` : The mapped value of the new element.
	template<typename key_u_, typename mapped_u_>
	iterator push_back(key_u_&& key_, mapped_u_&& mapped_) noexcept;

	/// Forwards `size`, `allocator` and `alignment` unchanged to
	/// `reserve_storage` for each of the keys and values allocators.
	/// - `cap` : The number of elements to allocate storage for.
	/// - `allocator` : The memory manager ID to use for allocation (default:
	///   `hxslab_allocator_current`)
	/// - `alignment` : The alignment for the allocation. (default:
	///   `hxalignment`)
	void reserve(hxsize_t size_,
			hxslab_allocator_t allocator_=hxslab_allocator_current,
			hxalignment_t alignment_=hxalignment) noexcept;

	/// Returns the number of elements in the map.
	hxattr_nodiscard hxsize_t size(void) const { return m_size_; }

	/// Swaps the contents with `x`. Requires `hxallocator_dynamic_capacity`.
	/// - `x` : The map to swap with.
	void swap(hxflat_map& x_) noexcept;

	/// Returns an iterator to the first element whose key is ordered after
	/// `key`. Returns `end` if no such element exists.
	/// - `key` : The key to search for.
	hxattr_nodiscard const_iterator upper_bound(const key_t_& key_) const;

	hxattr_nodiscard iterator upper_bound(const key_t_& key_);

#if HX_CPLUSPLUS >= 202302L
	/// Returns a copy of the mapped value for the first key matching `key`, or
	/// a mapped value constructed from `args` when lookup fails.
	/// - `key` : The key to search for.
	/// - `args` : The arguments used to construct the value when lookup fails.
	template<typename self_t_, typename... args_t_>
	hxattr_nodiscard mapped_t_ value_or(
		this self_t_&& self_, const key_t_& key_, args_t_&&... args_);

	/// Iterator overload of `value_or`.
	/// - `it` : A const iterator to a mapped value or `end()`.
	/// - `args` : The arguments used to construct the value for `end()`.
	template<typename self_t_, typename... args_t_>
	hxattr_nodiscard mapped_t_ value_or(
		this self_t_&& self_, const_iterator it_, args_t_&&... args_);
#endif // HX_CPLUSPLUS >= 202302L

	/// Returns a const pointer to the underlying mapped value storage. Only
	/// the first `size()` elements are constructed.
	hxattr_nodiscard const mapped_t_* values(void) const noexcept;

	hxattr_nodiscard mapped_t_* values(void) noexcept;

private:
	/// \cond HIDDEN
	template<hxflat_map_concept_, hxflat_map_concept_, hxsize_t, typename, int>
	friend class hxflat_map;

	// Internal. Not using hxswap with temporaries is an internal contract the
	// heapsort code relies on.
	class sort_value_ {
	public:
		// Refers to the pointed-to key and mapped value in place. m_key_val_
		// and m_mapped_val_ remain raw memory.
		sort_value_(key_t_* key_, mapped_t_* mapped_) :
			m_key_(key_), m_mapped_(mapped_) { }
		sort_value_(const sort_value_&) = default;

		// Detaches into owned storage and takes over as the live instance.
		sort_value_(sort_value_&& sv_) {
			::new(&m_key_val_) key_t_(hxmove(*sv_.m_key_));
			::new(&m_mapped_val_) mapped_t_(hxmove(*sv_.m_mapped_));
			m_key_ = &m_key_val_;
			m_mapped_ = &m_mapped_val_;
		}

		~sort_value_(void) {
			if(m_key_ == &m_key_val_) {
				m_key_val_.~key_t_();
				m_mapped_val_.~mapped_t_();
			}
		}

		void operator=(sort_value_&& sv_) {
			*m_key_ = hxmove(*sv_.m_key_);
			*m_mapped_ = hxmove(*sv_.m_mapped_);
		}

		friend bool operator<(const sort_value_& a_, const sort_value_& b_) {
			return hxkey_less(*a_.m_key_, *b_.m_key_);
		}

		friend void hxswap(sort_value_ a_, sort_value_ b_) {
			hxswap(*a_.m_key_, *b_.m_key_);
			hxswap(*a_.m_mapped_, *b_.m_mapped_);
		}

	private:
		// Raw storage unless m_key_ and m_mapped_ point at it.
		union { key_t_ m_key_val_; };
		union { mapped_t_ m_mapped_val_; };
		key_t_* m_key_;
		mapped_t_* m_mapped_;
	};

	class sort_iterator_ {
	public:
		sort_iterator_(key_t_* key_, mapped_t_* mapped_) : m_key_(key_), m_mapped_(mapped_) { }
		friend bool operator==(const sort_iterator_& a_, const sort_iterator_& b_) { return a_.m_key_ == b_.m_key_; }
		friend bool operator!=(const sort_iterator_& a_, const sort_iterator_& b_) { return a_.m_key_ != b_.m_key_; }
		friend bool operator<(const sort_iterator_& a_, const sort_iterator_& b_) { return a_.m_key_ < b_.m_key_; }
		sort_value_ operator*(void) const { return sort_value_(m_key_, m_mapped_); }
		sort_iterator_ operator+(hxsize_t n_) const { return sort_iterator_(m_key_ + n_, m_mapped_ + n_); }
		sort_iterator_ operator-(hxsize_t n_) const { return sort_iterator_(m_key_ - n_, m_mapped_ - n_); }
		hxsize_t operator-(const sort_iterator_& x_) const { return m_key_ - x_.m_key_; }
		sort_iterator_& operator++(void) { ++m_key_; ++m_mapped_; return *this; }
		sort_iterator_& operator--(void) { --m_key_; --m_mapped_; return *this; }
		sort_iterator_ operator++(int) { sort_iterator_ t_(*this); operator++(); return t_; }
		sort_iterator_ operator--(int) { sort_iterator_ t_(*this); operator--(); return t_; }

		key_t_* m_key_;
		mapped_t_* m_mapped_;
	};

#if (HX_HARDENING_MODE) == HX_HARDENING_MODE_DEBUG
	// Returns true if keys [original_size, size) are sorted, ordered after
	// key [original_size - 1] when original_size is nonzero, and, unless
	// traits & hxtrait_multi is set, contain no adjacent equal keys.
	static bool correct_(const key_t_* k_, hxsize_t original_size_, hxsize_t size_) noexcept;
#endif

	template<typename key_u_, typename mapped_u_>
	iterator insert_at_(hxsize_t index_, key_u_&& key_, mapped_u_&& mapped_) noexcept;

	hxsize_t m_size_;
	hxallocator<key_t_, capacity_> m_keys_;
	hxallocator<mapped_t_, capacity_> m_values_;
	/// \endcond
};

/// `hxflat_multimap` - A `hxflat_map` with `hxtrait_multi` set in `traits`,
/// allowing multiple elements with equal keys.
template<hxflat_map_concept_ key_t_, hxflat_map_concept_ mapped_t_,
	hxsize_t capacity_=hxallocator_dynamic_capacity,
	typename compare_t_=hxkey_three_way_t<key_t_>, int traits_=hxtrait_three_way>
using hxflat_multimap = hxflat_map<key_t_, mapped_t_, capacity_, compare_t_,
	traits_ | hxtrait_multi>;

#include "detail/hxflat_map.inl"
HX_NS_END_
