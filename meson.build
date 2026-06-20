# SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
# SPDX-License-Identifier: MIT
# This file is licensed under the terms of the LICENSE.md file.

project('libhatchet', ['c', 'cpp'],
	version : '1.0',
	license : 'MIT',
	default_options : [
		'c_std=c17',
		'cpp_std=c++23',
	]
)

hx_warning_args = [
	'-Wall',
	'-Wextra',
	'-pedantic-errors',
	'-Werror',
	'-Wfatal-errors',
	'-Wcast-qual',
	'-Wdisabled-optimization',
	'-Wshadow',
	'-Wundef',
	'-Wconversion',
	'-Wdate-time',
	'-Wmissing-declarations',
]

hx_include_directory = include_directories('include')
hx_c_args = hx_warning_args
hx_cpp_args = hx_warning_args + ['-fno-exceptions', '-fno-rtti']

libhatchet = static_library('hatchet',
	sources : [
		'src/hxconsole.cpp',
		'src/hxfile_c.cpp',
		'src/hxfile_posix.cpp',
		'src/hxmemory_manager.cpp',
		'src/hxprofiler.cpp',
		'src/hxradix_sort.cpp',
		'src/hxsettings.c',
		'src/hxtask_queue.cpp',
		'src/hxtest.cpp',
		'src/hxutility.c',
		'src/libhatchet.cpp',
	],
	include_directories : hx_include_directory,
	c_args : hx_c_args,
	cpp_args : hx_cpp_args,
)

libhatchet_dep = declare_dependency(
	include_directories: hx_include_directory,
	link_with: libhatchet,
)

if get_option('build_example')
	subdir('example')
endif
