# SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
# SPDX-License-Identifier: MIT
# This file is licensed under the terms of the LICENSE.md file.

# A different IDF_PATH in the environment is not used.
HX_ESP_IDF_="$HOME/esp-idf"

platform_sdk() {
	if [ -f "$HX_ESP_IDF_/export.sh" ]; then
		return 0
	fi
	echo "ESP-IDF requires $HX_ESP_IDF_."
	return 1
}

# export.sh requires IDF_PATH when sourced by sh and does not support set -u.
platform_environment() {
	export IDF_PATH="$HX_ESP_IDF_"
	set +u
	. "$HX_ESP_IDF_/export.sh" || return 1
	set -u
}
