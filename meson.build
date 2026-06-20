# SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
# SPDX-License-Identifier: MIT
# This file is licensed under the terms of the LICENSE.md file.

project('libhatchet', ['c', 'cpp'],
	version : '1.0',
	license : 'MIT',
	meson_version : '>=1.1',
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

if get_option('build_module')
	# WARNING: This was only tested with GCC 15.2.0, Clang 20.1.8, Meson 1.7.0
	# and Ninja 1.12.1. Module support was a work in progress at that time.
	#
	# This required a custom_target to make it work. -std=c++23 must be passed
	# explicitly since project default_options do not apply to custom_target
	# commands. get_option('cpp_args') forwards project-level flags (e.g. -m32).
	# GCC writes gcm.cache/hx.gcm in the build directory as a side effect and
	# consumers find it via GCC's default module search path. Depfiles are
	# omitted because GCC's -MMD output includes module rules that ninja 1.12+
	# misinterprets as dyndep, producing an "inputs may not also have inputs"
	# error. -fmodules-ts enables GCC C++20 module support. Clang instead needs
	# -Wno-include-angled-in-module-purview to suppress warnings for system
	# headers re-included from the module purview and -fmodule-output=@OUTPUT1@
	# to produce hx.pcm alongside the object file. Consumers find it via
	# -fprebuilt-module-path pointing at the build root. Does not declare
	# DHX_USE_MODULE=1. That is for accessing macros later.

	_hx_cpp = meson.get_compiler('cpp')
	_hx_is_clang = _hx_cpp.get_id() == 'clang'

	hx_module_include_args = ['-I' + meson.project_source_root() / 'include']
	hx_module_search_args = _hx_is_clang ? ['-fprebuilt-module-path=' + meson.project_build_root()] : []

	# internal
	_hx_module_special_args = _hx_is_clang ? ['-Wno-include-angled-in-module-purview'] : ['-fmodules-ts']
	_hx_module_pcm_args = _hx_is_clang ? ['-fmodule-output=@OUTPUT1@'] : []
	_hx_module_outputs = _hx_is_clang ? ['hxmodule.cppm.o', 'hx.pcm'] : ['hxmodule.cppm.o']

	libhatchet_module = custom_target('hatchet',
		input : 'src/hxmodule.cppm',
		output : _hx_module_outputs,
		command : _hx_cpp.cmd_array() + get_option('cpp_args') + hx_warning_args
			+ hx_module_include_args + _hx_module_special_args + _hx_module_pcm_args
			+ [ '-c', '@INPUT@', '-o', '@OUTPUT0@', ],
	)
else
	libhatchet = static_library('hatchet',
		sources : run_command(
			'find', 'src', '(', '-name', '*.c', '-o', '-name', '*.cpp', ')',
			check : true,
		).stdout().split(),
		include_directories : hx_include_directory,
		c_args : hx_warning_args,
		cpp_args : hx_warning_args,
	)

	libhatchet_dep = declare_dependency(
		include_directories: hx_include_directory,
		link_with: libhatchet,
	)
endif

if get_option('build_example')
	subdir('example')
endif
