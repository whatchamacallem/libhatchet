#pragma once
// SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
// SPDX-License-Identifier: MIT
// This file is licensed under the MIT license found in the LICENSE.md file.

/// \file
/// These are versions of the `hxhash_table::node_t` template parameter for
/// integers and strings.

#include "libhatchet.h"

// HX_USE_MACROS_WITH_MODULE allows including macros alongside the module.
#if HX_USE_MACROS_WITH_MODULE
#error Header does not provide macros alone.
#endif

#include "hxhash_table.hpp"

HX_NS_BEGIN_

/// `hxhash_table_node_integer` - `node_t` for use with `hxhash_table` for integer
/// types. See the documentation of `hxhash_table` for the required interface.
/// Copy and move construction produce an unlinked node with the same key and
/// hash. Copy and move assignment leave the linkage of either node unchanged.
/// This serves as an example of a `node_t` that does not use a base class.
template<typename key_t_>
class hxhash_table_node_integer {
public:
	using key_t = key_t_;

	/// Constructs a node wrapping the key and caches its hash.
	/// - `key` : Key value represented by the node.
	hxhash_table_node_integer(const key_t_& key_) :
		m_hash_next_(hxnull), m_key_(key_), m_hash_(hxkey_hash(key_)) { }

	/// Constructs an unlinked node with the same key.
	hxhash_table_node_integer(const hxhash_table_node_integer& src_) :
		m_hash_next_(hxnull), m_key_(src_.m_key_), m_hash_(src_.m_hash_) { }

	/// Assigns nothing. Hash table linkage of either node is not affected.
	hxhash_table_node_integer& operator=(const hxhash_table_node_integer& other_) {
		hxassertmsg(this != &other_, "self_assignment"); (void)other_;
		return *this;
	}

	/// Returns the next node in the table's embedded linked list.
	hxhash_table_node_integer* hash_next(void) const { return m_hash_next_; }
	/// Sets the next node in the table's embedded linked list.
	/// - `next` : The new next node pointer.
	void set_hash_next(hxhash_table_node_integer* next_) { m_hash_next_ = next_; }

	/// The key and hash identify the `node_t` and should not change once added.
	const key_t_& hash_key(void) const { return m_key_; }
	/// Returns the cached hash value for the stored key.
	hxhash_t hash_value(void) const { return m_hash_; }
	/// Returns the hash of a key without constructing a node.
	/// - `key` : The key to hash.
	static hxhash_t hash_value(key_t_ key_) { return hxkey_hash(key_); }

private:
	hxhash_table_node_integer(void) = delete;

	hxhash_table_node_integer* m_hash_next_;
	key_t_ m_key_;
	hxhash_t m_hash_;
};

/// `hxhash_table_node_string_literal` - Subclass of `hxhash_table_set_node` for
/// static C strings. This code expects the provided strings to outlive the
/// container because it is intended for use with string literals.
class hxhash_table_node_string_literal
	: public hxhash_table_set_node<const char*> {
public:
	/// Constructor initializes the node with a string key and computes its hash.
	/// - `k` : Non-null string key used to initialize the node. The string must
	///   outlive the node.
	hxattr_nonnull(2) hxhash_table_node_string_literal(const char* k_)
		: hxhash_table_set_node<const char*>(k_) { }

	/// Returns the next node in the table's embedded linked list.
	hxhash_table_node_string_literal* hash_next(void) const {
		return static_cast<hxhash_table_node_string_literal*>(
			hxhash_table_set_node<const char*>::hash_next());
	}
};

/// `hxhash_table_node_string` - Subclass of `hxhash_table_set_node` for C
/// strings. Allocates a copy, resulting in a string pool per hash table. The
/// key is stored as a pointer to `const` to keep the hash table code
/// const-correct.
template <hxsystem_allocator_t allocator_=hxsystem_allocator_heap>
class hxhash_table_node_string : public hxhash_table_set_node<const char*> {
public:
	/// Constructor allocates and duplicates the string key, then initializes the
	/// node.
	/// - `k` : The string key to allocate, duplicate, and initialize the node with.
	hxhash_table_node_string(const char* k_)
		: hxhash_table_set_node<const char*>(hxstring_duplicate(k_, allocator_)) { }

	/// Destructor frees the allocated string key.
	~hxhash_table_node_string(void) { hxfree(const_cast<char *>(this->hash_key())); }

	/// Returns the next node in the table's embedded linked list.
	hxhash_table_node_string* hash_next(void) const {
		return static_cast<hxhash_table_node_string*>(
			hxhash_table_set_node<const char*>::hash_next());
	}

private:
	// Copying is disallowed as it would involve a string copy.
	hxhash_table_node_string(void) = delete;
	hxhash_table_node_string(const hxhash_table_node_string&) = delete;
	void operator=(const hxhash_table_node_string&) = delete;
};

HX_NS_END_
