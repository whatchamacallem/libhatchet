#pragma once
// SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
// SPDX-License-Identifier: MIT
// This file is licensed under the MIT license found in the LICENSE.md file.

#include "libhatchet.h"

class hxtask_queue;

/// `hxtask` - Pure virtual base class for operations to be performed on a
/// different thread or at a later time.
class hxtask {
public:
	/// A virtual destructor.
	virtual ~hxtask( ) { }

	/// Executes the task. This is the main function to implement in derived
	/// classes. It is also wrapped in a `hxprofiler` scope when called.
	/// - `q` : Pointer to the task queue managing this task.
	virtual void execute(hxtask_queue* q_) = 0;

	/// This call is the last time this object is touched by the `hxtask_queue`.
	/// An `on_completion` override may delete or re-enqueue the `this` pointer. 
	/// - `q` : Pointer to the task queue managing this task.
	virtual void on_completion(hxtask_queue* q_) { (void)q_; };

	/// Returns the label of the task, or `"task"` by default.
	virtual const char* get_label(void) const { return "task"; }

private:
	friend class hxtask_queue;

	// Reduce confusion by separating these out.
	void process(hxtask_queue* q_) {
		execute(q_);
		on_completion(q_);
	}
};
