# SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
# SPDX-License-Identifier: MIT
# This file is licensed under the terms of the LICENSE.md file.
#
# The host of hxboot. Resets the board, serves the files it asks for and judges
# the one TAP document that hxboot starts and the launched program ends. A
# program that asks for nothing is judged the same way, so it also runs a board
# without hxboot.

import argparse
import sys
from typing import Final, Iterator, NamedTuple, Optional, Sequence

import hxboot_files
import hxboot_link
import hxboot_tty
import hxtap_consumer

# Matches HX_BOOT_IMAGE_NAME.
c_boot_image_name: Final = "hxboot.hxb"


class hxboot_server_options(NamedTuple):
	target_name: str
	# None finds it by USB vendor ID.
	device_path: Optional[str]
	root_directory: str
	# Seconds the board may write nothing before it is taken to have crashed.
	timeout_seconds: float
	# Refuses the image so hxboot launches its gold slot.
	gold_run: bool
	expected_failure_count: int


def hxboot_parse_options(command_arguments: Sequence[str]) -> hxboot_server_options:
	argument_parser = argparse.ArgumentParser()
	argument_parser.add_argument("--target", required=True, metavar="<board>")
	argument_parser.add_argument("--port", metavar="<tty>")
	argument_parser.add_argument("--root", default=".", metavar="<dir>")
	argument_parser.add_argument("--timeout", type=float, default=60.0, metavar="<seconds>")
	argument_parser.add_argument("--gold-run", action="store_true")
	argument_parser.add_argument("--expected-failures", type=int, default=0, metavar="<count>")
	parsed_arguments = argument_parser.parse_args(command_arguments)
	return hxboot_server_options(parsed_arguments.target, parsed_arguments.port,
		parsed_arguments.root, parsed_arguments.timeout, parsed_arguments.gold_run,
		parsed_arguments.expected_failures)


def hxboot_serve(serial_port: hxboot_tty.tty, file_service: hxboot_files.file_service,
		server_options: hxboot_server_options) -> Iterator[str]:
	"""Yields the lines that are not requests until the board goes quiet."""
	while True:
		log_line = serial_port.read_line(server_options.timeout_seconds)
		if log_line is None:
			return
		link_request = hxboot_link.parse(log_line)
		if link_request is not None:
			if (server_options.gold_run
					and link_request.operation_arguments[:1] == (c_boot_image_name,)):
				link_reply = hxboot_link.reply(hxboot_link.status.not_found, b"")
			else:
				link_reply = file_service.serve(link_request)
			serial_port.write(hxboot_link.frame(link_request, link_reply))
			continue
		if log_line.startswith("TAP version"):
			file_service.reset()
		yield log_line


def main(command_arguments: Sequence[str]) -> int:
	server_options = hxboot_parse_options(command_arguments)
	serial_port = hxboot_tty.tty(hxboot_tty.find_board(server_options.target_name),
		server_options.device_path)
	file_service = hxboot_files.file_service(server_options.root_directory)
	serial_port.open()
	try:
		serial_port.reset()
		if not hxtap_consumer.consume_stream(
				hxboot_serve(serial_port, file_service, server_options),
				server_options.expected_failure_count):
			return 1
		return 0
	finally:
		serial_port.close()


if __name__ == "__main__":
	sys.exit(main(sys.argv[1:]))
