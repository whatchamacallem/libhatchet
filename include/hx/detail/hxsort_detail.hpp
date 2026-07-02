#pragma once
// SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
// SPDX-License-Identifier: MIT
// This file is licensed under the MIT license found in the LICENSE.md file.

#ifndef LIBHATCHET_VER
#error Internal. Do not include this file directly.
#endif

#ifndef HX_DOXYGEN_PARSER

template<typename iterator_t_, typename less_t_> hxattr_hot hxconstexpr
void hxinsertion_sort(iterator_t_ begin_, iterator_t_ end_, const less_t_& less_);

template<typename iterator_t_, typename less_t_> hxattr_hot hxconstexpr
void hxheapsort(iterator_t_ begin_, iterator_t_ end_, const less_t_& less_);

namespace hxdetail_ {

hxinline_constexpr hxsize_t hxpartition_sort_cutoff_ = 32;

// Restores the heap property by sifting the current value down until it is not
// less than its children. Holds the value in a temporary so that each level
// costs a single move instead of a swap.
template<typename iterator_t_, typename less_t_> hxattr_hot hxconstexpr
void hxheapsort_heapify_(const iterator_t_ begin_, iterator_t_ current_,
		const iterator_t_ end_, const less_t_& less_) {
	const hxsize_t size_ = end_ - begin_;
	auto value_ = hxmove(*current_);
	for(;;) {
		const hxsize_t left_idx_ = ((current_ - begin_) << 1) + hxsize_t{1};
		if(left_idx_ >= size_) {
			break;
		}
		iterator_t_ next_ = begin_ + left_idx_;
		const iterator_t_ right_ = next_ + hxsize_t{1};
		if(right_ < end_ && less_(*next_, *right_)) {
			next_ = right_;
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
template<typename iterator_t_, typename less_t_> hxattr_hot hxconstexpr
void hxmake_heap_(iterator_t_ begin_, iterator_t_ end_, const less_t_& less_) {
	for(hxsize_t i_ = (end_ - begin_) >> 1; i_ > hxsize_t{0}; ) {
		--i_;
		hxheapsort_heapify_<iterator_t_>(begin_, begin_ + i_, end_, less_);
	}
}

// Sorts `[begin, end)` in place using dual-pivot quicksort. Based on Java's
// `Array.sort` implementation details. Should be resistant to degeneration.
// Average time: `Θ(n log n)`, worst time: `Θ(n²)`. This algorithm is only
// intended to sort ranges over a minimum length before calling back to the
// `sort_callback` parameter.
template<typename iterator_t_, typename less_t_, typename sort_callback_t_>  hxattr_hot hxconstexpr
void hxpartition_sort_(iterator_t_ begin_, iterator_t_ end_, const less_t_& less_,
						const sort_callback_t_& sort_callback_, int depth_) {
	hxassertmsg((end_ - begin_) > hxpartition_sort_cutoff_, "range_error Use hxinsertion_sort.");
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

	// Move the selected pivots out of the way by placing them at the ends of
	// the range.
	hxswap(*begin_, *p1_);
	hxswap(*back_, *p3_);

	// Three-way partition into [<p₁], [p₁ ≤ … ≤ p₂], [>p₂]

	// Points to end of less-than range, which is empty and is right after the
	// first pivot.
	iterator_t_ lt_ = begin_ + hxsize_t{1};
	// Points to end of greater-than range, which is empty and right before the
	// last pivot. This is an end iterator that goes left.
	iterator_t_ gt_ = back_ - hxsize_t{1};

	for(iterator_t_ i_ = lt_; !(gt_ < i_); ) {
		if(less_(*i_, *begin_)) {
			// Swap into less-than range and extend it. Values in [lt, i) are
			// mid range, so the value swapped to i is already classified.
			if(lt_ != i_) {
				hxswap(*i_, *lt_);
			}
			++i_;
			++lt_;
		}
		else if(less_(*back_, *i_)) {
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

	// Swap pivots into final slots. Insert the lt_ pivot value where the last
	// last less-than value is, if it exists.
	if(begin_ != --lt_) {
		hxswap(*begin_, *lt_);
	}
	// Swap the first greater-than value with the gt_ pivot value, if it exists.
	if(back_ != ++gt_) {
		hxswap(*back_, *gt_);
	}

	// Recurse on the three partitions. Do not re-sort the partition values. At
	// this time lt_ and gt_ point right at their pivot values and they are
	// being used where [begin, end) semantics are expected.
	sort_callback_(begin_,  lt_,  less_, depth_);
	sort_callback_(lt_ + hxsize_t{1}, gt_,  less_, depth_);
	sort_callback_(gt_ + hxsize_t{1}, end_, less_, depth_);
}

// Implements the introsort algorithm which is a hybrid of quicksort, heapsort
// and insertion sort. `hxsort` is implemented using `hxintro_sort_`.
// `hxintro_sort_` calls itself recursively until it hits its depth limit.
template<typename iterator_t_, typename less_t_> hxattr_hot hxconstexpr
void hxintro_sort_(iterator_t_ begin_, iterator_t_ end_, const less_t_& less_, int depth_) {
	hxassertmsg(!(end_ < begin_), "range_error hxsort");

	if((end_ - begin_) <= hxpartition_sort_cutoff_) {
		hxinsertion_sort<iterator_t_>(begin_, end_, less_);
	} else if(depth_ == 0) {
		hxheapsort<iterator_t_>(begin_, end_, less_);
	} else {
		// Have the partition sort call back to hxsort for each sub-partition.
		hxpartition_sort_<iterator_t_>(begin_, end_, less_, hxintro_sort_<iterator_t_, less_t_>, depth_ - 1);
	}
}

} // namespace hxdetail_ {
#endif // HX_DOXYGEN_PARSER
