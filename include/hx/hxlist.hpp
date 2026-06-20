#pragma once
// SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
// SPDX-License-Identifier: MIT
// This file is licensed under the MIT license found in the LICENSE.md file.

/// \file hxlist.hpp An embedded doubly linked list with intrusive node linkage.
/// This is the same as `hxconstexpr_list` except that it does not work with
/// `constexpr` as it uses pointer arithmetic to save one pointer per-node.

#include "libhatchet.h"

// HX_USE_MODULE allows including macros in addition to the hx module.
#if HX_USE_MODULE
#error Header does not provide macros only.
#endif

#include "hxkey.hpp"
#include "hxptr.hpp"
#include "hxutility.h"

HX_NS_BEGIN_

/// Intrusive doubly linked list node base. Derive from `hxlist_node` to make a
/// type linkable into an `hxlist`. Nodes default to unlinked on construction.
/// Copy and move construction and assignment produce or leave a fresh unlinked
/// node so that subclasses may implement the standard operators naturally.
class hxlist_node {
public:
	/// Constructs an unlinked node.
	hxlist_node(void) : m_list_link_(0) { }

	/// Constructs an unlinked node.
	hxlist_node(const hxlist_node&) : hxlist_node() { }

	/// Assigns nothing. List linkage of either node is not affected.
	hxlist_node& operator=(const hxlist_node& other_) {
		hxassertmsg(this != &other_, "self_assignment"); (void)other_;
		return *this;
	}

private:
	/// \cond HIDDEN
	template<typename, typename> friend class hxlist;
	intptr_t m_list_link_; // Previous address XOR next address.
	/// \endcond
};

/// `hxlist` - An intrusive doubly linked list that takes ownership of nodes via
/// a `deleter_t` callable, defaulting to `hxdefault_delete`. This is the same
/// as `hxconstexpr_list` except that it does not work with `constexpr` as it uses
/// pointer arithmetic to save one pointer per-node. `node_t` must derive from
/// `hxlist_node`. The destructor calls `clear()` which invokes the deleter on
/// all remaining nodes. Subclasses of `node_t` may be inserted heterogeneously.
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
/// - `node_t` : The node type. Must derive from `hxlist_node`.
/// - `deleter_t` : A callable that frees a node pointer. Defaults to
///    `hxdefault_delete`.
template<typename node_t_, typename deleter_t_=hxdefault_delete>
class hxlist {
public:
	using node_t = node_t_;

	/// Bidirectional iterator over const nodes.
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
		bool operator!=(const const_iterator& x_) const;
#endif

		/// Returns a const reference to the current node.
		const node_t_& operator*(void) const {
			hxassertmsg(this->m_current_node_ != hxnull
				&& this->m_current_node_ != this->m_sentinel_, "invalid_iterator");
			return *static_cast<const node_t_*>(m_current_node_);
		}
		/// Returns a const pointer to the current node.
		const node_t_* operator->(void) const {
			hxassertmsg(this->m_current_node_ != hxnull
				&& this->m_current_node_ != this->m_sentinel_, "invalid_iterator");
			return static_cast<const node_t_*>(m_current_node_);
		}

	protected:
		/// \cond HIDDEN
		template<typename, typename> friend class hxlist;
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

	/// Bidirectional iterator over mutable nodes.
	class iterator : public const_iterator {
	public:
		/// Constructs an iterator that must not be incremented or dereferenced.
		iterator(void) { }

		/// Advances to the next node and returns this iterator.
		iterator& operator++(void) { const_iterator::operator++(); return *this; }
		/// Post-increment: advances to the next node and returns the prior position.
		iterator operator++(int);
		/// Retreats to the previous node and returns this iterator.
		iterator& operator--(void) { const_iterator::operator--(); return *this; }
		/// Post-decrement: retreats to the previous node and returns the prior position.
		iterator operator--(int);

		/// Returns a mutable reference to the current node.
		node_t_& operator*(void) const {
			hxassertmsg(this->m_current_node_ != hxnull
				&& this->m_current_node_ != this->m_sentinel_, "invalid_iterator");
			return *static_cast<node_t_*>(this->m_current_node_);
		}
		/// Returns a mutable pointer to the current node.
		node_t_* operator->(void) const {
			hxassertmsg(this->m_current_node_ != hxnull
				&& this->m_current_node_ != this->m_sentinel_, "invalid_iterator");
			return static_cast<node_t_*>(this->m_current_node_);
		}

	private:
		/// \cond HIDDEN
		template<typename, typename> friend class hxlist;
		iterator(hxlist_node* prev_, hxlist_node* current_, const hxlist_node* sentinel_)
			: const_iterator(prev_, current_, sentinel_) { }
		/// \endcond
	};

	/// Constructs an empty list.
	explicit hxlist(void);

	/// Destroys the list by calling `clear()`, which invokes the deleter on
	/// every remaining node.
	~hxlist(void) { this->clear(); }

	/// Returns a reference to the last node. The list must not be empty.
	hxattr_nodiscard node_t_& back(void);

	/// Returns a const reference to the last node. The list must not be empty.
	hxattr_nodiscard const node_t_& back(void) const;

	/// Returns an iterator to the first node, or `end()` if the list is empty.
	iterator begin(void);

	/// Returns a const iterator to the first node, or `end()` if the list is empty.
	const_iterator begin(void) const;

	/// Returns a const iterator to the first node, or `cend()` if the list is empty.
	const_iterator cbegin(void) const { return this->begin(); }

	/// Returns a const iterator to the sentinel, representing one past the last node.
	const_iterator cend(void) const { return this->end(); }

	/// Removes all nodes, invoking `deleter` on each. If `deleter` evaluates to
	/// false nodes are unlinked but not freed.
	/// - `deleter` : Override deleter callable. Called only if it evaluates to true.
	template<typename deleter_override_t_>
	void clear(const deleter_override_t_& deleter_);

	/// Removes all nodes using the list's default `deleter_t`.
	void clear(void) { this->clear(deleter_t_()); }

	/// Returns `true` if the list contains no nodes.
	hxattr_nodiscard bool empty(void) const { return m_size_ == 0u; }

	/// Returns an iterator to the sentinel, representing one past the last node.
	/// This is also the iterator to one before the first node.
	iterator end(void);

	/// Returns a const iterator to the sentinel, representing one past the last
	/// node. This is also the iterator to one before the first node.
	const_iterator end(void) const;

	/// Unlinks the node at `pos` and invokes `deleter` on it. If `deleter`
	/// evaluates to false it is not called.
	/// - `pos` : The node to unlink and erase.
	/// - `deleter` : Override deleter callable. Called only if it evaluates to true.
	template<typename deleter_override_t_>
	void erase(const_iterator pos_, const deleter_override_t_& deleter_);

	/// Unlinks and deletes the node at `pos` using the list's default `deleter_t`.
	/// - `pos` : Iterator to the node to unlink and delete.
	void erase(const_iterator pos_) { this->erase(pos_, deleter_t_()); }

	/// `extract` - Returns an `hxptr` owning the node at `pos` after unlinking it
	/// from the list.
	/// - `pos` : Iterator to the node to unlink and return.
	hxptr<node_t_, deleter_t_> extract(const_iterator pos_);

	/// Returns a reference to the first node. The list must not be empty.
	hxattr_nodiscard node_t_& front(void);

	/// Returns a const reference to the first node. The list must not be empty.
	hxattr_nodiscard const node_t_& front(void) const;

	/// `insert_after` - Inserts the node owned by `ptr` immediately after `pos`.
	/// Returns an iterator to the inserted node.
	/// - `pos` : Iterator to the node after which `ptr` is inserted.
	/// - `ptr` : The `hxptr` owning the node to insert.
	template<typename ptr_deleter_t_>
	iterator insert_after(const_iterator pos_, hxptr<node_t_, ptr_deleter_t_>&& ptr_);

	/// `insert_after` - Inserts `ptr` immediately after `pos`. Returns an
	/// iterator to the inserted node.
	/// - `pos` : Iterator to the node after which `ptr` is inserted.
	/// - `ptr` : The node to insert. Must not be null.
	iterator insert_after(const_iterator pos_, node_t_* ptr_);

	/// `insert` - Inserts the node owned by `ptr` immediately before `pos`.
	/// Returns an iterator to the inserted node.
	/// - `pos` : Iterator to the node before which `ptr` is inserted. May be `end()`.
	/// - `ptr` : The `hxptr` owning the node to insert.
	template<typename ptr_deleter_t_>
	iterator insert(const_iterator pos_, hxptr<node_t_, ptr_deleter_t_>&& ptr_);

	/// `insert` - Inserts `ptr` immediately before `pos`. May be `end()`.
	/// Returns an iterator to the inserted node.
	/// - `pos` : Iterator to the node before which `ptr` is inserted.
	/// - `ptr` : The node to insert. Must not be null.
	iterator insert(const_iterator pos_, node_t_* ptr_);

	/// `pop_back` - Returns an `hxptr` owning the last node. The list must
	/// not be empty.
	hxptr<node_t_, deleter_t_> pop_back(void);

	/// `pop_front` - Returns an `hxptr` owning the first node. The list must
	/// not be empty.
	hxptr<node_t_, deleter_t_> pop_front(void);

	/// `push_back` - Inserts the node owned by `ptr` at the back of the list.
	/// Returns an iterator to the inserted node.
	/// - `ptr` : The `hxptr` owning the node to append. Must not be null.
	template<typename ptr_deleter_t_>
	iterator push_back(hxptr<node_t_, ptr_deleter_t_>&& ptr_);

	/// `push_back` - Inserts `ptr` at the back of the list. Must not be null.
	/// Returns an iterator to the inserted node.
	/// - `ptr` : The node to append.
	iterator push_back(node_t_* ptr_);

	/// `push_front` - Inserts the node owned by `ptr` at the front of the list.
	/// Returns an iterator to the inserted node.
	/// - `ptr` : The `hxptr` owning the node to prepend. Must not be null.
	template<typename ptr_deleter_t_>
	iterator push_front(hxptr<node_t_, ptr_deleter_t_>&& ptr_);

	/// `push_front` - Inserts `ptr` at the front of the list. Must not be null.
	/// Returns an iterator to the inserted node.
	/// - `ptr` : The node to prepend.
	iterator push_front(node_t_* ptr_);

	/// Resets the list to empty without invoking the deleter on any node.
	/// Ownership of all nodes is abandoned. Use only when nodes are managed
	/// elsewhere or have already been freed.
	void release_all(void);

	/// Removes all nodes for which `predicate` returns true, invoking the
	/// default deleter on each removed node.
	/// - `predicate` : A callable taking a `node_t` reference, returning bool.
	template<typename predicate_t_>
	size_t remove_if(predicate_t_ predicate_);

	/// Reverses the order of nodes in the list in-place. WARNING: Iterators are
	/// invalidated.
	void reverse(void);

	/// Returns the number of nodes currently in the list.
	hxattr_nodiscard size_t size(void) const { return m_size_; }

	/// Transfers all nodes from `other` and inserts them before `pos`, taking
	/// ownership. `other` is left empty after the call.
	/// - `pos` : Iterator before which nodes are inserted.
	/// - `other` : The list to splice from. Left empty after the call.
	void splice(const_iterator pos_, hxlist& other_);

private:
	hxlist(const hxlist&) = delete;
	hxlist(hxlist&&) = delete;
	hxlist& operator=(const hxlist&) = delete;
	hxlist& operator=(hxlist&&) = delete;

	void insert_(hxlist_node* prev_, hxlist_node* next_, hxlist_node* ptr_);
	void extract_(hxlist_node* prev_, hxlist_node* ptr_);

	size_t m_size_;
	hxlist_node m_sentinel_;
	// When empty m_sentinel_.m_list_link_ == 0 and m_tail_ == &m_sentinel_.
	hxlist_node* m_tail_;
};

#include "detail/hxlist.inl"

HX_NS_END_
