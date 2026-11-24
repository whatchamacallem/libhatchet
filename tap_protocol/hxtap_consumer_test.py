# SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
# SPDX-License-Identifier: MIT
# This file is licensed under the terms of the LICENSE.md file.
#
# The tests of hxtap_consumer.py. Each one kills a mutant of the consumer that
# the others do not. Quiet unless run with -v. From the top level directory:
# python3 tap_protocol/hxtap_consumer_test.py [-v]

import io
import sys
import unittest.mock

import hxtap_consumer

# Used for white box testing of the internal consumer class.
from hxtap_consumer import tap_consumer_


class hxtap_consumer_test(unittest.TestCase):
	def test_lines_before_the_version_are_ignored(self) -> None:
		tap_consumer = tap_consumer_()
		log_lines = ["not ok 1 - rom", "1..1", "TAP version 14", "ok 1 - a", "1..1"]
		self.assertEqual([tap_consumer.consume(log_line) for log_line in log_lines],
			[tap_consumer_.verdict.running] * 4 + [tap_consumer_.verdict.passed])

	def test_bail_out_before_the_version_fails(self) -> None:
		tap_consumer = tap_consumer_()
		self.assertEqual(tap_consumer.consume("Bail out! no image"), tap_consumer_.verdict.failed)

	def test_bail_out_in_a_subtest_fails(self) -> None:
		tap_consumer = tap_consumer_()
		log_lines = ["TAP version 14", "    Bail out! no card"]
		self.assertEqual([tap_consumer.consume(log_line) for log_line in log_lines],
			[tap_consumer_.verdict.running, tap_consumer_.verdict.failed])

	def test_second_version_fails(self) -> None:
		tap_consumer = tap_consumer_()
		log_lines = ["TAP version 14", "ok 1 - a", "TAP version 14"]
		self.assertEqual([tap_consumer.consume(log_line) for log_line in log_lines],
			[tap_consumer_.verdict.running] * 2 + [tap_consumer_.verdict.failed])

	def test_version_13_fails(self) -> None:
		tap_consumer = tap_consumer_()
		self.assertEqual(tap_consumer.consume("TAP version 13"), tap_consumer_.verdict.failed)

	def test_comments_and_other_lines_are_ignored(self) -> None:
		tap_consumer = tap_consumer_()
		log_lines = ["TAP version 14", "# Subtest: a", "pragma +strict", "okay 2", "  ok 2 - a",
			"ok 1 - a", "1..1"]
		self.assertEqual([tap_consumer.consume(log_line) for log_line in log_lines],
			[tap_consumer_.verdict.running] * 6 + [tap_consumer_.verdict.passed])

	def test_lowercase_skip_fails(self) -> None:
		tap_consumer = tap_consumer_()
		log_lines = ["TAP version 14", "ok 1 - a # skip"]
		self.assertEqual([tap_consumer.consume(log_line) for log_line in log_lines],
			[tap_consumer_.verdict.running, tap_consumer_.verdict.failed])

	def test_todo_is_not_an_expected_failure(self) -> None:
		tap_consumer = tap_consumer_(1)
		log_lines = ["TAP version 14", "not ok 1 - a # TODO"]
		self.assertEqual([tap_consumer.consume(log_line) for log_line in log_lines],
			[tap_consumer_.verdict.running, tap_consumer_.verdict.failed])

	def test_skip_on_the_plan_fails(self) -> None:
		tap_consumer = tap_consumer_()
		log_lines = ["TAP version 14", "ok 1 - a", "1..1 # SKIP the rest"]
		self.assertEqual([tap_consumer.consume(log_line) for log_line in log_lines],
			[tap_consumer_.verdict.running] * 2 + [tap_consumer_.verdict.failed])

	def test_escaped_hash_is_not_an_excuse(self) -> None:
		tap_consumer = tap_consumer_()
		log_lines = ["TAP version 14", "ok 1 - a \\# SKIP", "1..1"]
		self.assertEqual([tap_consumer.consume(log_line) for log_line in log_lines],
			[tap_consumer_.verdict.running] * 2 + [tap_consumer_.verdict.passed])

	def test_subtest_numbers_from_1_and_its_parent_continues(self) -> None:
		tap_consumer = tap_consumer_()
		log_lines = ["TAP version 14", "ok 1 - a", "# Subtest: b", "    ok 1 - c", "    1..1",
			"ok 2 - b", "1..2"]
		self.assertEqual([tap_consumer.consume(log_line) for log_line in log_lines],
			[tap_consumer_.verdict.running] * 6 + [tap_consumer_.verdict.passed])

	def test_failed_subtest_is_one_expected_failure(self) -> None:
		tap_consumer = tap_consumer_(1)
		log_lines = ["TAP version 14", "    not ok 1 - c", "    1..1", "not ok 1 - b", "1..1"]
		self.assertEqual([tap_consumer.consume(log_line) for log_line in log_lines],
			[tap_consumer_.verdict.running] * 4 + [tap_consumer_.verdict.passed])

	def test_failed_subtest_reported_ok_fails(self) -> None:
		tap_consumer = tap_consumer_()
		log_lines = ["TAP version 14", "    not ok 1 - c", "    1..1", "ok 1 - b"]
		self.assertEqual([tap_consumer.consume(log_line) for log_line in log_lines],
			[tap_consumer_.verdict.running] * 3 + [tap_consumer_.verdict.failed])

	def test_passed_subtest_reported_not_ok_fails(self) -> None:
		tap_consumer = tap_consumer_(1)
		log_lines = ["TAP version 14", "    ok 1 - c", "    1..1", "not ok 1 - b"]
		self.assertEqual([tap_consumer.consume(log_line) for log_line in log_lines],
			[tap_consumer_.verdict.running] * 3 + [tap_consumer_.verdict.failed])

	def test_subtest_without_a_plan_fails(self) -> None:
		tap_consumer = tap_consumer_()
		log_lines = ["TAP version 14", "    ok 1 - c", "ok 1 - b"]
		self.assertEqual([tap_consumer.consume(log_line) for log_line in log_lines],
			[tap_consumer_.verdict.running] * 2 + [tap_consumer_.verdict.failed])

	def test_subtest_ended_by_a_plan_fails(self) -> None:
		tap_consumer = tap_consumer_()
		log_lines = ["TAP version 14", "ok 1 - a", "    ok 1 - c", "    1..1", "1..1"]
		self.assertEqual([tap_consumer.consume(log_line) for log_line in log_lines],
			[tap_consumer_.verdict.running] * 4 + [tap_consumer_.verdict.failed])

	def test_test_point_after_a_subtest_plan_fails(self) -> None:
		tap_consumer = tap_consumer_()
		log_lines = ["TAP version 14", "    ok 1 - c", "    1..1", "    ok 2 - d"]
		self.assertEqual([tap_consumer.consume(log_line) for log_line in log_lines],
			[tap_consumer_.verdict.running] * 3 + [tap_consumer_.verdict.failed])

	def test_subtest_indented_twice_fails(self) -> None:
		tap_consumer = tap_consumer_()
		log_lines = ["TAP version 14", "        ok 1 - d"]
		self.assertEqual([tap_consumer.consume(log_line) for log_line in log_lines],
			[tap_consumer_.verdict.running, tap_consumer_.verdict.failed])

	def test_plan_of_0_fails(self) -> None:
		tap_consumer = tap_consumer_()
		log_lines = ["TAP version 14", "1..0"]
		self.assertEqual([tap_consumer.consume(log_line) for log_line in log_lines],
			[tap_consumer_.verdict.running, tap_consumer_.verdict.failed])

	def test_plan_above_the_count_fails(self) -> None:
		tap_consumer = tap_consumer_()
		log_lines = ["TAP version 14", "ok 1 - a", "1..2"]
		self.assertEqual([tap_consumer.consume(log_line) for log_line in log_lines],
			[tap_consumer_.verdict.running] * 2 + [tap_consumer_.verdict.failed])

	def test_plan_below_the_count_fails(self) -> None:
		tap_consumer = tap_consumer_()
		log_lines = ["TAP version 14", "ok 1 - a", "ok 2 - b", "1..1"]
		self.assertEqual([tap_consumer.consume(log_line) for log_line in log_lines],
			[tap_consumer_.verdict.running] * 3 + [tap_consumer_.verdict.failed])

	def test_unexpected_failure_fails(self) -> None:
		tap_consumer = tap_consumer_()
		log_lines = ["TAP version 14", "not ok 1 - a", "1..1"]
		self.assertEqual([tap_consumer.consume(log_line) for log_line in log_lines],
			[tap_consumer_.verdict.running] * 2 + [tap_consumer_.verdict.failed])

	def test_missing_expected_failure_fails(self) -> None:
		tap_consumer = tap_consumer_(1)
		log_lines = ["TAP version 14", "ok 1 - a", "1..1"]
		self.assertEqual([tap_consumer.consume(log_line) for log_line in log_lines],
			[tap_consumer_.verdict.running] * 2 + [tap_consumer_.verdict.failed])

	def test_unnumbered_test_point_fails(self) -> None:
		tap_consumer = tap_consumer_()
		log_lines = ["TAP version 14", "ok - a"]
		self.assertEqual([tap_consumer.consume(log_line) for log_line in log_lines],
			[tap_consumer_.verdict.running, tap_consumer_.verdict.failed])

	def test_repeated_number_fails(self) -> None:
		tap_consumer = tap_consumer_()
		log_lines = ["TAP version 14", "ok 1 - a", "ok 1 - b"]
		self.assertEqual([tap_consumer.consume(log_line) for log_line in log_lines],
			[tap_consumer_.verdict.running] * 2 + [tap_consumer_.verdict.failed])

	def test_skipped_number_fails(self) -> None:
		tap_consumer = tap_consumer_()
		log_lines = ["TAP version 14", "ok 1 - a", "ok 3 - b"]
		self.assertEqual([tap_consumer.consume(log_line) for log_line in log_lines],
			[tap_consumer_.verdict.running] * 2 + [tap_consumer_.verdict.failed])

	def test_main_takes_crlf_lines(self) -> None:
		standard_input = io.TextIOWrapper(io.BytesIO(b"TAP version 14\r\nok 1 - a\r\n1..1\r\n"))
		with unittest.mock.patch("sys.stdin", standard_input):
			self.assertEqual(hxtap_consumer.main(), 0)

	def test_main_fails_a_document_without_its_plan(self) -> None:
		standard_input = io.TextIOWrapper(io.BytesIO(b"TAP version 14\nok 1 - a\n"))
		with unittest.mock.patch("sys.stdin", standard_input):
			self.assertEqual(hxtap_consumer.main(), 1)


if __name__ == "__main__":
	# The consumer's output shows only for a test that fails, unless -v.
	verbose_run = "-v" in sys.argv[1:] or "--verbose" in sys.argv[1:]
	unittest.main(verbosity=0, buffer=not verbose_run)
