# libhatchet target tests

`test_target.cpp` is the main program for testing on embedded targets. The full
test suite in the `test` directory is too large for a dev board, so this is a
small suite checking what a port depends on. It needs nothing but libc and
reports with the Test Anything Protocol (TAP).

`test_target.sh` at the top level builds and runs it with Meson using the
machine files in the `meson` directory for `clang` and `gcc`, with and without
the libhatchet module. `meson test` consumes the report. It then builds it
with the Pico SDK and with ESP-IDF when they are installed. `install_deb.sh
--sdks` installs them.

## Raspberry Pi Pico

The Pico SDK is a CMake project, so `CMakeLists.txt` at the top level builds
`test_target.cpp` with it when `PICO_BOARD` is set. The scripts use the SDK and
picotool where `install_deb.sh --sdks` installs them, `~/pico-sdk` and
`~/picotool`, and ignore `PICO_SDK_PATH`. `test_target.sh` only builds
`build/pico/hxtest_target.uf2` and does not need a board. `PICO_BOARD` defaults
to `pico2`. The Pi Pico 1 is `pico`.

`libhatchet_flash_pico.sh` builds it, flashes it and then streams the report it
sends over USB serial. The board reboots into BOOTSEL afterwards so that the
next build can be flashed without the button.

1. Hold the **BOOTSEL** button on the Pico, then plug the Pico in and release the button.
2. If using a `test_msvc_wslc.ps1 --clone-sdks` container, share the Pico from
   Windows from an Administrator prompt with `usbipd bind --busid <busid>`.
   `usbipd list` shows the busid. usbipd-win sees the Pico in BOOTSEL and the
   Pico running the program as two devices, so each is shared once. The script
   attaches the Pico in the container each time it reboots.
3. Run: `./libhatchet_flash_pico.sh`

If the program crashes you may have to unplug, hold the BOOTSEL button and then
plug the Pico in again.

## ESP32

ESP-IDF projects are CMake projects too. `idf.py` passes `-DESP_PLATFORM=1`,
which selects the ESP-IDF section of `CMakeLists.txt` at the top level. That
section adds this directory as an ESP-IDF component, built by
`CMakeLists.txt` here with the settings in `sdkconfig.defaults`. The scripts
use ESP-IDF where `install_deb.sh --sdks` installs it, `~/esp-idf`, and ignore
`IDF_PATH`. `test_target.sh` only builds `build/esp32/hxtest_target.bin` and
does not need a board. `IDF_TARGET` defaults to `esp32`, the WT32-ETH01's chip.
`install_deb.sh --sdks` installs the toolchains for `esp32` and `esp32c6`.

`libhatchet_flash_esp32.sh` detects the chip, builds for it and flashes it. It
then resets the chip and prints the report from the serial port, without the
ESP-IDF boot log. It exits with an error when a test fails, the program crashes
or the report does not finish.

Chips with USB, such as the ESP32-C6, need no buttons. Do not hold BOOT, which
starts the chip in its bootloader instead of the program. The WT32-ETH01 has no
USB. It needs a 3.3 V USB to UART adapter on TX0, RX0 and GND, and IO0 held low
while it is reset to flash it. Release IO0 and reset it again when the script
says to.

1. If using WSL, share the board from Windows once from an Administrator prompt
   with `usbipd bind --busid <busid>`. `usbipd list` shows the busid.
2. Attach it, e.g. in a `test_msvc_wslc.ps1 --clone-sdks` container with
   `usbip attach -r windows -b <busid>`. It has to be attached again after a
   reset by its button or after being plugged in again.
3. Run: `./libhatchet_flash_esp32.sh`

© 2017-2026 Adrian Johnston. This project is licensed under the terms of the MIT
license found in the `LICENSE.md` file.
