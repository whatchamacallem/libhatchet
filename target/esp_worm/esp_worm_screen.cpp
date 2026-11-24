// SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
// SPDX-License-Identifier: MIT
// This file is licensed under the MIT license found in the LICENSE.md file.

#include "esp_worm_screen.hpp"

#include <hx/hxutility.h>

#include <string.h>

#include "driver/gpio.h"
#include "driver/sdspi_host.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_panel_st7789.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// The screen's pins and settings come from Waveshare's demo.

#if !defined ESP_WORM_SCREEN_BACKLIGHT_PIN
// Active high.
#define ESP_WORM_SCREEN_BACKLIGHT_PIN 22
#endif

#if !defined ESP_WORM_SCREEN_DATA_COMMAND_PIN
#define ESP_WORM_SCREEN_DATA_COMMAND_PIN 15
#endif

#if !defined ESP_WORM_SCREEN_RESET_PIN
#define ESP_WORM_SCREEN_RESET_PIN 21
#endif

#if !defined ESP_WORM_SCREEN_SELECT_PIN
#define ESP_WORM_SCREEN_SELECT_PIN 14
#endif

char g_esp_worm_screen_text[c_esp_worm_screen_rows * c_esp_worm_screen_columns];
uint8_t g_esp_worm_screen_colors[c_esp_worm_screen_rows * c_esp_worm_screen_columns];

namespace {

constexpr int c_screen_height = 172;
constexpr int c_screen_width = 320;

// The cell of misc-fixed 8x13.
constexpr int c_font_height = 13;
constexpr int c_font_width = 8;
constexpr size_t c_font_map_size = 128u;
constexpr size_t c_font_glyph_count = 95u;
// Row 0 of each glyph is blank and not stored.
constexpr size_t c_font_glyph_size = 12u;

// Each pixel of the font is 2x2 pixels of the screen.
constexpr int c_cell_height = c_font_height * 2;
constexpr int c_cell_width = c_font_width * 2;
constexpr hxsize_t c_pair_row_size = c_screen_width / 2;

constexpr int c_text_height = static_cast<int>(c_esp_worm_screen_rows) * c_cell_height;
constexpr int c_text_width = static_cast<int>(c_esp_worm_screen_columns) * c_cell_width;
static_assert(c_text_height <= c_screen_height && c_text_width == c_screen_width);

// The esp_worm image is loaded into RAM.
const uint8_t c_screen_font[] = {
#embed ESP_WORM_FONT_FILE
};
static_assert(sizeof c_screen_font == c_font_map_size
	+ c_font_glyph_count * c_font_glyph_size);

// Red, green and blue at full intensity in RGB565, high byte first as
// LCD_RGB_DATA_ENDIAN_BIG has the panel take them. Each is in both halves, the
// two screen pixels of a font pixel.
constexpr uint32_t c_screen_red = 0x00f800f8u;
constexpr uint32_t c_screen_green = 0xe007e007u;
constexpr uint32_t c_screen_blue = 0x1f001f00u;

const uint32_t c_screen_palette[8] = {
	0u,
	c_screen_red,
	c_screen_green,
	c_screen_red | c_screen_green,
	c_screen_blue,
	c_screen_red | c_screen_blue,
	c_screen_green | c_screen_blue,
	c_screen_red | c_screen_green | c_screen_blue
};

// Leaves the frame function room for snprintf.
StackType_t s_screen_stack[4096];
StaticTask_t s_screen_task_storage;
esp_lcd_panel_handle_t s_screen_panel;
void (*s_screen_frame_function)(void);

// The text and colors last drawn. Zero, so the first update draws every row.
char s_screen_shown_text[c_esp_worm_screen_rows * c_esp_worm_screen_columns];
uint8_t s_screen_shown_colors[c_esp_worm_screen_rows * c_esp_worm_screen_columns];

// A row of text is filled into one strip while the panel takes the other. Each
// draw returns once the draw before it is sent, so the strip filled next is
// free. A strip is pairs of screen pixels.
uint32_t s_screen_strips[2][c_pair_row_size * c_cell_height];
size_t s_screen_strip_index;

// A row is drawn from the copy it is compared with, so a write during the draw
// shows by the next update.
esp_err_t s_screen_update(void) {
	s_screen_frame_function();
	for(hxsize_t row_index = 0; row_index < c_esp_worm_screen_rows; ++row_index) {
		const hxsize_t first_cell = row_index * c_esp_worm_screen_columns;
		const hxsize_t end_cell = first_cell + c_esp_worm_screen_columns;
		if(::memcmp(s_screen_shown_text + first_cell, g_esp_worm_screen_text + first_cell,
					c_esp_worm_screen_columns) == 0
				&& ::memcmp(s_screen_shown_colors + first_cell,
					g_esp_worm_screen_colors + first_cell, c_esp_worm_screen_columns) == 0) {
			continue;
		}
		::memcpy(s_screen_shown_text + first_cell, g_esp_worm_screen_text + first_cell,
			c_esp_worm_screen_columns);
		::memcpy(s_screen_shown_colors + first_cell, g_esp_worm_screen_colors + first_cell,
			c_esp_worm_screen_columns);

		uint32_t* hxrestrict pixel_pair = s_screen_strips[s_screen_strip_index];
		for(size_t pixel_row = 0u; pixel_row < static_cast<size_t>(c_font_height); ++pixel_row) {
			for(hxsize_t cell_index = first_cell; cell_index < end_cell; ++cell_index) {
				const size_t code = static_cast<unsigned char>(s_screen_shown_text[cell_index]);
				hxassertf(code < c_font_map_size, "esp_worm_screen code %u",
					static_cast<unsigned int>(code));
				const unsigned int colors = s_screen_shown_colors[cell_index];
				hxassertf((colors & 0x88u) == 0u, "esp_worm_screen colors %#x", colors);
				const uint32_t background = c_screen_palette[colors >> 4];
				const uint32_t color_difference = background ^ c_screen_palette[colors & 0x07u];
				const unsigned int glyph_row = pixel_row == 0u ? 0u : c_screen_font[c_font_map_size
					+ c_screen_font[code] * c_font_glyph_size + pixel_row - 1u];
#pragma GCC unroll 8
				for(int pixel_shift = c_font_width - 1; pixel_shift >= 0; --pixel_shift) {
					// All ones where the glyph has ink.
					const uint32_t ink_mask = 0u - ((glyph_row >> pixel_shift) & 1u);
					const uint32_t pair_color = background ^ (color_difference & ink_mask);
					pixel_pair[0] = pair_color;
					pixel_pair[c_pair_row_size] = pair_color;
					++pixel_pair;
				}
			}
			pixel_pair += c_pair_row_size;
		}

		const int top_pixel = static_cast<int>(row_index) * c_cell_height;
		const esp_err_t draw_result = esp_lcd_panel_draw_bitmap(s_screen_panel, 0, top_pixel,
			c_screen_width, top_pixel + c_cell_height, s_screen_strips[s_screen_strip_index]);
		if(draw_result != ESP_OK) {
			return draw_result;
		}
		s_screen_strip_index ^= 1u;
	}
	return ESP_OK;
}

// A task, because each draw waits for the bus while a card transfer holds it.
void s_screen_task(void*) {
	for(;;) {
		vTaskDelay(pdMS_TO_TICKS(50u));
		const esp_err_t update_result = s_screen_update();
		hxassertf(update_result == ESP_OK, "esp_lcd_panel_draw_bitmap %d", update_result);
		(void)update_result;
	}
}

} // namespace

bool esp_worm_screen_start(void (*frame_function)(void)) {
	s_screen_frame_function = frame_function;
	esp_lcd_panel_io_spi_config_t io_settings = {};
	io_settings.cs_gpio_num = static_cast<gpio_num_t>(ESP_WORM_SCREEN_SELECT_PIN);
	io_settings.dc_gpio_num = static_cast<gpio_num_t>(ESP_WORM_SCREEN_DATA_COMMAND_PIN);
	io_settings.spi_mode = 0;
	io_settings.pclk_hz = 12000000u;
	io_settings.trans_queue_depth = 10u;
	io_settings.lcd_cmd_bits = 8;
	io_settings.lcd_param_bits = 8;
	esp_lcd_panel_io_handle_t screen_io = hxnil;
	if(esp_lcd_new_panel_io_spi(SDSPI_DEFAULT_HOST, &io_settings, &screen_io) != ESP_OK) {
		return false;
	}

	esp_lcd_panel_dev_config_t panel_settings = {};
	panel_settings.rgb_ele_order = LCD_RGB_ELEMENT_ORDER_RGB;
	panel_settings.data_endian = LCD_RGB_DATA_ENDIAN_BIG;
	panel_settings.bits_per_pixel = 16u;
	panel_settings.reset_gpio_num = static_cast<gpio_num_t>(ESP_WORM_SCREEN_RESET_PIN);
	// The panel's 172 rows are the middle of the ST7789's 240.
	if(esp_lcd_new_panel_st7789(screen_io, &panel_settings, &s_screen_panel) != ESP_OK
			|| esp_lcd_panel_reset(s_screen_panel) != ESP_OK
			|| esp_lcd_panel_init(s_screen_panel) != ESP_OK
			|| esp_lcd_panel_invert_color(s_screen_panel, true) != ESP_OK
			|| esp_lcd_panel_swap_xy(s_screen_panel, true) != ESP_OK
			|| esp_lcd_panel_mirror(s_screen_panel, true, false) != ESP_OK
			|| esp_lcd_panel_set_gap(s_screen_panel, 0, 34) != ESP_OK) {
		return false;
	}

	::memset(g_esp_worm_screen_text, ' ', sizeof g_esp_worm_screen_text);
	::memset(g_esp_worm_screen_colors, 0x07, sizeof g_esp_worm_screen_colors);

	// The rows below the text are drawn black from the second strip while it is
	// still zero. The first update fills only the first strip before its draw
	// waits for this one. The display stays off from reset until disp_on_off, so
	// its power on contents never show.
	if(esp_lcd_panel_draw_bitmap(s_screen_panel, 0, c_text_height, c_screen_width,
				c_screen_height, s_screen_strips[1]) != ESP_OK
			|| s_screen_update() != ESP_OK) {
		return false;
	}

	const gpio_num_t backlight_pin = static_cast<gpio_num_t>(ESP_WORM_SCREEN_BACKLIGHT_PIN);
	gpio_config_t backlight_settings = {};
	backlight_settings.pin_bit_mask = 1ull << backlight_pin;
	backlight_settings.mode = GPIO_MODE_OUTPUT;
	if(esp_lcd_panel_disp_on_off(s_screen_panel, true) != ESP_OK
			|| gpio_config(&backlight_settings) != ESP_OK
			|| gpio_set_level(backlight_pin, 1u) != ESP_OK) {
		return false;
	}

	return xTaskCreateStatic(s_screen_task, "screen", sizeof s_screen_stack, hxnil,
		tskIDLE_PRIORITY + 1u, s_screen_stack, &s_screen_task_storage) != hxnull;
}
