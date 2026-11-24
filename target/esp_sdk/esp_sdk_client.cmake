# SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
# SPDX-License-Identifier: MIT
# This file is licensed under the terms of the LICENSE.md file.
#
# The functions of the esp_sdk platform. A client of esp_sdk is a RAM image
# without startup code. It is linked against the addresses of esp_sdk.elf, the
# way ESP-IDF links against the mask ROM. hxboot is the client in the hxboot
# slot and launches the others. Included after project(esp_sdk).

idf_build_get_property(ESP_SDK_PYTHON PYTHON)
set(ESP_SDK_EXPORTS_SCRIPT "${CMAKE_CURRENT_LIST_DIR}/esp_sdk_exports.py")
set(HX_BOOT_IMAGE_SCRIPT "${CMAKE_CURRENT_LIST_DIR}/../hxboot/server/hxboot_image.py")

# Each client compiles libhatchet with its own settings.
file(GLOB ESP_SDK_HATCHET_SOURCES "${CMAKE_CURRENT_LIST_DIR}/../../src/*.cpp")

# The MEMORY regions of the clients and a PROVIDE for each symbol of esp_sdk.
add_custom_command(OUTPUT "${CMAKE_BINARY_DIR}/esp_sdk.exports.ld"
	COMMAND "${ESP_SDK_PYTHON}" -I "${ESP_SDK_EXPORTS_SCRIPT}" exports
		--boot-size ${ESP_SDK_BOOT_SIZE} "$<TARGET_FILE:esp_sdk.elf>"
		"${CMAKE_BINARY_DIR}/esp_sdk.exports.ld"
	DEPENDS esp_sdk.elf "${ESP_SDK_EXPORTS_SCRIPT}"
	VERBATIM)
add_custom_target(esp_sdk_exports DEPENDS "${CMAKE_BINARY_DIR}/esp_sdk.exports.ld")

partition_table_get_partition_info(ESP_SDK_BOOT_OFFSET "--partition-name hxboot" "offset")
partition_table_get_partition_info(ESP_SDK_GOLD_OFFSET "--partition-name gold" "offset")

# The start of every test command. A test adds to it.
set(ESP_SDK_RUN_COMMAND bash "${CMAKE_CURRENT_LIST_DIR}/run_esp_sdk.sh"
	--python "${ESP_SDK_PYTHON}" --chip ${IDF_TARGET} --vendor ${HX_BOARD_USB_VENDOR}
	--build "${CMAKE_BINARY_DIR}" --flash ${ESP_SDK_BOOT_OFFSET} hxboot.hxb)

# esp_sdk_client(<name> MEMORY_REGION <image|boot> SOURCE_FILES <file>...
#	[INCLUDE_DIRECTORIES <directory>...] [COMPILE_DEFINITIONS <definition>...]
#	REQUIRED_COMPONENTS <component>...)
#
# Builds <name>.elf to run from the region esp_sdk_<region>, then checks it
# and builds <name>.hxb with the target <name>_image. The components only add
# their include directories and definitions.
function(esp_sdk_client ESP_SDK_CLIENT_NAME)
	cmake_parse_arguments(PARSE_ARGV 1 ESP_SDK_CLIENT "" "MEMORY_REGION"
		"SOURCE_FILES;INCLUDE_DIRECTORIES;COMPILE_DEFINITIONS;REQUIRED_COMPONENTS")

	# A component gets the build properties from directory scoped commands and
	# the components every component requires.
	foreach(ESP_SDK_BUILD_PROPERTY COMPILE_OPTIONS C_COMPILE_OPTIONS CXX_COMPILE_OPTIONS
			ASM_COMPILE_OPTIONS COMPILE_DEFINITIONS INCLUDE_DIRECTORIES)
		idf_build_get_property(ESP_SDK_IDF_${ESP_SDK_BUILD_PROPERTY} ${ESP_SDK_BUILD_PROPERTY}
			GENERATOR_EXPRESSION)
	endforeach()
	idf_build_get_property(ESP_SDK_CLIENT_COMPONENTS __COMPONENT_REQUIRES_COMMON)
	list(APPEND ESP_SDK_CLIENT_COMPONENTS ${ESP_SDK_CLIENT_REQUIRED_COMPONENTS})
	list(TRANSFORM ESP_SDK_CLIENT_COMPONENTS REPLACE "(.+)" "$<COMPILE_ONLY:idf::\\1>")

	# PUBLIC passes this on to the executable.
	add_library(${ESP_SDK_CLIENT_NAME}_library STATIC ${ESP_SDK_CLIENT_SOURCE_FILES})
	target_include_directories(${ESP_SDK_CLIENT_NAME}_library PUBLIC
		"${ESP_SDK_IDF_INCLUDE_DIRECTORIES}"
		"${CMAKE_CURRENT_FUNCTION_LIST_DIR}/main"
		"${CMAKE_CURRENT_FUNCTION_LIST_DIR}/../hxboot/loader"
		"${CMAKE_CURRENT_FUNCTION_LIST_DIR}/../../include"
		${ESP_SDK_CLIENT_INCLUDE_DIRECTORIES})
	target_compile_definitions(${ESP_SDK_CLIENT_NAME}_library PUBLIC
		"${ESP_SDK_IDF_COMPILE_DEFINITIONS}" ${ESP_SDK_CLIENT_COMPILE_DEFINITIONS})
	target_compile_options(${ESP_SDK_CLIENT_NAME}_library PUBLIC "${ESP_SDK_IDF_COMPILE_OPTIONS}"
		"$<$<COMPILE_LANGUAGE:C>:${ESP_SDK_IDF_C_COMPILE_OPTIONS}>"
		"$<$<COMPILE_LANGUAGE:CXX>:${ESP_SDK_IDF_CXX_COMPILE_OPTIONS}>"
		"$<$<COMPILE_LANGUAGE:ASM>:${ESP_SDK_IDF_ASM_COMPILE_OPTIONS}>"
		"$<$<COMPILE_LANGUAGE:CXX>:-fno-exceptions;-fno-rtti;-nostdinc++>")
	target_link_libraries(${ESP_SDK_CLIENT_NAME}_library PUBLIC ${ESP_SDK_CLIENT_COMPONENTS})

	file(GENERATE OUTPUT "${CMAKE_BINARY_DIR}/${ESP_SDK_CLIENT_NAME}.region.ld"
		CONTENT "REGION_ALIAS(\"esp_sdk_client\", esp_sdk_${ESP_SDK_CLIENT_MEMORY_REGION});\n")
	set(ESP_SDK_CLIENT_SCRIPTS "${CMAKE_BINARY_DIR}/esp_sdk.exports.ld"
		"${CMAKE_BINARY_DIR}/${ESP_SDK_CLIENT_NAME}.region.ld"
		"${CMAKE_CURRENT_FUNCTION_LIST_DIR}/esp_sdk_client.ld")
	add_executable(${ESP_SDK_CLIENT_NAME}.elf
		"${CMAKE_CURRENT_FUNCTION_LIST_DIR}/esp_sdk_client.cpp")
	add_dependencies(${ESP_SDK_CLIENT_NAME}.elf esp_sdk_exports)
	set_property(TARGET ${ESP_SDK_CLIENT_NAME}.elf APPEND PROPERTY LINK_DEPENDS
		${ESP_SDK_CLIENT_SCRIPTS})
	# The global ldflags have -nostartfiles. A -T that the driver sees stops
	# picolibc.specs from adding picolibc.ld. gp keeps esp_sdk's
	# __global_pointer$, which is not exported. The check reports unresolved
	# symbols.
	list(TRANSFORM ESP_SDK_CLIENT_SCRIPTS PREPEND "-T")
	target_link_options(${ESP_SDK_CLIENT_NAME}.elf PRIVATE -nostdlib ${ESP_SDK_CLIENT_SCRIPTS}
		-Wl,--gc-sections -Wl,--no-relax-gp -Wl,--no-warn-rwx-segments
		"-Wl,-Map=${CMAKE_BINARY_DIR}/${ESP_SDK_CLIENT_NAME}.map"
		-Wl,--unresolved-symbols=ignore-all)
	# The whole archive, so every .init_array is linked.
	target_link_libraries(${ESP_SDK_CLIENT_NAME}.elf PRIVATE
		-Wl,--whole-archive ${ESP_SDK_CLIENT_NAME}_library -Wl,--no-whole-archive -lgcc)

	# esp_sdk.bin is made with .bin_timestamp, its only declared output.
	add_custom_command(OUTPUT "${CMAKE_BINARY_DIR}/${ESP_SDK_CLIENT_NAME}.hxb"
		COMMAND "${ESP_SDK_PYTHON}" -I "${ESP_SDK_EXPORTS_SCRIPT}" check
			"$<TARGET_FILE:esp_sdk.elf>" "$<TARGET_FILE:${ESP_SDK_CLIENT_NAME}.elf>"
		COMMAND "${ESP_SDK_PYTHON}" -I "${HX_BOOT_IMAGE_SCRIPT}" --target ${IDF_TARGET}
			--esp-sdk "${CMAKE_BINARY_DIR}/esp_sdk.bin" "$<TARGET_FILE:${ESP_SDK_CLIENT_NAME}.elf>"
			"${CMAKE_BINARY_DIR}/${ESP_SDK_CLIENT_NAME}.hxb"
		DEPENDS ${ESP_SDK_CLIENT_NAME}.elf esp_sdk.elf "${CMAKE_BINARY_DIR}/.bin_timestamp"
			gen_esp_sdk_binary "${ESP_SDK_EXPORTS_SCRIPT}" "${HX_BOOT_IMAGE_SCRIPT}"
		VERBATIM)
	add_custom_target(${ESP_SDK_CLIENT_NAME}_image ALL
		DEPENDS "${CMAKE_BINARY_DIR}/${ESP_SDK_CLIENT_NAME}.hxb")
endfunction()

# esp_sdk_program(<name> CLIENTS <client>... COMMAND <command>...)
#
# Defines the target <name>, which builds what ESP_SDK_RUN_COMMAND flashes and
# the clients, and the test <name>, which runs the command.
function(esp_sdk_program ESP_SDK_PROGRAM_NAME)
	cmake_parse_arguments(PARSE_ARGV 1 ESP_SDK_PROGRAM "" "" "CLIENTS;COMMAND")
	list(TRANSFORM ESP_SDK_PROGRAM_CLIENTS APPEND _image)
	add_custom_target(${ESP_SDK_PROGRAM_NAME})
	add_dependencies(${ESP_SDK_PROGRAM_NAME} app bootloader partition_table_bin hxboot_image
		${ESP_SDK_PROGRAM_CLIENTS})
	add_test(NAME ${ESP_SDK_PROGRAM_NAME} COMMAND ${ESP_SDK_PROGRAM_COMMAND})
	set_tests_properties(${ESP_SDK_PROGRAM_NAME} PROPERTIES SKIP_RETURN_CODE 77 RUN_SERIAL TRUE)
endfunction()
