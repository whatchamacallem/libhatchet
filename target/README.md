# libhatchet target tests

`test_target.cpp` is the main program for testing on embedded targets. The full
test suite in the `test` directory is too large for a dev board, so this is a
small suite checking what a port depends on. It needs nothing but libc and
reports with the Test Anything Protocol (TAP).

`test_target.sh` at the top level builds and runs it with Meson using the
machine files in the `meson` directory for `clang` and `gcc`, with and without
the libhatchet module. `meson test` consumes the report. It then builds it
with the Pico SDK when that is installed. `install_deb.sh --sdks` installs the
SDK.

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
2. If using WSL in PowerShell as Admin run `.\attach_pico.ps1` and keep the window open.
3. In WSL run: `./libhatchet_flash_pico.sh`

If the program crashes you may have to unplug, hold the BOOTSEL button and then
plug the Pico in again.

© 2017-2026 Adrian Johnston. This project is licensed under the terms of the MIT
license found in the `LICENSE.md` file.
