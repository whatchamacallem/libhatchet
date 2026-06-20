# SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
# SPDX-License-Identifier: MIT
# This file is licensed under the terms of the LICENSE.md file.

if get_option('build_module')
	# DHX_USE_MODULE=1 allows accessing macros from certain headers.
	_module_consumer_args = _hx_is_clang ? [] : ['-fmodules-ts']

	_example_obj = custom_target('hx_example',
		input : 'example.cpp',
		output : 'example.cpp.o',
		depends : _hx_is_clang ? [libhatchet_module[1]] : [libhatchet_module],
		command : _hx_cpp.cmd_array() + get_option('cpp_args') + hx_warning_args
			+ hx_module_include_args + hx_module_search_args + _module_consumer_args
			+ [
				'-DHX_USE_MODULE=1',
				'-c', '@INPUT@', '-o', '@OUTPUT@',
			],
	)

	executable('hxexample',
		sources : [libhatchet_module, _example_obj],
		link_args : hx_warning_args,
	)
else
	executable('hxexample',
		sources : ['example.cpp'],
		dependencies : libhatchet_dep,
		cpp_args : hx_warning_args,
	)
endif
