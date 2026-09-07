#pragma once
// SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
// SPDX-License-Identifier: MIT
// This file is licensed under the MIT license found in the LICENSE.md file.

/// \file
/// A fixed-size hash table with embedded singly-linked list buckets. Used for
/// both maps and sets. This is not intended to be the fastest, it is intended
/// to enforce libhatchet's real-time memory allocation strategy and to avoid
/// code bloat.

#include "libhatchet.h"

// HX_USE_MACROS_WITH_MODULE allows including macros alongside the module.
#if HX_USE_MACROS_WITH_MODULE
#error Header does not provide macros alone.
#endif

#include "hxkey.hpp"
#include "hxptr.hpp"
#include "hxallocator.hpp"
#include "hxrange.hpp"

HX_NS_BEGIN_

#include "detail/hxpow2_allocator.hpp"

class hxhash_node_base;

#if HX_CPLUSPLUS >= 202002L
/// \cond HIDDEN
template<typename node_t_>
concept hxhash_table_concept_ =
	requires(node_t_& node_, const node_t_& const_node_) {
		sizeof(typename node_t_::key_t); // node must publish a key_t type
		{ &node_ } -> hxconvertible_to<hxhash_node_base*>; // node_t must derive from hxhash_node_base
		// returns the node's key
		{ const_node_.hash_key() } -> hxconvertible_to<const typename node_t_::key_t&>;
	};
/// \endcond
#else
#define hxhash_table_concept_ typename
#endif

/// `hxhash_node_base` - Intrusive hash-chain node base. Derive from
/// `hxhash_node_base` to make a node linkable into an `hxhash_table`.
class hxhash_node_base {
protected:
	friend class hxhash_table_base_;
	template<hxhash_table_concept_ node_t_, typename deleter_t_, bool multi_t_,
		uint32_t table_size_bits_> friend class hxhash_table;

	/// Constructs an unlinked node with the given cached hash.
	/// - `hash` : The hash value to cache.
	hxhash_node_base(hxhash_t hash_) : hash_next(hxnull), hash_val(hash_) { }

	hxhash_node_base* hash_next;
	hxhash_t hash_val;
};

/// `hxhash_table_set_node` - Optional base class for unordered set entries.
/// Caches the hash value. See `hxhash_table_map_node` if you need a map node.
/// Copying and assignment are unimplemented and the key must be immutable in
/// any assignment operation in a subclass.
template<typename key_t_>
class hxhash_table_set_node : public hxhash_node_base {
public:
	using key_t = key_t_;

	/// Constructs a node from the key and caches its hash for reuse.
	/// - `key` : Key used to identify the node.
	template<typename ref_t_>
	hxhash_table_set_node(ref_t_&& key_)
		: hxhash_node_base(0), m_key_(hxforward<ref_t_>(key_)) {
		this->hash_val = hxkey_hash(m_key_);
	}

	/// Assignment is undefined. The key is immutable.
	hxhash_table_set_node(void) = delete;
	hxhash_table_set_node(const hxhash_table_set_node& src_) = delete;
	hxhash_table_set_node& operator=(const hxhash_table_set_node& x_) = delete;

	/// Returns `true` if this node and `x` have equal keys, using `hxkey_equal`.
	/// - `x` : The node to compare against.
	bool operator==(const hxhash_table_set_node& x_) const { return hxkey_equal(m_key_, x_.m_key_); }

	/// Returns `true` if this node's key is ordered before `x`'s key, using
	/// `hxkey_less`.
	/// - `x` : The node to compare against.
	bool operator<(const hxhash_table_set_node& x_) const { return hxkey_less(m_key_, x_.m_key_); }

	/// The key and hash identify the `node_t` and should not change once added.
	const key_t_& hash_key(void) const { return m_key_; }

	/// Returns the cached hash value for the stored key. Hash values are not
	/// required to be unique.
	hxhash_t hash_value(void) const { return this->hash_val; }

	/// Returns the hash value computed for `key`. Hash values are not required
	/// to be unique.
	/// - `key` : The key to hash.
	static hxhash_t hash_value(const key_t_& key_) { return hxkey_hash(key_); }

private:
	key_t_ m_key_;
};

/// `hxhash_table_map_node` - Base class for unordered map entries.
template<typename key_t_, typename value_t_>
class hxhash_table_map_node : public hxhash_table_set_node<key_t_> {
public:
	using key_t = key_t_;
	using value_t = value_t_;

	/// Default-initializes a node. `value_t` must default-construct when
	/// accessed via `operator[]`.
	/// - `key` : Key used to identify the node.
	hxhash_table_map_node(const key_t_& key_) :
		hxhash_table_set_node<key_t_>(key_) { }

	/// Constructs a node whose value is copy- or move-initialized.
	/// - `key` : Key used to identify the node.
	/// - `value` : Value forwarded into storage.
	template<typename ref_t_>
	hxhash_table_map_node(const key_t_& key_, ref_t_&& value_) :
		hxhash_table_set_node<key_t_>(key_), m_value_(hxforward<ref_t_>(value_)) { }

	/// Returns `true` if this node and `x` have equal keys and values, using
	/// `hxkey_equal`.
	/// - `x` : The node to compare against.
	bool operator==(const hxhash_table_map_node& x_) const;

	/// Returns `true` if this node is ordered before `x`, comparing keys first
	/// and then values with `hxkey_equal` and `hxkey_less`.
	/// - `x` : The node to compare against.
	bool operator<(const hxhash_table_map_node& x_) const;

	/// Returns the stored value.
	const value_t_& value(void) const { return m_value_; }

	/// Returns the stored value, allowing mutation.
	value_t_& value(void) { return m_value_; }

private:
	value_t_ m_value_;
};

/// \cond HIDDEN
// Internal. Type erased shared code.
class hxhash_table_base_ {
private:
	template<hxhash_table_concept_ node_t_, typename deleter_t_, bool multi_t_, uint32_t table_size_bits_>
	friend class hxhash_table;
	using hxhash_equal_fn_ = bool(*)(const hxhash_node_base* node_, const void* key_);
	using hxhash_deleter_fn_ = void(*)(hxhash_node_base* node_, void* context_);
	static hxsize_t chain_count_multi_(const hxhash_node_base* head_, const void* key_,
		hxhash_equal_fn_ equal_);
	static void chain_erase_at_(hxhash_node_base** head_, hxhash_node_base* node_);
	static hxsize_t chain_erase_multi_(hxhash_node_base** head_, const void* key_,
		hxhash_equal_fn_ equal_, hxhash_deleter_fn_ deleter_, void* deleter_context_);
	static hxhash_node_base* chain_extract_(hxhash_node_base** head_, const void* key_,
		hxhash_equal_fn_ equal_);
	static hxsize_t chain_length_(const hxhash_node_base* head_);
	static hxhash_node_base* chain_replace_(hxhash_node_base** head_, hxhash_node_base* node_,
		const void* key_, hxhash_equal_fn_ equal_);
	static void table_clear_(hxhash_node_base** table_, hxsize_t bucket_count_,
		hxhash_deleter_fn_ deleter_, void* deleter_context_);
};
/// \endcond

/// `hxhash_table` - A hash table that operates without reallocating memory or
/// copying data. Each bucket uses an embedded singly-linked list. This design
/// tries to use a sparse table that allocates the maximum amount of memory
/// upfront and then uses a high quality hash instead of relying on linear
/// probing for cache efficiency. Hash tables can act as either an unordered map
/// or an unordered set and support operations that allow for unique or
/// duplicate keys.
///
/// Any node `T` derived from `hxhash_node_base` and using key `K` will work as
/// long as `K` has an `operator==` or an `hxkey_equal_t` specialization.
///
/// ```
/// class T : public hxhash_node_base {
///   using key_t = K;               // Tell the hash table what key to use.
///   const key_t& hash_key() const; // Returns key constructed with.
/// };
/// ```
///
/// `hxhash_table_set_node` and `hxhash_table_map_node` are provided and
/// recommended as replacements for `std::unordered_set` and
/// `std::unordered_map`. Custom key types will require either an `operator==`
/// or an `hxkey_equal_t` specialization and will require an `hxkey_hash_t`
/// specialization.
///
/// They might be used as follows:
///
/// ```
/// // An unordered set of allowed or blocked internet addresses.
/// using ipv6_set_t = hxhash_table<hxhash_table_set_node<ipv6_address_t>>;
///
/// // A fixed-size unordered map of material identifiers to material
/// // properties. Missing materials can be safely resolved.
/// using material_db_t = hxhash_table<hxhash_table_map_node<material_id_t,
///                                  material_t>, hxdefault_delete, true, 1024>;
/// ```
///
/// `hx/hxhash_table_nodes.hpp` also provides specializations of the
/// `hxhash_table::node_t` template parameter for integers and strings.
///
/// - `node_t` : Must implement the interface/concept described above.
/// - `deleter_t` : A class type invoked as `deleter(T*)` to free the owned
///    pointer. See also `hxdo_not_delete`.
/// - `multi_t` : When `false`and a node with an equal key already exists,
///   insertion into the list will fail when a node with the same key already
///   exists.
/// - `table_size_bits` : If non-zero, `table_size_bits` configures the hash
///   table size to `2^table_size_bits`. Otherwise use `set_size_bits` to
///   configure hash bits dynamically.
template<hxhash_table_concept_ node_t_,
	typename deleter_t_=hxdefault_delete,
	bool multi_t_ = false,
	uint32_t table_size_bits_=hxallocator_dynamic_capacity>
class hxhash_table : private deleter_t_, private hxhash_table_base_ {
public:
	using node_t = node_t_;
	using key_t = typename node_t_::key_t;

	/// `const_iterator` - A const forward iterator over the elements of the
	/// hash table. Iteration is O(`n + (1 << table_size_bits)`). Iterators are
	/// only invalidated by the removal of the `node_t` referenced. WARNING: The
	/// iterators will automatically convert themselves to pointers when used in
	/// pointer context.
	class const_iterator
	{
	public:
		/// Constructs an iterator pointing to the end of the hash table.
		const_iterator(void) : m_next_bucket_(hxnull), m_bucket_end_(hxnull),
			m_current_node_(hxnull) { }
		/// Advances the iterator to the next element.
		const_iterator& operator++(void);
		/// Advances the iterator to the next element (post-increment).
		const_iterator operator++(int) { const_iterator t_(*this); operator++(); return t_; }
		/// Compares two iterators for equality.
		/// - `x` : The iterator to compare against.
		bool operator==(const const_iterator& x_) const;
#if HX_CPLUSPLUS < 202002L // C++20 defaults != from ==.
		/// Compares two iterators for inequality.
		/// - `x` : The iterator to compare against.
		bool operator!=(const const_iterator& x_) const { return !(*this == x_); }
#endif
		/// Dereferences the iterator to access the current `node_t`.
		const node_t_& operator*(void) const;
		/// Dereferences the iterator to access the current `node_t`'s pointer.
		const node_t_* operator->(void) const;
	private:
		/// \cond HIDDEN
		friend class hxhash_table;
		const_iterator(const hxhash_table* table_);
		const_iterator(const hxhash_table* table_, node_t_* node_);
		void next_bucket_(void);
		hxhash_node_base** m_next_bucket_;
		hxhash_node_base** m_bucket_end_;
	protected:
		node_t_* m_current_node_;
		/// \endcond
	};

	/// `iterator` - A mutable iterator that can modify the elements of the hash
	/// table.
	class iterator : public const_iterator
	{
	public:
		/// Constructs an iterator pointing to the end of the hash table.
		iterator(void) { }
		/// Advances the iterator to the next element.
		iterator& operator++(void) { const_iterator::operator++(); return *this; }
		/// Advances the iterator to the next element (post-increment).
		iterator operator++(int) { iterator t_(*this); const_iterator::operator++(); return t_; }
		/// Dereferences the iterator to access the current `node_t`.
		node_t_& operator*(void) const;
		/// Dereferences the iterator to access the current `node_t`'s pointer.
		node_t_* operator->(void) const;
	private:
		/// \cond HIDDEN
		friend class hxhash_table;
		iterator(hxhash_table* table_) : const_iterator(table_) { }
		iterator(hxhash_table* table_, node_t_* node_) : const_iterator(table_, node_) { }
		iterator(const const_iterator& src_) : const_iterator(src_) { }
		/// \endcond
	};

	/// Constructs an empty hash table with a capacity of `2^table_size_bits`
	/// and an optional deleter instance.
	/// - `deleter` : Callable with signature `bool deleter(node_t_*)`.
	explicit hxhash_table(deleter_t_ deleter_=deleter_t_());

	/// Destructs the hash table and deletes all resources.
	~hxhash_table(void) { this->clear(this->deleter()); }

	/// Links each `node_t` from a temporary range into this hash table by
	/// address, exactly as `insert(node_t*)` would. The range's nodes are
	/// threaded into the table in place and are not copied or allocated, so
	/// `range` must own storage it is relinquishing to the table.
	/// - `range` : The range of nodes to link into the table.
	template<hxrange_concept_ range_t_,
		hxenable_if_t<!hxis_lvalue_reference<range_t_>(), int> = 0>
	void add_range(range_t_&& range_) noexcept;

#if HX_CPLUSPLUS >= 202302L
	/// Returns the result of calling `callable` with the first node matching
	/// `key`, otherwise returns `hxnil`. Use `and_then` to return `hxptr`,
	/// `hxref` or `hxexpected`.
	/// - `key` : The key to search for.
	/// - `callable` : The function to call with the found node.
	template<typename self_t_, typename callable_t_>
	hxattr_nodiscard auto and_then(
		this self_t_&& self_, const key_t& key_, callable_t_&& callable_)
		-> hxremove_cvref_t<decltype(
			hxforward<callable_t_>(callable_)(
				hxforward_like<self_t_>(hxdeclval<node_t_&>())))>;

	/// Iterator overload of `and_then`.
	/// - `it` : A const iterator to a node or `end()`.
	/// - `callable` : The function to call with the node.
	template<typename self_t_, typename callable_t_>
	hxattr_nodiscard auto and_then(
		this self_t_&& self_, const_iterator it_, callable_t_&& callable_)
		-> hxremove_cvref_t<decltype(
			hxforward<callable_t_>(callable_)(hxforward_like<self_t_>(
				hxdeclval<const node_t_&>())))>;
#endif // HX_CPLUSPLUS >= 202302L

	/// Returns a const iterator pointing to the beginning of the hash table.
	const_iterator begin(void) const { return const_iterator(this); }

	/// Returns an iterator pointing to the beginning of the hash table.
	iterator begin(void) { return iterator(this); }

	/// Returns the number of buckets in the hash table.
	hxattr_nodiscard hxsize_t bucket_count(void) const { return m_table_.capacity(); }

	/// Returns a const iterator pointing to the beginning of the hash table.
	const_iterator cbegin(void) const { return const_iterator(this); }

	/// Returns a const iterator pointing to the end of the hash table.
	const_iterator cend(void) const { return const_iterator(); }

	/// Removes all nodes and if `deleter` is true then calls `deleter()` on
	/// every node.
	/// - `deleter` : Callable with signature `bool deleter(node_t_*)`.
	template<typename deleter_u_>
	void clear(deleter_u_&& deleter_) noexcept;

	/// Removes all nodes and calls the stored deleter on every node.
	void clear(void) noexcept { this->clear(this->deleter()); }

	/// Counts the number of nodes with the given key.
	/// - `key` : The key to count occurrences of in the hash table.
	hxattr_nodiscard hxsize_t count(const typename node_t_::key_t& key_) const;

	/// Returns a const reference to the stored deleter.
	hxattr_nodiscard const deleter_t_& deleter(void) const;

	hxattr_nodiscard deleter_t_& deleter(void);

	/// `emplace` - Returns an iterator to the node constructed with `hxnew`.
	/// The table must have `multi_t` set to `true`.
	/// - `allocator` : The memory manager ID to use for allocation. Defaults to
	///   `hxsystem_allocator_current`.
	/// - `align` : Alignment to use when allocating new pointers. Defaults to
	///   `hxalignment`.
	/// - `args` : Arguments forwarded to the node constructor.
	template<hxsystem_allocator_t allocator_=hxsystem_allocator_current,
		hxalignment_t align_=hxalignment, bool multi_=multi_t_, class... args_t_>
	hxenable_if_t<multi_, iterator> emplace(args_t_&&... args_) noexcept;

	/// Checks if the hash table is empty.
	hxattr_nodiscard bool empty(void) const { return m_size_ == 0; }

	/// Returns a const iterator pointing to the end of the hash table.
	const_iterator end(void) const { return const_iterator(); }

	/// Returns an iterator pointing to the end of the hash table.
	iterator end(void) { return iterator(); }

	/// Releases all nodes matching key and calls `deleter` on every node.
	/// Returns the number of nodes released. Deleter can be functions with
	/// signature `void deleter(node_t*)` and callables supporting
	/// `operator()(node_t*)` and with an `operator bool`. e.g., a free list or
	/// a null pointer.
	/// - `key` : The key to search for and remove from the hash table.
	/// - `deleter` : Callable with signature `bool deleter(node_t_*)`.
	template<typename deleter_u_>
	hxsize_t erase(const typename node_t_::key_t& key_, deleter_u_&& deleter_) noexcept;

	/// Removes and calls the stored deleter on nodes with an equivalent key.
	/// - `key` : The key to search for and remove from the hash table.
	hxsize_t erase(const typename node_t_::key_t& key_) noexcept;

	/// Removes and calls the stored deleter on the `node_t` referenced by
	/// `it`. Returns an iterator to the node following `it`, or `end()`.
	/// - `it` : A valid iterator into this hash table.
	iterator erase(const const_iterator& it_) noexcept;

	/// `extract` - Returns an `hxptr` owning the first `node_t` with the given
	/// key, or an empty `hxptr` if no matching node is found.
	/// - `key` : The key to search for and remove from the hash table.
	hxptr<node_t_, deleter_t_> extract(const typename node_t_::key_t& key_) noexcept;

	/// Removes the `node_t` referenced by `it` and returns an `hxptr` owning
	/// it.
	/// - `it` : A valid iterator into this hash table.
	hxptr<node_t_, deleter_t_> extract(const const_iterator& it_) noexcept;

	/// Returns an iterator to a `node_t` matching `key`, or `end()` if none
	/// exists. If `previous` is not `end()` it must be an iterator previously
	/// returned from `find()` with the same key that has not been removed. Then
	/// `find()` will return a subsequent node if any.
	/// - `key` : The key to search for in the hash table.
	/// - `previous` : A previously found iterator with the same key, or
	///   `end()`.
	hxattr_nodiscard hxattr_flatten const_iterator find(
		const typename node_t_::key_t& key_, const const_iterator& previous_=const_iterator()) const;

	hxattr_nodiscard hxattr_flatten iterator find(
		const typename node_t_::key_t& key_, const const_iterator& previous_=const_iterator());

	/// Returns `true` if a node matching `key` exists.
	/// - `key` : The key to search for.
	hxattr_nodiscard bool has_value(const key_t& key_) const;

	/// `insert` - Returns an iterator to the inserted node. When `multi_t` is
	/// `false` and a node with an equal key already exists, invokes `deleter_t`
	/// on `ptr` and returns an iterator to the existing node. `ptr` must not be
	/// null.
	/// - `ptr` : The node to insert.
	iterator insert(node_t_* ptr_) noexcept;

	/// `insert` - Returns an iterator to the inserted node. When `multi_t` is
	/// `false` and a node with an equal key already exists, returns an iterator
	/// to the existing node and `ptr` is destroyed, invoking its deleter.
	/// - `ptr` : The `hxptr` owning the node to insert.
	template<typename deleter_u_>
	iterator insert(hxptr<node_t_, deleter_u_>&& ptr_) noexcept;

	/// Returns the average number of nodes per bucket.
	hxattr_nodiscard float load_factor(void) const;

	/// Returns the size of the largest bucket.
	hxattr_nodiscard hxsize_t load_max(void) const;

#if HX_CPLUSPLUS >= 202302L
	/// Returns an iterator to the first node matching `key`, or the result
	/// of calling `callable` when lookup fails.
	/// - `key` : The key to search for.
	/// - `callable` : The function to call when lookup fails. Must return an
	///   iterator.
	template<typename self_t_, typename callable_t_>
	hxattr_nodiscard auto or_else(
		this self_t_&& self_, const key_t& key_, callable_t_&& callable_)
		-> decltype(self_.end());

	/// Iterator overload of `or_else`.
	/// - `it` : An iterator to a node or `end()`.
	/// - `callable` : The function to call for `end()`. Must return an
	///   iterator.
	template<typename self_t_, typename callable_t_>
	hxattr_nodiscard auto or_else(
		this self_t_&& self_, const_iterator it_, callable_t_&& callable_)
		-> decltype(self_.end());
#endif // HX_CPLUSPLUS >= 202302L

	/// Clears the hash table without deleting any nodes.
	void release_all(void);

	/// Removes all nodes matching the given key without deleting them.
	/// - `key` : The key to search for and release from the hash table.
	hxsize_t release_key(const typename node_t_::key_t& key_);

	/// `replace` - Replaces the first `node_t` matching `ptr`'s key, if any,
	/// with `ptr` and returns an `hxptr` owning the replaced node. Returns an
	/// empty `hxptr` and inserts `ptr` as a new node if no match is found.
	/// Avoids the extra lookup an `erase` followed by `insert` would require.
	/// `ptr` must not be null.
	/// - `ptr` : The node to insert in place of any matching node.
	hxptr<node_t_, deleter_t_> replace(node_t_* ptr_) noexcept;

	/// Sets the number of hash bits and allocate memory for the table. (only
	/// for dynamic capacity).
	/// - `bits` : The number of hash bits to set for the hash table.
	void set_size_bits(hxhash_t bits_);

	/// Returns the number of elements in the hash table.
	hxattr_nodiscard hxsize_t size(void) const { return m_size_; }

	/// `try_emplace` - Returns an iterator to the node if it was inserted, or
	/// an iterator to the existing node if a node with an equal key already
	/// exists. The table must have `multi_t` set to `false`.
	/// - `allocator` : The memory manager ID to use for allocation. Defaults to
	///   `hxsystem_allocator_current`.
	/// - `align` : Alignment to use when allocating new pointers. Defaults to
	///   `hxalignment`.
	/// - `key` : The key the node will have once constructed.
	/// - `args` : Arguments forwarded to the node constructor.
	template<hxsystem_allocator_t allocator_=hxsystem_allocator_current,
		hxalignment_t align_=hxalignment, bool multi_=multi_t_, class... args_t_>
	hxenable_if_t<!multi_, iterator> try_emplace(
		const typename node_t_::key_t& key_, args_t_&&... args_) noexcept;

#if HX_CPLUSPLUS >= 202302L
	/// Returns an iterator to the first node matching `key`, or an iterator to
	/// `default_value` when lookup fails.
	/// - `key` : The key to search for.
	/// - `default_value` : The node to reference when lookup fails.
	template<typename self_t_> hxattr_nodiscard
	auto value_or(this self_t_&& self_, const key_t& key_, const node_t_* default_value_)
		-> decltype(self_.end());

	/// Iterator overload of `value_or`.
	/// - `it` : A const iterator to a node or `end()`.
	/// - `default_value` : The node to reference for `end()`.
	template<typename self_t_> hxattr_nodiscard
	auto value_or(this self_t_&& self_, const_iterator it_, const node_t_* default_value_)
		-> decltype(self_.end());
#endif // HX_CPLUSPLUS >= 202302L

private:
	static_assert(table_size_bits_ < hxhash_bits, "Hash bits must be [0..hxhash_bits)");

	// Deleted for being bug prone.
	hxhash_table(const hxhash_table&) = delete;

	hxhash_node_base** get_bucket_head_(hxhash_t hash_);
	const hxhash_node_base*const* get_bucket_head_(hxhash_t hash_) const;

	hxsize_t m_size_;
	hxdetail_::hxpow2_allocator_<hxhash_node_base*, table_size_bits_, true> m_table_;
};

#include "detail/hxhash_table.inl"
HX_NS_END_
