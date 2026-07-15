// SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
// SPDX-License-Identifier: MIT
// This file is licensed under the MIT license found in the LICENSE.md file.

#include <hx/hxhash_table_nodes.hpp>
#include <hx/hxhash_table.hpp>
#include <hx/hxtest.hpp>

#include "./hxtest_util.hpp"

HX_NS_USE

hxattr_noinline static void hxtest_gdb_break_hxhash_table(void) { }

namespace {

class hxhash_table_test_f* g_test_current = 0;
class hxhash_table_test_f :
	public testing::Test
{
public:
	class hxtest_object {
	public:
		hxtest_object(void) {
			++g_test_current->m_constructed;
			id = g_test_current->m_next_id++;
		}
		~hxtest_object(void) {
			++g_test_current->m_destructed;
			id = -1;
		}
		void operator=(const hxtest_object& x) = delete;
		void operator=(int32_t x) = delete;
		bool operator==(const hxtest_object& x) const = delete;
		bool operator==(int32_t x) const { return id == x; }
		int32_t id;
	};
	class hxtest_integer : public hxhash_table_node_integer<int32_t> {
	public:
		hxtest_integer(int32_t k) : hxhash_table_node_integer(k) { }
		hxtest_integer* hash_next(void) const {
			return static_cast<hxtest_integer*>(hxhash_table_node_integer::hash_next());
		}
		hxtest_object value;
	};
	class hxtest_string : public hxhash_table_node_string<hxsystem_allocator_stack_0> {
	public:
		hxtest_string(const char* k) : hxhash_table_node_string(k) { }
		hxtest_string* hash_next(void) const {
			return static_cast<hxtest_string*>(hxhash_table_node_string::hash_next());
		}
		hxtest_object value;
	};
	class hxtest_string_literal : public hxhash_table_node_string_literal {
	public:
		hxtest_string_literal(const char* k) : hxhash_table_node_string_literal(k) { }
		hxtest_string_literal* hash_next(void) const {
			return static_cast<hxtest_string_literal*>(hxhash_table_node_string_literal::hash_next());
		}
	};
	hxhash_table_test_f(void) {
		hxassert(g_test_current == hxnull);
		m_constructed = 0;
		m_destructed = 0;
		m_next_id = 0;
		g_test_current = this;
	}
	~hxhash_table_test_f(void) {
		g_test_current = 0;
	}
	int32_t m_constructed;
	int32_t m_destructed;
	int32_t m_next_id;
};
struct hxtest_integer_node_t : hxhash_table_node_integer<int32_t> {
	explicit hxtest_integer_node_t(int32_t k) : hxhash_table_node_integer<int32_t>(k) { }
	// GCOVR_EXCL_START
	hxtest_integer_node_t* hash_next(void) const {
		hxassert_hard(false, "unused_overload");
		return static_cast<hxtest_integer_node_t*>(hxhash_table_node_integer::hash_next());
	}
	// GCOVR_EXCL_STOP
};
struct hxtest_set_node_t : hxhash_table_set_node<int32_t> {
	explicit hxtest_set_node_t(int32_t k) : hxhash_table_set_node<int32_t>(k) { }
	// GCOVR_EXCL_START
	hxtest_set_node_t* hash_next(void) const {
		hxassert_hard(false, "unused_overload");
		return static_cast<hxtest_set_node_t*>(hxhash_table_set_node::hash_next());
	}
	// GCOVR_EXCL_STOP
};
} // namespace

TEST_F(hxhash_table_test_f, null) {
	{
		using table_t = hxhash_table<hxtest_integer, hxdefault_delete, false, 4>;
		table_t table;
		EXPECT_EQ(table.size(), 0);
		const table_t& const_table = table;
		EXPECT_EQ(table.begin(), table.end());
		EXPECT_EQ(table.cbegin(), table.cend());
		EXPECT_EQ(const_table.begin(), const_table.cend());
		EXPECT_EQ(table.begin(), table.end());
		EXPECT_EQ(table.cbegin(), table.cend());
		EXPECT_EQ(const_table.begin(), const_table.cend());
		table.clear();
		table.release_all();
		EXPECT_EQ(table.load_factor(), 0.0f);
	}
	{
		using table_dynamic_t = hxhash_table<hxtest_integer, hxdefault_delete, false>;
		const table_dynamic_t dynamic_table;
		EXPECT_EQ(dynamic_table.bucket_count(), 0);
		EXPECT_EQ(dynamic_table.load_factor(), 0.0f);
	}
	EXPECT_EQ(m_constructed, 0);
	EXPECT_EQ(m_destructed, 0);
}

TEST_F(hxhash_table_test_f, single) {
const hxsystem_allocator_scope temporary_stack_scope(hxsystem_allocator_stack_0);
	static const int k = 77;
	{
		using table_t = hxhash_table<hxtest_integer, hxdefault_delete, false, 4>;
		table_t table;
		const table_t& const_table = table;
		hxtest_integer* node = hxnew<hxtest_integer>(k);
		EXPECT_EQ(&*table.insert(hxptr<hxtest_integer>(node)), node);
		EXPECT_NE(table.begin(), table.end());
		EXPECT_NE(table.cbegin(), table.cend());
		EXPECT_EQ(++table.begin(), table.end());
		EXPECT_EQ(++table.cbegin(), table.cend());
		EXPECT_EQ(table.size(), 1);
		EXPECT_EQ(table.count(k), 1);
		hxtest_integer* dup = hxnew<hxtest_integer>(k);
		EXPECT_EQ(&*table.insert(hxptr<hxtest_integer>(dup)), node);
		EXPECT_EQ(table.find(k), node);
		EXPECT_EQ(table.find(k, &*table.begin()), hxnullptr);
		EXPECT_EQ(const_table.find(k), node);
		EXPECT_EQ(const_table.find(k, &*const_table.begin()), hxnullptr);
		{
			hxptr<hxtest_integer> extracted = table.extract(k);
			EXPECT_EQ(extracted.get(), node);
			EXPECT_EQ(table.extract(k), hxnullptr);
			EXPECT_EQ(&*table.insert(hxmove(extracted)), node);
		}
		table.release_all();
		EXPECT_EQ(table.find(k), hxnullptr);
		EXPECT_EQ(table.size(), 0);
		EXPECT_EQ(table.count(k), 0);
		hxtest_integer* new_node = hxnew<hxtest_integer>(k);
		EXPECT_EQ(&*table.insert(hxptr<hxtest_integer>(new_node)), new_node);
		EXPECT_NE(new_node->value.id, node->value.id);
		EXPECT_EQ(table.size(), 1);
		hxdelete(node);
	}
	EXPECT_EQ(m_constructed, 3);
	EXPECT_EQ(m_destructed, 3);
}

TEST_F(hxhash_table_test_f, raw_pointer_insert_duplicate_invokes_deleter) {
	using table_t = hxhash_table<hxtest_integer, hxdefault_delete, false, 4>;
	{
		table_t table;
		hxtest_integer* node = hxnew<hxtest_integer>(55);
		hxtest_integer* const colliding = hxnew<hxtest_integer>(2); // Shares 55's bucket at table_size_bits 4.
		EXPECT_EQ(&*table.insert(node), node);
		EXPECT_EQ(&*table.insert(colliding), colliding);
		EXPECT_EQ(table.size(), 2);
		EXPECT_EQ(m_destructed, 0);
		hxtest_integer* dup = hxnew<hxtest_integer>(55);
		EXPECT_EQ(&*table.insert(dup), node);
		EXPECT_EQ(table.size(), 2);
		EXPECT_EQ(m_destructed, 1);
		EXPECT_EQ(table.find(55), node);
	}
	EXPECT_EQ(m_constructed, 3);
	EXPECT_EQ(m_destructed, 3);
	{
		using no_delete_table_t = hxhash_table<hxtest_integer, hxdo_not_delete, false, 4>;
		no_delete_table_t table;
		hxtest_integer* node = hxnew<hxtest_integer>(55);
		hxtest_integer* const colliding = hxnew<hxtest_integer>(2); // Shares 55's bucket at table_size_bits 4.
		EXPECT_EQ(&*table.insert(node), node);
		EXPECT_EQ(&*table.insert(colliding), colliding);
		hxtest_integer* dup = hxnew<hxtest_integer>(55);
		EXPECT_EQ(&*table.insert(dup), node);
		EXPECT_EQ(table.size(), 2);
		hxdelete(dup);
		table.release_all();
		hxdelete(node);
		hxdelete(colliding);
	}
	{
		using single_bucket_table_t = hxhash_table<hxtest_integer, hxdo_not_delete, false, 1>;
		single_bucket_table_t table;
		hxtest_integer* original = hxnew<hxtest_integer>(0);
		hxtest_integer* nonmatching = hxnew<hxtest_integer>(2);
		EXPECT_EQ(&*table.insert(original), original);
		EXPECT_EQ(&*table.insert(nonmatching), nonmatching);
		hxtest_integer* dup = hxnew<hxtest_integer>(0);
		EXPECT_EQ(&*table.insert(dup), original);
		EXPECT_EQ(table.size(), 2);
		EXPECT_EQ(table.find(0), original);
		hxdelete(dup);
		table.release_all();
		hxdelete(original);
		hxdelete(nonmatching);
	}
}

TEST_F(hxhash_table_test_f, map_node_usage) {
	const hxsystem_allocator_scope temporary_stack_scope(hxsystem_allocator_stack_0);
	using map_node_t = hxhash_table_map_node<int32_t, hxtest_object>;
	using table_t = hxhash_table<map_node_t, hxdefault_delete, false, 4>;
	{
		table_t table;
		map_node_t* n10 = hxnew<map_node_t>(10);
		EXPECT_EQ(&*table.insert(hxptr<map_node_t>(n10)), n10);
		EXPECT_EQ(table.find(10), n10);
		EXPECT_EQ(n10->hash_key(), 10);
		n10->value().id = 123;
		map_node_t* manual = hxnew<map_node_t>(20);
		manual->value().id = 321;
		EXPECT_EQ(&*table.insert(hxptr<map_node_t>(manual)), manual);
		EXPECT_EQ(table.size(), 2);
		EXPECT_EQ(table.count(10), 1);
		EXPECT_EQ(table.count(20), 1);
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
		EXPECT_EQ(table.size(), 2);
	}
	EXPECT_EQ(m_constructed, 2);
	EXPECT_EQ(m_destructed, 2);
}

TEST_F(hxhash_table_test_f, multiple) {
	static const int size_i = 78;
	static const unsigned int size_u = 78u;
	const hxsystem_allocator_scope temporary_stack_scope(hxsystem_allocator_stack_0);
	{
		using table_t = hxhash_table<hxtest_integer, hxdefault_delete, true>;
		table_t table;
		const table_t& const_table = table;
		table.set_size_bits(5);
		for(int i = 0; i < size_i; ++i) {
			hxtest_integer* n = hxnew<hxtest_integer>(i);
			table.insert(hxptr<hxtest_integer>(n));
			EXPECT_EQ(n->value.id, i);
			EXPECT_EQ(n->hash_key(), i);
		}
		int id_histogram[size_u] = {};
		EXPECT_EQ(table.size(), size_u);
		table_t::iterator it = table.begin();
		table_t::iterator cit = table.begin();
		for(int i = 0; i < size_i; ++i) {
			const hxtest_integer* const ti = table.find(i);
			EXPECT_EQ(ti->value, i);
			EXPECT_EQ(table.find(i, ti), hxnullptr);
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
		for(int i = 0; i < size_i; ++i) {
			hxtest_integer* ti = hxnew<hxtest_integer>(i);
			EXPECT_EQ(ti->value.id, i+size_i);
			table.insert(hxptr<hxtest_integer>(ti));
		}
		int key_histogram[size_u] = {};
		EXPECT_EQ(table.size(), size_u * 2u);
		it = table.begin();
		cit = table.begin();
		for(int i = 0; i < size_i; ++i) {
			const hxtest_integer* const ti = table.find(i);
			EXPECT_EQ(ti->hash_key(), i);
			const hxtest_integer* ti2 = const_table.find(i, ti);
			EXPECT_EQ(ti2->hash_key(), i);
			EXPECT_EQ(table.find(i, ti2), hxnullptr);
			EXPECT_EQ(table.count(i), 2);
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
		EXPECT_GT((table.load_factor() * 4.0f), static_cast<float>(table.load_max()));
		for(int i = 0; i < (size_i/2); ++i) {
			EXPECT_EQ(table.erase(i), 2);
		}
		for(int i = (size_i/2); i < size_i; ++i) {
			hxptr<hxtest_integer> ti = table.extract(i);
			EXPECT_EQ(ti->hash_key(), i);
		}
		for(int i = 0; i < (size_i/2); ++i) {
			EXPECT_EQ(table.release_key(i), 0);
			EXPECT_EQ(table.find(i), hxnullptr);
		}
		for(int i = (size_i/2); i < size_i; ++i) {
			const hxtest_integer* const ti = table.find(i);
			EXPECT_EQ(ti->hash_key(), i);
			EXPECT_EQ(table.find(i, ti), hxnullptr);
			EXPECT_EQ(table.count(i), 1);
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
	const hxsystem_allocator_scope temporary_stack_scope(hxsystem_allocator_stack_0);
	static const char* const colors[] = {
		"Red","Orange","Yellow",
		"Green","Cyan","Blue",
		"Indigo","Violet" };
	const hxsize_t sz = hxsize(colors);
	{
		using table_t = hxhash_table<hxtest_string, hxdefault_delete, false, 4>;
		table_t table;
		for(hxsize_t i = sz; i-- != 0;) {
			hxtest_string* n = hxnew<hxtest_string>(colors[i]);
			EXPECT_EQ(&*table.insert(hxptr<hxtest_string>(n)), n);
			EXPECT_STREQ(n->hash_key(), colors[i]);
		}
		EXPECT_NE(table.find("Cyan"), hxnullptr);
		EXPECT_EQ(table.find("Pink"), hxnullptr);
	}
	EXPECT_EQ(m_constructed, sz);
	EXPECT_EQ(m_destructed, sz);
}

TEST_F(hxhash_table_test_f, string_literal_nodes) {
	const hxsystem_allocator_scope temporary_stack_scope(hxsystem_allocator_stack_0);
	static const char* const literals[] = {
		"Crimson", "Teal", "Magenta", "Gold"
	};
	using table_t = hxhash_table<hxtest_string_literal, hxdefault_delete, false, 4>;
	table_t table;
	for(hxsize_t i = 0; i < hxsize(literals); ++i) {
		hxtest_string_literal* n = hxnew<hxtest_string_literal>(literals[i]);
		EXPECT_EQ(&*table.insert(hxptr<hxtest_string_literal>(n)), n);
		EXPECT_EQ(n->hash_key(), literals[i]);
		EXPECT_EQ(n->hash_value(), hxkey_hash(literals[i]));
	}
	EXPECT_EQ(table.size(), hxsize(literals));
	EXPECT_NE(table.find("Crimson"), hxnullptr);
	EXPECT_EQ(table.find("Cerulean"), hxnullptr);
}

TEST(hxhash_table_set_node_test, copy_move_construct_and_assign) {
	const hxtest_set_node_t a(1), b(2), c(3), d(4);
	using table_t = hxhash_table<hxtest_set_node_t, hxdo_not_delete, false, 4>;
	table_t table;
	hxtest_set_node_t e(a);
	EXPECT_EQ(e.hash_key(), 1);
	EXPECT_EQ(e.hash_value(), a.hash_value());
	table.insert(hxptr<hxtest_set_node_t, hxdo_not_delete>(&e));
	hxtest_set_node_t f(hxmove(b));
	EXPECT_EQ(f.hash_key(), 2);
	EXPECT_EQ(f.hash_value(), b.hash_value());
	table.insert(hxptr<hxtest_set_node_t, hxdo_not_delete>(&f));
	EXPECT_EQ(table.size(), 2);
	table.release_all();
}

TEST(hxhash_table_map_node_test, hash_next_type) {
	using node_t = hxhash_table_map_node<int32_t, int32_t>;
	using table_t = hxhash_table<node_t, hxdo_not_delete, true, 4>;
	node_t a(1, 10), b(1, 20), c(2, 30), d(4, 40); // d shares 1's bucket at table_size_bits 4.
	table_t table;
	table.insert(hxptr<node_t, hxdo_not_delete>(&a));
	table.insert(hxptr<node_t, hxdo_not_delete>(&d));
	table.insert(hxptr<node_t, hxdo_not_delete>(&b));
	table.insert(hxptr<node_t, hxdo_not_delete>(&c));
	EXPECT_EQ(table.size(), 4);
	const node_t* first = table.find(1);
	ASSERT_NE(first, hxnullptr);
	const node_t* second = table.find(1, first);
	ASSERT_NE(second, hxnullptr);
	EXPECT_NE(first, second);
	EXPECT_EQ(table.find(1, second), hxnullptr);
	hxsize_t count = 0;
	for(table_t::const_iterator it = table.begin(); it != table.end(); ++it) {
		++count;
	}
	EXPECT_EQ(count, 4);
	table.release_all();
}
struct hxtest_map_node_t : hxhash_table_map_node<int32_t, int32_t> {
	explicit hxtest_map_node_t(int32_t k, int32_t v)
		: hxhash_table_map_node<int32_t, int32_t>(k, v) { }
	hxtest_map_node_t* hash_next(void) const {
		return static_cast<hxtest_map_node_t*>(hxhash_table_map_node::hash_next());
	}
};

TEST(hxhash_table_map_node_test, subclass_hash_next_type) {
	using table_t = hxhash_table<hxtest_map_node_t, hxdo_not_delete, false, 4>;
	hxtest_map_node_t a(1, 10), b(2, 20), c(3, 30);
	table_t table;
	table.insert(hxptr<hxtest_map_node_t, hxdo_not_delete>(&a));
	table.insert(hxptr<hxtest_map_node_t, hxdo_not_delete>(&b));
	table.insert(hxptr<hxtest_map_node_t, hxdo_not_delete>(&c));
	EXPECT_EQ(table.size(), 3);
	hxtest_map_node_t* found = table.find(2);
	ASSERT_NE(found, hxnullptr);
	EXPECT_EQ(found->value(), 20);
	EXPECT_EQ(table.release_key(2), 1);
	EXPECT_EQ(table.size(), 2);
	EXPECT_EQ(table.find(2), hxnullptr);
	table.release_all();
}

TEST_F(hxhash_table_test_f, iterator_traverses_all_nodes_exactly_once) {
	const hxsystem_allocator_scope temporary_stack_scope(hxsystem_allocator_stack_0);
	using table_t = hxhash_table<hxtest_integer, hxdefault_delete, true, 4>;
	table_t table;
	static const int keys[] = { 1, 2, 3, 4, 5 };
	static const int n = static_cast<int>(sizeof(keys) / sizeof(keys[0]));
	for(int i = 0; i < n; ++i) {
		table.insert(hxptr<hxtest_integer>(hxnew<hxtest_integer>(keys[i])));
	}
	EXPECT_EQ(table.size(), static_cast<hxsize_t>(n));
	int visited = 0;
	for(table_t::const_iterator it = table.cbegin(); it != table.cend(); ++it) {
		++visited;
	}
	EXPECT_EQ(visited, n);
}

TEST_F(hxhash_table_test_f, iterator_begin_equals_end_on_empty_table) {
	using table_t = hxhash_table<hxtest_integer, hxdefault_delete, false, 4>;
	table_t table;
	EXPECT_EQ(table.begin(), table.end());
	EXPECT_EQ(table.cbegin(), table.cend());
}

TEST_F(hxhash_table_test_f, default_constructed_iterators_equal_end) {
	const hxsystem_allocator_scope temporary_stack_scope(hxsystem_allocator_stack_0);
	using table_t = hxhash_table<hxtest_integer, hxdefault_delete, false, 4>;
	table_t table;
	table.insert(hxptr<hxtest_integer>(hxnew<hxtest_integer>(10)));
	const table_t::const_iterator cd;
	EXPECT_EQ(cd, table.cend());
	EXPECT_FALSE(cd == table.cbegin());
	const table_t::iterator d;
	EXPECT_EQ(d, table.end());
	EXPECT_FALSE(d == table.begin());
}

TEST_F(hxhash_table_test_f, bucket_count_matches_table_size_bits) {
	using table_static_t = hxhash_table<hxtest_integer, hxdefault_delete, false, 4>;
	const table_static_t static_table;
	EXPECT_EQ(static_table.bucket_count(), 16);
	const hxsystem_allocator_scope temporary_stack_scope(hxsystem_allocator_stack_0);
	using table_dynamic_t = hxhash_table<hxtest_integer, hxdefault_delete, false>;
	table_dynamic_t dynamic_table;
	dynamic_table.set_size_bits(5);
	EXPECT_EQ(dynamic_table.bucket_count(), 32);
}

TEST_F(hxhash_table_test_f, const_iterator_post_increment_returns_prior) {
	const hxsystem_allocator_scope temporary_stack_scope(hxsystem_allocator_stack_0);
	using table_t = hxhash_table<hxtest_integer, hxdefault_delete, false, 4>;
	table_t table;
	table.insert(hxptr<hxtest_integer>(hxnew<hxtest_integer>(10)));
	table_t::const_iterator it = table.cbegin();
	const table_t::const_iterator prior = it++;
	EXPECT_EQ(prior, table.cbegin());
	EXPECT_EQ(it, table.cend());
}

TEST_F(hxhash_table_test_f, emplace_constructs_and_inserts_node) {
	const hxsystem_allocator_scope temporary_stack_scope(hxsystem_allocator_stack_0);
	using table_t = hxhash_table<hxtest_integer, hxdefault_delete, true, 4>;
	table_t table;
	const table_t::iterator a = table.emplace(7);
	EXPECT_EQ(a->hash_key(), 7);
	EXPECT_EQ(table.size(), 1);
	const table_t::iterator b = table.emplace(7);
	EXPECT_NE(a, b);
	EXPECT_EQ(b->hash_key(), 7);
	EXPECT_EQ(table.size(), 2);
	EXPECT_EQ(table.count(7), 2);
}

TEST_F(hxhash_table_test_f, try_emplace_inserts_then_returns_existing) {
	const hxsystem_allocator_scope temporary_stack_scope(hxsystem_allocator_stack_0);
	using table_t = hxhash_table<hxtest_integer, hxdefault_delete, false, 4>;
	table_t table;
	const table_t::iterator a = table.try_emplace(7, 7);
	EXPECT_EQ(a->hash_key(), 7);
	EXPECT_EQ(table.size(), 1);
	const table_t::iterator b = table.try_emplace(7, 7);
	EXPECT_EQ(a, b);
	EXPECT_EQ(table.size(), 1);
}

TEST_F(hxhash_table_test_f, count_returns_zero_for_absent_key) {
	const hxsystem_allocator_scope temporary_stack_scope(hxsystem_allocator_stack_0);
	using table_t = hxhash_table<hxtest_integer, hxdefault_delete, false, 4>;
	table_t table;
	table.insert(hxptr<hxtest_integer>(hxnew<hxtest_integer>(10)));
	EXPECT_EQ(table.count(10), 1);
	EXPECT_EQ(table.count(99), 0);
}

TEST_F(hxhash_table_test_f, count_multi_two_same_key) {
	const hxsystem_allocator_scope temporary_stack_scope(hxsystem_allocator_stack_0);
	using table_t = hxhash_table<hxtest_integer, hxdefault_delete, true, 4>;
	table_t table;
	table.insert(hxptr<hxtest_integer>(hxnew<hxtest_integer>(7)));
	table.insert(hxptr<hxtest_integer>(hxnew<hxtest_integer>(7)));
	EXPECT_EQ(table.count(7), 2);
}

TEST_F(hxhash_table_test_f, find_second_duplicate_via_previous) {
	const hxsystem_allocator_scope temporary_stack_scope(hxsystem_allocator_stack_0);
	using table_t = hxhash_table<hxtest_integer, hxdefault_delete, true, 4>;
	table_t table;
	hxtest_integer* n1 = hxnew<hxtest_integer>(34);
	hxtest_integer* const colliding = hxnew<hxtest_integer>(3); // Shares 34's bucket at table_size_bits 4.
	hxtest_integer* n2 = hxnew<hxtest_integer>(34);
	table.insert(hxptr<hxtest_integer>(n1));
	table.insert(hxptr<hxtest_integer>(colliding));
	table.insert(hxptr<hxtest_integer>(n2));
	EXPECT_EQ(table.size(), 3u);
	const hxtest_integer* first = table.find(34);
	ASSERT_NE(first, hxnullptr);
	const hxtest_integer* second = table.find(34, first);
	ASSERT_NE(second, hxnullptr);
	EXPECT_NE(second, first);
	EXPECT_EQ(table.find(34, second), hxnullptr);

	using single_bucket_table_t = hxhash_table<hxtest_integer, hxdefault_delete, true, 1>;
	single_bucket_table_t single_bucket_table;
	hxtest_integer* tail_match = hxnew<hxtest_integer>(0);
	hxtest_integer* middle_other = hxnew<hxtest_integer>(2);
	hxtest_integer* head_match = hxnew<hxtest_integer>(0);
	single_bucket_table.insert(hxptr<hxtest_integer>(tail_match));
	single_bucket_table.insert(hxptr<hxtest_integer>(middle_other));
	single_bucket_table.insert(hxptr<hxtest_integer>(head_match));
	const hxtest_integer* found_head = single_bucket_table.find(0);
	EXPECT_EQ(found_head, head_match);
	const hxtest_integer* found_tail = single_bucket_table.find(0, found_head);
	EXPECT_EQ(found_tail, tail_match);
	EXPECT_EQ(single_bucket_table.find(0, found_tail), hxnullptr);
	EXPECT_EQ(single_bucket_table.find(2, middle_other), hxnullptr);
}

TEST_F(hxhash_table_test_f, find_with_previous_skips_colliding_key_in_single_table) {
	const hxsystem_allocator_scope temporary_stack_scope(hxsystem_allocator_stack_0);
	using table_t = hxhash_table<hxtest_integer, hxdefault_delete, false, 4>;
	table_t table;
	hxtest_integer* const node_17 = hxnew<hxtest_integer>(17); // Shares 77's bucket at table_size_bits 4.
	hxtest_integer* const node_77 = hxnew<hxtest_integer>(77);
	table.insert(hxptr<hxtest_integer>(node_17));
	table.insert(hxptr<hxtest_integer>(node_77));
	EXPECT_EQ(table.find(77, node_77), hxnullptr);
}

TEST_F(hxhash_table_test_f, find_absent_key_in_nonempty_bucket_chain) {
	const hxsystem_allocator_scope temporary_stack_scope(hxsystem_allocator_stack_0);
	using table_t = hxhash_table<hxtest_integer, hxdefault_delete, true>;
	table_t table;
	table.set_size_bits(1);
	table.insert(hxptr<hxtest_integer>(hxnew<hxtest_integer>(1)));
	table.insert(hxptr<hxtest_integer>(hxnew<hxtest_integer>(3)));
	table.insert(hxptr<hxtest_integer>(hxnew<hxtest_integer>(5)));
	EXPECT_EQ(table.find(99), hxnullptr);
	EXPECT_NE(table.find(1), hxnullptr);
}

TEST_F(hxhash_table_test_f, insert_two_keys_same_bucket_both_findable) {
	const hxsystem_allocator_scope temporary_stack_scope(hxsystem_allocator_stack_0);
	using table_t = hxhash_table<hxtest_integer, hxdefault_delete, true>;
	table_t table;
	table.set_size_bits(1);
	table.insert(hxptr<hxtest_integer>(hxnew<hxtest_integer>(1)));
	table.insert(hxptr<hxtest_integer>(hxnew<hxtest_integer>(3)));
	EXPECT_NE(table.find(1), hxnullptr);
	EXPECT_NE(table.find(3), hxnullptr);
	EXPECT_EQ(table.size(), 2u);
}

TEST_F(hxhash_table_test_f, erase_head_node_updates_size) {
	const hxsystem_allocator_scope temporary_stack_scope(hxsystem_allocator_stack_0);
	using table_t = hxhash_table<hxtest_integer, hxdefault_delete, false, 4>;
	table_t table;
	table.insert(hxptr<hxtest_integer>(hxnew<hxtest_integer>(5)));
	hxtest_integer* const node_6 = hxnew<hxtest_integer>(6);
	table.insert(hxptr<hxtest_integer>(node_6));
	EXPECT_EQ(table.erase(6, hxdo_not_delete()), 1u);
	EXPECT_EQ(table.size(), 1u);
	EXPECT_EQ(table.find(6), hxnullptr);
	EXPECT_NE(table.find(5), hxnullptr);
	hxdelete(node_6);
	EXPECT_EQ(table.erase(5), 1u);
	EXPECT_EQ(table.size(), 0u);
	EXPECT_EQ(table.find(5), hxnullptr);

	using single_bucket_table_t = hxhash_table<hxtest_integer, hxdefault_delete, false, 1>;
	single_bucket_table_t single_bucket_table;
	single_bucket_table.insert(hxptr<hxtest_integer>(hxnew<hxtest_integer>(1)));
	hxtest_integer* const node_2 = hxnew<hxtest_integer>(2);
	single_bucket_table.insert(hxptr<hxtest_integer>(node_2));
	EXPECT_EQ(single_bucket_table.erase(2, hxdo_not_delete()), 1u);
	EXPECT_EQ(single_bucket_table.size(), 1u);
	EXPECT_EQ(single_bucket_table.find(2), hxnullptr);
	EXPECT_NE(single_bucket_table.find(1), hxnullptr);
	hxdelete(node_2);

	using fn_deleter_table_t = hxhash_table<hxtest_integer, hxdefault_delete, false, 4>;
	fn_deleter_table_t fn_deleter_table;
	fn_deleter_table.insert(hxptr<hxtest_integer>(hxnew<hxtest_integer>(9)));
	EXPECT_EQ(fn_deleter_table.erase(9, &hxdelete<hxtest_integer>), 1u);
	EXPECT_EQ(fn_deleter_table.size(), 0u);
	EXPECT_EQ(fn_deleter_table.find(9), hxnullptr);
}

TEST_F(hxhash_table_test_f, erase_interior_node_in_bucket_chain) {
	const hxsystem_allocator_scope temporary_stack_scope(hxsystem_allocator_stack_0);
	using table_t = hxhash_table<hxtest_integer, hxdefault_delete, true>;
	table_t table;
	table.set_size_bits(1);
	table.insert(hxptr<hxtest_integer>(hxnew<hxtest_integer>(1)));
	table.insert(hxptr<hxtest_integer>(hxnew<hxtest_integer>(3)));
	table.insert(hxptr<hxtest_integer>(hxnew<hxtest_integer>(5)));
	const hxsize_t erased = table.erase(3);
	EXPECT_EQ(erased, 1u);
	EXPECT_EQ(table.size(), 2u);
	EXPECT_EQ(table.find(3), hxnullptr);
	EXPECT_NE(table.find(1), hxnullptr);
	EXPECT_NE(table.find(5), hxnullptr);

	using single_bucket_table_t = hxhash_table<hxtest_integer, hxdefault_delete, true, 1>;
	{
		single_bucket_table_t all_matching_table;
		hxtest_integer* const all_first = hxnew<hxtest_integer>(0);
		hxtest_integer* const all_second = hxnew<hxtest_integer>(0);
		hxtest_integer* const all_third = hxnew<hxtest_integer>(0);
		all_matching_table.insert(hxptr<hxtest_integer>(all_first));
		all_matching_table.insert(hxptr<hxtest_integer>(all_second));
		all_matching_table.insert(hxptr<hxtest_integer>(all_third));
		EXPECT_EQ(all_matching_table.erase(0, &hxdelete<hxtest_integer>), 3u);
		EXPECT_EQ(all_matching_table.size(), 0u);
		EXPECT_EQ(all_matching_table.find(0), hxnullptr);
	}
	{
		single_bucket_table_t all_matching_no_delete_table;
		hxtest_integer* const nd_first = hxnew<hxtest_integer>(0);
		hxtest_integer* const nd_second = hxnew<hxtest_integer>(0);
		all_matching_no_delete_table.insert(hxptr<hxtest_integer>(nd_first));
		all_matching_no_delete_table.insert(hxptr<hxtest_integer>(nd_second));
		EXPECT_EQ(all_matching_no_delete_table.erase(0, hxdo_not_delete()), 2u);
		EXPECT_EQ(all_matching_no_delete_table.size(), 0u);
		hxdelete(nd_first);
		hxdelete(nd_second);
	}
	{
		single_bucket_table_t suffix_matching_table;
		hxtest_integer* const suffix_first = hxnew<hxtest_integer>(0);
		hxtest_integer* const suffix_second = hxnew<hxtest_integer>(0);
		suffix_matching_table.insert(hxptr<hxtest_integer>(suffix_first));
		suffix_matching_table.insert(hxptr<hxtest_integer>(suffix_second));
		suffix_matching_table.insert(hxptr<hxtest_integer>(hxnew<hxtest_integer>(2)));
		EXPECT_EQ(suffix_matching_table.erase(0, &hxdelete<hxtest_integer>), 2u);
		EXPECT_EQ(suffix_matching_table.size(), 1u);
		EXPECT_EQ(suffix_matching_table.find(0), hxnullptr);
		EXPECT_NE(suffix_matching_table.find(2), hxnullptr);
	}
	{
		single_bucket_table_t prefix_run_then_mismatch_table;
		prefix_run_then_mismatch_table.insert(hxptr<hxtest_integer>(hxnew<hxtest_integer>(2)));
		hxtest_integer* const prefix_second = hxnew<hxtest_integer>(0);
		hxtest_integer* const prefix_first = hxnew<hxtest_integer>(0);
		prefix_run_then_mismatch_table.insert(hxptr<hxtest_integer>(prefix_second));
		prefix_run_then_mismatch_table.insert(hxptr<hxtest_integer>(prefix_first));
		EXPECT_EQ(prefix_run_then_mismatch_table.erase(0, &hxdelete<hxtest_integer>), 2u);
		EXPECT_EQ(prefix_run_then_mismatch_table.size(), 1u);
		EXPECT_EQ(prefix_run_then_mismatch_table.find(0), hxnullptr);
		EXPECT_NE(prefix_run_then_mismatch_table.find(2), hxnullptr);
	}
	{
		single_bucket_table_t interior_matching_table;
		hxtest_integer* const interior_head = hxnew<hxtest_integer>(2);
		hxtest_integer* const interior_match = hxnew<hxtest_integer>(0);
		hxtest_integer* const interior_tail = hxnew<hxtest_integer>(2);
		interior_matching_table.insert(hxptr<hxtest_integer>(interior_tail));
		interior_matching_table.insert(hxptr<hxtest_integer>(interior_match));
		interior_matching_table.insert(hxptr<hxtest_integer>(interior_head));
		EXPECT_EQ(interior_matching_table.erase(0, hxdo_not_delete()), 1u);
		EXPECT_EQ(interior_matching_table.size(), 2u);
		EXPECT_EQ(interior_matching_table.find(0), hxnullptr);
		EXPECT_EQ(interior_matching_table.count(2), 2);
		hxdelete(interior_match);
		interior_matching_table.release_all();
		hxdelete(interior_head);
		hxdelete(interior_tail);
	}
}

TEST_F(hxhash_table_test_f, erase_returns_zero_for_absent_key) {
	const hxsystem_allocator_scope temporary_stack_scope(hxsystem_allocator_stack_0);
	using table_t = hxhash_table<hxtest_integer, hxdefault_delete, false, 4>;
	table_t table;
	table.insert(hxptr<hxtest_integer>(hxnew<hxtest_integer>(10)));
	EXPECT_EQ(table.erase(99), 0u);
	EXPECT_EQ(table.size(), 1u);
}

TEST_F(hxhash_table_test_f, extract_head_node_removes_from_table) {
	const hxsystem_allocator_scope temporary_stack_scope(hxsystem_allocator_stack_0);
	using table_t = hxhash_table<hxtest_integer, hxdefault_delete, false, 4>;
	table_t table;
	hxtest_integer* node = hxnew<hxtest_integer>(55);
	table.insert(hxptr<hxtest_integer>(node));
	const hxptr<hxtest_integer> extracted = table.extract(55);
	EXPECT_EQ(extracted.get(), node);
	EXPECT_EQ(table.find(55), hxnullptr);
	EXPECT_EQ(table.size(), 0u);
}

TEST_F(hxhash_table_test_f, extract_interior_node_keeps_others) {
	const hxsystem_allocator_scope temporary_stack_scope(hxsystem_allocator_stack_0);
	using table_t = hxhash_table<hxtest_integer, hxdefault_delete, true>;
	table_t table;
	table.set_size_bits(1);
	table.insert(hxptr<hxtest_integer>(hxnew<hxtest_integer>(1)));
	hxtest_integer* mid = hxnew<hxtest_integer>(3);
	table.insert(hxptr<hxtest_integer>(mid));
	table.insert(hxptr<hxtest_integer>(hxnew<hxtest_integer>(5)));
	const hxptr<hxtest_integer> extracted = table.extract(3);
	EXPECT_EQ(extracted.get(), mid);
	EXPECT_EQ(table.size(), 2u);
	EXPECT_NE(table.find(1), hxnullptr);
	EXPECT_NE(table.find(5), hxnullptr);
	EXPECT_EQ(table.find(3), hxnullptr);
}

TEST_F(hxhash_table_test_f, erase_iterator_head_node_returns_next) {
	const hxsystem_allocator_scope temporary_stack_scope(hxsystem_allocator_stack_0);
	using table_t = hxhash_table<hxtest_integer, hxdefault_delete, true, 1>;
	table_t table;
	hxtest_integer* const node_2 = hxnew<hxtest_integer>(2);
	table.insert(hxptr<hxtest_integer>(node_2));
	hxtest_integer* const node_0 = hxnew<hxtest_integer>(0);
	table.insert(hxptr<hxtest_integer>(node_0));
	const table_t::iterator it = table.begin();
	EXPECT_EQ(&*it, node_0);
	const table_t::iterator next = table.erase(it);
	EXPECT_EQ(&*next, node_2);
	EXPECT_EQ(table.size(), 1u);
	EXPECT_EQ(table.find(0), hxnullptr);
	EXPECT_NE(table.find(2), hxnullptr);
	hxdelete(node_0);
}

TEST_F(hxhash_table_test_f, erase_iterator_interior_node_returns_next) {
	const hxsystem_allocator_scope temporary_stack_scope(hxsystem_allocator_stack_0);
	using table_t = hxhash_table<hxtest_integer, hxdefault_delete, true, 1>;
	table_t table;
	hxtest_integer* const node_10 = hxnew<hxtest_integer>(10);
	table.insert(hxptr<hxtest_integer>(node_10));
	hxtest_integer* const node_9 = hxnew<hxtest_integer>(9);
	table.insert(hxptr<hxtest_integer>(node_9));
	hxtest_integer* const node_2 = hxnew<hxtest_integer>(2);
	table.insert(hxptr<hxtest_integer>(node_2));
	hxtest_integer* const node_0 = hxnew<hxtest_integer>(0);
	table.insert(hxptr<hxtest_integer>(node_0));
	table_t::iterator it = table.begin();
	while(&*it != node_9) {
		++it;
	}
	const table_t::iterator next = table.erase(it);
	EXPECT_EQ(&*next, node_10);
	EXPECT_EQ(table.size(), 3u);
	EXPECT_EQ(table.find(9), hxnullptr);
	hxdelete(node_9);
}

TEST_F(hxhash_table_test_f, erase_iterator_tail_node_returns_end) {
	const hxsystem_allocator_scope temporary_stack_scope(hxsystem_allocator_stack_0);
	using table_t = hxhash_table<hxtest_integer, hxdefault_delete, true, 1>;
	table_t table;
	hxtest_integer* const node_2 = hxnew<hxtest_integer>(2);
	table.insert(hxptr<hxtest_integer>(node_2));
	hxtest_integer* const node_0 = hxnew<hxtest_integer>(0);
	table.insert(hxptr<hxtest_integer>(node_0));
	table_t::iterator it = table.begin();
	while(&*it != node_2) {
		++it;
	}
	const table_t::iterator next = table.erase(it);
	EXPECT_EQ(next, table.end());
	EXPECT_EQ(table.size(), 1u);
	EXPECT_EQ(table.find(2), hxnullptr);
	EXPECT_NE(table.find(0), hxnullptr);
	hxdelete(node_2);
}

TEST_F(hxhash_table_test_f, replace_no_match_inserts_new_node) {
	const hxsystem_allocator_scope temporary_stack_scope(hxsystem_allocator_stack_0);
	using table_t = hxhash_table<hxtest_integer, hxdo_not_delete, false, 4>;
	table_t table;
	hxtest_integer* const node_1 = hxnew<hxtest_integer>(1);
	const hxptr<hxtest_integer, hxdo_not_delete> replaced = table.replace(node_1);
	EXPECT_EQ(replaced.get(), hxnullptr);
	EXPECT_EQ(table.size(), 1u);
	EXPECT_EQ(table.find(1), node_1);
	hxdelete(node_1);
}

TEST_F(hxhash_table_test_f, replace_head_match_keeps_size_and_swaps_node) {
	const hxsystem_allocator_scope temporary_stack_scope(hxsystem_allocator_stack_0);
	using table_t = hxhash_table<hxtest_integer, hxdo_not_delete, false, 1>;
	table_t table;
	hxtest_integer* const node_2 = hxnew<hxtest_integer>(2);
	table.insert(hxptr<hxtest_integer>(node_2));
	hxtest_integer* const node_0a = hxnew<hxtest_integer>(0);
	table.insert(hxptr<hxtest_integer>(node_0a));
	hxtest_integer* const node_0b = hxnew<hxtest_integer>(0);
	const hxptr<hxtest_integer, hxdo_not_delete> replaced = table.replace(node_0b);
	EXPECT_EQ(replaced.get(), node_0a);
	EXPECT_EQ(table.size(), 2u);
	EXPECT_EQ(table.find(0), node_0b);
	EXPECT_NE(table.find(2), hxnullptr);
	hxdelete(node_0a);
	hxdelete(node_0b);
	hxdelete(node_2);
}

TEST_F(hxhash_table_test_f, replace_interior_match_keeps_size_and_swaps_node) {
	const hxsystem_allocator_scope temporary_stack_scope(hxsystem_allocator_stack_0);
	using table_t = hxhash_table<hxtest_integer, hxdo_not_delete, false, 1>;
	table_t table;
	hxtest_integer* const node_9 = hxnew<hxtest_integer>(9);
	table.insert(hxptr<hxtest_integer>(node_9));
	hxtest_integer* const node_0a = hxnew<hxtest_integer>(0);
	table.insert(hxptr<hxtest_integer>(node_0a));
	hxtest_integer* const node_2 = hxnew<hxtest_integer>(2);
	table.insert(hxptr<hxtest_integer>(node_2));
	hxtest_integer* const node_0b = hxnew<hxtest_integer>(0);
	const hxptr<hxtest_integer, hxdo_not_delete> replaced = table.replace(node_0b);
	EXPECT_EQ(replaced.get(), node_0a);
	EXPECT_EQ(table.size(), 3u);
	EXPECT_EQ(table.find(0), node_0b);
	EXPECT_NE(table.find(2), hxnullptr);
	EXPECT_NE(table.find(9), hxnullptr);
	hxdelete(node_0a);
	hxdelete(node_0b);
	hxdelete(node_2);
	hxdelete(node_9);
}

TEST(hxhash_table_node_integer_test, conforms_to_forward_iter_api) {
	using table_t = hxhash_table<hxhash_table_node_integer<int32_t>, hxdo_not_delete, false, 4>;
	hxhash_table_node_integer<int32_t> a(1), b(2), c(3);
	table_t table;
	table.insert(hxptr<hxhash_table_node_integer<int32_t>, hxdo_not_delete>(&a));
	table.insert(hxptr<hxhash_table_node_integer<int32_t>, hxdo_not_delete>(&b));
	table.insert(hxptr<hxhash_table_node_integer<int32_t>, hxdo_not_delete>(&c));
	EXPECT_TRUE(hxtest_check_forward_iter_api(table.begin(), table.end()));
	const table_t& const_table = table;
	EXPECT_TRUE(hxtest_check_forward_iter_api(const_table.begin(), const_table.end()));
	table.release_all();
}

TEST(hxhash_table_node_integer_test, copy_move_construct_and_assign) {
	const hxtest_integer_node_t a(10), b(20);
	using table_t = hxhash_table<hxtest_integer_node_t, hxdo_not_delete, false, 4>;
	table_t table;
	hxtest_integer_node_t e(a);
	EXPECT_EQ(e.hash_key(), 10);
	EXPECT_EQ(e.hash_value(), a.hash_value());
	table.insert(hxptr<hxtest_integer_node_t, hxdo_not_delete>(&e));
	EXPECT_EQ(table.size(), 1);
	e = b;
	EXPECT_EQ(e.hash_key(), 10);
	EXPECT_EQ(e.hash_value(), a.hash_value());
	EXPECT_NE(table.find(10), hxnullptr);
	EXPECT_EQ(table.size(), 1);
	table.release_all();
}
