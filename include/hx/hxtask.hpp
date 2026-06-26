#pragma once
// SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
// SPDX-License-Identifier: MIT
// This file is licensed under the MIT license found in the LICENSE.md file.

/// \file
/// Pure virtual base class for operations to be performed on a different thread
/// or at a later time.

#include "libhatchet.h"

// HX_USE_MODULE allows including macros in addition to the hx module.
#if HX_USE_MODULE
#error Header does not provide macros only.
#endif

HX_NS_BEGIN_

class hxtask_queue;

/// `hxtask` - Pure virtual base class for operations to be performed on a
/// different thread or at a later time.
class hxtask {
public:
	/// Executes the task. This is the main function to implement in derived
	/// classes. It is also wrapped in a `hxprofiler` scope when called.
	/// Returns `true` on success or `false` on failure.
	/// - `q` : Pointer to the task queue managing this task.
	virtual bool execute(hxtask_queue*) = 0;

	/// Indicates successful execution. This call is the last time this object
	/// is touched by the `hxtask_queue`. An `on_completion` override may delete
	/// or re-enqueue the `this` pointer.
	/// - `q` : Pointer to the task queue managing this task.
	virtual void on_completion(hxtask_queue*) { }

	/// Indicates failed execution. This call is the last time this object is
	/// touched by the `hxtask_queue`. An `on_failure` override may delete or
	/// re-enqueue the `this` pointer.
	/// - `q` : Pointer to the task queue managing this task.
	virtual void on_failure(hxtask_queue*) { }

	/// Indicates cancelled execution. This call is the last time this object is
	/// touched by the `hxtask_queue`. WARNING: on_cancel may not reenqueue.
	/// - `q` : Pointer to the task queue managing this task.
	virtual void on_cancel(hxtask_queue*) { }

	/// Returns the label of the task, or `"task"` by default.
	virtual const char* get_label(void) const { return "task"; }

private:
	friend class hxtask_queue;

	// Reduce confusion by separating these out.
	void process(hxtask_queue* q_) {
		if(execute(q_)) {
			on_completion(q_);
		}
		else {
			on_failure(q_);
		}
	}
};

HX_NS_END_
