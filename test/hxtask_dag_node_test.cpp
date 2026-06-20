// SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
// SPDX-License-Identifier: MIT
// This file is licensed under the MIT license found in the LICENSE.md file.

#include <hx/hxtest.hpp>

#include <hx/hxtask_dag_node.hpp>

HX_NS_USE

namespace {

// Tracks every callback the queue fires on a node.
class hxtask_dag_node_test_tracker_t : public hxtask_dag_node<4> {
public:
	bool m_succeed    = true;   // execute() return value
	bool m_executed   = false;
	bool m_completed  = false;
	bool m_failed     = false;
	bool m_cancelled  = false;

	bool execute(hxtask_queue*) override {
		m_executed = true;
		return m_succeed;
	}

	void on_completion(hxtask_queue* q_) override {
		m_completed = true;
		hxtask_dag_node::on_completion(q_);
	}

	void on_failure(hxtask_queue* q_) override {
		m_failed = true;
		hxtask_dag_node::on_failure(q_);
	}

	void on_cancel(hxtask_queue* q_) override {
		m_cancelled = true;
		hxtask_dag_node::on_cancel(q_);
	}
};

} // namespace {

// A -> B -> C, all succeed: each node executes once in order.
TEST(hxtask_dag_node_test, linear_chain_success) {
	const hxsystem_allocator_scope s(hxsystem_allocator_temporary_stack);

	hxtask_dag_node_test_tracker_t a, b, c;
	a.add_successor(&b);
	b.add_successor(&c);

	hxtask_queue q(3, 0);
	q.enqueue(&a);
	q.wait_for_all();

	EXPECT_TRUE(a.m_executed);   EXPECT_TRUE(a.m_completed);
	EXPECT_FALSE(a.m_failed);    EXPECT_FALSE(a.m_cancelled);

	EXPECT_TRUE(b.m_executed);   EXPECT_TRUE(b.m_completed);
	EXPECT_FALSE(b.m_failed);    EXPECT_FALSE(b.m_cancelled);

	EXPECT_TRUE(c.m_executed);   EXPECT_TRUE(c.m_completed);
	EXPECT_FALSE(c.m_failed);    EXPECT_FALSE(c.m_cancelled);
}

// A fails -> B and C receive on_cancel, never execute.
TEST(hxtask_dag_node_test, linear_chain_failure_propagates) {
	const hxsystem_allocator_scope s(hxsystem_allocator_temporary_stack);

	hxtask_dag_node_test_tracker_t a, b, c;
	a.m_succeed = false;
	a.add_successor(&b);
	b.add_successor(&c);

	hxtask_queue q(3, 0);
	q.enqueue(&a);
	q.wait_for_all();

	EXPECT_TRUE(a.m_executed);   EXPECT_TRUE(a.m_failed);
	EXPECT_FALSE(a.m_completed); EXPECT_FALSE(a.m_cancelled);

	EXPECT_FALSE(b.m_executed);  EXPECT_TRUE(b.m_cancelled);
	EXPECT_FALSE(b.m_completed); EXPECT_FALSE(b.m_failed);

	EXPECT_FALSE(c.m_executed);  EXPECT_TRUE(c.m_cancelled);
	EXPECT_FALSE(c.m_completed); EXPECT_FALSE(c.m_failed);
}

// A is cancelled -> B and C receive on_cancel, never execute.
TEST(hxtask_dag_node_test, linear_chain_cancel_propagates) {
	const hxsystem_allocator_scope s(hxsystem_allocator_temporary_stack);

	hxtask_dag_node_test_tracker_t a, b, c;
	a.add_successor(&b);
	b.add_successor(&c);

	hxtask_queue q(3, 0);
	q.enqueue(&a);
	q.cancel(&a);
	q.wait_for_all();

	EXPECT_FALSE(a.m_executed);  EXPECT_TRUE(a.m_cancelled);
	EXPECT_FALSE(a.m_completed); EXPECT_FALSE(a.m_failed);

	EXPECT_FALSE(b.m_executed);  EXPECT_TRUE(b.m_cancelled);
	EXPECT_FALSE(b.m_completed); EXPECT_FALSE(b.m_failed);

	EXPECT_FALSE(c.m_executed);  EXPECT_TRUE(c.m_cancelled);
	EXPECT_FALSE(c.m_completed); EXPECT_FALSE(c.m_failed);
}

// A -> {B, C} -> D (diamond), all succeed: D executes once.
TEST(hxtask_dag_node_test, diamond_all_succeed) {
	const hxsystem_allocator_scope s(hxsystem_allocator_temporary_stack);

	hxtask_dag_node_test_tracker_t a, b, c, d;
	a.add_successor(&b);
	a.add_successor(&c);
	b.add_successor(&d);
	c.add_successor(&d);

	hxtask_queue q(4, 0);
	q.enqueue(&a);
	q.wait_for_all();

	EXPECT_TRUE(a.m_executed);   EXPECT_TRUE(a.m_completed);
	EXPECT_TRUE(b.m_executed);   EXPECT_TRUE(b.m_completed);
	EXPECT_TRUE(c.m_executed);   EXPECT_TRUE(c.m_completed);
	EXPECT_TRUE(d.m_executed);   EXPECT_TRUE(d.m_completed);
	EXPECT_FALSE(d.m_failed);    EXPECT_FALSE(d.m_cancelled);
}

// A -> {B, C} -> D: B fails, C succeeds -> D receives on_failure.
TEST(hxtask_dag_node_test, diamond_one_arm_fails) {
	const hxsystem_allocator_scope s(hxsystem_allocator_temporary_stack);

	hxtask_dag_node_test_tracker_t a, b, c, d;
	b.m_succeed = false;
	a.add_successor(&b);
	a.add_successor(&c);
	b.add_successor(&d);
	c.add_successor(&d);

	hxtask_queue q(4, 0);
	q.enqueue(&a);
	q.wait_for_all();

	EXPECT_TRUE(a.m_executed);   EXPECT_TRUE(a.m_completed);
	EXPECT_TRUE(b.m_executed);   EXPECT_TRUE(b.m_failed);
	EXPECT_TRUE(c.m_executed);   EXPECT_TRUE(c.m_completed);
	EXPECT_FALSE(d.m_executed);  EXPECT_TRUE(d.m_cancelled);
	EXPECT_FALSE(d.m_completed); EXPECT_FALSE(d.m_failed);
}

// B and C are independent roots, both pointing to D.
// B is cancelled while queued; C runs and fails.
// D should receive on_cancel (cancelled > failed).
TEST(hxtask_dag_node_test, cancel_beats_fail_at_join) {
	const hxsystem_allocator_scope s(hxsystem_allocator_temporary_stack);

	hxtask_dag_node_test_tracker_t b, c, d;
	c.m_succeed = false;
	b.add_successor(&d);
	c.add_successor(&d);

	hxtask_queue q(2, 0);
	q.enqueue(&b);
	q.enqueue(&c);
	// Cancel B while it is queued (single-threaded queue hasn't run yet).
	// on_cancel propagates to D, dropping D's pending from 2 to 1.
	q.cancel(&b);
	q.wait_for_all();  // C runs, fails; D's last pending hits 0 with state=cancelled.

	EXPECT_FALSE(b.m_executed);  EXPECT_TRUE(b.m_cancelled);
	EXPECT_TRUE(c.m_executed);   EXPECT_TRUE(c.m_failed);
	EXPECT_FALSE(d.m_executed);  EXPECT_TRUE(d.m_cancelled);
	EXPECT_FALSE(d.m_failed);
}

// 20-node DAG across 5 layers with out-degrees 1-4 and in-degrees 1-4.
TEST(hxtask_dag_node_test, stress_20_node_dag) {
	const hxsystem_allocator_scope s(hxsystem_allocator_temporary_stack);

	hxtask_dag_node_test_tracker_t nodes[20];

	static const uint8_t edges[][2] = {
		{0,1},{0,2},{0,3},{0,4},{1,5},{1,6},{2,5},{2,6},{2,7},{3,7},{3,8},
		{4,8},{4,9},{4,10},{5,11},{5,12},{6,11},{6,12},{6,13},{7,12},{7,13},
		{8,13},{8,14},{8,15},{9,14},{10,14},{10,15},{11,16},{11,17},{12,16},
		{12,17},{12,18},{13,17},{13,18},{14,18},{14,19},{15,17},{15,19}
	};
	for (const auto& e : edges) {
		nodes[e[0]].add_successor(&nodes[e[1]]);
	}

	hxtask_queue q(20, 0);
	q.enqueue(&nodes[0]);
	q.wait_for_all();

	for (size_t i = 0u; i < 20u; ++i) {
		EXPECT_TRUE(nodes[i].m_executed && nodes[i].m_completed);
		EXPECT_FALSE(nodes[i].m_failed || nodes[i].m_cancelled);
	}
}
