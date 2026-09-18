// SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
// SPDX-License-Identifier: MIT
// This file is licensed under the MIT license found in the LICENSE.md file.

#include <hx/libhatchet.h>

#if HX_USE_STD_LIB

#include <hx/hxallocator.hpp>
#include <hx/hxarray.hpp>
#include <hx/hxbitset.hpp>
#include <hx/hxconsole.hpp>
#include <hx/hxdeque.hpp>
#include <hx/hxexpected.hpp>
#include <hx/hxfile.hpp>
#include <hx/hxflat_map.hpp>
#include <hx/hxflat_set.hpp>
#include <hx/hxfree_list.hpp>
#include <hx/hxhandle_table.hpp>
#include <hx/hxhash_table.hpp>
#include <hx/hxhash_table_nodes.hpp>
#include <hx/hxinitializer_list.hpp>
#include <hx/hxkey.hpp>
#include <hx/hxlist.hpp>
#include <hx/hxlist_constexpr.hpp>
#include <hx/hxpair.hpp>
#include <hx/hxprofiler.hpp>
#include <hx/hxptr.hpp>
#include <hx/hxradix_sort.hpp>
#include <hx/hxrandom.hpp>
#include <hx/hxrange.hpp>
#include <hx/hxref.hpp>
#include <hx/hxset_algorithms.hpp>
#include <hx/hxslab_allocator.h>
#include <hx/hxslot_map.hpp>
#include <hx/hxsort.hpp>
#include <hx/hxtask.hpp>
#include <hx/hxtask_dag_node.hpp>
#include <hx/hxtask_queue.hpp>
#include <hx/hxtest.hpp>
#include <hx/hxthread.hpp>
#include <hx/hxutility.h>
#include <hx/hxvector.hpp>

#include <algorithm>
#include <bitset>
#include <deque>
#include <functional>
#include <initializer_list>
#include <iterator>
#include <limits>
#include <list>
#include <map>
#include <memory>
#include <new>
#include <numeric>
#include <set>
#include <string>
#include <tuple>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <vector>

HX_NS_USE

// This is expected to be safe because of libhatchet's hx prefix.
using namespace std;

TEST(hxstd_compatibility_test, containers_interoperate) {
	vector<int> stdvalues;
	stdvalues.push_back(31);
	stdvalues.push_back(32);

	const hxvector<int> values(stdvalues);
	EXPECT_EQ(values.size(), 2);
	EXPECT_EQ(values[0], 31);
	EXPECT_EQ(values[1], 32);

	const vector<int> roundtrip(values.begin(), values.end());
	EXPECT_TRUE(equal(roundtrip.begin(), roundtrip.end(), stdvalues.begin()));
}

TEST(hxstd_compatibility_test, placement_new_unambiguous) {
	hxbitset<8> bits;
	bits.set(3);
	EXPECT_TRUE(bits.test(3));

	bitset<8> stdbits;
	stdbits.set(3);
	EXPECT_TRUE(stdbits.test(3));

	int storage;
	const int* const ptr = ::new (static_cast<void*>(&storage)) int(31);
	EXPECT_EQ(*ptr, 31);
}

#endif // HX_USE_STD_LIB
