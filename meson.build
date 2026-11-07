# SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
# SPDX-License-Identifier: MIT
# This file is licensed under the terms of the LICENSE.md file.

project('libhatchet', ['c', 'cpp'],
	version : '1.0',
	license : 'MIT',
	meson_version : '>=1.3',
	default_options : [
		'c_std=c17',
		'cpp_std=c++23',
	]
)

hxwarning_args = [
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
	'-Wno-c2y-extensions',
	'-Wno-maybe-uninitialized',
	'-Wno-unknown-warning-option'
]

hxinclude_directory = include_directories('include')
_hxcpp = meson.get_compiler('cpp')

# -m32 is auto enabled when available.
_hxm32 = get_option('m32').require(
	meson.get_compiler('c').links('int main(void) { return 0; }', args : '-m32', name : '-m32'),
	error_message : '-m32 requires 32-bit libc.').allowed()
hxm32_arg = _hxm32 ? ['-m32'] : []
add_project_arguments(hxm32_arg, language : ['c', 'cpp'])
add_project_link_arguments(hxm32_arg, language : ['c', 'cpp'])

# The module is auto enabled when supported. Apple clang uses its own version
# numbers and does not support named modules.
_hxcpp_version = _hxcpp.version()
_hxcpp_modules = (
	(_hxcpp.get_id() == 'gcc' and _hxcpp_version.version_compare('>=15'))
	or (_hxcpp.get_id() == 'clang' and _hxcpp_version.version_compare('>=18')
		and _hxcpp.get_define('__apple_build_version__') == ''))

hxbuild_module = get_option('build_module').require(_hxcpp_modules,
	error_message : 'Named modules require GCC 15 or LLVM Clang 18.').allowed()

if hxbuild_module
	# WARNING: The earliest versions this was tested with were GCC 15.2.0, Clang
	# 18.1.3, Meson 1.3.2 and Ninja 1.11.1. Module support was still a work in
	# progress at that time.
	#
	# This required a custom_target to make it work. -std=c++2x must be passed
	# explicitly since project default_options do not apply to custom_target
	# commands. get_option('cpp_args') and hxm32_arg forward project-level flags
	# for the same reason.
	# GCC writes gcm.cache/hx.gcm in the build directory as a side effect and
	# consumers find it via GCC's default module search path. Depfiles are
	# omitted because GCC's -MMD output includes module rules that ninja 1.12+
	# misinterprets as dyndep, producing an "inputs may not also have inputs"
	# error. -fmodules-ts enables GCC C++23 module support. Clang instead needs
	# -Wno-include-angled-in-module-purview to suppress warnings for system
	# headers re-included from the module purview and -fmodule-output=@OUTPUT1@
	# to produce hx.pcm alongside the object file. Consumers find it via
	# -fprebuilt-module-path pointing at the build root. Does not declare
	# HX_USE_MACROS_WITH_MODULE=1. That is for accessing macros alongside the
	# module.

	_hxis_clang = _hxcpp.get_id() == 'clang'

	hxmodule_include_args = ['-I' + meson.project_source_root() / 'include']
	hxmodule_search_args = _hxis_clang ? ['-fprebuilt-module-path=' + meson.project_build_root()] : []

	# Consumers of the module compile with these.
	hxmodule_consumer_args = hxmodule_include_args + hxmodule_search_args + (_hxis_clang ? [] : ['-fmodules-ts'])

	# internal
	_hxmodule_special_args = _hxis_clang ? ['-Wno-include-angled-in-module-purview', '-fdiagnostics-absolute-paths'] : ['-fmodules-ts']
	_hxmodule_pcm_args = _hxis_clang ? ['-fmodule-output=@OUTPUT1@'] : []
	_hxmodule_outputs = _hxis_clang ? ['hxmodule.cppm.o', 'hx.pcm'] : ['hxmodule.cppm.o']

	libhatchet_module = custom_target('hatchet',
		input : 'src/hxmodule.cppm',
		output : _hxmodule_outputs,
		build_by_default : true,
		command : _hxcpp.cmd_array() + get_option('cpp_args') + hxm32_arg + hxwarning_args
			+ hxmodule_include_args + _hxmodule_special_args + _hxmodule_pcm_args
			+ [ '-c', '@INPUT@', '-o', '@OUTPUT0@', ],
	)
else
	libhatchet = static_library('hatchet',
		sources : run_command(
			'find', 'src', '(', '-name', '*.c', '-o', '-name', '*.cpp', ')',
			check : true,
		).stdout().split(),
		include_directories : hxinclude_directory,
		c_args : hxwarning_args,
		cpp_args : hxwarning_args,
	)

	libhatchet_dep = declare_dependency(
		include_directories: hxinclude_directory,
		link_with: libhatchet,
	)
endif

# The target test program is built when libhatchet is the main project. It
# reports with the Test Anything Protocol and "meson test" consumes the report.
# A cross file with an exe_wrapper runs it too. Without one it is only built.
if not meson.is_subproject()
	if hxbuild_module
		# -DHX_USE_MACROS_WITH_MODULE=1 allows accessing macros from certain headers.
		_hxtarget_obj = custom_target('hxtest_target_obj',
			input : 'target/test_target.cpp',
			output : 'test_target.cpp.o',
			depends : [libhatchet_module],
			command : _hxcpp.cmd_array() + get_option('cpp_args') + hxm32_arg + hxwarning_args
				+ hxmodule_consumer_args
				+ [ '-DHX_USE_MACROS_WITH_MODULE=1', '-c', '@INPUT@', '-o', '@OUTPUT@', ],
		)

		hxtest_target = executable('hxtest_target',
			sources : [libhatchet_module, _hxtarget_obj],
			link_args : hxwarning_args + get_option('cpp_link_args'),
			link_language : 'cpp',
		)
	else
		hxtest_target = executable('hxtest_target',
			sources : ['target/test_target.cpp'],
			dependencies : libhatchet_dep,
			cpp_args : hxwarning_args,
		)
	endif

	if meson.can_run_host_binaries()
		test('hxtest_target', hxtest_target, protocol : 'tap')
	endif
endif
