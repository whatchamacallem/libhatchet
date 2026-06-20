#pragma once
// SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
// SPDX-License-Identifier: MIT
// This file is licensed under the MIT license found in the LICENSE.md file.

/// \file hx/hxlist.hpp An embedded doubly linked list with intrusive node
/// linkage.

#include "hxkey.hpp"
#include "hxptr.hpp"
#include "hxutility.h"

/// Intrusive doubly linked list node base. Derive from `hxlist_node` to make a
/// type linkable into an `hxlist`. Nodes default to unlinked on construction.
/// Copy and move construction and assignment produce or leave a fresh unlinked
/// node so that subclasses may implement the standard operators naturally.
class hxlist_node {
public:
	/// Constructs an unlinked node with both pointers set to null.
	hxconstexpr hxlist_node(void) : m_list_prev_(hxnull), m_list_next_(hxnull) { }

	/// Constructs an unlinked node.
	hxconstexpr hxlist_node(const hxlist_node&) : hxlist_node() { }

	/// Assigns nothing. List linkage of either node is not affected.
	hxconstexpr hxlist_node& operator=(const hxlist_node& other_) {
		hxassertmsg(this != &other_, "self_assignment"); (void)other_;
		return *this;
	}

private:
	/// \cond HIDDEN
	template<typename, typename> friend class hxlist;
	hxlist_node* m_list_prev_;
	hxlist_node* m_list_next_;
	/// \endcond
};

/// `hxlist` - An intrusive doubly linked list that takes ownership of nodes via
/// a `deleter_t` callable, defaulting to `hxdefault_delete`. `node_t` must
/// derive from `hxlist_node`. The destructor calls `clear()` which invokes the
/// deleter on all remaining nodes. Subclasses of `node_t` may be inserted
/// heterogeneously. Use `hxdo_not_delete` to avoid freeing nodes.
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
/// - `node_t` : The node type. Must derive from `hxlist_node`.
/// - `deleter_t` : A callable that frees a node pointer. Defaults to
///    `hxdefault_delete`. Use `hxconsteval_delete` for `consteval` work.
template<typename node_t_, typename deleter_t_=hxdefault_delete>
class hxlist {
public:
	using node_t = node_t_;

	/// Bidirectional iterator over const nodes. Incrementing past `end()` or
	/// decrementing past `begin()` is undefined.
	class const_iterator {
	public:
		/// Constructs a singular iterator that must not be incremented or
		/// dereferenced.
		hxconstexpr const_iterator(void) : m_current_node_(hxnull)
#if (HX_HARDENING_MODE) >= HX_HARDENING_MODE_VERBOSE
			, m_sentinel_(hxnull)
#endif
		{ }

		/// Advances to the next node and returns this iterator.
		hxconstexpr const_iterator& operator++(void);
		/// Post-increment: advances to the next node and returns the prior
		/// position.
		hxconstexpr const_iterator operator++(int);
		/// Retreats to the previous node and returns this iterator.
		hxconstexpr const_iterator& operator--(void);
		/// Post-decrement: retreats to the previous node and returns the prior
		/// position.
		hxconstexpr const_iterator operator--(int);

		/// Returns `true` if both iterators point to the same node.
		/// - `x` : The iterator to compare against.
		hxconstexpr bool operator==(const const_iterator& x_) const;
		/// Returns `true` if the iterators point to different nodes.
		/// - `x` : The iterator to compare against.
		hxconstexpr bool operator!=(const const_iterator& x_) const;

		/// Returns a const reference to the current node.
		hxconstexpr const node_t_& operator*(void) const {
			hxassertmsg(m_current_node_ != hxnull, "invalid_iterator");
			return *static_cast<const node_t_*>(m_current_node_);
		}
		/// Returns a const pointer to the current node.
		hxconstexpr const node_t_* operator->(void) const {
			hxassertmsg(m_current_node_ != hxnull, "invalid_iterator");
			return static_cast<const node_t_*>(m_current_node_);
		}
		/// Returns a const pointer to the current node.
		hxconstexpr operator const node_t_*(void) const {
			hxassertmsg(m_current_node_ != hxnull, "invalid_iterator");
			return static_cast<const node_t_*>(m_current_node_);
		}

	protected:
		/// \cond HIDDEN
		template<typename, typename> friend class hxlist;		
		hxconstexpr const_iterator(hxlist_node* current_, const hxlist_node* sentinel_)
			: m_current_node_(current_)
#if (HX_HARDENING_MODE) >= HX_HARDENING_MODE_VERBOSE
			, m_sentinel_(sentinel_)
#endif
		{ (void)sentinel_; }

		hxlist_node* m_current_node_;
#if (HX_HARDENING_MODE) >= HX_HARDENING_MODE_VERBOSE
		const hxlist_node* m_sentinel_;
#endif
		/// \endcond
	};

	/// Bidirectional iterator over mutable nodes. Incrementing past `end()` or
	/// decrementing past `begin()` is undefined.
	class iterator : public const_iterator {
	public:
		/// Constructs a singular iterator that must not be incremented or
		/// dereferenced.
		hxconstexpr iterator(void) { }

		/// Advances to the next node and returns this iterator.
		hxconstexpr iterator& operator++(void) { const_iterator::operator++(); return *this; }
		/// Post-increment: advances to the next node and returns the prior position.
		hxconstexpr iterator operator++(int);
		/// Retreats to the previous node and returns this iterator.
		hxconstexpr iterator& operator--(void) { const_iterator::operator--(); return *this; }
		/// Post-decrement: retreats to the previous node and returns the prior position.
		hxconstexpr iterator operator--(int);

		/// Returns a mutable reference to the current node.
		hxconstexpr node_t_& operator*(void) const {
			hxassertmsg(this->m_current_node_ != hxnull, "invalid_iterator");
			return *static_cast<node_t_*>(this->m_current_node_);
		}
		/// Returns a mutable pointer to the current node.
		hxconstexpr node_t_* operator->(void) const {
			hxassertmsg(this->m_current_node_ != hxnull, "invalid_iterator");
			return static_cast<node_t_*>(this->m_current_node_);
		}
		/// Returns a pointer to the current node.
		hxconstexpr operator node_t_*(void) const {
			hxassertmsg(this->m_current_node_ != hxnull, "invalid_iterator");
			return static_cast<node_t_*>(this->m_current_node_);
		}

	private:
		/// \cond HIDDEN
		template<typename, typename> friend class hxlist;
		hxconstexpr iterator(hxlist_node* current_, const hxlist_node* sentinel_)
			: const_iterator(current_, sentinel_) { }
		/// \endcond
	};

	/// Constructs an empty list with the sentinel node linked to itself.
	hxconstexpr explicit hxlist(void);

	/// Destroys the list by calling `clear()`, which invokes the deleter on
	/// every remaining node.
	hxconstexpr ~hxlist(void) { this->clear(); }

	/// Returns a pointer to the last node. The list must not be empty.
	hxattr_nodiscard hxconstexpr node_t_& back(void);
	/// Returns a const pointer to the last node. The list must not be empty.
	hxattr_nodiscard hxconstexpr const node_t_& back(void) const;

	/// Returns an iterator to the first node, or `end()` if the list is empty.
	hxconstexpr iterator begin(void) { return iterator(m_head_.m_list_next_, &m_head_); }
	/// Returns a const iterator to the first node, or `end()` if the list is empty.
	hxconstexpr const_iterator begin(void) const;

	/// Returns a const iterator to the first node, or `cend()` if the list is empty.
	hxconstexpr const_iterator cbegin(void) const { return begin(); }

	/// Returns a const iterator to the sentinel, representing one past the last node.
	hxconstexpr const_iterator cend(void) const { return end(); }

	/// Removes all nodes, invoking `deleter` on each. If `deleter` evaluates to
	/// false nodes are unlinked but not freed.
	/// - `deleter` : Override deleter callable. Called only if it evaluates to true.
	template<typename deleter_override_t_>
	hxconstexpr void clear(const deleter_override_t_& deleter_);

	/// Removes all nodes using the list's default `deleter_t`.
	hxconstexpr void clear(void) { this->clear(deleter_t_()); }

	/// Returns `true` if the list contains no nodes.
	hxattr_nodiscard hxconstexpr bool empty(void) const { return m_head_.m_list_next_ == &m_head_; }

	/// Returns an iterator to the sentinel, representing one past the last node.
	/// This is also the iterator to one before the first node.
	hxconstexpr iterator end(void) { return iterator(&m_head_, &m_head_); }
	/// Returns a const iterator to the sentinel, representing one past the last
	/// node. This is also the iterator to one before the first node.
	hxconstexpr const_iterator end(void) const;

	/// Unlinks `ptr` from the list and invokes `deleter` on it. If `deleter`
	/// evaluates to false it is not called.
	/// - `ptr` : The node to unlink and erase.
	/// - `deleter` : Override deleter callable. Called only if it evaluates to true.
	template<typename deleter_override_t_>
	hxconstexpr void erase(node_t_* ptr_, const deleter_override_t_& deleter_);

	/// Unlinks and deletes `ptr` using the list's default `deleter_t`.
	/// - `ptr` : The node to unlink and delete.
	hxconstexpr void erase(node_t_* ptr_) { this->erase(ptr_, deleter_t_()); }

	/// `extract` - Returns an `hxptr` owning `ptr` after unlinking it from the
	/// list. `ptr` must not be null.
	/// - `ptr` : The node to unlink and return.
	hxconstexpr hxptr<node_t_, deleter_t_> extract(node_t_* ptr_);

	/// Returns a pointer to the first node. The list must not be empty.
	hxattr_nodiscard hxconstexpr node_t_& front(void);

	/// Returns a const pointer to the first node. The list must not be empty.
	hxattr_nodiscard hxconstexpr const node_t_& front(void) const;

	/// `insert_after` - Inserts the node owned by `ptr` immediately after `pos`.
	/// - `pos` : The node after which `ptr` is inserted.
	/// - `ptr` : The `hxptr` owning the node to insert.
	template<typename ptr_deleter_t_>
	hxconstexpr void insert_after(node_t_* pos_, hxptr<node_t_, ptr_deleter_t_>&& ptr_);

	/// `insert_after` - Inserts `ptr` immediately after `pos`. Both must not be null.
	/// - `pos` : The node after which `ptr` is inserted.
	/// - `ptr` : The node to insert.
	hxconstexpr void insert_after(node_t_* pos_, node_t_* ptr_);

	/// `insert` - Inserts the node owned by `ptr` immediately before `pos`.
	/// - `pos` : The node before which `ptr` is inserted.
	/// - `ptr` : The `hxptr` owning the node to insert.
	template<typename ptr_deleter_t_>
	hxconstexpr void insert(node_t_* pos_, hxptr<node_t_, ptr_deleter_t_>&& ptr_);

	/// `insert` - Inserts `ptr` immediately before `pos`. Both must not be null.
	/// - `pos` : The node before which `ptr` is inserted.
	/// - `ptr` : The node to insert.
	hxconstexpr void insert(node_t_* pos_, node_t_* ptr_);

	/// `pop_back` - Returns an `hxptr` owning the last node, or an empty
	/// `hxptr` if the list is empty.
	hxconstexpr hxptr<node_t_, deleter_t_> pop_back(void);

	/// `pop_front` - Returns an `hxptr` owning the first node, or an empty
	/// `hxptr` if the list is empty.
	hxconstexpr hxptr<node_t_, deleter_t_> pop_front(void);

	/// `push_back` - Inserts the node owned by `ptr` at the back of the list.
	/// - `ptr` : The `hxptr` owning the node to append. Must not be null.
	template<typename ptr_deleter_t_>
	hxconstexpr void push_back(hxptr<node_t_, ptr_deleter_t_>&& ptr_);

	/// `push_back` - Inserts `ptr` at the back of the list. Must not be null.
	/// - `ptr` : The node to append.
	hxconstexpr void push_back(node_t_* ptr_);

	/// `push_front` - Inserts the node owned by `ptr` at the front of the list.
	/// - `ptr` : The `hxptr` owning the node to prepend. Must not be null.
	template<typename ptr_deleter_t_>
	hxconstexpr void push_front(hxptr<node_t_, ptr_deleter_t_>&& ptr_);

	/// `push_front` - Inserts `ptr` at the front of the list. Must not be null.
	/// - `ptr` : The node to prepend.
	hxconstexpr void push_front(node_t_* ptr_);

	/// Resets the list to empty without invoking the deleter on any node.
	/// Ownership of all nodes is abandoned. Use only when nodes are managed
	/// elsewhere or have already been freed.
	hxconstexpr void release_all(void);

	/// Removes all nodes for which `predicate` returns true, invoking the
	/// default deleter on each removed node.
	/// - `predicate` : A callable taking a `node_t` reference, returning bool.
	template<typename predicate_t_>
	hxconstexpr size_t remove_if(predicate_t_ predicate_);

	/// Reverses the order of nodes in the list in-place.
	hxconstexpr void reverse(void);

	/// Returns the number of nodes currently in the list.
	hxattr_nodiscard hxconstexpr size_t size(void) const { return m_size_; }

	/// Transfers all nodes from `other` and inserts them before `pos`, taking
	/// ownership. `other` is left empty after the call.
	/// - `pos` : Iterator before which nodes are inserted.
	/// - `other` : The list to splice from. Left empty after the call.
	hxconstexpr void splice(const_iterator pos_, hxlist& other_);

private:
	hxlist(const hxlist&) = delete;
	hxlist(hxlist&&) = delete;
	hxlist& operator=(const hxlist&) = delete;
	hxlist& operator=(hxlist&&) = delete;

	hxconstexpr void insert_(hxlist_node* prev_, hxlist_node* next_, hxlist_node* ptr_);
	hxconstexpr void extract_(hxlist_node* ptr_);

	size_t m_size_;
	hxlist_node m_head_;
};

#include "detail/hxlist.inl"
