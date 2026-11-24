# SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
# SPDX-License-Identifier: MIT
# This file is licensed under the terms of the LICENSE.md file.
#
# esp_worm on esp_sdk. Its Wi-Fi network and tunnel keys may be set with -D.
# Otherwise the network comes from the session directory and test keys are made
# there for the test to play the host with.

set(ESP_WORM_DIRECTORY "${CMAKE_BINARY_DIR}/esp_worm")
cmake_path(SET ESP_WORM_SESSION_DIRECTORY NORMALIZE "${CMAKE_CURRENT_LIST_DIR}/../../tmp/esp_worm")
file(MAKE_DIRECTORY "${ESP_WORM_DIRECTORY}" "${ESP_WORM_SESSION_DIRECTORY}")

set(ESP_WORM_WIFI_FILE "${ESP_WORM_SESSION_DIRECTORY}/wifi.sh")

# Without the file esp_worm is built without a network and the board is not
# used. Creating the file changes the directory and CMake configures again.
if(NOT EXISTS "${ESP_WORM_WIFI_FILE}")
	set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "${ESP_WORM_SESSION_DIRECTORY}")
endif()
foreach(ESP_WORM_SETTING ESP_WORM_WIFI_PASSWORD ESP_WORM_WIFI_SSID)
	if(NOT DEFINED ${ESP_WORM_SETTING} AND EXISTS "${ESP_WORM_WIFI_FILE}")
		execute_process(COMMAND sh -c "set -u && . \"$1\" && printf %s \"$${ESP_WORM_SETTING}\""
			sh "${ESP_WORM_WIFI_FILE}" OUTPUT_VARIABLE ${ESP_WORM_SETTING} ERROR_QUIET)
		if(${ESP_WORM_SETTING} STREQUAL "")
			message(FATAL_ERROR "${ESP_WORM_WIFI_FILE} has to declare ESP_WORM_WIFI_SSID and"
				" ESP_WORM_WIFI_PASSWORD, the Wi-Fi network esp_worm joins.")
		endif()
		set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "${ESP_WORM_WIFI_FILE}")
	endif()
endforeach()

if(NOT DEFINED ESP_WORM_TUNNEL_PEER_KEY OR NOT DEFINED ESP_WORM_TUNNEL_PRIVATE_KEY)
	find_program(ESP_WORM_WG wg REQUIRED)
	foreach(ESP_WORM_KEY_OWNER host device)
		set(ESP_WORM_KEY_FILE "${ESP_WORM_SESSION_DIRECTORY}/${ESP_WORM_KEY_OWNER}")
		if(NOT EXISTS "${ESP_WORM_KEY_FILE}.key")
			execute_process(COMMAND "${ESP_WORM_WG}" genkey OUTPUT_FILE "${ESP_WORM_KEY_FILE}.key"
				COMMAND_ERROR_IS_FATAL ANY)
			file(CHMOD "${ESP_WORM_KEY_FILE}.key" PERMISSIONS OWNER_READ OWNER_WRITE)
			execute_process(COMMAND "${ESP_WORM_WG}" pubkey INPUT_FILE "${ESP_WORM_KEY_FILE}.key"
				OUTPUT_FILE "${ESP_WORM_KEY_FILE}.pub" COMMAND_ERROR_IS_FATAL ANY)
		endif()
	endforeach()
	file(STRINGS "${ESP_WORM_SESSION_DIRECTORY}/host.pub" ESP_WORM_TUNNEL_PEER_KEY)
	file(STRINGS "${ESP_WORM_SESSION_DIRECTORY}/device.key" ESP_WORM_TUNNEL_PRIVATE_KEY)
endif()

# wireguard-lwip has no CMakeLists.txt.
set(FETCHCONTENT_BASE_DIR "${CMAKE_CURRENT_LIST_DIR}/../../cache/esp_worm")
include(FetchContent)
FetchContent_Declare(wireguard_lwip
	GIT_REPOSITORY https://github.com/smartalock/wireguard-lwip.git
	GIT_TAG cfba91666590e86bd67842c71cedf9a423be823a)
FetchContent_MakeAvailable(wireguard_lwip)

set(ESP_WORM_FONT_FILE "${ESP_WORM_DIRECTORY}/hxfont.raw")
add_custom_command(OUTPUT "${ESP_WORM_FONT_FILE}"
	COMMAND "${ESP_SDK_PYTHON}" -I "${CMAKE_CURRENT_LIST_DIR}/hxfont.py" "${ESP_WORM_FONT_FILE}"
	DEPENDS "${CMAKE_CURRENT_LIST_DIR}/hxfont.py"
	VERBATIM)

file(GLOB ESP_WORM_SOURCES "${CMAKE_CURRENT_LIST_DIR}/*.cpp")
file(GLOB ESP_WORM_WIREGUARD_SOURCES "${wireguard_lwip_SOURCE_DIR}/src/*.c"
	"${wireguard_lwip_SOURCE_DIR}/src/crypto/refc/*.c")
esp_sdk_client(esp_worm MEMORY_REGION image
	SOURCE_FILES ${ESP_WORM_SOURCES}
		"${CMAKE_CURRENT_LIST_DIR}/../hxboot/esp_sdk/hxboot_port_freertos.cpp"
		${ESP_SDK_HATCHET_SOURCES} ${ESP_WORM_WIREGUARD_SOURCES}
		# ESP-IDF's lwip component leaves out lwIP's TFTP server.
		"$ENV{IDF_PATH}/components/lwip/lwip/src/apps/tftp/tftp.c"
		"${ESP_WORM_FONT_FILE}"
	INCLUDE_DIRECTORIES "${CMAKE_CURRENT_LIST_DIR}" "${CMAKE_CURRENT_LIST_DIR}/../../tap_protocol"
		"${wireguard_lwip_SOURCE_DIR}/src"
	COMPILE_DEFINITIONS
		"ESP_WORM_FONT_FILE=\"${ESP_WORM_FONT_FILE}\""
		"ESP_WORM_TUNNEL_PEER_KEY=\"${ESP_WORM_TUNNEL_PEER_KEY}\""
		"ESP_WORM_TUNNEL_PRIVATE_KEY=\"${ESP_WORM_TUNNEL_PRIVATE_KEY}\""
		"ESP_WORM_WIFI_PASSWORD=\"${ESP_WORM_WIFI_PASSWORD}\""
		"ESP_WORM_WIFI_SSID=\"${ESP_WORM_WIFI_SSID}\""
		HX_HARDENING_MODE=HX_HARDENING_MODE_NONE
		# libhatchet formats a log line on the stack, which may be lwIP's.
		HX_MAX_LINE=256
		HX_USE_CONSOLE=0
		HX_USE_FILE_IO=2
		HX_USE_LOGGING=1
		HX_USE_PROFILER=0
		HX_USE_SLAB_ALLOCATOR=0
		HX_USE_STD_LIB=0
		# hxboot reports test 1.
		HX_USE_TEST_FRAMEWORK=1
		HX_USE_THREADS=0
	REQUIRED_COMPONENTS esp_driver_gpio esp_driver_sdspi esp_driver_spi
		esp_driver_usb_serial_jtag esp_event esp_hw_support esp_lcd esp_netif esp_wifi fatfs
		freertos lwip)
set_source_files_properties("${wireguard_lwip_SOURCE_DIR}/src/wireguard.c"
	PROPERTIES COMPILE_OPTIONS "-Wno-unterminated-string-initialization")
set_source_files_properties("${wireguard_lwip_SOURCE_DIR}/src/crypto/refc/x25519.c"
	PROPERTIES COMPILE_OPTIONS "-Wno-stringop-overread")

esp_sdk_program(esp_worm CLIENTS esp_worm
	COMMAND bash "${CMAKE_CURRENT_LIST_DIR}/test_esp_worm.sh" "${ESP_WORM_SESSION_DIRECTORY}"
		"${ESP_WORM_DIRECTORY}" ${ESP_SDK_RUN_COMMAND} --serve esp_worm.hxb)

# hxboot's test launches the gold program.
if("${ESP_WORM_WIFI_SSID}" STREQUAL "" OR "${ESP_WORM_WIFI_PASSWORD}" STREQUAL "")
	set(ESP_WORM_TESTS esp_worm)
	if(TEST hxboot AND "${ESP_SDK_GOLD_PROGRAM}" STREQUAL "esp_worm")
		list(APPEND ESP_WORM_TESTS hxboot)
	endif()
	set_tests_properties(${ESP_WORM_TESTS} PROPERTIES ENVIRONMENT
		"ESP_SDK_UNUSABLE=${ESP_WORM_WIFI_FILE} is missing, so esp_worm has no Wi-Fi network.")
endif()
