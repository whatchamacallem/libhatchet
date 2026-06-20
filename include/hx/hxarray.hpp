#pragma once
// SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
// SPDX-License-Identifier: MIT
// This file is licensed under the MIT license found in the LICENSE.md file.

/// \file hxarray.hpp Implements `std::array` with a few things added and a few
/// unimplemented.

#include "libhatchet.h"

// HX_USE_MODULE allows including macros in addition to the hx module.
#if HX_USE_MODULE
#error Header does not provide macros only.
#endif

#include "hxallocator.hpp"
#include "hxsort.hpp"
#include "hxinitializer_list.hpp"

HX_NS_BEGIN_

#if HX_CPLUSPLUS >= 202002L

/// Concept smoke testing the `hxarray` element. Any kind of constructor or
/// assignment operator may or may not be required depending on use. All
/// operator usage should be reasonably predictable. Only the destructor is
/// explicitly required.
template<typename T_>
concept hxarray_element_concept_ = requires(T_& x_) {
	sizeof(T_);
	{ x_.~T_() };
};
#else
#define hxarray_element_concept_ typename
#endif

/// `hxarray` - Implements `std::array` with a few things added. It also
/// implements a dynamically allocated array that does not change size using
/// `hxallocator_dynamic_capacity` which provides an optimization the standard
/// does not. Uses raw pointers as an iterator type so that you get compile
/// errors and debug symbols that use plain C++ pointers instead. There are
/// exhaustive asserts. `hxarray` uses the `hxkey_less` and `hxkey_equal`
/// overloads. They default to using operators `<` and `==`. See
/// `hxalgorithm.hpp` for callable versions of the algorithms here.
///
/// Unlike `hxvector`, `hxarray` always has a fixed size equal to its capacity.
/// Size-changing operations are not provided. Use `hxvector` when dynamic
/// sizing is needed. When `capacity` is `hxallocator_dynamic_capacity`, storage
/// must be allocated by calling `set_size` before use.
/// - `T` : Element type stored by the array.
/// - `capacity` : Fixed element count or `hxallocator_dynamic_capacity` for
///   heap-allocated storage set once by `set_size`.
template<hxarray_element_concept_ T_, hxsize_t capacity_=hxallocator_dynamic_capacity>
class hxarray : public hxallocator<T_, capacity_> {
public:
	/// Random access iterator.
	using iterator = T_*;

	/// Const random access iterator.
	using const_iterator = const T_*;

	/// Publishes the value type. Doesn't end with `_t` because of the standard.
	using value_type = T_;

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

	/// Constructs from a C-style array. e.g.,
	/// ```cpp
	/// static const int initial_values[4] = { 5, 4, 3, 2 };
	/// hxarray<int, 4u> current_values(initial_values);
	/// ```
	/// - `array` : A const array. The array length must equal capacity or capacity must be dynamic.
	template<typename other_value_t_, hxsize_t array_length_>
	hxarray(const other_value_t_(&array_)[array_length_]) noexcept;

	/// Pass values of `std::initializer_list` as initializers.
	/// - `x` : A `std::initializer_list<other_value_t>` of exactly `capacity` elements.
	template<typename other_value_t_>
	hxarray(std::initializer_list<other_value_t_> x_) noexcept;

	/// Destructs the array and destroys all elements.
	~hxarray(void) noexcept;

	/// Assigns the contents of another `hxarray` of the same capacity.
	/// - `x` : A non-temporary `hxarray<T, capacity>`.
	void operator=(const hxarray& x_) noexcept;

	/// Assign from a C-style array. The array length must equal the capacity
	/// of this array.
	/// - `array` : A const array of `array_length` `value_t`.
	template<typename other_value_t_, hxsize_t array_length_>
	void operator=(const other_value_t_(&array_)[array_length_]) noexcept;

	/// Returns a const reference to the element at the specified index.
	/// - `index` : The 0-based offset of the element.
	const T_& operator[](hxsize_t index_) const;

	/// Returns a reference to the element at the specified index.
	/// - `index` : The 0-based offset of the element.
	T_& operator[](hxsize_t index_);

	/// Returns true if the predicate returns true for every element and false
	/// otherwise. Will stop iterating when the predicate returns false. e.g.,
	/// ```cpp
	/// // Assert an array of ints contains all 10s.
	/// EXPECT_TRUE(ints.all_of([&](const int& value) -> bool {
	///   return value == 10;
	/// }));
	/// ```
	/// - `fn` : A callable returning boolean. `!all_of(x)` -> `any_not(x)`.
	template<typename callable_t_>
	hxattr_nodiscard bool all_of(callable_t_&& fn_) const;

	/// A non-const version of `all_of`.
	template<typename callable_t_>
	bool all_of(callable_t_&& fn_);

	/// Returns true if the predicate returns true for any element and false
	/// otherwise. Will stop iterating when the predicate returns true. e.g.,
	/// ```cpp
	/// // Assert an array of ints contains at least one 10.
	/// EXPECT_TRUE(ints.any_of([&](const int& value) -> bool {
	///   return value == 10;
	/// }));
	/// ```
	/// - `fn` : A callable returning boolean. `!any_of(x)` -> `none_of(x)`.
	template<typename callable_t_>
	hxattr_nodiscard bool any_of(callable_t_&& fn_) const;

	/// A non-const version of `any_of`.
	template<typename callable_t_>
	bool any_of(callable_t_&& fn_);

	/// Assigns elements from a range defined by random access iterators.
	/// `iter_t::operator-` is required.
	/// - `begin` : The beginning iterator.
	/// - `end` : The end iterator.
	template<typename iter_t_>
	void assign(iter_t_ begin_, iter_t_ end_) noexcept;

#if HX_CPLUSPLUS >= 202002L
	/// Assigns elements from a range referenced by an lvalue. `range_t::begin`
	/// and `range_t::end` are required. Use `operator=` to assign from a
	/// C-style array.
	/// - `range` : The range to copy elements from.
	template<typename range_t_>
	void assign_range(range_t_& range_) noexcept;

	/// Assigns elements from a temporary range. This overload enables moving
	/// the range elements into the array when forwarding rvalues.
	/// - `range` : The range to move elements from.
	template<typename range_t_>
	requires(!hxis_lvalue_reference<range_t_>::value)
	void assign_range(range_t_&& range_) noexcept;
#endif

	/// Returns a `const T*` to the beginning of the array.
	const T_* begin(void) const { return this->data(); }

	/// Returns a `T*` to the beginning of the array.
	T_* begin(void) { return this->data(); }

	/// Performs a binary search using `hxkey_less`. Returns `end()` when not
	/// found.
	/// - `value` : The value to locate.
	hxattr_nodiscard const T_* binary_search(const T_& value_) const;

	/// Non-const version. Performs a binary search using `hxkey_less`. Returns
	/// `end()` when not found.
	/// - `value` : The value to locate.
	hxattr_nodiscard T_* binary_search(const T_& value_);

	/// Returns a `const T*` to the beginning of the array (alias for `begin`).
	const T_* cbegin(void) const { return this->data(); }

	/// Returns a `const T*` to the end of the array.
	const T_* cend(void) const { return this->data() + this->capacity(); }

	/// Returns true if the arrays compare equivalent using `hxkey_equal`.
	/// Callers must check the return value to detect mismatches.
	/// - `x` : The other array.
	hxattr_nodiscard bool equal(const hxarray& x_) const;

	/// Returns a `const T*` to the end of the array.
	const T_* end(void) const { return this->data() + this->capacity(); }

	/// Returns a `T*` to the end of the array.
	T_* end(void) { return this->data() + this->capacity(); }

	/// Finds the first occurrence of `value` using `hxkey_equal`. Returns
	/// `end()` if no element matches.
	/// - `value` : The value to locate.
	hxattr_nodiscard const T_* find(const T_& value_) const;

	/// Non-const version of `find` using `hxkey_equal`.
	/// - `value` : The value to locate.
	hxattr_nodiscard T_* find(const T_& value_);

	/// Finds the first element for which the predicate returns true. Returns
	/// `end()` if no element matches.
	/// - `fn` : A callable returning boolean.
	template<typename callable_t_>
	hxattr_nodiscard const T_* find_if(callable_t_&& fn_) const;

	/// Non-const version of `find_if`.
	template<typename callable_t_>
	hxattr_nodiscard T_* find_if(callable_t_&& fn_);

	/// Calls a function, lambda, or `std::function` on each element.
	/// - `fn` : A callable.
	template<typename callable_t_>
	void for_each(callable_t_&& fn_) const;

	/// Non-const version of `for_each`.
	template<typename callable_t_>
	void for_each(callable_t_&& fn_);

	/// Returns a `const T*` to the element at `index` or `hxnull` otherwise.
	/// - `index` : The 0-based offset of the element.
	hxattr_nodiscard const T_* get(hxsize_t index_) const;

	/// Returns a `T*` to the element at `index` or `hxnull` otherwise.
	/// - `index` : The 0-based offset of the element.
	hxattr_nodiscard T_* get(hxsize_t index_);

	/// Sorts the array with insertion sort using `hxkey_less`.
	void insertion_sort(void) noexcept;

	/// Returns true if this array compares less than `x` using `hxkey_equal`
	/// and `hxkey_less`.
	/// Callers must check the return value to observe the ordering result.
	/// - `x` : The other array.
	hxattr_nodiscard bool less(const hxarray& x_) const;

	/// Returns the capacity of the array.
	hxattr_nodiscard hxsize_t max_size(void) const { return this->capacity(); }

	/// Copies another `hxarray` using `memcpy`.
	/// - `x` : The other array.
	void memcpy(const hxarray& x_);

	/// Calls `memset` on the array. The default fill byte is `0x00`.
	/// - `byte` : The byte that is repeated. May be a negative char value.
	void memset(int byte_=0x00);

	/// Returns the number of elements in the array.
	hxattr_nodiscard hxsize_t size(void) const { return this->capacity(); }

	/// Returns the number of bytes in the array. (Non-standard.)
	hxattr_nodiscard hxsize_t size_bytes(void) const { return hxsizeof<T_>() * this->capacity(); }

	/// Allocates storage for `size` elements and default constructs them when
	/// `capacity` is `hxallocator_dynamic_capacity`. The current capacity
	/// becomes `size` elements. Reallocation is not allowed. When `capacity` is
	/// fixed, `size` must equal `capacity`.
	/// - `size` : The number of elements to allocate and construct.
	void set_size(hxsize_t size_) noexcept;

	/// Sorts the array using `hxkey_less`.
	void sort(void) noexcept;

private:
	// Destroys elements in the range [begin, end).
	void destruct_(T_* begin_, T_* end_) noexcept;
};

// Without the "requires" keyword these end up being ambiguous.
#if HX_CPLUSPLUS >= 202002L

/// `bool hxequal(hxarray<T, N>& x, hxarray<T, N>& y)` - Compares the contents
/// of `x` and `y` for equivalence.
template<typename T_, hxsize_t capacity_>
bool hxkey_equal(const hxarray<T_, capacity_>& x_, const hxarray<T_, capacity_>& y_) {
	return x_.equal(y_);
}

/// `bool hxkey_less(hxarray<T, N>& x, hxarray<T, N>& y)` - Compares the
/// contents of `x` and `y` lexicographically using `hxkey_equal` and
/// `hxkey_less` on each element.
template<typename T_, hxsize_t capacity_>
bool hxkey_less(const hxarray<T_, capacity_>& x_, const hxarray<T_, capacity_>& y_) {
	return x_.less(y_);
}

/// `void hxswap(hxarray<T>& x, hxarray<T>& y)` - Exchanges the contents of x
/// and y. Only works with `hxallocator_dynamic_capacity`. Dynamically allocated
/// arrays are swapped with very little overhead.
template<typename T_>
void hxswap(hxarray<T_, hxallocator_dynamic_capacity>& x_,
			hxarray<T_, hxallocator_dynamic_capacity>& y_) noexcept {
	x_.swap(y_);
}
#endif // HX_CPLUSPLUS >= 202002L

#include "detail/hxarray.inl"

HX_NS_END_
