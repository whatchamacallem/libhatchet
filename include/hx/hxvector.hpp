#pragma once
// SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
// SPDX-License-Identifier: MIT
// This file is licensed under the MIT license found in the LICENSE.md file.

/// \file
/// Implements `std::vector`, `std::inplace_vector` and
/// `std::back_insert_iterator` with a few things added and a few things
/// unimplemented.

#include "libhatchet.h"

// HX_USE_MACROS_WITH_MODULE allows including macros alongside the module.
#if HX_USE_MACROS_WITH_MODULE
#error Header does not provide macros alone.
#endif

#include "hxallocator.hpp"
#include "hxalgorithm.hpp"
#include "hxinitializer_list.hpp"
#include "hxsort.hpp"

HX_NS_BEGIN_

#if HX_CPLUSPLUS >= 202002L
/// \cond HIDDEN
template<typename T_>
concept hxvector_concept_ = requires(T_& x_) {
	sizeof(T_);
	x_.~T_();
};
/// \endcond
#else
#define hxvector_concept_ typename
#endif

/// \cond HIDDEN
// Internal. Extends the array using placement new when assigned to.
template<typename array_t_>
class hxarray_back_inserter_ {
public:
	// Internal. Extends the array using placement new when assigned to.
	template<typename arg_t_>
	typename array_t_::value_t& operator=(arg_t_&& arg_) {
		return m_x_.push_back(hxforward<arg_t_>(arg_));
	}
private:
	friend array_t_;
	hxarray_back_inserter_(array_t_& x_) : m_x_(x_) { }
#if HX_CPLUSPLUS >= 201703L
	// Copy elision is C++17.
	hxarray_back_inserter_(const hxarray_back_inserter_& x_) = delete;
#endif
	// No address-of operator. It wouldn't be what was expected.
	void operator&(void) const = delete;
	array_t_& m_x_;
};
/// \endcond

/// `hxvector` - Implements `std::vector`, `std::inplace_vector` and
/// `std::back_insert_iterator` with a few things added and a few things
/// unimplemented.
/// Uses raw pointers as an iterator type so that you get compile errors and
/// debug symbols that use plain C++ pointers instead. There are exhaustive
/// asserts. `hxvector` uses the `hxkey_less_t` and `hxkey_equal_t` callables.
/// They default to using operators `<` and `==`. See `hxalgorithm.hpp` for
/// callable versions of the algorithms here.
///
/// `hxvector` can be constructed from C string literals as follows:
///   `hxvector<char, HX_MAX_LINE> string_buffer("example C string");`
/// however `operator+=` does not support C strings.
///
/// Please run both a memory sanitizer and an undefined behavior sanitizer. Use
/// a C array for now if you need `constexpr` support. The C++ standard made
/// special provisions for `std::vector` that were  not made for this class. The
/// excessive number of operators is due to the rules about default operators.
/// - `T` : Element type stored by the array.
/// - `capacity` : Maximum element count or `hxallocator_dynamic_capacity` for
///   dynamic storage.
template<hxvector_concept_ T_, hxsize_t capacity_=hxallocator_dynamic_capacity>
class hxvector : private hxallocator<T_, capacity_> {
public:
	/// Random access iterator.
	using iterator = T_*;

	/// Const random access iterator.
	using const_iterator = const T_*;

	/// Publishes the value type.
	using value_t = T_;

	/// Constructs an empty array with a capacity of `capacity`. `m_end_` will be 0
	/// if `capacity` is 0.
	explicit hxvector(void);

	/// Constructs an array of a given size using `T`'s default constructor.
	/// - `size` : Sets array size as if `resize(size)` were called.
	explicit hxvector(hxsize_t size_) noexcept;

	/// Constructs an array of a given size by making copies of `x`.
	/// - `size` : Sets array size as if `resize(size, x)` were called.
	/// - `x` : The `const T&` to be duplicated.
	explicit hxvector(hxsize_t size_, const T_& x_) noexcept;

	/// Copy constructs an array. Non-explicit to allow assignment constructor.
	/// - `x` : An `hxvector<T>`.
	hxvector(const hxvector& x_) noexcept;

	/// Copy constructs an array. Non-explicit to allow assignment constructor.
	/// - `x` : A non-temporary `hxvector<T>`.
	template <hxsize_t capacity_x_>
	hxvector(const hxvector<T_, capacity_x_>& x_) noexcept;

	/// Copy construct from a temporary. Refuses to copy construct from a
	/// statically allocated temporary for efficiency. Only works with
	/// `hxallocator_dynamic_capacity`.
	/// - `x` : A temporary `hxvector<T>`.
	hxvector(hxvector&& x_) noexcept;

	/// Constructs from a C-style array. Usable as an `initializer_list` when the
	/// `std` namespace is not available. e.g.,
	/// ```cpp
	/// static const int initial_values[] = { 5, 4, 3 };
	/// hxvector<int, 32> current_values(initial_values);
	/// ```
	/// - `array` : A const array of `array_length_` `value_t`.
	template<typename other_value_t_, hxsize_t array_length_>
	hxvector(const other_value_t_(&array_)[array_length_]) noexcept;

	/// Pass values of `std::initializer_list` as initializers to an array of `T`.
	/// WARNING: This constructor will override the other constructors when
	/// uniform initialization is used. e.g., `hxvector<int> x{1, 2}` is an array
	/// containing `{1, 2}` and `hxvector<int> x(1, 2)` is the array containing
	/// `{2}`.
	/// - `x` : A `std::initializer_list<other_value_t>`.
	template <typename other_value_t_>
	hxvector(std::initializer_list<other_value_t_> x_) noexcept;

#if HX_CPLUSPLUS >= 202002L
	/// Constructs by copying elements from a range referenced by an lvalue.
	/// - `range` : A range of at most `capacity` elements.
	template<hxrange_concept_ range_t_>
	requires(!hxis_same<hxremove_cvref_t<range_t_>, hxvector>())
	explicit hxvector(range_t_& range_) noexcept;

	/// Constructs by moving elements from a temporary range. This overload
	/// enables moving the range elements into the array when forwarding
	/// rvalues.
	/// - `range` : A temporary range of at most `capacity` elements.
	template<hxrange_concept_ range_t_>
	requires(!hxis_lvalue_reference<range_t_>()
			&& !hxis_same<hxremove_cvref_t<range_t_>, hxvector>())
	explicit hxvector(range_t_&& range_) noexcept;
#endif

	/// Destructs the array and destroys all elements.
	~hxvector(void) noexcept;

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

	/// Assigns the contents of another `hxvector` to this array. Standard except
	/// reallocation is disallowed.
	/// - `x` : A non-temporary `hxvector<T>`.
	void operator=(const hxvector& x_) noexcept;

	template <hxsize_t capacity_x_>
	void operator=(const hxvector<T_, capacity_x_>& x_) noexcept;

	/// Swap contents with a temporary array using `swap`. Only works with
	/// `hxallocator_dynamic_capacity`. Dynamically allocated arrays are swapped
	/// with very little overhead.
	/// - `x` : A temporary `hxvector<T>`.
	void operator=(hxvector&& x_) noexcept;

	/// Assign from a C-style array. Usable as an `initializer_list` when the
	/// `std` namespace is not available. e.g.,
	/// ```cpp
	/// static const int initial_values[] = { 5, 4, 3 };
	/// hxvector<int, 32> current_values(initial_values);
	/// ```
	/// - `array` : A const array of `array_length` `value_t`.
	template<typename other_value_t_, hxsize_t array_length_>
	void operator=(const other_value_t_(&array_)[array_length_]) noexcept;

	/// Returns a const reference to the element at the specified index.
	/// - `index` : The 0-based offset of the element.
	const T_& operator[](hxsize_t index_) const;

	T_& operator[](hxsize_t index_);

	/// Appends an element. (Non-standard.) Vector math is not a goal so this
	/// should not end up overloaded. Perfect argument forwarding would be too
	/// ambiguous.
	/// - `x` : An object to append. Not a temporary.
	void operator+=(const T_& x_) noexcept;

	/// Appends an element. (Non-standard.) Vector math is not a goal so this
	/// should not end up overloaded. Perfect argument forwarding would be too
	/// ambiguous.
	/// - `x` : An object to append. Passed as a temporary.
	void operator+=(T_&& x_) noexcept;

	/// Appends the contents of another array. (Non-standard, from Python.)
	/// Vector math is not a goal so this should not end up overloaded.
	/// - `x` : Another array. Not a temporary.
	template <hxsize_t capacity_x_>
	void operator+=(const hxvector<T_, capacity_x_>& x_) noexcept;

	/// Appends the contents of another array. (Non-standard, from Python.)
	/// Vector math is not a goal so this should not end up overloaded.
	/// - `x` : Another array passed as a temporary.
	template <hxsize_t capacity_x_>
	void operator+=(hxvector<T_, capacity_x_>&& x_) noexcept;

	/// Used to write code with pointer semantics that writes to either a
	/// pointer or a hxvector. Allows an array to be passed as a reference and
	/// then used as an output iterator similar to `std::back_insert_iterator`.
	/// This operator is used to grow the array while `operator++` is ignored.
	/// Uses a single call to placement new when copying.
	hxarray_back_inserter_<hxvector<T_, capacity_>> operator*(void);

	/// Allows an array to be passed as a reference and then used as an output
	/// iterator similar to `std::back_insert_iterator`. This operator doesn't
	/// do anything but allow the container to be used with pointer sematics.
	/// See `hxalgorithm.hpp` for usage.
	hxvector& operator++(void) { return *this; }

	/// Postfix version.
	hxvector& operator++(int) { return *this; }

	/// Appends elements from a range referenced by an lvalue by copying each
	/// element with `push_back`.
	/// - `range` : The range to copy elements from.
	template<hxrange_concept_ range_t_>
	void add_range(range_t_& range_) noexcept;

	/// Appends elements from a temporary range by moving each element with
	/// `push_back`. This overload enables moving the range elements into the
	/// array when forwarding rvalues.
	/// - `range` : The range to move elements from.
	template<hxrange_concept_ range_t_,
		hxenable_if_t<!hxis_lvalue_reference<range_t_>(), int> = 0>
	void add_range(range_t_&& range_) noexcept;

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

	/// Assigns elements from a range defined by random access iterators.
	/// `iter_t::operator-` is required.
	/// - `begin` : The beginning iterator.
	/// - `end` : The end iterator.
	template <typename iter_t_>
	void assign(iter_t_ begin_, iter_t_ end_) noexcept;

	/// Returns a const reference to the last element in the array.
	const T_& back(void) const;

	T_& back(void);

	/// Returns a `const T*` to the beginning of the array.
	const T_* begin(void) const { return this->data(); }

	T_* begin(void) { return this->data(); }

	/// Performs a binary search using `hxkey_less`. Returns `end()` when not
	/// found.
	/// - `value` : The value to locate.
	hxattr_nodiscard const T_* binary_search(const T_& value_) const;

	hxattr_nodiscard T_* binary_search(const T_& value_);

	/// Returns the capacity of the array or 0 if unallocated.
	hxattr_nodiscard hxsize_t capacity(void) const;

	/// Returns a `const T*` to the beginning of the array (alias for
	/// `begin`).
	const T_* cbegin(void) const { return this->data(); }

	/// Returns a `const T*` to the end of the array.
	const T_* cend(void) const { return m_end_; }

	/// Clears the array, destroying all elements.
	void clear(void) noexcept;

	/// Returns a pointer to a potentially uninitialized array of `T`.
	using hxallocator<T_, capacity_>::data;

	/// Emplaces an element at the end of the array using forwarded arguments.
	/// Returns a reference to the new element. Exactly the same as `push_back`.
	/// - `args` : Arguments forwarded to `T`'s constructor.
	template<typename... args_t_>
	T_& emplace_back(args_t_&&... args_) noexcept;

	/// Returns true if the array is empty.
	/// Callers must check the return value instead of discarding it.
	hxattr_nodiscard bool empty(void) const { return m_end_ == this->data(); }

	/// Returns a `const T*` to the end of the array.
	const T_* end(void) const { return m_end_; }

	T_* end(void) { return m_end_; }

	/// Returns true if the arrays compare equivalent using `hxkey_equal`.
	/// Callers must check the return value to detect mismatches.
	/// - `x` : The other array.
	template<hxsize_t capacity_x_>
	hxattr_nodiscard bool equal(const hxvector<T_, capacity_x_>& x_) const;

	/// Erases the element indicated. Should not compile with hxnull. Support
	/// for erasing ranges has not been added yet.
	/// - `it` : Non-null pointer to an element currently stored in the array.
	void erase(T_* it_) noexcept hxattr_nonnull(2);

	/// Erases the element indicated. Use `hxsize_t{0}` to write the integer
	/// literal 0.
	/// - `index` : Index of the element to erase.
	void erase(hxsize_t index_) noexcept;

	/// Removes elements for which the predicate returns true. (Non-standard.)
	/// Equivalent to calling `erase_unordered` inside a reverse loop. Returns
	/// the number of erased elements. e.g.,
	/// ```cpp
	/// // Erase all the 10s in an array.
	/// ints.erase_if_unordered([](const int& value) -> bool { return value == 10; });
	/// ```
	/// - `callable` : A callable returning boolean.
	template<typename callable_t_>
	hxsize_t erase_if_unordered(callable_t_&& callable_) noexcept;

	/// Variant of `erase` that moves the end element down to replace the erased
	/// element. Should not compile with hxnull. (Non-standard.) Can be used to
	/// erase elements of an array as it is traversed as follows:
	/// ```cpp
	/// for(hxsize_t i = a.size(); i--; ) {
	/// 	if(should_erase(a[i])) {
	/// 		a.erase_unordered(i);
	/// 	}
	/// }
	/// ```
	/// - `it` : Non-null pointer to an element currently stored in the array.
	void erase_unordered(const T_* it_) noexcept hxattr_nonnull(2);

	/// Variant of `erase` that moves the end element down to replace the erased
	/// element. Use `hxsize_t{0}` to write the integer literal 0. (Non-standard.)
	/// - `index` : The index of the element to erase.
	void erase_unordered(hxsize_t index_) noexcept;

	/// Finds the first occurrence of `value` using `hxkey_equal`.
	/// Returns `end` if no element matches.
	/// - `value` : The value to locate.
	hxattr_nodiscard const T_* find(const T_& value_) const;

	hxattr_nodiscard T_* find(const T_& value_);

	/// Finds the first element for which the predicate returns true. Returns
	/// `end` if no element matches. e.g.,
	/// ```cpp
	/// // Search for a 10 and check if it was found.
	/// if(int* t = ints.find_if([](int& x) { return x == 10; }); t != ints.end()) {
	///   // ... Process the 10.
	/// }
	/// ```
	/// - `callable` : A callable returning boolean.
	template<typename callable_t_>
	hxattr_nodiscard const T_* find_if(callable_t_&& callable_) const;

	template<typename callable_t_>
	hxattr_nodiscard T_* find_if(callable_t_&& callable_);

	/// Calls a function, lambda, or `std::function` on each element.
	/// (Non-standard.) Lambdas and `std::function` instances can be provided as
	/// temporaries, so that has to be allowed. The `&&` variant of
	/// `callable_t::operator()` may be selected using `hxmove`. This is the
	/// standard way to signal to the callable that it is a temporary. e.g.,
	/// ```cpp
	/// hxvector<int> a(3, 0);
	/// a.for_each([](int& x) { ++x; }); // Produces { 1, 1, 1 }.
	/// ```
	/// - `callable` : A callable.
	template<typename callable_t_>
	void for_each(callable_t_&& callable_) const;

	template<typename callable_t_>
	void for_each(callable_t_&& callable_);

	/// Returns a const reference to the first element in the array.
	const T_& front(void) const;

	T_& front(void);

	/// Returns true when the array is full (size equals capacity).
	/// (Non-standard.)
	/// Callers must check the return value before adding more elements.
	hxattr_nodiscard bool full(void) const { return m_end_ == this->data() + this->capacity(); }

	/// Appends `size` elements generated by invoking `callable` repeatedly.
	/// - `size` : Number of elements to append.
	/// - `callable` : callable returning the elements to append.
	template<typename callable_t_>
	void generate_n(hxsize_t size_, callable_t_&& callable_) noexcept;

	/// Returns a hash mixing the hashes of every element in order using
	/// `hxkey_hash`.
	hxattr_nodiscard hxhash_t hash(void) const;

	/// Inserts the element at the offset indicated. Should not compile with
	/// `hxnull`. `insert(begin(), x)` and `insert(end(), x)` will work as long as
	/// the array is allocated. Not intended for objects that are expensive to
	/// move. Support for inserting ranges has not been added yet. Consider using
	/// `emplace_back` for storing large objects.
	/// - `it` : Non-null pointer to the location where the new element will be
	///   inserted. Must point inside or one past the current range.
	/// - `x` : The new element.
	template<typename ref_t_>
	void insert(const T_* it_, ref_t_&& x_) noexcept hxattr_nonnull(2);

	/// Inserts the element at the offset indicated. Use `hxsize_t{0}` to write
	/// the integer literal 0. `insert(begin(), x)` and `insert(end(), x)` will
	/// work as long as the array is allocated.
	/// - `index` : Index of the location where the new element will be inserted.
	/// - `x` : The new element.
	template<typename ref_t_>
	void insert(hxsize_t index_, ref_t_&& x_) noexcept;

	/// Sorts the array with insertion sort using `hxkey_less`. (Non-standard.)
	void insertion_sort(void) noexcept;

	/// Returns true if this array compares less than `x` using `hxkey_equal`
	/// and `hxkey_less`. Sorts `[1]` before `[1, 2]`.
	/// Callers must check the return value to observe the ordering result.
	/// - `x` : The other array.
	template<hxsize_t capacity_x_>
	hxattr_nodiscard bool less(const hxvector<T_, capacity_x_>& x_) const;

	/// Converts the array into a max-heap using `hxkey_less`. (Non-standard.)
	void make_heap(void) noexcept;

	/// Returns the capacity of the array or 0 if unallocated. This is the
	/// standard way to report that reallocation is not allowed.
	hxattr_nodiscard hxsize_t max_size(void) const { return this->capacity(); }

	/// Copies another `hxvector` using `memcpy`.
	/// - `x` : The other array.
	template <hxsize_t capacity_x_>
	void memcpy(const hxvector<T_, capacity_x_>& x_);

	/// Calls `memset` on the array. The default fill byte is `0x00`.
	/// - `byte` : The byte that is repeated. May be a negative char value.
	void memset(int byte_=0x00);

#if HX_CPLUSPLUS >= 202302L
	/// Returns an iterator to the element at `index`, or the result of
	/// calling `callable` when the index is outside the array.
	/// - `index` : The 0-based position of the element.
	/// - `callable` : The function to call when lookup fails. Must return an
	///   iterator.
	template<typename self_t_, typename callable_t_>
	hxattr_nodiscard auto or_else(
		this self_t_&& self_, hxsize_t index_, callable_t_&& callable_)
		-> decltype(self_.end());

	/// Iterator overload of `or_else`.
	/// - `it` : An iterator to an element or `end()`.
	/// - `callable` : The function to call for `end()`. Must return an iterator.
	template<typename self_t_, typename callable_t_>
	hxattr_nodiscard auto or_else(
		this self_t_&& self_, const T_* it_, callable_t_&& callable_)
		-> decltype(self_.end());
#endif // HX_CPLUSPLUS >= 202302L

	/// Removes the end element from the array.
	void pop_back(void) noexcept;

	/// Removes the first (maximum) element from a max-heap. This implements
	/// `std::pop_heap` and `std::priority_queue` using `hxkey_less` for ordering.
	/// See `push_heap`.
	void pop_heap(void) noexcept;

	/// Appends an element to the end of the array. `args_t` may be any types
	/// that can be used to construct `T`. Returns a reference to the new
	/// element. Exactly the same as `emplace_back`.
	/// - `args` : Arguments forwarded to `T`'s constructor.
	template<typename... args_t_>
	T_& push_back(args_t_&&... args_) noexcept;

	/// Inserts an element into a max-heap. This implements `std::push_heap` and
	/// `std::priority_queue` using `hxkey_less` for ordering. See `pop_heap`.
	/// Returns a reference to the element added.
	/// - `arg` : The element to add.
	template<typename ref_t_>
	T_& push_heap(ref_t_&& arg_) noexcept;

	/// Reserves storage for at least the specified number of elements.
	/// - `size` : The number of elements to reserve storage for.
	/// - `allocator` : The memory manager ID to use for allocation (default:
	///   `hxsystem_allocator_current`)
	/// - `alignment` : The alignment for the allocation. (default: `hxalignment`)
	void reserve(hxsize_t size_,
			hxsystem_allocator_t allocator_=hxsystem_allocator_current,
			hxalignment_t alignment_=hxalignment);

	/// Resizes the array to the specified size, constructing or destroying
	/// elements as needed. Requires a default constructor. New elements are
	/// default-initialized, leaving integers and floats uninitialized.
	/// - `size` : The new size of the array.
	void resize(hxsize_t size_) noexcept;

	/// An overload with an initial value for new elements. Resizes the array to
	/// the specified size, copy constructing or destroying elements as needed.
	/// - `size` : The new size of the array.
	/// - `x` : Initial value for new elements.
	void resize(hxsize_t size_, const T_& x_) noexcept;

	/// Returns the number of elements in the array.
	hxattr_nodiscard hxsize_t size(void) const { return m_end_ - this->data(); }

	/// Returns the number of bytes in the array. (Non-standard.)
	hxattr_nodiscard hxsize_t size_bytes(void) const { return hxsizeof<T_>() * this->size(); }

	/// Sorts the array using `hxkey_less`. (Non-standard.)
	void sort(void) noexcept;

	/// Swap contents with a temporary array. Only works with
	/// `hxallocator_dynamic_capacity`. Dynamically allocated arrays are swapped
	/// with very little overhead.
	/// - `x` : The array to swap with.
	void swap(hxvector& x_) noexcept;

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
	void* push_back_unconstructed_(void);

	T_* m_end_;
};

/// `hxkey_equal_t<hxvector<T>>` - Compares the contents of `x` and `y` for
/// equivalence.
template<typename T_, hxsize_t capacity_x_>
class hxkey_equal_t<hxvector<T_, capacity_x_> > {
public:
	template<hxsize_t capacity_y_>
	hxattr_nodiscard hxinline hxattr_flatten
	bool operator()(
			const hxvector<T_, capacity_x_>& x_, const hxvector<T_, capacity_y_>& y_) const {
		return x_.equal(y_);
	}
};

/// `hxkey_less_t<hxvector<T>>` - Compares the contents of `x` and `y`
/// lexicographically using `hxkey_equal_t` and `hxkey_less_t` on each element.
template<typename T_, hxsize_t capacity_x_>
class hxkey_less_t<hxvector<T_, capacity_x_> > {
public:
	template<hxsize_t capacity_y_>
	hxattr_nodiscard hxinline hxattr_flatten
	bool operator()(
			const hxvector<T_, capacity_x_>& x_, const hxvector<T_, capacity_y_>& y_) const {
		return x_.less(y_);
	}
};

/// `hxkey_hash_t<hxvector<T>>` - Returns a hash mixing the hashes of every
/// element in order.
template<typename T_, hxsize_t capacity_x_>
class hxkey_hash_t<hxvector<T_, capacity_x_> > {
public:
	hxattr_nodiscard hxinline hxattr_flatten
	hxhash_t operator()(const hxvector<T_, capacity_x_>& x_) const { return x_.hash(); }
};

#include "detail/hxvector.inl"
HX_NS_END_
