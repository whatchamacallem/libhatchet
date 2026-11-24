# SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
# SPDX-License-Identifier: MIT
# This file is licensed under the terms of the LICENSE.md file.
#
# A standalone script that reads a TAP-Protocol 14 log from standard input.
# SKIP and TODO are disallowed for being excuses. Test plans are expected
# at the end, which is overfitting the protocol.

import enum
import re
import sys
from dataclasses import dataclass, field
from typing import Iterable


# Internal.
@dataclass
class tap_consumer_:
	# A SKIP or TODO directive.
	excuse_pattern = re.compile(
		r"\s*(?:(?:not )?ok\b|1\.\.\d+)(?:[^#\\]|\\.)*#\s*(?:SKIP|TODO)\b", re.IGNORECASE)

	# A test point or plan without its indent. The groups are the "not " and
	# number of a test point and the count of a plan.
	line_pattern = re.compile(r"(?:(not )?ok(?: ([0-9]+))?|1\.\.([0-9]+))(?: |$)")

	class verdict(enum.Enum):
		running = 0
		passed = 1
		failed = 2

	@dataclass
	class level:
		test_count: int = 0
		failure_count: int = 0
		planned: bool = False

	expected_failure_count: int = 0

	levels: list[level] = field(default_factory=list[level])

	def consume(self, log_line: str) -> verdict:
		line_body = log_line.lstrip(" ")
		if line_body.startswith("Bail out!"):
			return self.verdict.failed
		if log_line.startswith("TAP version"):
			if self.levels or log_line != "TAP version 14":
				print("error: TAP version 14 required: " + log_line, file=sys.stderr, flush=True)
				return self.verdict.failed
			self.levels.append(self.level())
			return self.verdict.running
		if not self.levels:
			return self.verdict.running
		if self.excuse_pattern.match(log_line) is not None:
			print("error: no excuses: " + log_line, file=sys.stderr, flush=True)
			return self.verdict.failed
		line_match = self.line_pattern.match(line_body)
		line_depth, indent_remainder = divmod(len(log_line) - len(line_body), 4)
		if line_match is None or indent_remainder != 0:
			return self.verdict.running
		failure_prefix, test_number, plan_count = line_match.groups()

		# A line one level deeper starts a subtest. One a level shallower ends it.
		if line_depth == len(self.levels):
			self.levels.append(self.level())
		elif line_depth == len(self.levels) - 2:
			subtest_level = self.levels.pop()
			if (plan_count is not None or not subtest_level.planned
					or (failure_prefix is None) != (subtest_level.failure_count == 0)):
				print("error: subtest not ended by its plan and its result: " + log_line,
					file=sys.stderr, flush=True)
				return self.verdict.failed
		elif line_depth != len(self.levels) - 1:
			print("error: indented past a subtest: " + log_line, file=sys.stderr, flush=True)
			return self.verdict.failed

		test_level = self.levels[-1]
		if test_level.planned:
			print("error: after the plan: " + log_line, file=sys.stderr, flush=True)
			return self.verdict.failed
		if plan_count is not None:
			if int(plan_count) == 0 or int(plan_count) != test_level.test_count:
				print("error: %d test points: %s" % (test_level.test_count, log_line),
					file=sys.stderr, flush=True)
				return self.verdict.failed
			test_level.planned = True
			if len(self.levels) != 1:
				return self.verdict.running
			if test_level.failure_count != self.expected_failure_count:
				return self.verdict.failed
			if test_level.failure_count != 0:
				print("note: expected failures: %d" % test_level.failure_count, file=sys.stderr,
					flush=True)
			return self.verdict.passed
		test_level.test_count += 1
		if test_number is None or int(test_number) != test_level.test_count:
			print("error: not test point %d: %s" % (test_level.test_count, log_line),
				file=sys.stderr, flush=True)
			return self.verdict.failed
		if failure_prefix is not None:
			test_level.failure_count += 1
		return self.verdict.running


def consume_stream(log_lines: Iterable[str], expected_failure_count: int = 0) -> bool:
	consumer = tap_consumer_(expected_failure_count)
	for log_line in log_lines:
		print(log_line, flush=True)
		tap_verdict = consumer.consume(log_line)
		if tap_verdict != tap_consumer_.verdict.running:
			return tap_verdict == tap_consumer_.verdict.passed
	return False


def main() -> int:
	log_lines = (line_bytes.decode("utf-8", "replace").rstrip("\r\n")
		for line_bytes in sys.stdin.buffer)
	if not consume_stream(log_lines):
		return 1
	return 0


if __name__ == "__main__":
	sys.exit(main())
