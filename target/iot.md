# Adding IoT boards

## Procedures

- A board is only supported once its hardware is on hand and the test program
  has passed on it. There is no configuration for boards that have never been
  run.
- Every supported board is built whenever its SDK is installed. Building needs
  no hardware.
- Every supported board that is connected is tested. During development,
  only connect the boards to be tested.
- `test_target.sh --require-all` is for a machine with one of each board, such
  as one running continuous integration. It stops with an error at the first
  board that can't be built, isn't connected, doesn't respond or fails.
- Without `--require-all` a board that is connected still has to pass. One that
  is connected but doesn't respond is an error. One that isn't connected is
  built and passes with a warning.
- `test_target.sh --target <target>` builds and tests only that board.
  `--target meson` runs only the desktop tests.
- The report is TAP, the Test Anything Protocol, over the board's serial port.
  There is no custom protocol to maintain.

## Boards

| Target | Board | SDK | USB vendor ID | Flash script |
| --- | --- | --- | --- | --- |
| `pico2` | Raspberry Pi Pico 2 | Pico SDK | `2e8a` | `flash_pico.sh` |
| `esp32c6` | ESP32-C6 | ESP-IDF | `303a` | `flash_esp32.sh` |

## How it fits together

`HX_BOARDS_` in `test_target.sh` lists the boards as `<sdk>:<target>`, e.g.
`esp32:esp32c6`. For each one, in order, `test_target.sh`:

1. Checks that the SDK is installed with `has_sdk_<sdk>`.
2. Checks that the board is connected with
   `flash_<sdk>.sh --target <target> --connected`.
3. Builds it in `build/<target>` with `build_<sdk> <target>`.
4. Tests it with `flash_<sdk>.sh --target <target> --no-build`.

Each flash script handles one SDK and takes the same options:

- `--target <target>` selects the board. The default is the one board there is
  hardware for.
- `--connected` exits with 0 when the board's USB vendor ID is connected and 1
  when it is not. It doesn't talk to the board.
- `--no-build` flashes what `test_target.sh` built. Without it the script builds
  on its own, for working on one board.
- `--port` overrides the serial port it finds.
- It exits with an error when flashing fails, a test fails or the report
  doesn't finish.

`libhatchet_usb.sh` has the USB functions the flash scripts share: checking
that a vendor ID is connected, attaching it with usbip in a
`test_msvc_wslc.ps1 --clone-sdks` container, and finding its serial port.

`test_target.cpp` has a section for each SDK, under a macro the SDK defines,
with `target_start` and `target_finish`. `CMakeLists.txt` at the top level has
a section for each SDK. Meson isn't used for boards. `install_deb.sh --sdks`
installs each SDK, with the toolchains for the boards in the table and no
others.

## Adding a board for a supported SDK

Another ESP32 chip or another Pico board, for example.

1. Get the board.
2. If it needs another toolchain, add it to `install_deb.sh`. ESP-IDF's
   `install.sh` takes a list of chips.
3. Add `<sdk>:<target>` to `HX_BOARDS_`.
4. Make the flash script tell the boards apart. A board is found by its USB
   vendor ID alone, which is enough while each SDK has one board. A second
   Espressif chip also uses `303a` and a second Pico also uses `2e8a`. The chip
   type can tell them apart: `esptool chip-id` for an ESP32, or the product ID
   of a Pico in BOOTSEL. Add only what the boards on hand need.
5. Run `test_target.sh --target <target>` with the new board connected. Then
   run `test_target.sh --require-all` with every board connected.
6. Add the board to the table above and to `README.md`.

## Adding an SDK

1. Install the SDK in `install_deb.sh --sdks`, in `~` like the others.
2. Add `has_sdk_<sdk>` and `build_<sdk>` to `test_target.sh`.
3. If the SDK uses CMake, add a section ending with `return()` to the top level
   `CMakeLists.txt`.
4. If the board needs anything around the tests, add a section to
   `test_target.cpp`. For example, the Pico waits for USB serial and then
   reboots into BOOTSEL. Settings such as `HX_USE_THREADS` go in the build.
5. Copy a flash script to `flash_<sdk>.sh`, keep its options and
   source `libhatchet_usb.sh`. Checking the report with `grep`, as
   `flash_pico.sh` does, is enough unless the board has to be reset
   once the port is open, as the ESP32-C6 does.
6. Add `<sdk>:<target>` to `HX_BOARDS_`, then test and document it as above.

## Continuous integration bench

- Keep one of each board in the table plugged into a powered USB hub on the
  machine that runs `test_target.sh --require-all`.
- A board has to be flashable over USB without pressing buttons. The Pico 2
  reboots into BOOTSEL after each run and the ESP32-C6 resets over USB. A board
  that needs a button to be flashed has to be fixed first, for example with a
  dev board that has auto-reset.
- In a `test_msvc_wslc.ps1 --clone-sdks` container, share each board once with
  `usbipd bind`. The flash scripts attach the boards.
- A board that hangs fails the run. Unplugging it recovers it.
- The test containers that `test_msvc_wslc.ps1` runs can't reach USB devices,
  so `test_all.sh` builds the boards there and tests none.

## Later

Add these only once there is hardware that needs them:

- More than one board of the same type, found by USB serial number.
- Testing boards in parallel. Each board already has its own build directory.
- Building boards in parallel once the list is long. ESP-IDF builds are the
  slowest.

© 2017-2026 Adrian Johnston. This project is licensed under the terms of the MIT
license found in the `LICENSE.md` file.
