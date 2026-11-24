# ESP32-C6-LCD-1.47

The ESP side of this project is being tested with a random board.
ESP32-C6-LCD-1.47. I'll work on using more standard boards after this.

## Board

- Waveshare ESP32-C6-LCD-1.47. The version without touch, with an RGB LED.
- ESP32-C6FH8 (QFN32) rev v0.2 with 8 MB of embedded flash, as esptool
  reports. Waveshare's board docs say C6FH4 with 4 MB. Its current comparison
  table says C6FH8 with 8 MB.
- USB is the chip's USB Serial/JTAG, `303a:1001`.

## TF card

The card is on SPI. The C6 has no SD host.

| Setting | Signal | GPIO |
| --- | --- | --- |
| `ESP_WORM_CARD_CLOCK_PIN` | SCLK | 7 |
| `ESP_WORM_CARD_DATA_IN_PIN` | MOSI | 6 |
| `ESP_WORM_CARD_DATA_OUT_PIN` | MISO | 5 |
| `ESP_WORM_CARD_SELECT_PIN` | CS | 4 |

- The LCD shares SCLK and MOSI. Its chip select is GPIO 14. MISO is the card's
  alone.
- The ESP32-C6-Touch-LCD-1.47 differs. Its card is on 1, 2, 3 and 4 for SCLK,
  MOSI, MISO and CS.

## LCD

The LCD is an ST7789 showing 172x320 on the card's SPI bus. These values come
from Waveshare's ESP32-C6-LCD-1.47-Test demo for ESP-IDF. The schematic has
not been checked. esp_worm swaps x and y and mirrors x to show 320x172, which
moves the gap to y.

| Setting | Signal | GPIO | Level |
| --- | --- | --- | --- |
| `ESP_WORM_SCREEN_SELECT_PIN` | CS | 14 | |
| `ESP_WORM_SCREEN_DATA_COMMAND_PIN` | DC | 15 | |
| `ESP_WORM_SCREEN_RESET_PIN` | RST | 21 | Active low |
| `ESP_WORM_SCREEN_BACKLIGHT_PIN` | Backlight | 22 | Active high |

| Setting | Value |
| --- | --- |
| x gap | 34 |
| Color inversion | On |
| Color order | RGB |
| SPI clock | 12 MHz |
| SPI mode | 0 |

## Sources

- https://docs.waveshare.com/ESP32-C6-LCD-1.47
- https://www.waveshare.com/wiki/ESP32-C6-LCD-1.47
- https://docs.waveshare.com/ESP32-C6-Touch-LCD-1.47
- https://github.com/trianglesis/esp32-c6_LVGL
- https://github.com/federicomusto/waveshare-ESP32-C6-LCD-1.47
