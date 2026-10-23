#pragma once
// SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
// SPDX-License-Identifier: MIT
// This file is licensed under the MIT license found in the LICENSE.md file.

#ifndef LIBHATCHET_VER
#error Internal. Do not include this file directly.
#endif

#ifndef HX_DOXYGEN_PARSER

template<hxrandom_iterator_concept_ iterator_t_, typename less_t_> hxconstexpr
void hxinsertion_sort(iterator_t_ begin_, iterator_t_ end_, const less_t_& less_);

namespace hxdetail_ {

// Tested to provide the minimum total operator count when sorting.
hxinline_constexpr hxsize_t hxinsertion_sort_cutoff_ = 10;
hxinline_constexpr hxsize_t hxheapsort_cutoff_ = 20;

// Internal. This is the part that is not forcibly inlined. Restores the heap
// property by sifting `value_` down from the hole at `current_` until it is
// not less than its children. Each level costs a single move instead of a
// swap.
template<hxrandom_iterator_concept_ iterator_t_, typename value_t_, typename less_t_>
inline hxconstexpr hxattr_flatten
void hxheapsort_heapify_(const iterator_t_ begin_, iterator_t_ current_,
		const iterator_t_ end_, value_t_& value_, const less_t_& less_) {
	const hxsize_t size_ = end_ - begin_;
	for(;;) {
		const hxsize_t left_idx_ = ((current_ - begin_) << 1) + hxsize_t{1};
		if(left_idx_ >= size_) {
			break;
		}
		iterator_t_ next_ = begin_ + left_idx_;
		const iterator_t_ right_ = next_ + hxsize_t{1};
		if(right_ < end_) {
			if(less_(*next_, *right_)) {
				next_ = right_;
			}
		}
		if(!less_(value_, *next_)) {
			break;
		}
		*current_ = hxmove(*next_);
		current_ = next_;
	}
	*current_ = hxmove(value_);
}

// `hxmake_heap_` - Converts the range `[begin, end)` into a max heap using the
// provided comparator and Floyd's linear time bottom-up construction.
template<hxrandom_iterator_concept_ iterator_t_, typename less_t_>
hxinline hxconstexpr
void hxmake_heap_(hxrestrict_t<iterator_t_> begin_, iterator_t_ end_, const less_t_& less_) {
	for(hxsize_t i_ = (end_ - begin_) >> 1; i_ > hxsize_t{0}; ) {
		--i_;
		auto value_ = hxmove(*(begin_ + i_));
		hxheapsort_heapify_<iterator_t_>(begin_, begin_ + i_, end_, value_, less_);
	}
}

template<hxrandom_iterator_concept_ iterator_t_, typename less_t_>
hxinline hxconstexpr hxattr_flatten
void hxheapsort_(iterator_t_ begin_, iterator_t_ end_, const less_t_& less_) {
	hxassertf((end_ - begin_) > hxheapsort_cutoff_, "bad_range too small %zd",
		static_cast<hxsize_t>(end_ - begin_));
	hxdetail_::hxmake_heap_<iterator_t_>(begin_, end_, less_);
	const hxrestrict_t<iterator_t_> begin_r_(begin_);
	for(iterator_t_ it_ = end_ - ptrdiff_t{1}; begin_r_ < it_; --it_) {
		auto value_ = hxmove(*it_);
		*it_ = hxmove(*begin_r_);
		hxdetail_::hxheapsort_heapify_<iterator_t_>(begin_r_, begin_r_, it_, value_, less_);
	}
}

// Sorts `[begin, end)` in place using dual-pivot quicksort. Based on Java's
// `Array.sort` implementation details. Should be resistant to degeneration.
// Time: `O(n log n)`, guaranteed by the depth counter and heapsort fallback in
// `hxintro_sort_`. This algorithm is only intended to sort ranges over a
// minimum length before calling back to the `sort_callback` parameter.
template<hxrandom_iterator_concept_ iterator_t_, typename less_t_, typename sort_callback_t_>
hxinline hxconstexpr hxattr_flatten
void hxpartition_sort_(hxrestrict_t<iterator_t_> begin_, iterator_t_ end_, const less_t_& less_,
						const sort_callback_t_& sort_callback_, int depth_) {
	hxassertf((end_ - begin_) > hxinsertion_sort_cutoff_, "bad_range too small %zd",
		static_cast<hxsize_t>(end_ - begin_));
	const hxsize_t length_ = end_ - begin_;

	// Select 5 pivot values at 1/7th increments. And allow them to be naturally
	// sorted.
	const hxsize_t seventh_ = (length_ >> 3) + (length_ >> 6) + hxsize_t{1};
	iterator_t_ p2_ = begin_ + (length_ >> 1);
	iterator_t_ p1_ = p2_ - seventh_;
	iterator_t_ p0_ = p1_ - seventh_;
	iterator_t_ p3_ = p2_ + seventh_;
	iterator_t_ p4_ = p3_ + seventh_;

	// This is a Bose-Nelson sorting network for 5 elements. It should work well
	// with a processor that has branch prediction. (This intentionally swaps
	// pointers instead of the potentially heavy values pointed at.)
	if(less_(*p3_, *p0_)) { hxswap(p3_, p0_); }
	if(less_(*p4_, *p1_)) { hxswap(p4_, p1_); }
	if(less_(*p2_, *p0_)) { hxswap(p2_, p0_); }
	if(less_(*p3_, *p1_)) { hxswap(p3_, p1_); }
	if(less_(*p1_, *p0_)) { hxswap(p1_, p0_); }
	if(less_(*p4_, *p2_)) { hxswap(p4_, p2_); }
	if(less_(*p2_, *p1_)) { hxswap(p2_, p1_); }
	if(less_(*p4_, *p3_)) { hxswap(p4_, p3_); }
	if(less_(*p3_, *p2_)) { hxswap(p3_, p2_); }

	iterator_t_ back_ = end_ - hxsize_t{1}; // Pointer to the last value.

	// Move the selected pivots into temporaries. The values displaced from the
	// ends of the range fill the pivot slots and the ends become holes.
	auto pivot1_ = hxmove(*p1_);
	*p1_ = hxmove(*begin_);
	auto pivot2_ = hxmove(*p3_);
	*p3_ = hxmove(*back_);

	// Three-way partition into [<p₁], [p₁ ≤ … ≤ p₂], [>p₂]

	// Points to end of less-than range, which is empty and is right after the
	// first pivot.
	iterator_t_ lt_ = begin_ + hxsize_t{1};
	// Points to end of greater-than range, which is empty and right before the
	// last pivot. This is an end iterator that goes left.
	iterator_t_ gt_ = back_ - hxsize_t{1};

	for(iterator_t_ i_ = lt_; !(gt_ < i_); ) {
		if(less_(*i_, pivot1_)) {
			// Swap into less-than range and extend it. Values in [lt, i) are
			// mid range, so the value swapped to i is already classified.
			if(lt_ != i_) {
				hxswap(*i_, *lt_);
			}
			++i_;
			++lt_;
		}
		else if(less_(pivot2_, *i_)) {
			// Swap into greater-than range and extend it. If gt == i then the
			// loop is about to terminate due to --gt.
			if(gt_ != i_) {
				hxswap(*i_, *gt_);
			}
			--gt_;
		}
		else {
			// Leave the value in the mid range.
			++i_;
		}
	}

	// Move the pivots into their final slots. The last less-than value, if any,
	// fills the hole at begin and the first greater-than value, if any, fills
	// the hole at back.
	--lt_;
	if(begin_ != lt_) {
		*begin_ = hxmove(*lt_);
	}
	*lt_ = hxmove(pivot1_);
	++gt_;
	if(back_ != gt_) {
		*back_ = hxmove(*gt_);
	}
	*gt_ = hxmove(pivot2_);

	// Recurse on the three partitions. Do not re-sort the partition values. At
	// this time lt_ and gt_ point right at their pivot values and they are
	// being used where [begin, end) semantics are expected. The mid range is
	// already sorted when the pivots are equal.
	sort_callback_(begin_, lt_, less_, depth_);
	if(less_(*lt_, *gt_)) {
		sort_callback_(lt_ + hxsize_t{1}, gt_, less_, depth_);
	}
	sort_callback_(gt_ + hxsize_t{1}, end_, less_, depth_);
}

// hxattr_noinline prevents recursive inlining resulting in code bloat.
// Implements the introsort algorithm which is a hybrid of quicksort, heapsort
// and insertion sort. hxattr_noinline prevent this function from recursively
// inlining itself and blowing out the instruction cache.
template<hxrandom_iterator_concept_ iterator_t_, typename less_t_>
inline hxconstexpr hxattr_noinline
void hxintro_sort_(iterator_t_ begin_, iterator_t_ end_, const less_t_& less_, int depth_) {
	hxassertf(!(end_ < begin_), "bad_range end before begin %zd",
		static_cast<hxsize_t>(end_ - begin_));

	// The heapsort cutoff applies once the depth limit is reached.
	const hxsize_t cutoff_ = depth_ == 0 ? hxheapsort_cutoff_ : hxinsertion_sort_cutoff_;
	if((end_ - begin_) <= cutoff_) {
		hxinsertion_sort<iterator_t_>(begin_, end_, less_);
	} else if(depth_ == 0) {
		hxheapsort_<iterator_t_>(begin_, end_, less_);
	} else {
		hxpartition_sort_<iterator_t_>(begin_, end_, less_,
			hxintro_sort_<iterator_t_, less_t_>, depth_ - 1);
	}
}

} // namespace hxdetail_ {
#endif // HX_DOXYGEN_PARSER
