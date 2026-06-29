#pragma once
// SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
// SPDX-License-Identifier: MIT
// This file is licensed under the MIT license found in the LICENSE.md file.

/// \file
/// An embedded doubly linked list with intrusive node linkage. This is the same
/// as `hxlist` except that it works with `constexpr` at the expense of using an
/// additional pointer per-node. This is the recommended compile time sequence
/// container. `hxvector` and `hxlist` do not work at compile time.

#include "libhatchet.h"

// HX_USE_MODULE allows including macros in addition to the hx module.
#if HX_USE_MODULE
#error Header does not provide macros only.
#endif

#include "hxkey.hpp"
#include "hxptr.hpp"
#include "hxutility.h"

HX_NS_BEGIN_

/// `hxconstexpr_list_node` - Intrusive doubly linked list node base. This is
/// the same as `hxlist` except that it works with `constexpr` at the expense of
/// using an additional pointer per-node. Derive from `hxconstexpr_list_node` to
/// make a type linkable into an `hxconstexpr_list`. Nodes default to unlinked
/// on construction. Copy and move construction and assignment produce or leave
/// a fresh unlinked node so that subclasses may implement the standard
/// operators naturally.
class hxconstexpr_list_node {
public:
	/// Constructs an unlinked node.
	hxconstexpr hxconstexpr_list_node(void) : m_list_prev_(hxnull), m_list_next_(hxnull) { }

	/// Constructs an unlinked node.
	hxconstexpr hxconstexpr_list_node(const hxconstexpr_list_node&) : hxconstexpr_list_node() { }

	/// Assigns nothing. List linkage of either node is not affected.
	hxconstexpr hxconstexpr_list_node& operator=(const hxconstexpr_list_node& other_) {
		hxassertmsg(this != &other_, "self_assignment"); (void)other_;
		return *this;
	}

private:
	/// \cond HIDDEN
	template<typename, typename> friend class hxconstexpr_list;
	hxconstexpr_list_node* m_list_prev_;
	hxconstexpr_list_node* m_list_next_;
	/// \endcond
};

/// `hxconstexpr_list` - An intrusive doubly linked list that takes ownership of
/// nodes via a `deleter_t` callable, defaulting to `hxdefault_delete`. `node_t`
/// must derive from `hxconstexpr_list_node`. The destructor calls `clear()`
/// which invokes the deleter on all remaining nodes. Subclasses of `node_t` may
/// be inserted heterogeneously. Use `hxdo_not_delete` to avoid freeing nodes.
/// Note: It is possible to iterate backwards from `--list.end()` to
/// `--list.begin()` however reverse iterators are not provided.
///
/// For example:
///
/// ```cpp
///   struct example_t : public hxconstexpr_list_node {
///       example_t(int x) : value(x) { }
///       int value;
///   };
///
///   hxconstexpr_list<example_t> list;
///   list.push_back(hxnew<example_t>(7));
///
///   for(example_t& n : list) {
///       ::printf("%d\n", n.value);
///   }
/// ```
///
/// - `node_t` : The node type. Must derive from `hxconstexpr_list_node`.
/// - `deleter_t` : A callable that frees a node pointer. Defaults to
///    `hxdefault_delete`. Use `hxconsteval_delete` for `consteval` work.
template<typename node_t_, typename deleter_t_=hxdefault_delete>
class hxconstexpr_list {
public:
	/// `node_t` - The node type stored in the list. Derives from
	/// `hxconstexpr_list_node`.
	using node_t = node_t_;

	/// `const_iterator` - Bidirectional iterator over const nodes.
	class const_iterator {
	public:
		// GCOVR_EXCL_START
		/// Constructs an iterator that must not be incremented or dereferenced.
		hxconstexpr const_iterator(void) : m_current_node_(hxnull)
#if (HX_HARDENING_MODE) == HX_HARDENING_MODE_DEBUG
			, m_sentinel_(hxnull)
#endif
		{ }
		// GCOVR_EXCL_STOP

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
#if HX_CPLUSPLUS < 202002L // C++20 defaults != from ==.
		/// Returns `true` if the iterators point to different nodes.
		/// - `x` : The iterator to compare against.
		hxconstexpr bool operator!=(const const_iterator& x_) const;
#endif

		/// Returns a const reference to the current node.
		hxconstexpr const node_t_& operator*(void) const {
			hxassertmsg(this->m_current_node_ != hxnull
				&& this->m_current_node_ != this->m_sentinel_, "invalid_iterator");
			return *static_cast<const node_t_*>(m_current_node_);
		}
		/// Returns a const pointer to the current node.
		hxconstexpr const node_t_* operator->(void) const {
			hxassertmsg(this->m_current_node_ != hxnull
				&& this->m_current_node_ != this->m_sentinel_, "invalid_iterator");
			return static_cast<const node_t_*>(m_current_node_);
		}

	protected:
		/// \cond HIDDEN
		template<typename, typename> friend class hxconstexpr_list;
		hxconstexpr const_iterator(hxconstexpr_list_node* current_, const hxconstexpr_list_node* sentinel_)
			: m_current_node_(current_)
#if (HX_HARDENING_MODE) == HX_HARDENING_MODE_DEBUG
			, m_sentinel_(sentinel_)
#endif
		{ (void)sentinel_; }

		hxconstexpr_list_node* m_current_node_;
#if (HX_HARDENING_MODE) == HX_HARDENING_MODE_DEBUG
		const hxconstexpr_list_node* m_sentinel_;
#endif
		/// \endcond
	};

	/// `iterator` - Bidirectional iterator over mutable nodes.
	class iterator : public const_iterator {
	public:
		// GCOVR_EXCL_START
		/// Constructs an iterator that must not be incremented or dereferenced.
		hxconstexpr iterator(void) { }
		// GCOVR_EXCL_STOP

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
			hxassertmsg(this->m_current_node_ != hxnull
				&& this->m_current_node_ != this->m_sentinel_, "invalid_iterator");
			return *static_cast<node_t_*>(this->m_current_node_);
		}
		/// Returns a mutable pointer to the current node.
		hxconstexpr node_t_* operator->(void) const {
			hxassertmsg(this->m_current_node_ != hxnull
				&& this->m_current_node_ != this->m_sentinel_, "invalid_iterator");
			return static_cast<node_t_*>(this->m_current_node_);
		}

	private:
		/// \cond HIDDEN
		template<typename, typename> friend class hxconstexpr_list;
		hxconstexpr iterator(hxconstexpr_list_node* current_, const hxconstexpr_list_node* sentinel_)
			: const_iterator(current_, sentinel_) { }
		/// \endcond
	};

	/// Constructs an empty list with an optional deleter instance.
	/// - `deleter` : The deleter to invoke on nodes when erasing or clearing.
	hxconstexpr explicit hxconstexpr_list(deleter_t_ deleter_=deleter_t_());

	/// Destroys the list by calling `clear()`, which invokes the deleter on
	/// every remaining node.
	hxconstexpr ~hxconstexpr_list(void) { this->clear(m_deleter_); }

	/// Returns a reference to the last node. The list must not be empty.
	hxattr_nodiscard hxconstexpr node_t_& back(void);

	/// Returns a const reference to the last node. The list must not be empty.
	hxattr_nodiscard hxconstexpr const node_t_& back(void) const;

	/// Returns an iterator to the first node, or `end()` if the list is empty.
	hxconstexpr iterator begin(void) { return iterator(m_sentinel_.m_list_next_, &m_sentinel_); }

	/// Returns a const iterator to the first node, or `end()` if the list is empty.
	hxconstexpr const_iterator begin(void) const;

	/// Returns a const iterator to the first node, or `cend()` if the list is empty.
	hxconstexpr const_iterator cbegin(void) const { return this->begin(); }

	/// Returns a const iterator to the sentinel, representing one past the last node.
	hxconstexpr const_iterator cend(void) const { return this->end(); }

	/// Removes all nodes, invoking `deleter` on each. If `deleter` evaluates to
	/// false nodes are unlinked but not freed.
	/// - `deleter` : Override deleter callable. Called only if it evaluates to true.
	template<typename deleter_override_t_>
	hxconstexpr void clear(const deleter_override_t_& deleter_) noexcept;

	/// Removes all nodes using the stored deleter.
	hxconstexpr void clear(void) { this->clear(m_deleter_); }

	/// Returns `true` if the list contains no nodes.
	hxattr_nodiscard hxconstexpr bool empty(void) const;

	/// Returns an iterator to the sentinel, representing one past the last node.
	/// This is also the iterator to one before the first node.
	hxconstexpr iterator end(void) { return iterator(&m_sentinel_, &m_sentinel_); }

	/// Returns a const iterator to the sentinel, representing one past the last
	/// node. This is also the iterator to one before the first node.
	hxconstexpr const_iterator end(void) const;

	/// Unlinks the node at `pos` and invokes `deleter` on it. If `deleter`
	/// evaluates to false it is not called.
	/// - `pos` : The node to unlink and erase.
	/// - `deleter` : Override deleter callable. Called only if it evaluates to true.
	template<typename deleter_override_t_>
	hxconstexpr void erase(const_iterator pos_, const deleter_override_t_& deleter_) noexcept;

	/// Unlinks and deletes the node at `pos` using the stored deleter.
	/// - `pos` : Iterator to the node to unlink and delete.
	hxconstexpr void erase(const_iterator pos_) noexcept { this->erase(pos_, m_deleter_); }

	/// `extract` - Returns an `hxptr` owning the node at `pos` after unlinking it
	/// from the list.
	/// - `pos` : Iterator to the node to unlink and return.
	hxconstexpr hxptr<node_t_, deleter_t_> extract(const_iterator pos_);

	/// Returns a reference to the first node. The list must not be empty.
	hxattr_nodiscard hxconstexpr node_t_& front(void);

	/// Returns a const reference to the first node. The list must not be empty.
	hxattr_nodiscard hxconstexpr const node_t_& front(void) const;

	/// `insert_after` - Inserts the node owned by `ptr` immediately after `pos`.
	/// Returns an iterator to the inserted node.
	/// - `pos` : Iterator to the node after which `ptr` is inserted.
	/// - `ptr` : The `hxptr` owning the node to insert.
	template<typename ptr_deleter_t_>
	hxconstexpr iterator insert_after(const_iterator pos_, hxptr<node_t_, ptr_deleter_t_>&& ptr_);

	/// `insert_after` - Inserts `ptr` immediately after `pos`. Returns an
	/// iterator to the inserted node.
	/// - `pos` : Iterator to the node after which `ptr` is inserted.
	/// - `ptr` : The node to insert. Must not be null.
	hxconstexpr iterator insert_after(const_iterator pos_, node_t_* ptr_);

	/// `insert` - Inserts the node owned by `ptr` immediately before `pos`.
	/// Returns an iterator to the inserted node.
	/// - `pos` : Iterator to the node before which `ptr` is inserted. May be `end()`.
	/// - `ptr` : The `hxptr` owning the node to insert.
	template<typename ptr_deleter_t_>
	hxconstexpr iterator insert(const_iterator pos_, hxptr<node_t_, ptr_deleter_t_>&& ptr_);

	/// `insert` - Inserts `ptr` immediately before `pos`. May be `end()`.
	/// Returns an iterator to the inserted node.
	/// - `pos` : Iterator to the node before which `ptr` is inserted.
	/// - `ptr` : The node to insert. Must not be null.
	hxconstexpr iterator insert(const_iterator pos_, node_t_* ptr_);

	/// `pop_back` - Returns an `hxptr` owning the last node. The list must
	/// not be empty.
	hxconstexpr hxptr<node_t_, deleter_t_> pop_back(void);

	/// `pop_front` - Returns an `hxptr` owning the first node. The list must
	/// not be empty.
	hxconstexpr hxptr<node_t_, deleter_t_> pop_front(void);

	/// `push_back` - Inserts the node owned by `ptr` at the back of the list.
	/// Returns an iterator to the inserted node.
	/// - `ptr` : The `hxptr` owning the node to append. Must not be null.
	template<typename ptr_deleter_t_>
	hxconstexpr iterator push_back(hxptr<node_t_, ptr_deleter_t_>&& ptr_);

	/// `push_back` - Inserts `ptr` at the back of the list. Must not be null.
	/// Returns an iterator to the inserted node.
	/// - `ptr` : The node to append.
	hxconstexpr iterator push_back(node_t_* ptr_);

	/// `push_front` - Inserts the node owned by `ptr` at the front of the list.
	/// Returns an iterator to the inserted node.
	/// - `ptr` : The `hxptr` owning the node to prepend. Must not be null.
	template<typename ptr_deleter_t_>
	hxconstexpr iterator push_front(hxptr<node_t_, ptr_deleter_t_>&& ptr_);

	/// `push_front` - Inserts `ptr` at the front of the list. Must not be null.
	/// Returns an iterator to the inserted node.
	/// - `ptr` : The node to prepend.
	hxconstexpr iterator push_front(node_t_* ptr_);

	/// Resets the list to empty without invoking the deleter on any node.
	/// Ownership of all nodes is abandoned. Use only when nodes are managed
	/// elsewhere or have already been freed.
	hxconstexpr void release_all(void);

	/// Removes all nodes for which `predicate` returns true, invoking the
	/// stored deleter on each removed node.
	/// - `predicate` : A callable taking a `node_t` reference, returning bool.
	template<typename predicate_t_>
	hxconstexpr hxsize_t remove_if(predicate_t_ predicate_) noexcept;

	/// Reverses the order of nodes in the list in-place.
	hxconstexpr void reverse(void);

	/// Returns the number of nodes currently in the list.
	hxattr_nodiscard hxconstexpr hxsize_t size(void) const { return m_size_; }

	/// Transfers all nodes from `other` and inserts them before `pos`, taking
	/// ownership. `other` is left empty after the call.
	/// - `pos` : Iterator before which nodes are inserted.
	/// - `other` : The list to splice from. Left empty after the call.
	hxconstexpr void splice(const_iterator pos_, hxconstexpr_list& other_);

private:
	hxconstexpr_list(const hxconstexpr_list&) = delete;
	hxconstexpr_list(hxconstexpr_list&&) = delete;
	hxconstexpr_list& operator=(const hxconstexpr_list&) = delete;
	hxconstexpr_list& operator=(hxconstexpr_list&&) = delete;

	hxconstexpr void insert_(hxconstexpr_list_node* prev_, hxconstexpr_list_node* next_,
		hxconstexpr_list_node* ptr_);
	hxconstexpr void extract_(hxconstexpr_list_node* ptr_);

	deleter_t_ m_deleter_;
	hxsize_t m_size_;
	hxconstexpr_list_node m_sentinel_;
};

#include "detail/hxconstexpr_list.inl"
HX_NS_END_
