# SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
# SPDX-License-Identifier: MIT
# This file is licensed under the terms of the LICENSE.md file.
#
# The Waveshare ESP32-C6-LCD-1.47. Included before ESP-IDF.

set(IDF_TARGET esp32c6)

# The USB Serial/JTAG of the chip.
set(HX_BOARD_USB_VENDOR 303a)

set(HX_BOARD_PROGRAMS hxboot esp_worm)

# The image in the gold slot, which hxboot launches when none is served.
set(ESP_SDK_GOLD_PROGRAM esp_worm)
