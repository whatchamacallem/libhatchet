# libhatchet target tests

`test_target.cpp` is the main program for testing on embedded targets. The full
test suite in the `test` directory is too large for a dev board, so this is a
small suite checking what a port depends on. It needs nothing but libc and
reports with the Test Anything Protocol (TAP).

`test_target.sh` at the top level builds and runs it with Meson using the
machine files in the `meson` directory for `clang` and `gcc`, with and without
the `hx` module. `meson test` consumes the report. It then builds it with the
Pico SDK when that is installed. `install_deb.sh --sdks` installs the SDK.

## Raspberry Pi Pico

The Pico SDK is a CMake project, so `CMakeLists.txt` at the top level builds
`test_target.cpp` with it when `PICO_BOARD` is set. The SDK is used from
`PICO_SDK_PATH` or from `~/pico-sdk` along with picotool. `test_target.sh` only
builds `build/hxtest_target.uf2` and does not need a board. `PICO_BOARD`
defaults to `pico2`. The Pi Pico 1 is `pico`.

`libhatchet_flash_pico.sh` builds it, flashes it and then streams the report it
sends over USB serial. The board reboots into BOOTSEL afterwards so that the
next build can be flashed without the button.

1. Hold the **BOOTSEL** button on the Pico, then plug the Pico in and release the button.
2. If using WSL in PowerShell as Admin run `.\attach_pico.ps1` and keep the window open.
3. In WSL run: `./libhatchet_flash_pico.sh`

If the program crashes you may have to unplug, hold the BOOTSEL button and then
plug the Pico in again.

## Adding a Platform

Build `test_target.cpp` with `src/*.cpp`. Use `-DHX_USE_THREADS=0`,
`-DHX_USE_FILE_IO=0` and the other settings in `hxsettings.h` to avoid what the
platform does not support.

- A toolchain Meson drives is a machine file in the `meson` directory. A cross
  file with an `exe_wrapper` also runs the tests. Without one Meson only builds
  them.
- An SDK with its own build system, like the Pico SDK, builds it from there.
- Anything the platform has to do around the tests, such as starting stdio,
  goes in `main` under a macro the SDK defines, as `PICO_ON_DEVICE` does.

© 2017-2026 Adrian Johnston. This project is licensed under the terms of the MIT
license found in the `LICENSE.md` file.
