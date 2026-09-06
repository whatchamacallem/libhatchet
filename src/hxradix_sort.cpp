// SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
// SPDX-License-Identifier: MIT
// This file is licensed under the MIT license found in the LICENSE.md file.

#include "../include/hx/hxradix_sort.hpp"
#include "../include/hx/hxsort.hpp"

HX_NS_BEGIN_

// An unsigned integer used for the histogram. uint32_t or hxsize_t would be a
// reasonable choice here.
using hxhistogram_t = uint32_t;

hxattr_hot void hxradix_sort_void(hxradix_sort_key_void* begin, hxradix_sort_key_void* end,
		hxsystem_allocator_t allocator) {
	hxassertf(end >= begin, "radix_sort end < begin %zd", end - begin);
	// Check for size overflowing hxhistogram_t.
	hxassertf(static_cast<size_t>(end - begin) < ~static_cast<hxhistogram_t>(0),
		"hxhistogram_t overflow %zd", end - begin);

	const hxhistogram_t size = static_cast<hxhistogram_t>(end - begin);
	if(size < HX_RADIX_SORT_MIN_SIZE) {
		hxinsertion_sort(begin, end);
		return;
	}

	const hxsystem_allocator_scope allocator_scope(allocator);

	// A single allocation holds the second working buffer and the histograms.
	const size_t buffer_bytes = static_cast<size_t>(size) * sizeof(hxradix_sort_key_void);
	hxradix_sort_key_void* const buf1 = reinterpret_cast<hxradix_sort_key_void*>(
		hxmalloc(buffer_bytes + 1024u * sizeof(hxhistogram_t)));
	hxhistogram_t* const histograms = reinterpret_cast<hxhistogram_t*>(buf1 + size);
	::memset(histograms, 0x00, 1024u * sizeof(hxhistogram_t)); // 4k

	// Build histograms
	hxhistogram_t* hxrestrict hist0 = histograms + (256 * 0);
	hxhistogram_t* hxrestrict hist1 = histograms + (256 * 1);
	hxhistogram_t* hxrestrict hist2 = histograms + (256 * 2);
	hxhistogram_t* hxrestrict hist3 = histograms + (256 * 3);

	for(const hxradix_sort_key_void* hxrestrict it = begin; it != end; ++it) {
		const hxhistogram_t x = it->get_modified_key();
		++hist0[x & 0xffu];
		++hist1[(x >> 8) & 0xffu];
		++hist2[(x >> 16) & 0xffu];
		++hist3[x >> 24];
	}

	// A pass where a single bucket holds every key is a stable identity
	// permutation and is skipped.
	const hxhistogram_t first_key = begin->get_modified_key();
	const bool skip0 = hist0[first_key & 0xffu] == size;
	const bool skip1 = hist1[(first_key >> 8) & 0xffu] == size;
	const bool skip2 = hist2[(first_key >> 16) & 0xffu] == size;
	const bool skip3 = hist3[first_key >> 24] == size;

	// Convert histograms to start indices
	hxhistogram_t sum0 = 0u, sum1 = 0u, sum2 = 0u, sum3 = 0u;
	for(hxsize_t i = 0; i < 256; ++i) {
		const hxhistogram_t t0 = hist0[i] + sum0; hist0[i] = sum0; sum0 = t0;
		const hxhistogram_t t1 = hist1[i] + sum1; hist1[i] = sum1; sum1 = t1;
		const hxhistogram_t t2 = hist2[i] + sum2; hist2[i] = sum2; sum2 = t2;
		const hxhistogram_t t3 = hist3[i] + sum3; hist3[i] = sum3; sum3 = t3;
	}

	// Making this into a loop would hurt ARM perf.
	hxradix_sort_key_void* src = begin;
	if(!skip0) {
		hxradix_sort_key_void* const hxrestrict dst = buf1;
		for(const hxradix_sort_key_void* hxrestrict it = src, * const src_end = src + size;
				it != src_end; ++it) {
			dst[hist0[it->get_modified_key() & 0xffu]++] = *it;
		}
		src = dst;
	}
	if(!skip1) {
		hxradix_sort_key_void* const hxrestrict dst = (src == begin) ? buf1 : begin;
		for(const hxradix_sort_key_void* hxrestrict it = src, * const src_end = src + size;
				it != src_end; ++it) {
			dst[hist1[(it->get_modified_key() >> 8) & 0xffu]++] = *it;
		}
		src = dst;
	}
	if(!skip2) {
		hxradix_sort_key_void* const hxrestrict dst = (src == begin) ? buf1 : begin;
		for(const hxradix_sort_key_void* hxrestrict it = src, * const src_end = src + size;
				it != src_end; ++it) {
			dst[hist2[(it->get_modified_key() >> 16) & 0xffu]++] = *it;
		}
		src = dst;
	}
	if(!skip3) {
		hxradix_sort_key_void* const hxrestrict dst = (src == begin) ? buf1 : begin;
		for(const hxradix_sort_key_void* hxrestrict it = src, * const src_end = src + size;
				it != src_end; ++it) {
			dst[hist3[it->get_modified_key() >> 24]++] = *it;
		}
		src = dst;
	}
	if(src != begin) {
		::memcpy(begin, src, buffer_bytes);
	}

	hxfree(buf1);
}

hxattr_hot void hxradix_sort_void11(hxradix_sort_key_void* begin, hxradix_sort_key_void* end,
		hxsystem_allocator_t allocator) {
	hxassertf(end >= begin, "radix_sort end < begin %zd", end - begin);
	// Check for size overflowing hxhistogram_t.
	hxassertf(static_cast<size_t>(end - begin) < ~static_cast<hxhistogram_t>(0),
		"hxhistogram_t overflow %zd", end - begin);

	const hxhistogram_t size = static_cast<hxhistogram_t>(end - begin);
	if(size < HX_RADIX_SORT_MIN_SIZE) {
		hxinsertion_sort(begin, end);
		return;
	}

	const hxsystem_allocator_scope allocator_scope(allocator);

	// A single allocation holds two working buffers for extremely large data
	// sets and the histograms.
	const size_t buffer_bytes = static_cast<size_t>(size) * sizeof(hxradix_sort_key_void);
	hxradix_sort_key_void* const buf1 = reinterpret_cast<hxradix_sort_key_void*>(
		hxmalloc(buffer_bytes * 2u + 5120u * sizeof(hxhistogram_t)));
	hxradix_sort_key_void* const buf2 = buf1 + size;
	hxhistogram_t* const histograms = reinterpret_cast<hxhistogram_t*>(buf2 + size);
	::memset(histograms, 0x00, 5120u * sizeof(hxhistogram_t)); // 20k

	hxhistogram_t* hxrestrict hist0 = histograms + 0u; // 2048 values
	hxhistogram_t* hxrestrict hist1 = histograms + 2048u; // 2048 values
	hxhistogram_t* hxrestrict hist2 = histograms + 4096u; // 1024 values

	for(const hxradix_sort_key_void* hxrestrict it = begin; it != end; ++it) {
		const hxhistogram_t x = it->get_modified_key();
		++hist0[x & 0x7ffu];
		++hist1[(x >> 11) & 0x7ffu];
		++hist2[x >> 22];
	}

	// A pass where a single bucket holds every key is a stable identity
	// permutation and is skipped.
	const hxhistogram_t first_key = begin->get_modified_key();
	const bool skip0 = hist0[first_key & 0x7ffu] == size;
	const bool skip1 = hist1[(first_key >> 11) & 0x7ffu] == size;
	const bool skip2 = hist2[first_key >> 22] == size;

	// Convert histograms to start indices
	hxhistogram_t sum0 = 0u, sum1 = 0u, sum2 = 0u;
	for(hxsize_t i = 0; i < 1024; ++i) {
		const hxhistogram_t t0 = hist0[i] + sum0; hist0[i] = sum0; sum0 = t0;
		const hxhistogram_t t1 = hist1[i] + sum1; hist1[i] = sum1; sum1 = t1;
		const hxhistogram_t t2 = hist2[i] + sum2; hist2[i] = sum2; sum2 = t2;
	}
	for(hxsize_t i = 1024; i < 2048; ++i) {
		const hxhistogram_t t0 = hist0[i] + sum0; hist0[i] = sum0; sum0 = t0;
		const hxhistogram_t t1 = hist1[i] + sum1; hist1[i] = sum1; sum1 = t1;
	}

	// 0 to 3 pass radix sort. Passes ping-pong between the first two buffers
	// except that a full 3 pass chain rotates through buf2 to end at begin. A
	// trailing copy repairs an odd pass count for less than a pass costs.
	hxradix_sort_key_void* src = begin;
	if(!skip0) {
		hxradix_sort_key_void* const hxrestrict dst = buf1;
		for(const hxradix_sort_key_void* hxrestrict it = src, * const src_end = src + size;
				it != src_end; ++it) {
			dst[hist0[it->get_modified_key() & 0x7ffu]++] = *it;
		}
		src = dst;
	}
	if(!skip1) {
		hxradix_sort_key_void* const hxrestrict dst = (src == begin) ? buf1
			: (skip2 ? begin : buf2);
		for(const hxradix_sort_key_void* hxrestrict it = src, * const src_end = src + size;
				it != src_end; ++it) {
			dst[hist1[(it->get_modified_key() >> 11) & 0x7ffu]++] = *it;
		}
		src = dst;
	}
	if(!skip2) {
		hxradix_sort_key_void* const hxrestrict dst = (src == begin) ? buf1 : begin;
		for(const hxradix_sort_key_void* hxrestrict it = src, * const src_end = src + size;
				it != src_end; ++it) {
			dst[hist2[it->get_modified_key() >> 22]++] = *it;
		}
		src = dst;
	}
	if(src != begin) {
		::memcpy(begin, src, buffer_bytes);
	}

	hxfree(buf1);
}

HX_NS_END_
