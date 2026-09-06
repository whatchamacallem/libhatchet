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

/// `hxhash_table_node_integer` - `node_t` for use with `hxhash_table` for
/// integer types. See the documentation of `hxhash_table` for the required
/// interface. Copying and assignment are unimplemented and the key must be
/// immutable in any assignment operation in a subclass.
template<typename key_t_>
class hxhash_table_node_integer : public hxhash_node_base {
public:
	using key_t = key_t_;

	/// Constructs a node wrapping the key and caches its hash.
	/// - `key` : Key value represented by the node.
	hxhash_table_node_integer(const key_t_& key_) :
		hxhash_node_base(hxkey_hash(key_)), m_key_(key_) { }

	/// Assignment is undefined. The key is immutable.
	hxhash_table_node_integer(void) = delete;
	hxhash_table_node_integer(const hxhash_table_node_integer& src_) = delete;
	hxhash_table_node_integer& operator=(const hxhash_table_node_integer& x_) = delete;

	/// The key and hash identify the `node_t` and should not change once added.
	const key_t_& hash_key(void) const { return m_key_; }
	/// Returns the cached hash value for the stored key.
	hxhash_t hash_value(void) const { return this->hash_val; }
	/// Returns the hash of a key without constructing a node.
	/// - `key` : The key to hash.
	static hxhash_t hash_value(const key_t_& key_) { return hxkey_hash(key_); }

private:
	key_t_ m_key_;
};

/// `hxhash_table_node_string_literal` - Subclass of `hxhash_table_set_node` for
/// static C strings. This code expects the provided strings to outlive the
/// container because it is intended for use with string literals.
class hxhash_table_node_string_literal
	: public hxhash_table_set_node<const char*> {
public:
	/// Constructor initializes the node with a string key and computes its
	/// hash. Copying and assignment are unimplemented and the key must be
	/// immutable in any assignment operation in a subclass.
	/// - `k` : Non-null string key used to initialize the node. The string must
	///   outlive the node.
	hxattr_nonnull(2) hxhash_table_node_string_literal(const char* k_)
		: hxhash_table_set_node<const char*>(k_) { }

	/// Assignment is undefined. The key is immutable.
	hxhash_table_node_string_literal(void) = delete;
	hxhash_table_node_string_literal(const hxhash_table_node_string_literal&) = delete;
	void operator=(const hxhash_table_node_string_literal&) = delete;
};

/// `hxhash_table_node_string` - Subclass of `hxhash_table_set_node` for C
/// strings. Allocates a copy, resulting in a string pool per hash table. The
/// key is stored as a pointer to `const` to keep the hash table code
/// const-correct. Copying and assignment are unimplemented and the key must be
/// immutable in any assignment operation in a subclass.
template <hxsystem_allocator_t allocator_=hxsystem_allocator_heap>
class hxhash_table_node_string : public hxhash_table_set_node<const char*> {
public:
	/// Constructor allocates and duplicates the string key, then initializes
	/// the node.
	/// - `k` : The string key to allocate, duplicate, and initialize the node
	///   with.
	hxhash_table_node_string(const char* k_)
		: hxhash_table_set_node<const char*>(hxstring_duplicate(k_, allocator_)) { }

	/// Destructor frees the allocated string key.
	~hxhash_table_node_string(void) { hxfree(const_cast<char *>(this->hash_key())); }

	/// Assignment is undefined. The key is immutable.
	hxhash_table_node_string(void) = delete;
	hxhash_table_node_string(const hxhash_table_node_string&) = delete;
	void operator=(const hxhash_table_node_string&) = delete;
};

HX_NS_END_
