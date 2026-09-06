// SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
// SPDX-License-Identifier: MIT
// This file is licensed under the MIT license found in the LICENSE.md file.

// Type erased parts of hxhash_table. If they show up in the profiler then
// manually inline them in the .inl as link time optimization may not be enough.

#include "../include/hx/hxhash_table.hpp"

HX_NS_BEGIN_

hxsize_t hxhash_table_base_::chain_count_multi_(const hxhash_node_base* head_, const void* key_,
		hxhash_equal_fn_ equal_) {
	hxsize_t total_ = 0;
	for(const hxhash_node_base* hxrestrict node_ = head_; node_ != hxnull; node_ = node_->hash_next) {
		if(equal_(node_, key_)) {
			++total_;
		}
	}
	return total_;
}

void hxhash_table_base_::chain_erase_at_(hxhash_node_base** head_, hxhash_node_base* node_) {
	hxhash_node_base* previous_ = hxnull;
	for(hxhash_node_base* it_ = *head_; it_ != node_; it_ = it_->hash_next) {
		previous_ = it_;
	}
	if(previous_ != hxnull) {
		previous_->hash_next = node_->hash_next;
	}
	else {
		*head_ = node_->hash_next;
	}
}

hxsize_t hxhash_table_base_::chain_erase_multi_(hxhash_node_base** head_, const void* key_,
		hxhash_equal_fn_ equal_, hxhash_deleter_fn_ deleter_, void* deleter_context_) {
	hxhash_node_base* node_ = *head_;
	if(node_ == hxnull) {
		return 0;
	}
	hxsize_t count_ = 0;
	// The matching prefix pointed to by the head.
	if(equal_(node_, key_)) {
		do {
			hxhash_node_base* const next_ = node_->hash_next;
			if(deleter_ != hxnull) {
				deleter_(node_, deleter_context_);
			}
			++count_;
			node_ = next_;
		} while(node_ != hxnull && equal_(node_, key_));
		*head_ = node_;
	}
	// Those following a non-matching node.
	if(node_ != hxnull) {
		hxhash_node_base* previous_ = node_;
		node_ = node_->hash_next;
		while(node_ != hxnull) {
			hxhash_node_base* const next_ = node_->hash_next;
			if(equal_(node_, key_)) {
				previous_->hash_next = next_;
				if(deleter_ != hxnull) {
					deleter_(node_, deleter_context_);
				}
				++count_;
			}
			else {
				previous_ = node_;
			}
			node_ = next_;
		}
	}
	return count_;
}

hxhash_node_base* hxhash_table_base_::chain_extract_(hxhash_node_base** head_, const void* key_,
		hxhash_equal_fn_ equal_) {
	hxhash_node_base* previous_ = hxnull;
	for(hxhash_node_base* node_ = *head_; node_ != hxnull; node_ = node_->hash_next) {
		if(equal_(node_, key_)) {
			if(previous_ != hxnull) {
				previous_->hash_next = node_->hash_next;
			}
			else {
				*head_ = node_->hash_next;
			}
			return node_;
		}
		previous_ = node_;
	}
	return hxnull;
}

hxsize_t hxhash_table_base_::chain_length_(const hxhash_node_base* head_) {
	hxsize_t count_ = 0;
	for(const hxhash_node_base* hxrestrict node_ = head_; node_ != hxnull; node_ = node_->hash_next) {
		++count_;
	}
	return count_;
}

hxhash_node_base* hxhash_table_base_::chain_replace_(hxhash_node_base** head_, hxhash_node_base* node_,
		const void* key_, hxhash_equal_fn_ equal_) {
	hxhash_node_base* previous_ = hxnull;
	for(hxhash_node_base* existing_ = *head_; existing_ != hxnull; existing_ = existing_->hash_next) {
		if(equal_(existing_, key_)) {
			node_->hash_next = existing_->hash_next;
			if(previous_ != hxnull) {
				previous_->hash_next = node_;
			}
			else {
				*head_ = node_;
			}
			return existing_;
		}
		previous_ = existing_;
	}
	node_->hash_next = *head_;
	*head_ = node_;
	return hxnull;
}

void hxhash_table_base_::table_clear_(hxhash_node_base** table_, hxsize_t bucket_count_,
		hxhash_deleter_fn_ deleter_, void* deleter_context_) {
	hxhash_node_base** const it_end_ = table_ + bucket_count_;
	for(hxhash_node_base** hxrestrict it_ = table_; it_ != it_end_; ++it_) {
		hxhash_node_base* node_ = *it_;
		if(node_ != hxnull) {
			*it_ = hxnull;
			do {
				hxhash_node_base* const next_ = node_->hash_next;
				deleter_(node_, deleter_context_);
				node_ = next_;
			} while(node_ != hxnull);
		}
	}
}

HX_NS_END_
