# SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
# SPDX-License-Identifier: MIT
# This file is licensed under the terms of the LICENSE.md file.

# A different PICO_SDK_PATH in the environment is not used.
HX_PICO_SDK_="$HOME/pico-sdk"

platform_sdk() {
	if [ -f "$HX_PICO_SDK_/pico_sdk_init.cmake" ] &&
			command -v arm-none-eabi-gcc > /dev/null 2>&1; then
		return 0
	fi
	echo "The Pico SDK requires arm-none-eabi-gcc and $HX_PICO_SDK_."
	return 1
}

platform_environment() {
	export PICO_SDK_PATH="$HX_PICO_SDK_"
}
