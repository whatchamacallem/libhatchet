#pragma once
// SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
// SPDX-License-Identifier: MIT
// This file is licensed under the MIT license found in the LICENSE.md file.

/// \file
/// An embedded doubly linked list with intrusive node linkage. This is the same
/// as `hxconstexpr_list` except that it does not work with `constexpr` as it
/// uses pointer arithmetic to save one pointer per-node.

#include "libhatchet.h"

// HX_USE_MACROS_WITH_MODULE allows including macros alongside the module.
#if HX_USE_MACROS_WITH_MODULE
#error Header does not provide macros alone.
#endif

#include "hxalgorithm.hpp"
#include "hxptr.hpp"
#include "hxutility.h"

HX_NS_BEGIN_

class hxlist_node;

#if HX_CPLUSPLUS >= 202002L
/// \cond HIDDEN
template<typename T_>
concept hxlist_concept_ = requires(T_& x_, T_* ptr_) {
	sizeof(T_);
	x_.~T_();
	{ ptr_ } -> hxconvertible_to<hxlist_node*>; // T_ must derive from hxlist_node.
};
/// \endcond
#else
#define hxlist_concept_ typename
#endif

/// `hxlist_node` - Intrusive doubly linked list node base. Derive from
/// `hxlist_node` to make a type linkable into an `hxlist`. Nodes default to
/// unlinked on construction. Copy construction produces an unlinked node and
/// assignment does nothing.
class hxlist_node {
public:
	/// Constructs an unlinked node.
	hxlist_node(void) : m_list_link_(0) { }

	/// Constructs an unlinked node.
	hxlist_node(const hxlist_node&) : hxlist_node() { }

	/// Assigns nothing. List linkage of either node is not affected.
	/// - `x` : The other node.
	hxlist_node& operator=(const hxlist_node& x_) {
		hxassertmsg(this != &x_, "self_assignment"); (void)x_;
		return *this;
	}

private:
	/// \cond HIDDEN
	friend class hxlist_base_;
	template<hxlist_concept_, typename> friend class hxlist;
	intptr_t m_list_link_; // Previous address XOR next address.
	/// \endcond
};

/// \cond HIDDEN
class hxlist_base_ {
private:
	template<hxlist_concept_, typename> friend class hxlist;

	hxlist_base_(void);
	void extract_(hxlist_node* prev_, hxlist_node* current_);
	hxlist_node* front_(void) const;
	void insert_after_(hxlist_node* prev_, hxlist_node* current_, hxlist_node* ptr_);
	void insert_before_(hxlist_node* prev_, hxlist_node* current_, hxlist_node* ptr_);
	hxlist_node* pop_back_(void);
	hxlist_node* pop_front_(void);
	void push_back_(hxlist_node* ptr_);
	void push_front_(hxlist_node* ptr_);
	void release_all_(void);
	void reverse_(void);
	void splice_(hxlist_node* prev_, hxlist_node* current_, hxlist_base_& x_);

	hxsize_t m_size_;
	hxlist_node m_sentinel_;
	hxlist_node* m_tail_;
};
/// \endcond

/// `hxlist` - An intrusive doubly linked list that takes ownership of nodes via
/// a `deleter_t` callable, defaulting to `hxdefault_delete`. This is the same
/// as `hxconstexpr_list` except that it does not work with `constexpr` as it
/// uses pointer arithmetic to save one pointer per-node. `T` must derive from
/// `hxlist_node`. The destructor calls `clear()` which invokes the deleter on
/// all remaining nodes. Subclasses of `T` may be inserted heterogeneously.
/// Use `hxdo_not_delete` to avoid freeing nodes. Note: It is possible to
/// iterate backwards from `--list.end()` to `--list.begin()` however reverse
/// iterators are not provided.
///
/// For example:
///
/// ```cpp
///   struct example_t : public hxlist_node {
///       example_t(int x) : value(x) { }
///       int value;
///   };
///
///   hxlist<example_t> list;
///   list.push_back(hxnew<example_t>(7));
///
///   for(example_t& n : list) {
///       ::printf("%d\n", n.value);
///   }
/// ```
///
/// - `T` : The node type. Must derive from `hxlist_node`.
/// - `deleter_t` : A class type invoked as `deleter(T*)` to free the owned
///    pointer. See also `hxdo_not_delete`.
template<hxlist_concept_ T_, typename deleter_t_=hxdefault_delete>
class hxlist : private deleter_t_ {
public:
	using T = T_;

	/// `const_iterator` - Bidirectional iterator over const nodes.
	class const_iterator {
	public:
		/// Constructs an iterator that must not be incremented or dereferenced.
		const_iterator(void) : m_prev_(hxnull), m_current_node_(hxnull)
#if (HX_HARDENING_MODE) == HX_HARDENING_MODE_DEBUG
			, m_sentinel_(hxnull)
#endif
		{ }
		/// Advances to the next node and returns this iterator.
		const_iterator& operator++(void);
		/// Post-increment: advances to the next node and returns the prior
		/// position.
		const_iterator operator++(int);
		/// Retreats to the previous node and returns this iterator.
		const_iterator& operator--(void);
		/// Post-decrement: retreats to the previous node and returns the prior
		/// position.
		const_iterator operator--(int);
		/// Returns `true` if both iterators point to the same node.
		/// - `x` : The iterator to compare against.
		bool operator==(const const_iterator& x_) const;
#if HX_CPLUSPLUS < 202002L // C++20 defaults != from ==.
		/// Returns `true` if the iterators point to different nodes.
		/// - `x` : The iterator to compare against.
		bool operator!=(const const_iterator& x_) const { return !(*this == x_); }
#endif
		/// Returns a const reference to the current node.
		const T_& operator*(void) const {
			hxassertmsg(this->m_current_node_ != hxnull
				&& this->m_current_node_ != this->m_sentinel_, "bad_iter");
			return *static_cast<const T_*>(m_current_node_);
		}
		/// Returns a const pointer to the current node.
		const T_* operator->(void) const {
			hxassertmsg(this->m_current_node_ != hxnull
				&& this->m_current_node_ != this->m_sentinel_, "bad_iter");
			return static_cast<const T_*>(m_current_node_);
		}
	protected:
		/// \cond HIDDEN
		template<hxlist_concept_, typename> friend class hxlist;
		const_iterator(hxlist_node* prev_, hxlist_node* current_, const hxlist_node* sentinel_)
			: m_prev_(prev_), m_current_node_(current_)
#if (HX_HARDENING_MODE) == HX_HARDENING_MODE_DEBUG
			, m_sentinel_(sentinel_)
#endif
		{ (void)sentinel_; }
		hxlist_node* m_prev_;
		hxlist_node* m_current_node_;
#if (HX_HARDENING_MODE) == HX_HARDENING_MODE_DEBUG
		const hxlist_node* m_sentinel_;
#endif
		/// \endcond
	};

	/// `iterator` - Bidirectional iterator over mutable nodes.
	class iterator : public const_iterator {
	public:
		/// Constructs an iterator that must not be incremented or dereferenced.
		iterator(void) { }
		/// Advances to the next node and returns this iterator.
		iterator& operator++(void) { const_iterator::operator++(); return *this; }
		/// Post-increment: advances to the next node and returns the prior
		/// position.
		iterator operator++(int);
		/// Retreats to the previous node and returns this iterator.
		iterator& operator--(void) { const_iterator::operator--(); return *this; }
		/// Post-decrement: retreats to the previous node and returns the prior
		/// position.
		iterator operator--(int);
		/// Returns a mutable reference to the current node.
		T_& operator*(void) const {
			hxassertmsg(this->m_current_node_ != hxnull
				&& this->m_current_node_ != this->m_sentinel_, "bad_iter");
			return *static_cast<T_*>(this->m_current_node_);
		}
		/// Returns a mutable pointer to the current node.
		T_* operator->(void) const {
			hxassertmsg(this->m_current_node_ != hxnull
				&& this->m_current_node_ != this->m_sentinel_, "bad_iter");
			return static_cast<T_*>(this->m_current_node_);
		}
	private:
		/// \cond HIDDEN
		template<hxlist_concept_, typename> friend class hxlist;
		iterator(hxlist_node* prev_, hxlist_node* current_, const hxlist_node* sentinel_)
			: const_iterator(prev_, current_, sentinel_) { }
		iterator(const const_iterator& x_) : const_iterator(x_) { }
		/// \endcond
	};

	/// Constructs an empty list with an optional deleter instance.
	/// - `deleter` : Callable with signature `bool deleter(T*)`.
	explicit hxlist(deleter_t_ deleter_=deleter_t_());

	/// Destroys the list by calling `clear()`, which invokes the deleter on
	/// every remaining node.
	~hxlist(void) { this->clear(this->deleter()); }

	/// Returns `true` if this list and `x` contain the same nodes in the same
	/// order, using `T`'s `operator==`.
	/// - `x` : The list to compare against.
	hxattr_nodiscard bool operator==(const hxlist& x_) const;

	/// Returns `true` if this list compares less than `x` lexicographically,
	/// using `T`'s `operator==` and `operator<`.
	/// - `x` : The list to compare against.
	hxattr_nodiscard bool operator<(const hxlist& x_) const;

	/// Links each `T` from a temporary range into this list by address, exactly
	/// as `push_back(T*)` would. The range's nodes are threaded into the list
	/// in place and are not copied or allocated, so `range` must own storage it
	/// is relinquishing to the list.
	/// - `range` : The range of nodes to link into the list.
	template<hxrange_concept_ range_t_,
		hxenable_if_t<!hxis_lvalue_reference<range_t_>(), int> = 0>
	void add_range(range_t_&& range_) noexcept;

	/// Returns a const reference to the last node. The list must not be empty.
	hxattr_nodiscard const T_& back(void) const;

	hxattr_nodiscard T_& back(void);

	/// Returns a const iterator to the first node, or `end()` if the list is
	/// empty.
	const_iterator begin(void) const;

	iterator begin(void);

	/// Returns a const iterator to the first node, or `cend()` if the list is
	/// empty.
	const_iterator cbegin(void) const { return this->begin(); }

	/// Returns a const iterator to the sentinel, representing one past the
	/// last node.
	const_iterator cend(void) const { return this->end(); }

	/// Removes all nodes, invoking `deleter` on each. If `deleter` evaluates to
	/// false nodes are unlinked but not freed.
	/// - `deleter` : Callable with signature `bool deleter(T*)`.
	template<typename deleter_u_>
	void clear(deleter_u_&& deleter_) noexcept;

	/// Removes all nodes using the stored deleter.
	void clear(void) { this->clear(this->deleter()); }

	/// Returns `true` if the list contains no nodes.
	hxattr_nodiscard bool empty(void) const { return m_base_.m_size_ == 0; }

	/// Returns a const iterator to the sentinel, representing one past the last
	/// node. This is also the iterator to one before the first node.
	const_iterator end(void) const;

	iterator end(void);

	/// Unlinks the node at `it` and invokes `deleter` on it. If `deleter`
	/// evaluates to false it is not called.
	/// - `it` : The node to unlink and erase.
	/// - `deleter` : Callable with signature `bool deleter(T*)`.
	template<typename deleter_u_>
	void erase(const_iterator it_, deleter_u_&& deleter_) noexcept;

	/// Unlinks and deletes the node at `it` using the stored deleter.
	/// - `it` : Iterator to the node to unlink and delete.
	void erase(const_iterator it_) noexcept;

	/// `extract` - Returns an `hxptr` owning the node at `it` after unlinking
	/// it from the list.
	/// - `it` : Iterator to the node to unlink and return.
	hxptr<T_, deleter_t_> extract(const_iterator it_);

	/// Finds the first node for which the predicate returns true. Returns
	/// `end()` if no node matches.
	/// - `callable` : A callable taking a `T` reference, returning bool.
	template<typename callable_t_>
	hxattr_nodiscard const_iterator find_if(callable_t_&& callable_) const;

	/// Non-const version of `find_if`.
	template<typename callable_t_>
	hxattr_nodiscard iterator find_if(callable_t_&& callable_);

	/// Calls a function, lambda, or `std::function` on each node.
	/// - `callable` : A callable taking a `T` reference.
	template<typename callable_t_>
	void for_each(callable_t_&& callable_) const;

	/// Non-const version of `for_each`.
	template<typename callable_t_>
	void for_each(callable_t_&& callable_);

	/// Returns a const reference to the stored deleter.
	hxattr_nodiscard const deleter_t_& deleter(void) const;

	hxattr_nodiscard deleter_t_& deleter(void);

	/// Returns a const reference to the first node. The list must not be empty.
	hxattr_nodiscard const T_& front(void) const;

	hxattr_nodiscard T_& front(void);

	/// `insert_after` - Inserts the node owned by `ptr` immediately after `it`.
	/// Returns an iterator to the inserted node.
	/// - `it` : Iterator to the node after which `ptr` is inserted.
	/// - `ptr` : The `hxptr` owning the node to insert.
	template<typename deleter_u_>
	iterator insert_after(const_iterator it_, hxptr<T_, deleter_u_>&& ptr_);

	/// `insert_after` - Inserts `ptr` immediately after `it`. Returns an
	/// iterator to the inserted node.
	/// - `it` : Iterator to the node after which `ptr` is inserted.
	/// - `ptr` : The node to insert. Must not be null.
	iterator insert_after(const_iterator it_, T_* ptr_);

	/// `insert` - Inserts the node owned by `ptr` immediately before `it`.
	/// Returns an iterator to the inserted node.
	/// - `it` : Iterator to the node before which `ptr` is inserted. May be
	///   `end()`.
	/// - `ptr` : The `hxptr` owning the node to insert.
	template<typename deleter_u_>
	iterator insert(const_iterator it_, hxptr<T_, deleter_u_>&& ptr_);

	/// `insert` - Inserts `ptr` immediately before `it`. May be `end()`.
	/// Returns an iterator to the inserted node.
	/// - `it` : Iterator to the node before which `ptr` is inserted.
	/// - `ptr` : The node to insert. Must not be null.
	iterator insert(const_iterator it_, T_* ptr_);

	/// `pop_back` - Returns an `hxptr` owning the last node. The list must
	/// not be empty.
	hxptr<T_, deleter_t_> pop_back(void);

	/// `pop_front` - Returns an `hxptr` owning the first node. The list must
	/// not be empty.
	hxptr<T_, deleter_t_> pop_front(void);

	/// `push_back` - Inserts the node owned by `ptr` at the back of the list.
	/// Returns an iterator to the inserted node.
	/// - `ptr` : The `hxptr` owning the node to append. Must not be null.
	template<typename deleter_u_>
	iterator push_back(hxptr<T_, deleter_u_>&& ptr_);

	/// `push_back` - Inserts `ptr` at the back of the list. Must not be null.
	/// Returns an iterator to the inserted node.
	/// - `ptr` : The node to append.
	iterator push_back(T_* ptr_);

	/// `push_front` - Inserts the node owned by `ptr` at the front of the list.
	/// Returns an iterator to the inserted node.
	/// - `ptr` : The `hxptr` owning the node to prepend. Must not be null.
	template<typename deleter_u_>
	iterator push_front(hxptr<T_, deleter_u_>&& ptr_);

	/// `push_front` - Inserts `ptr` at the front of the list. Must not be null.
	/// Returns an iterator to the inserted node.
	/// - `ptr` : The node to prepend.
	iterator push_front(T_* ptr_);

	/// Resets the list to empty without invoking the deleter on any node.
	/// Ownership of all nodes is abandoned. Use only when nodes are managed
	/// elsewhere or have already been freed.
	void release_all(void);

	/// Removes all nodes for which predicate returns true, invoking `deleter`
	/// on each removed node. If `deleter` evaluates to false nodes are unlinked
	/// but not freed. Returns the number of nodes removed.
	/// - `callable` : A callable taking a `T` reference, returning bool.
	/// - `deleter` : Callable with signature `bool deleter(T*)`.
	template<typename callable_t_, typename deleter_u_>
	hxsize_t remove_if(callable_t_&& callable_, deleter_u_&& deleter_) noexcept;

	/// Removes all nodes for which predicate returns true, invoking the
	/// stored deleter on each removed node. Returns the number of nodes
	/// removed.
	/// - `callable` : A callable taking a `T` reference, returning bool.
	template<typename callable_t_>
	hxsize_t remove_if(callable_t_&& callable_) noexcept;

	/// Reverses the order of nodes in the list in-place. WARNING: Iterators are
	/// invalidated.
	void reverse(void);

	/// Returns the number of nodes currently in the list.
	hxattr_nodiscard hxsize_t size(void) const { return m_base_.m_size_; }

	/// Transfers all nodes from `x` and inserts them before `it`, taking
	/// ownership. `x` is left empty after the call.
	/// - `it` : Iterator before which nodes are inserted.
	/// - `x` : The list to splice from. Left empty after the call.
	void splice(const_iterator it_, hxlist& x_);

private:
	hxlist(const hxlist&) = delete;
	hxlist(hxlist&&) = delete;
	hxlist& operator=(const hxlist&) = delete;
	hxlist& operator=(hxlist&&) = delete;

	hxlist_base_ m_base_;
};

#include "detail/hxlist.inl"
HX_NS_END_
