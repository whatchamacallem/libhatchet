#pragma once
// SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
// SPDX-License-Identifier: MIT
// This file is licensed under the MIT license found in the LICENSE.md file.

/// \file
/// Implements `std::array` with a few things added and a few unimplemented.

#include "libhatchet.h"

// HX_USE_MACROS_WITH_MODULE allows including macros alongside the module.
#if HX_USE_MACROS_WITH_MODULE
#error Header does not provide macros alone.
#endif

#include "hxallocator.hpp"
#include "hxrange.hpp"
#include "hxinitializer_list.hpp"
#include "hxsort.hpp"

HX_NS_BEGIN_

#if HX_CPLUSPLUS >= 202002L
/// \cond HIDDEN
template<typename T_>
concept hxarray_concept_ = requires(T_& x_) {
	sizeof(T_);
	x_.~T_();
};
/// \endcond
#else
#define hxarray_concept_ typename
#endif

/// `hxarray` - Implements `std::array` with a few things added. It also
/// implements a dynamically allocated array that does not change size using
/// `hxallocator_dynamic_capacity` which provides an optimization the standard
/// does not. Uses raw pointers as an iterator type so that you get compile
/// errors and debug symbols that use plain C++ pointers instead. There are
/// exhaustive asserts. `hxarray` uses the `hxkey_less_t` and `hxkey_equal_t`
/// callables. They default to using operators `<` and `==`. See
/// `hxrange.hpp` for callable versions of the algorithms here.
///
/// Unlike `hxvector`, `hxarray` always has a fixed size equal to its capacity.
/// Size-changing operations are not provided. Use `hxvector` when dynamic
/// sizing is needed. When `capacity` is `hxallocator_dynamic_capacity`, storage
/// must be allocated by calling `reserve` before use.
/// - `T` : Element type stored by the array.
/// - `capacity` : Fixed element count or `hxallocator_dynamic_capacity` for
///   heap-allocated storage set once by `reserve`.
template<hxarray_concept_ T_, hxsize_t capacity_=hxallocator_dynamic_capacity>
class hxarray : private hxallocator<T_, capacity_> {
public:
	/// `iterator` - Random access iterator.
	using iterator = T_*;

	/// `const_iterator` - Const random access iterator.
	using const_iterator = const T_*;

	/// `value_type` - Publishes the value type.
	using value_t = T_;

	/// Default constructs all elements.
	explicit hxarray(void) noexcept;

	/// Constructs all elements as copies of `x`.
	/// - `x` : The `const T&` to be duplicated.
	explicit hxarray(const T_& x_) noexcept;

	/// Copy constructs from another `hxarray` of the same capacity.
	/// - `x` : An `hxarray<T, capacity>`.
	hxarray(const hxarray& x_) noexcept;

	/// Move constructs from a temporary `hxarray`. Requires
	/// `hxallocator_dynamic_capacity`.
	/// - `x` : A temporary `hxarray<T, hxallocator_dynamic_capacity>`.
	hxarray(hxarray&& x_) noexcept;

	/// Constructs from a C-style array. Usable as an `initializer_list` when the
	/// `std` namespace is not available. e.g.,
	/// ```cpp
	/// static const int initial_values[] = { 5, 4, 3 };
	/// hxarray<int, 3> current_values(initial_values);
	/// ```
	/// - `array` : A const array of exactly `capacity` elements.
	template<typename other_value_t_, hxsize_t array_length_>
	hxarray(const other_value_t_(&array_)[array_length_]) noexcept;

	/// Pass values of `std::initializer_list` as initializers.
	/// - `x` : A `std::initializer_list<other_value_t>` of exactly `capacity`
	///   elements.
	template<typename other_value_t_>
	hxarray(std::initializer_list<other_value_t_> x_) noexcept;

#if HX_CPLUSPLUS >= 202002L
	/// Constructs by copying elements from a range referenced by an lvalue.
	/// - `range` : A range of exactly `capacity` elements.
	template<hxrange_concept_ range_t_>
	requires(!hxis_same<hxremove_cvref_t<range_t_>, hxarray<T_, capacity_> >())
	explicit hxarray(range_t_& range_) noexcept;

	/// Constructs by moving elements from a temporary range. This overload
	/// enables moving the range elements into the array when forwarding
	/// rvalues.
	/// - `range` : A temporary range of exactly `capacity` elements.
	template<hxrange_concept_ range_t_>
	requires(!hxis_lvalue_reference<range_t_>()
			&& !hxis_same<hxremove_cvref_t<range_t_>, hxarray<T_, capacity_> >())
	explicit hxarray(range_t_&& range_) noexcept;
#endif

	/// Destructs the array and destroys all elements.
	~hxarray(void) noexcept;

#if HX_CPLUSPLUS >= 202302L
	/// Returns the result of calling `callable` with the element at `index`,
	/// otherwise returns `hxnil`. Use `and_then` to return `hxptr`, `hxref` or
	/// `hxexpected`.
	/// - `index` : The 0-based position of the element.
	/// - `callable` : The function to call with the element.
	template<typename self_t_, typename callable_t_>
	hxattr_nodiscard auto and_then(
		this self_t_&& self_, hxsize_t index_, callable_t_&& callable_)
		-> hxremove_cvref_t<decltype(
			hxforward<callable_t_>(callable_)(
				hxforward_like<self_t_>(hxdeclval<T_&>())))>;

	/// Iterator overload of `and_then`.
	/// - `it` : A const iterator to an element or `end()`.
	/// - `callable` : The function to call with the element.
	template<typename self_t_, typename callable_t_>
	hxattr_nodiscard auto and_then(
		this self_t_&& self_, const T_* it_, callable_t_&& callable_)
		-> hxremove_cvref_t<decltype(
			hxforward<callable_t_>(callable_)(hxforward_like<self_t_>(
				hxdeclval<const T_&>())))>;
#endif // HX_CPLUSPLUS >= 202302L

	/// Assigns the contents of another `hxarray` of the same capacity.
	/// - `x` : A non-temporary `hxarray<T, capacity>`.
	void operator=(const hxarray& x_) noexcept;

	/// Move assigns from a temporary `hxarray`. Requires
	/// `hxallocator_dynamic_capacity`.
	/// - `x` : A temporary `hxarray<T, hxallocator_dynamic_capacity>`.
	void operator=(hxarray&& x_) noexcept;

	/// Assign from a C-style array. Usable as an `initializer_list` when the
	/// `std` namespace is not available. e.g.,
	/// ```cpp
	/// static const int initial_values[] = { 5, 4, 3 };
	/// current_values = initial_values;
	/// ```
	/// - `array` : A const array of exactly `capacity` elements.
	template<typename other_value_t_, hxsize_t array_length_>
	void operator=(const other_value_t_(&array_)[array_length_]) noexcept;

	/// Returns a const reference to the element at the specified index.
	/// - `index` : The 0-based offset of the element.
	const T_& operator[](hxsize_t index_) const;

	T_& operator[](hxsize_t index_);

	/// Returns true if the predicate returns true for every element and false
	/// otherwise. Will stop iterating when the predicate returns false. e.g.,
	/// ```cpp
	/// // Assert an array of ints contains all 10s.
	/// EXPECT_TRUE(ints.all_of([&](const int& value) -> bool {
	///   return value == 10;
	/// }));
	/// ```
	/// - `callable` : A callable returning boolean. `!all_of(x)` -> `any_not(x)`.
	template<typename callable_t_>
	hxattr_nodiscard bool all_of(callable_t_&& callable_) const;

	template<typename callable_t_>
	bool all_of(callable_t_&& callable_);

	/// Returns true if the predicate returns true for any element and false
	/// otherwise. Will stop iterating when the predicate returns true. e.g.,
	/// ```cpp
	/// // Assert an array of ints contains at least one 10.
	/// EXPECT_TRUE(ints.any_of([&](const int& value) -> bool {
	///   return value == 10;
	/// }));
	/// ```
	/// - `callable` : A callable returning boolean. `!any_of(x)` -> `none_of(x)`.
	template<typename callable_t_>
	hxattr_nodiscard bool any_of(callable_t_&& callable_) const;

	template<typename callable_t_>
	bool any_of(callable_t_&& callable_);

	/// Returns a `const T*` to the beginning of the array.
	const T_* begin(void) const { return this->data(); }

	T_* begin(void) { return this->data(); }

	/// Performs a binary search using `hxkey_less`. Returns `end()` when not
	/// found.
	/// - `value` : The value to locate.
	hxattr_nodiscard const T_* binary_search(const T_& value_) const;

	hxattr_nodiscard T_* binary_search(const T_& value_);

	/// Returns the capacity of the array.
	hxattr_nodiscard hxsize_t capacity(void) const;

	/// Returns a `const T*` to the beginning of the array (alias for `begin`).
	const T_* cbegin(void) const { return this->data(); }

	/// Returns a `const T*` to the end of the array.
	const T_* cend(void) const { return this->data() + this->capacity(); }

	/// Returns a pointer to a potentially uninitialized array of `T`.
	using hxallocator<T_, capacity_>::data;

	/// Returns true if the array contains no elements.
	hxinline hxattr_nodiscard bool empty(void) const { return this->size() == 0; }

	/// Returns a `const T*` to the end of the array.
	const T_* end(void) const { return this->data() + this->capacity(); }

	T_* end(void) { return this->data() + this->capacity(); }

	/// Returns true if the arrays compare equivalent using `hxkey_equal`.
	/// Callers must check the return value to detect mismatches.
	/// - `x` : The other array.
	hxattr_nodiscard bool equal(const hxarray& x_) const;

	/// Finds the first occurrence of `value` using `hxkey_equal`. Returns
	/// `end()` if no element matches.
	/// - `value` : The value to locate.
	hxattr_nodiscard const T_* find(const T_& value_) const;

	hxattr_nodiscard T_* find(const T_& value_);

	/// Finds the first element for which the predicate returns true. Returns
	/// `end()` if no element matches.
	/// - `callable` : A callable returning boolean.
	template<typename callable_t_>
	hxattr_nodiscard const T_* find_if(callable_t_&& callable_) const;

	template<typename callable_t_>
	hxattr_nodiscard T_* find_if(callable_t_&& callable_);

	/// Returns true if the array has been allocated.
	hxinline hxattr_nodiscard bool full(void) const { return this->capacity() != 0; }

	/// Calls a function, lambda, or `std::function` on each element.
	/// - `callable` : A callable.
	template<typename callable_t_>
	void for_each(callable_t_&& callable_) const;

	template<typename callable_t_>
	void for_each(callable_t_&& callable_);

	/// Returns a hash mixing the hashes of every element in order using
	/// `hxkey_hash`.
	hxattr_nodiscard hxhash_t hash(void) const;

	/// Sorts the array with insertion sort using `hxkey_less`.
	void insertion_sort(void) noexcept;

	/// Returns true if this array compares less than `x` using `hxkey_equal`
	/// and `hxkey_less`.
	/// Callers must check the return value to observe the ordering result.
	/// - `x` : The other array.
	hxattr_nodiscard bool less(const hxarray& x_) const;

	/// Returns the capacity of the array.
	hxinline hxattr_nodiscard hxsize_t max_size(void) const { return this->capacity(); }

	/// Copies another `hxarray` using `memcpy`.
	/// - `x` : The other array.
	void memcpy(const hxarray& x_);

	/// Calls `memset` on the array. The default fill byte is `0x00`.
	/// - `byte` : The byte that is repeated. May be a negative char value.
	void memset(int byte_=0x00);

#if HX_CPLUSPLUS >= 202302L
	/// Returns an iterator to the element at `index`, or the result of
	/// calling `callable` when the index is outside the array.
	/// - `index` : The 0-based position of the element.
	/// - `callable` : The function to call when lookup fails. Must return a
	///   iterator.
	template<typename self_t_, typename callable_t_>
	hxattr_nodiscard auto or_else(
		this self_t_&& self_, hxsize_t index_, callable_t_&& callable_)
		-> decltype(self_.end());

	/// Iterator overload of `or_else`.
	/// - `it` : An iterator to an element or `end()`.
	/// - `callable` : The function to call for `end()`. Must return an
	///   iterator.
	template<typename self_t_, typename callable_t_>
	hxattr_nodiscard auto or_else(
		this self_t_&& self_, const T_* it_, callable_t_&& callable_)
		-> decltype(self_.end());
#endif // HX_CPLUSPLUS >= 202302L

	/// Allocates storage for `size` elements and default constructs them when
	/// `capacity` is `hxallocator_dynamic_capacity`. The current capacity
	/// becomes `size` elements. Reallocation is not allowed. When `capacity` is
	/// fixed, `size` must equal `capacity`.
	/// - `size` : The number of elements to allocate and construct.
	/// - `allocator` : The memory manager ID to use for allocation.
	/// - `alignment` : The alignment to use for the allocation.
	void reserve(hxsize_t size_,
			hxsystem_allocator_t allocator_=hxsystem_allocator_current,
			hxalignment_t alignment_=hxalignment) noexcept;

	/// Returns the number of elements in the array.
	hxinline hxattr_nodiscard hxsize_t size(void) const { return this->capacity(); }

	/// Returns the number of bytes in the array. (Non-standard.)
	hxinline hxattr_nodiscard hxsize_t size_bytes(void) const {
		return hxsizeof<T_>() * this->capacity();
	}

	/// Sorts the array using `hxkey_less`.
	void sort(void) noexcept;

	/// Swap contents with a temporary array. Only works with
	/// `hxallocator_dynamic_capacity`. Dynamically allocated arrays are swapped
	/// with very little overhead.
	/// - `x` : The array to swap with.
	void swap(hxarray& x_) noexcept;

#if HX_CPLUSPLUS >= 202302L
	/// Returns a copy of the element at `index`, or a `T` constructed from
	/// `args` when the index is outside the array.
	/// - `index` : The 0-based position of the element.
	/// - `args` : The arguments used to construct the value when lookup fails.
	template<typename self_t_, typename... args_t_>
	hxattr_nodiscard T_ value_or(
		this self_t_&& self_, hxsize_t index_, args_t_&&... args_);

	/// Iterator overload of `value_or`.
	/// - `it` : A const iterator to an element or `end()`.
	/// - `args` : The arguments used to construct the value for `end()`.
	template<typename self_t_, typename... args_t_>
	hxattr_nodiscard T_ value_or(
		this self_t_&& self_, const T_* it_, args_t_&&... args_);
#endif // HX_CPLUSPLUS >= 202302L

private:
	void destruct_(T_* begin_, T_* end_) noexcept;
};

/// `hxkey_equal_t<hxarray<T, N>>` - Compares the contents of `x` and `y` for
/// equivalence.
template<typename T_, hxsize_t capacity_>
class hxkey_equal_t<hxarray<T_, capacity_> > {
public:
	hxattr_nodiscard hxinline hxattr_flatten
	bool operator()(
			const hxarray<T_, capacity_>& x_, const hxarray<T_, capacity_>& y_) const {
		return x_.equal(y_);
	}
};

/// `hxkey_less_t<hxarray<T, N>>` - Compares the contents of `x` and `y`
/// lexicographically using `hxkey_equal_t` and `hxkey_less_t` on each element.
template<typename T_, hxsize_t capacity_>
class hxkey_less_t<hxarray<T_, capacity_> > {
public:
	hxattr_nodiscard hxinline hxattr_flatten
	bool operator()(
			const hxarray<T_, capacity_>& x_, const hxarray<T_, capacity_>& y_) const {
		return x_.less(y_);
	}
};

/// `hxkey_hash_t<hxarray<T, N>>` - Returns a hash mixing the hashes of every
/// element in order.
template<typename T_, hxsize_t capacity_>
class hxkey_hash_t<hxarray<T_, capacity_> > {
public:
	hxattr_nodiscard hxinline hxattr_flatten
	hxhash_t operator()(const hxarray<T_, capacity_>& x_) const { return x_.hash(); }
};

#include "detail/hxarray.inl"
HX_NS_END_
