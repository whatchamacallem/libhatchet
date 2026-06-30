#pragma once
// SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
// SPDX-License-Identifier: MIT
// This file is licensed under the MIT license found in the LICENSE.md file.

/// \file
/// A fixed-size hash table with embedded singly-linked list buckets.

#include "libhatchet.h"

// HX_USE_MACROS_WITH_MODULE allows including macros alongside the module.
#if HX_USE_MACROS_WITH_MODULE
#error Header does not provide macros alone.
#endif

#include "hxkey.hpp"
#include "hxptr.hpp"
#include "hxutility.h"
#include "hxallocator.hpp"

HX_NS_BEGIN_

#include "detail/hxhash_table_detail.hpp"

#if HX_CPLUSPLUS >= 202002L
/// `hxhash_table_concept` - Concept capturing the interface requirements for
/// `hxhash_table` nodes.
template<typename node_t_>
concept hxhash_table_concept_ =
	requires(node_t_& node_, const node_t_& const_node_) {
		sizeof(node_t_);
		sizeof(typename node_t_::key_t);
		node_.set_hash_next(static_cast<node_t_*>(hxnull));
		{ const_node_.hash_next() } -> hxsame_as<node_t_*>;
		{ const_node_.hash_key() } -> hxconvertible_to<const typename node_t_::key_t&>;
		{ const_node_.hash_value() } -> hxconvertible_to<hxhash_t>;
	};
#else
#define hxhash_table_concept_ typename
#endif

/// `hxhash_table_set_node` - Optional base class for unordered set entries.
/// Caches the hash value. See `hxhash_table_map_node` if you need a map node.
/// Copy and move construction produce an unlinked node with the same key and
/// hash. Copy and move assignment leave the linkage of either node unchanged.
/// The hash table uses duck typing, so only the interface is required.
template<typename key_t_>
class hxhash_table_set_node {
public:
	using key_t = key_t_;

	/// Constructs a node from the key and caches its hash for reuse.
	/// - `key` : Key used to identify the node.
	template<typename ref_t_>
	hxhash_table_set_node(ref_t_&& key_)
		: m_hash_next_(hxnull), m_key_(hxforward<ref_t_>(key_)), m_hash_(hxkey_hash(m_key_)) { }

	/// Constructs an unlinked node with the same key.
	hxhash_table_set_node(const hxhash_table_set_node& src_)
		: m_hash_next_(hxnull), m_key_(src_.m_key_), m_hash_(src_.m_hash_) { }

	/// Returns the next node in the table's embedded linked list.
	hxhash_table_set_node* hash_next(void) const { return m_hash_next_; }

	/// Sets the next node in the table's embedded linked list.
	/// - `next` : The new next node pointer.
	void set_hash_next(hxhash_table_set_node* next_) { m_hash_next_ = next_; }

	/// The key and hash identify the `node_t` and should not change once added.
	const key_t_& hash_key(void) const { return m_key_; }

	/// Returns the cached hash value for the stored key. Hash values are not
	/// required to be unique.
	hxhash_t hash_value(void) const { return m_hash_; }

	/// Returns the hash value computed for `key`. Hash values are not required
	/// to be unique.
	/// - `key` : The key to hash.
	static hxhash_t hash_value(key_t_ key_) { return hxkey_hash(key_); }

private:
	hxhash_table_set_node(void) = delete;
	// Deleted for being bug prone and pointless.
	hxhash_table_set_node& operator=(const hxhash_table_set_node& n_) = delete;

	// The hash table uses m_hash_next_ to implement an embedded linked list.
	hxhash_table_set_node* m_hash_next_;
	key_t_ m_key_;
	hxhash_t m_hash_;
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

	/// Returns the next node in the table's embedded linked list.
	hxhash_table_map_node* hash_next(void) const {
		return static_cast<hxhash_table_map_node*>(
			hxhash_table_set_node<key_t_>::hash_next());
	}

	/// Returns the stored value.
	const value_t_& value(void) const { return m_value_; }

	/// Returns the stored value, allowing mutation.
	value_t_& value(void) { return m_value_; }

private:
	value_t_ m_value_;
};

/// `hxhash_table` - A hash table that operates without reallocating memory or
/// copying data. Each bucket uses an embedded singly-linked list. This design
/// tries to use a sparse table that allocates the maximum amount of memory
/// upfront and then uses a high quality hash instead of relying on linear
/// probing for cache efficiency. Hash tables can act as either an unordered map
/// or an unordered set and support operations that allow for unique or
/// duplicate keys. While this interface is designed to feel familiar, code
/// using standard containers will need some adjustments. In particular, table
/// modification is non-standard. Iterators are provided but are not used to
/// modify the table.
///
/// Any node `T` using key `K` will work as long as it has the following fields
/// and `K` has an `operator==` or an `hxkey_equal` overload.
///
/// ```
/// class T {
///   using key_t = K;               // Tell the hash table what key to use.
///   T* hash_next() const;          // Next node in hxhash_table's embedded linked list.
///   void set_hash_next(T*);        // Sets the next node in the embedded linked list.
///   const key_t& hash_key() const; // Returns key constructed with.
///   hxhash_t hash_value() const;   // Returns hash of key constructed with.
/// };
/// ```
///
/// `hxhash_table_set_node` and `hxhash_table_map_node` are provided and
/// recommended as replacements for `std::unordered_set` and
/// `std::unordered_map`. Custom key types will require either an `operator==`
/// or an `hxkey_equal` overload and will require an `hxkey_hash` overload.
///
/// They might be used as follows:
///
/// ```
/// // An unordered set of allowed or blocked internet addresses.
/// using ipv6_set_t = hxhash_table<hxhash_table_set_node<ipv6_address_t>>;
///
/// // A fixed-size unordered map of material identifiers to material
/// // properties. Missing materials can be safely resolved.
/// using material_db_t = hxhash_table<hxhash_table_map_node<material_id_t, material_t>, hxdefault_delete, true, 1024>;
/// ```
///
/// `hx/hxhash_table_nodes.hpp` also provides specializations of the
/// `hxhash_table::node_t` template parameter for integers and strings.
///
/// `node_t` must implement the interface/concept described above. If non-zero,
/// `table_size_bits` configures the hash table size to `2^table_size_bits`.
/// Otherwise use `set_table_size_bits` to configure hash bits dynamically. See
/// `hxdo_not_delete` for situations where the table does not own the nodes.
/// When `multi_t` is `false` and a node with an equal key already exists,
/// `insert` invokes the deleter on the rejected node and returns an iterator to
/// the existing node. When `multi_t` is `true`, duplicate keys are always
/// inserted. In both cases `insert` returns an iterator.
template<hxhash_table_concept_ node_t_,
	typename deleter_t_=hxdefault_delete,
	bool multi_t_ = false,
	hxhash_t table_size_bits_=hxallocator_dynamic_capacity>
class hxhash_table {
public:
	using node_t = node_t_;
	using key_t = typename node_t_::key_t;

	/// `const_iterator` - A forward iterator over the const elements of the
	/// hash table. Iteration is Θ(`n + (1 << table_size_bits)`). Iterators are
	/// only invalidated by the removal of the `node_t` referenced. WARNING: The
	/// iterators will automatically convert themselves to pointers when used in
	/// pointer context.
	class const_iterator
	{
	public:
		/// Constructs an iterator pointing to the end of the hash table.
		const_iterator(void) : m_hash_table_(hxnull), m_next_index_(0), m_current_node_(hxnull) { }

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
		bool operator!=(const const_iterator& x_) const;
#endif

		/// Dereferences the iterator to access the current `node_t`.
		const node_t_& operator*(void) const;

		/// Dereferences the iterator to access the current `node_t`'s pointer.
		const node_t_* operator->(void) const;

	private:
		friend class hxhash_table;
		const_iterator(const hxhash_table* table_);
		const_iterator(const hxhash_table* table_, node_t_* node_);

		// Advance the iterator to the next non-empty bucket.
		void next_bucket(void);

		hxhash_table* m_hash_table_;
		// hxsize_t avoids zero-extension on 64-bit in next_bucket loop
		hxsize_t m_next_index_;

	protected:
		// Used by const_iterator.
		/// \cond HIDDEN
		node_t_* m_current_node_;
		/// \endcond
	};

	/// `iterator` - A mutable iterator that can modify the elements of the hash
	/// table.
	class iterator : public const_iterator
	{
	public:
		/// Constructs an iterator pointing to the beginning of the hash table.
		/// - `tbl` : The hash table to iterate over.
		iterator(hxhash_table* tbl_) : const_iterator(tbl_) { }

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
		friend class hxhash_table;
		iterator(hxhash_table* table_, node_t_* node_) : const_iterator(table_, node_) { }
	};

	/// Constructs an empty hash table with a capacity of `2^table_size_bits` and
	/// an optional deleter instance.
	/// - `deleter` : The deleter to invoke on nodes when erasing or clearing.
	explicit hxhash_table(deleter_t_ deleter_=deleter_t_());

	/// Destructs the hash table and deletes all resources.
	~hxhash_table(void) { this->clear(m_deleter_); }

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

	/// Removes all nodes and calls `deleter()` on every node. Deleter can be
	/// function pointers with signature `void deleter(node_t*)` or callables
	/// supporting `operator()(node_t*)` and `operator bool`. deleter could be
	/// a free list or a null function pointer.
	/// - `deleter` : A function or callable to call on each removed `node_t`.
	template<typename deleter_override_t_>
	void clear(const deleter_override_t_& deleter_) noexcept;

	/// Removes all nodes and calls the stored deleter on every node.
	void clear(void) noexcept { this->clear(m_deleter_); }

	/// Counts the number of nodes with the given key.
	/// - `key` : The key to count occurrences of in the hash table.
	hxattr_nodiscard hxsize_t count(const typename node_t_::key_t& key_) const;

	/// `emplace` - Returns an iterator to the node constructed with `hxnew`.
	/// The table must have `multi_t` set to `true`.
	/// - `allocator` : The memory manager ID to use for allocation. Defaults to
	///   `hxsystem_allocator_current`.
	/// - `align` : Alignment to use when allocating new pointers. Defaults to
	///   `hxalignment`.
	/// - `args` : Arguments forwarded to the node constructor.
	template<hxsystem_allocator_t allocator_=hxsystem_allocator_current, hxalignment_t align_=hxalignment,
		bool multi_=multi_t_, class... args_t_>
	hxenable_if_t<multi_, iterator> emplace(args_t_&&... args_) noexcept;

	/// Checks if the hash table is empty.
	hxattr_nodiscard bool empty(void) const { return m_size_ == 0; }

	/// Returns a const iterator pointing to the end of the hash table.
	const_iterator end(void) const { return const_iterator(); }

	/// Returns an iterator pointing to the end of the hash table.
	iterator end(void) { return iterator(); }

	/// Releases all nodes matching key and calls `deleter` on every node. Returns
	/// the number of nodes released. Deleter can be functions with signature `void
	/// deleter(node_t*)` and callables supporting `operator()(node_t*)` and with
	/// an `operator bool`. e.g., a free list or a null pointer.
	/// - `key` : The key to search for and remove from the hash table.
	/// - `deleter` : A function or callable to call on each removed `node_t`.
	template<typename deleter_override_t_>
	hxsize_t erase(const typename node_t_::key_t& key_, const deleter_override_t_& deleter_) noexcept;

	/// Removes and calls the stored deleter on nodes with an equivalent key.
	/// - `key` : The key to search for and remove from the hash table.
	hxsize_t erase(const typename node_t_::key_t& key_) noexcept {
		return this->erase(key_, m_deleter_);
	}

	/// `extract` - Returns an `hxptr` owning the first `node_t` with the given
	/// key, or an empty `hxptr` if no matching node is found.
	/// - `key` : The key to search for and remove from the hash table.
	hxptr<node_t_, deleter_t_> extract(const typename node_t_::key_t& key_) noexcept;

	/// Returns a `node_t` matching key if any. If previous is non-null it must be
	/// a node previously returned from `find()` with the same key and that has not
	/// been removed. Then `find()` will return a subsequent node if any.
	/// The previous object is non-const as it may be modified.
	/// - `key` : The key to search for in the hash table.
	/// - `previous` : A previously found `node_t` with the same key, or hxnull.
	hxattr_nodiscard node_t_* find(
		const typename node_t_::key_t& key_, const node_t_* previous_=hxnull);

	/// `const` version of `find`.
	/// - `key` : The key to search for in the hash table.
	/// - `previous` : A previously found `node_t` with the same key, or hxnull.
	hxattr_nodiscard const node_t_* find(
		const typename node_t_::key_t& key_, const node_t_* previous_=hxnull) const;

	/// `insert` - Returns an iterator to the inserted node. When `multi_t` is
	/// `false` and a node with an equal key already exists, invokes `deleter_t`
	/// on `ptr` and returns an iterator to the existing node. `ptr` must not be
	/// null.
	/// - `ptr` : The node to insert.
	iterator insert(node_t_* ptr_) noexcept;

	/// `insert` - Returns an iterator to the inserted node. When `multi_t` is
	/// `false` and a node with an equal key already exists, returns an iterator
	/// to the existing node and invokes the deleter on `ptr`. `ptr` must not be
	/// null.
	/// - `ptr` : The `hxptr` owning the node to insert.
	template<typename ptr_deleter_t_>
	iterator insert(hxptr<node_t_, ptr_deleter_t_>&& ptr_) noexcept;

	/// Returns the average number of nodes per bucket.
	hxattr_nodiscard float load_factor(void) const;

	/// Returns the size of the largest bucket.
	hxattr_nodiscard hxsize_t load_max(void) const;

	/// Clears the hash table without deleting any nodes.
	void release_all(void);

	/// Removes all nodes matching the given key without deleting them.
	/// - `key` : The key to search for and release from the hash table.
	hxsize_t release_key(const typename node_t_::key_t& key_);

	/// Sets the number of hash bits and allocate memory for the table. (only
	/// for dynamic capacity).
	/// - `bits` : The number of hash bits to set for the hash table.
	void set_table_size_bits(hxhash_t bits_) { m_table_.set_table_size_bits(bits_); }

	/// Returns the number of elements in the hash table.
	hxattr_nodiscard hxsize_t size(void) const { return m_size_; }

	/// `try_emplace` - Returns an iterator to the node if it was inserted, or an
	/// iterator to the existing node if a node with an equal key already exists.
	/// The table must have `multi_t` set to `false`.
	/// - `allocator` : The memory manager ID to use for allocation. Defaults to
	///   `hxsystem_allocator_current`.
	/// - `align` : Alignment to use when allocating new pointers. Defaults to
	///   `hxalignment`.
	/// - `key` : The key the node will have once constructed.
	/// - `args` : Arguments forwarded to the node constructor.
	template<hxsystem_allocator_t allocator_=hxsystem_allocator_current, hxalignment_t align_=hxalignment,
		bool multi_=multi_t_, class... args_t_>
	hxenable_if_t<!multi_, iterator> try_emplace(
		const typename node_t_::key_t& key_, args_t_&&... args_) noexcept;

private:
	static_assert(table_size_bits_ < hxhash_bits, "Hash bits must be [0..hxhash_bits).");

	// Not ideal.
	hxhash_table(const hxhash_table&) = delete;

	// Pointer to head of singly-linked list for key's hash value.
	node_t_** get_bucket_head_(hxhash_t hash_);

	const node_t_*const* get_bucket_head_(hxhash_t hash_) const;

	deleter_t_ m_deleter_;
	hxsize_t m_size_;
	hxdetail_::hxhash_table_internal_allocator_<node_t_, table_size_bits_> m_table_;
};

#include "detail/hxhash_table.inl"
HX_NS_END_
