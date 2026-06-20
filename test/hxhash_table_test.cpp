// SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
// SPDX-License-Identifier: MIT
// This file is licensed under the MIT license found in the LICENSE.md file.

#include <hx/hxhash_table_nodes.hpp>
#include <hx/hxhash_table.hpp>
#include <hx/hxtest.hpp>

hxattr_noinline static void hxtest_gdb_break_hxhash_table(void) {}

namespace {

class hxhash_table_test_f* s_hxtest_current = 0;

class hxhash_table_test_f :
	public testing::Test
{
public:
	class hxtest_object {
	public:
		hxtest_object(void) {
			++s_hxtest_current->m_constructed;
			id = s_hxtest_current->m_next_id++;
		}
		~hxtest_object(void) {
			++s_hxtest_current->m_destructed;
			id = -1;
		}

		void operator=(const hxtest_object& x) { id = x.id; }
		void operator=(int32_t x) { id = x; }
		bool operator==(const hxtest_object& x) const { return id == x.id; }
		bool operator==(int32_t x) const { return id == x; }

		// XXX???
		operator float(void) const { return static_cast<float>(id); }

		int32_t id;
	};

	class hxtest_integer : public hxhash_table_node_integer<int32_t> {
	public:
		hxtest_integer(int32_t k) : hxhash_table_node_integer(k) { }
		hxtest_integer* hash_next(void) const {
			return static_cast<hxtest_integer*>(hxhash_table_node_integer::hash_next());
		}
		hxtest_integer*& hash_next(void) {
			return reinterpret_cast<hxtest_integer*&>(hxhash_table_node_integer::hash_next());
		}
		hxtest_object value;
	};

	class hxtest_string : public hxhash_table_node_string<hxsystem_allocator_temporary_stack> {
	public:
		hxtest_string(const char* k) : hxhash_table_node_string(k) { }
		hxtest_string* hash_next(void) const {
			return static_cast<hxtest_string*>(hxhash_table_node_string::hash_next());
		}
		hxtest_string*& hash_next(void) {
			return reinterpret_cast<hxtest_string*&>(hxhash_table_node_string::hash_next());
		}
		hxtest_object value;
	};

	class hxtest_string_literal : public hxhash_table_node_string_literal {
	public:
		hxtest_string_literal(const char* k) : hxhash_table_node_string_literal(k) { }
		hxtest_string_literal* hash_next(void) const {
			return static_cast<hxtest_string_literal*>(hxhash_table_node_string_literal::hash_next());
		}
		hxtest_string_literal*& hash_next(void) {
			return reinterpret_cast<hxtest_string_literal*&>(hxhash_table_node_string_literal::hash_next());
		}
	};

	hxhash_table_test_f(void) {
		hxassert(s_hxtest_current == hxnull);
		m_constructed = 0;
		m_destructed = 0;
		m_next_id = 0;
		s_hxtest_current = this;
	}
	~hxhash_table_test_f(void) override {
		s_hxtest_current = 0;
	}

	int32_t m_constructed;
	int32_t m_destructed;
	int32_t m_next_id;
};

// A plain hxhash_table_node_integer subclass used to test copy/move operators.
struct hxtest_integer_node_t : hxhash_table_node_integer<int32_t> {
	explicit hxtest_integer_node_t(int32_t k) : hxhash_table_node_integer<int32_t>(k) { }
	hxtest_integer_node_t* hash_next(void) const {
		return static_cast<hxtest_integer_node_t*>(hxhash_table_node_integer::hash_next());
	}
	hxtest_integer_node_t*& hash_next(void) {
		return reinterpret_cast<hxtest_integer_node_t*&>(hxhash_table_node_integer::hash_next());
	}
};

// A plain hxhash_table_set_node subclass used to test copy/move operators.
struct hxtest_set_node_t : hxhash_table_set_node<int32_t> {
	explicit hxtest_set_node_t(int32_t k) : hxhash_table_set_node<int32_t>(k) { }
	hxtest_set_node_t* hash_next(void) const {
		return static_cast<hxtest_set_node_t*>(hxhash_table_set_node::hash_next());
	}
	hxtest_set_node_t*& hash_next(void) {
		return reinterpret_cast<hxtest_set_node_t*&>(hxhash_table_set_node::hash_next());
	}
};

} // namespace {

TEST_F(hxhash_table_test_f, null) {
	{
		using table_t = hxhash_table<hxtest_integer, 4, false>;
		table_t table;
		EXPECT_EQ(table.size(), 0u);
		const table_t& const_table = table;

		// "Returns an iterator pointing to the beginning of the hash table." Empty table => begin == end and load factor 0.
		EXPECT_EQ(table.begin(), table.end());
		EXPECT_EQ(table.cbegin(), table.cend());
		EXPECT_EQ(const_table.begin(), const_table.cend());
		EXPECT_EQ(table.begin(), table.end());
		EXPECT_EQ(table.cbegin(), table.cend());
		EXPECT_EQ(const_table.begin(), const_table.cend());

		// "Removes all nodes and calls deleter_t::operator() on every node." Clearing untouched table keeps load factor { 0.0 }.
		table.clear();
		EXPECT_EQ(table.load_factor(), 0.0f);

	}
	EXPECT_EQ(m_constructed, 0);
	EXPECT_EQ(m_destructed, 0);
}

TEST_F(hxhash_table_test_f, single) {
const hxsystem_allocator_scope temporary_stack_scope(hxsystem_allocator_temporary_stack);

	static const int k = 77;
	{
		using table_t = hxhash_table<hxtest_integer, 4, false>;
		table_t table;
		const table_t& const_table = table;
		hxtest_integer* node = hxnew<hxtest_integer>(k);
		// insert returns an iterator to the inserted node on success.
		EXPECT_EQ(table.insert(hxptr<hxtest_integer>(node)), node);

		// Iterator + count checks confirm single-entry semantics.
		EXPECT_NE(table.begin(), table.end());
		EXPECT_NE(table.cbegin(), table.cend());
		EXPECT_EQ(++table.begin(), table.end());
		EXPECT_EQ(++table.cbegin(), table.cend());
		EXPECT_EQ(table.size(), 1u);
		EXPECT_EQ(table.count(k), 1u);

		// insert with a duplicate key returns an iterator to the existing node.
		hxtest_integer* dup = hxnew<hxtest_integer>(k);
		EXPECT_EQ(table.insert(hxptr<hxtest_integer>(dup)), node);

		// find() hit and miss, both mutable and const.
		EXPECT_EQ(table.find(k), node);
		EXPECT_EQ(table.find(k, table.begin()), hxnullptr);
		EXPECT_EQ(const_table.find(k), node);
		EXPECT_EQ(const_table.find(k, const_table.begin()), hxnullptr);

		// extract() hit and miss.
		EXPECT_EQ(table.extract(k), node);
		EXPECT_EQ(table.extract(k), hxnullptr);

		EXPECT_EQ(table.insert(hxptr<hxtest_integer>(node)), node);
		// release_all() removes all nodes without deleting them.
		table.release_all();
		EXPECT_EQ(table.find(k), hxnullptr);
		EXPECT_EQ(table.size(), 0u);
		EXPECT_EQ(table.count(k), 0u);

		// insert on an empty table inserts and returns an iterator to the node.
		hxtest_integer* new_node = hxnew<hxtest_integer>(k);
		EXPECT_EQ(table.insert(hxptr<hxtest_integer>(new_node)), new_node);
		EXPECT_NE(new_node->value.id, node->value.id);
		EXPECT_EQ(table.size(), 1u);

		// Destructor frees new_node. Node was already extracted above.
		hxdelete(node);
	}
	EXPECT_EQ(m_constructed, 3);
	EXPECT_EQ(m_destructed, 3);
}

TEST_F(hxhash_table_test_f, map_node_usage) {
	const hxsystem_allocator_scope temporary_stack_scope(hxsystem_allocator_temporary_stack);

	using map_node_t = hxhash_table_map_node<int32_t, hxtest_object>;
	using table_t = hxhash_table<map_node_t, 4, false>;
	{
		table_t table;

		// insert inserts a new node and returns an iterator to it when key is absent.
		map_node_t* n10 = hxnew<map_node_t>(10);
		EXPECT_EQ(table.insert(hxptr<map_node_t>(n10)), n10);
		EXPECT_EQ(table.find(10), n10);
		EXPECT_EQ(n10->hash_key(), 10);
		n10->value().id = 123;

		map_node_t* manual = hxnew<map_node_t>(20);
		manual->value().id = 321;
		// Link external allocation through insert to co-exist with first entry.
		EXPECT_EQ(table.insert(hxptr<map_node_t>(manual)), manual);

		EXPECT_EQ(table.size(), 2u);
		EXPECT_EQ(table.count(10), 1u);
		EXPECT_EQ(table.count(20), 1u);

		const table_t& const_table = table;
		const map_node_t* const_lookup = const_table.find(10);
		EXPECT_NE(const_lookup, hxnullptr);
		if(const_lookup != hxnullptr) {
			EXPECT_EQ(const_lookup->value().id, 123);
		}

		map_node_t* manual_lookup = table.find(20);
		EXPECT_NE(manual_lookup, hxnullptr);
		if(manual_lookup != hxnullptr) {
			EXPECT_EQ(manual_lookup->value().id, 321);
		}

		EXPECT_EQ(table.size(), 2u);
	}

	EXPECT_EQ(m_constructed, 2);
	EXPECT_EQ(m_destructed, 2);
}

TEST_F(hxhash_table_test_f, multiple) {
	static const int size_i = 78; // Used to do range checking modulo size.
	static const unsigned int size_u = 78u;
	const hxsystem_allocator_scope temporary_stack_scope(hxsystem_allocator_temporary_stack);
	{
		// Table will be overloaded.
		using table_t = hxhash_table<hxtest_integer>;
		table_t table;
		const table_t& const_table = table;
		// "Use set_table_size_bits to configure hash bits dynamically." Force 2^5 buckets before load test.
		table.set_table_size_bits(5);

		// Insert keys { 0..size-1 } with value.id mirroring the key.
		for(int i = 0; i < size_i; ++i) {
			hxtest_integer* n = hxnew<hxtest_integer>(i);
			table.insert(hxptr<hxtest_integer>(n));
			EXPECT_EQ(n->value.id, i);
			EXPECT_EQ(n->hash_key(), i);
		}

		// Check properties of size unique keys.
		int id_histogram[size_u] = {};
		EXPECT_EQ(table.size(), size_u);
		table_t::iterator it = table.begin();
		table_t::iterator cit = table.begin();
		for(int i = 0; i < size_i; ++i) {
			hxtest_integer* ti = table.find(i);
			EXPECT_EQ(ti->value, i);
			EXPECT_EQ(table.find(i, ti), hxnullptr);

			// Iteration over.
			EXPECT_NE(it, table.end());
			EXPECT_NE(cit, table.cend());
			EXPECT_EQ(it, cit);
			EXPECT_LT(static_cast<unsigned int>(it->value.id), size_u);
			id_histogram[it->value.id]++;
			EXPECT_LT(static_cast<unsigned int>(cit->value.id), size_u);
			id_histogram[cit->value.id]++;
			++cit;
			it++;
		}
		EXPECT_EQ(table.end(), it);
		EXPECT_EQ(table.cend(), cit);
		for(int i = 0; i < size_i; ++i) {
			EXPECT_EQ(id_histogram[i], 2);
		}

		// insert second size elements
		for(int i = 0; i < size_i; ++i) {
			hxtest_integer* ti = hxnew<hxtest_integer>(i);
			EXPECT_EQ(ti->value.id, i+size_i);
			table.insert(hxptr<hxtest_integer>(ti));
		}

		// Check properties of 2*size duplicate keys.
		int key_histogram[size_u] = {};
		EXPECT_EQ(table.size(), size_u * 2u);
		it = table.begin();
		cit = table.begin();
		for(int i = 0; i < size_i; ++i) {
			hxtest_integer* ti = table.find(i);
			EXPECT_EQ(ti->hash_key(), i);
			const hxtest_integer* ti2 = const_table.find(i, ti); // test const version
			EXPECT_EQ(ti2->hash_key(), i);
			EXPECT_EQ(table.find(i, ti2), hxnullptr);

			EXPECT_EQ(table.count(i), 2u);

			EXPECT_LT(static_cast<unsigned int>(it->hash_key()), size_u);
			key_histogram[it->hash_key()]++;
			++it;
			EXPECT_LT(static_cast<unsigned int>(it->hash_key()), size_u);
			key_histogram[it->hash_key()]++;
			it++;
			EXPECT_LT(static_cast<unsigned int>(cit->hash_key()), size_u);
			key_histogram[cit->hash_key()]++;
			++cit;
			EXPECT_LT(static_cast<unsigned int>(cit->hash_key()), size_u);
			key_histogram[cit->hash_key()]++;
			cit++;
		}
		EXPECT_EQ(table.end(), it);
		EXPECT_EQ(table.cend(), cit);
		for(int i = 0; i < size_i; ++i) {
			EXPECT_EQ(key_histogram[i], 4);
		}

		// load_max() should be within 4x the mean for a heavily loaded table.
		EXPECT_GT((table.load_factor() * 4.0f), (float)table.load_max());

		// Erase keys [0..size/2), remove 1 of 2 of keys [size/2..size)
		for(int i = 0; i < (size_i/2); ++i) {
			EXPECT_EQ(table.erase(i), 2);
		}
		for(int i = (size_i/2); i < size_i; ++i) {
			hxtest_integer* ti = table.extract(i);
			EXPECT_EQ(ti->hash_key(), i);
			hxdelete(ti);
		}

		// Check properties of size_i/2 remaining keys.
		for(int i = 0; i < (size_i/2); ++i) {
			EXPECT_EQ(table.release_key(i), 0);
			EXPECT_EQ(table.find(i), hxnullptr);
		}
		for(int i = (size_i/2); i < size_i; ++i) {
			hxtest_integer* ti = table.find(i);
			EXPECT_EQ(ti->hash_key(), i);
			EXPECT_EQ(table.find(i, ti), hxnullptr);
			EXPECT_EQ(table.count(i), 1u);
		}

		it = table.begin();
		cit = table.begin();
		for(int i = 0; i < (size_i/2); ++i) {
			++it;
			cit++;
		}
		EXPECT_EQ(table.end(), it);
		EXPECT_EQ(table.cend(), cit);

		hxtest_gdb_break_hxhash_table();
	}
	EXPECT_EQ(m_constructed, 2*size_i);
	EXPECT_EQ(m_destructed, 2*size_i);
}

TEST_F(hxhash_table_test_f, strings) {
	const hxsystem_allocator_scope temporary_stack_scope(hxsystem_allocator_temporary_stack);

	static const char* colors[] = {
		"Red","Orange","Yellow",
		"Green","Cyan","Blue",
		"Indigo","Violet" };
	const size_t sz = hxsize(colors);

	{
		using table_t = hxhash_table<hxtest_string, 4, false>;
		table_t table;

		// Insert colors in reverse. insert returns an iterator to the inserted node.
		for(size_t i = sz; i-- != 0;) {
			hxtest_string* n = hxnew<hxtest_string>(colors[i]);
			EXPECT_EQ(table.insert(hxptr<hxtest_string>(n)), n);
			EXPECT_STREQ(n->hash_key(), colors[i]);
		}
		EXPECT_NE(table.find("Cyan"), hxnullptr);
		EXPECT_EQ(table.find("Pink"), hxnullptr);

	}
	EXPECT_EQ(m_constructed, sz);
	EXPECT_EQ(m_destructed, sz);
}

TEST_F(hxhash_table_test_f, string_literal_nodes) {
	const hxsystem_allocator_scope temporary_stack_scope(hxsystem_allocator_temporary_stack);

	static const char* const literals[] = {
		"Crimson", "Teal", "Magenta", "Gold"
	};

	using table_t = hxhash_table<hxtest_string_literal, 4, false>;
	table_t table;

	for(unsigned int i = 0; i < hxsize(literals); ++i) {
		// String literal keys are owned externally. The node stores only the pointer.
		hxtest_string_literal* n = hxnew<hxtest_string_literal>(literals[i]);
		EXPECT_EQ(table.insert(hxptr<hxtest_string_literal>(n)), n);
		EXPECT_EQ(n->hash_key(), literals[i]);
		EXPECT_EQ(n->hash_value(), hxkey_hash(literals[i]));
	}

	EXPECT_EQ(table.size(), (size_t)hxsize(literals));
	EXPECT_NE(table.find("Crimson"), hxnullptr);
	EXPECT_EQ(table.find("Cerulean"), hxnullptr);
}

// copy/move construct produce insertable unlinked nodes with the same key and
// hash; copy/move assign leave table linkage of the destination unchanged.
TEST(hxhash_table_set_node_test, copy_move_construct_and_assign) {
	const hxtest_set_node_t a(1), b(2), c(3), d(4);
	using table_t = hxhash_table<hxtest_set_node_t, 4, false, hxdo_not_delete>;
	table_t table;
	// copy-construct: e is unlinked, key and hash match a.
	hxtest_set_node_t e(a);
	EXPECT_EQ(e.hash_key(), 1);
	EXPECT_EQ(e.hash_value(), a.hash_value());
	table.insert(hxptr<hxtest_set_node_t, hxdo_not_delete>(&e));
	// move-construct: f is unlinked, key and hash match b.
	hxtest_set_node_t f(hxmove(b));
	EXPECT_EQ(f.hash_key(), 2);
	EXPECT_EQ(f.hash_value(), b.hash_value());
	table.insert(hxptr<hxtest_set_node_t, hxdo_not_delete>(&f));
	EXPECT_EQ(table.size(), 2u);
	// copy-assign: e stays linked in the table.
	e = c;
	EXPECT_NE(table.find(1), hxnullptr);
	// move-assign: f stays linked in the table.
	f = hxmove(d);
	EXPECT_NE(table.find(2), hxnullptr);
	EXPECT_EQ(table.size(), 2u);
	table.release_all();
}

// hxhash_table_map_node used directly as node_t: hash_next must return
// hxhash_table_map_node* not hxhash_table_set_node*, exercising the override.
TEST(hxhash_table_map_node_test, hash_next_type) {
	using node_t = hxhash_table_map_node<int32_t, int32_t>;
	using table_t = hxhash_table<node_t, 4, true, hxdo_not_delete>;
	node_t a(1, 10), b(1, 20), c(2, 30);
	table_t table;
	table.insert(hxptr<node_t, hxdo_not_delete>(&a));
	table.insert(hxptr<node_t, hxdo_not_delete>(&b));
	table.insert(hxptr<node_t, hxdo_not_delete>(&c));
	EXPECT_EQ(table.size(), 3u);
	// find both duplicates via previous
	const node_t* first = table.find(1);
	ASSERT_NE(first, hxnullptr);
	const node_t* second = table.find(1, first);
	ASSERT_NE(second, hxnullptr);
	EXPECT_NE(first, second);
	EXPECT_EQ(table.find(1, second), hxnullptr);
	// iterator must traverse all three nodes without casting
	size_t count = 0u;
	for(table_t::const_iterator it = table.begin(); it != table.end(); ++it) {
		++count;
	}
	EXPECT_EQ(count, 3u);
	table.release_all();
}

// A subclass of hxhash_table_map_node that overrides hash_next to return its
// own type, allowing it to be used as node_t without casts in hxhash_table.
struct hxtest_map_node_t : hxhash_table_map_node<int32_t, int32_t> {
	explicit hxtest_map_node_t(int32_t k, int32_t v)
		: hxhash_table_map_node<int32_t, int32_t>(k, v) { }
	hxtest_map_node_t* hash_next(void) const {
		return static_cast<hxtest_map_node_t*>(hxhash_table_map_node::hash_next());
	}
	hxtest_map_node_t*& hash_next(void) {
		return reinterpret_cast<hxtest_map_node_t*&>(hxhash_table_map_node::hash_next());
	}
};

// A subclass of hxhash_table_map_node used as node_t: the concept enforces
// that hash_next returns hxtest_map_node_t* exactly.
TEST(hxhash_table_map_node_test, subclass_hash_next_type) {
	using table_t = hxhash_table<hxtest_map_node_t, 4, false, hxdo_not_delete>;
	hxtest_map_node_t a(1, 10), b(2, 20), c(3, 30);
	table_t table;
	table.insert(hxptr<hxtest_map_node_t, hxdo_not_delete>(&a));
	table.insert(hxptr<hxtest_map_node_t, hxdo_not_delete>(&b));
	table.insert(hxptr<hxtest_map_node_t, hxdo_not_delete>(&c));
	EXPECT_EQ(table.size(), 3u);
	hxtest_map_node_t* found = table.find(2);
	ASSERT_NE(found, hxnullptr);
	EXPECT_EQ(found->value(), 20);
	// release by key and verify size
	EXPECT_EQ(table.release_key(2), 1u);
	EXPECT_EQ(table.size(), 2u);
	EXPECT_EQ(table.find(2), hxnullptr);
	table.release_all();
}

// copy construct produces an insertable unlinked node with the same key and
// hash; copy assign leaves both key and linkage of the destination unchanged.
TEST(hxhash_table_node_integer_test, copy_move_construct_and_assign) {
	const hxtest_integer_node_t a(10), b(20);
	using table_t = hxhash_table<hxtest_integer_node_t, 4, false, hxdo_not_delete>;
	table_t table;
	// copy-construct: e is unlinked, key and hash match a.
	hxtest_integer_node_t e(a);
	EXPECT_EQ(e.hash_key(), 10);
	EXPECT_EQ(e.hash_value(), a.hash_value());
	table.insert(hxptr<hxtest_integer_node_t, hxdo_not_delete>(&e));
	EXPECT_EQ(table.size(), 1u);
	// copy-assign: key is immutable, key and hash of e are unchanged, linkage is preserved.
	e = b;
	EXPECT_EQ(e.hash_key(), 10);
	EXPECT_EQ(e.hash_value(), a.hash_value());
	EXPECT_NE(table.find(10), hxnullptr);
	EXPECT_EQ(table.size(), 1u);
	table.release_all();
}
