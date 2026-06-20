# libhatchet Pico test

Builds and runs the libhatchet test suite on a Raspberry Pi Pico 2, streaming
results over USB serial.

## WSL

1. If using WSL in PowerShell as Admin run `.\attach_pico.ps1` and keep the window open.
2. Hold the **BOOTSEL** button on the Pico, then plug the Pico in and release the button.
3. In WSL run: `./libhatchet_flash_pico.sh`

After each run the firmware should reboot the Pico into BOOTSEL again
automatically, so the script can flash again without the button.
