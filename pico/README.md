# libhatchet Pico test

Builds and runs the libhatchet test suite on a Raspberry Pi Pico 2, streaming
results over USB serial. It should work on the Pi Pico 1 although a couple of
the tests may need to be modified to use less memory.

You may need to customize the environment variables at the top of
`./libhatchet_flash_pico.sh` before this works for you.

1. Hold the **BOOTSEL** button on the Pico, then plug the Pico in and release the button.
2. If using WSL in PowerShell as Admin run `.\attach_pico.ps1` and keep the window open.
3. In WSL run: `./libhatchet_flash_pico.sh`

After each run the firmware should reboot the Pico into BOOTSEL again
automatically, so the script can flash again without the button.

If the program crashes you may have to unplug, hold the BOOTSEL button and then
plug the Pico 2 in again.
