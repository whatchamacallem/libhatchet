# SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
# SPDX-License-Identifier: MIT
# This file is licensed under the terms of the LICENSE.md file.

if get_option('build_module')
	# DHX_USE_MODULE=1 allows accessing macros from certain headers.
	_module_consumer_args = _hxis_clang ? [] : ['-fmodules-ts']

	_example_obj = custom_target('hxexample',
		input : 'example.cpp',
		output : 'example.cpp.o',
		depends : _hxis_clang ? [libhatchet_module[1]] : [libhatchet_module],
		command : _hxcpp.cmd_array() + get_option('cpp_args') + hxwarning_args
			+ hxmodule_include_args + hxmodule_search_args + _module_consumer_args
			+ [
				'-DHX_USE_MODULE=1',
				'-c', '@INPUT@', '-o', '@OUTPUT@',
			],
	)

	executable('hxexample',
		sources : [libhatchet_module, _example_obj],
		link_args : hxwarning_args + get_option('cpp_link_args'),
	)
else
	executable('hxexample',
		sources : ['example.cpp'],
		dependencies : libhatchet_dep,
		cpp_args : hxwarning_args,
	)
endif
