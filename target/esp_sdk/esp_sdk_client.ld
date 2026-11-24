/* SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
 * SPDX-License-Identifier: MIT
 * This file is licensed under the terms of the LICENSE.md file.
 *
 * The link of a client of esp_sdk into the region aliased as esp_sdk_client.
 * bss is last, so esp_sdk zeros the image's tail. No __global_pointer$ is
 * defined, so gp keeps esp_sdk's.
 */

ENTRY(esp_sdk_client_start)

/* Without PHDRS, ld may load the ELF headers in front of .text when the region
 * starts past them within a page, moving the first load address below it. */
PHDRS
{
	esp_sdk_client_load PT_LOAD;
}

SECTIONS
{
	.text : {
		*(.text* .iram1.*)
	} > esp_sdk_client :esp_sdk_client_load

	.rodata : {
		*(.rodata* .srodata* .sdata2* .got*)
	} > esp_sdk_client

	/* Ordered as ESP-IDF's startup orders its own. */
	.init_array : {
		esp_sdk_client_init_array_begin = .;
		KEEP(*(.preinit_array))
		KEEP(*(SORT_BY_INIT_PRIORITY(.init_array.*)))
		KEEP(*(.init_array))
		esp_sdk_client_init_array_end = .;
	} > esp_sdk_client

	.data : {
		*(.data* .sdata* .dram1.*)
	} > esp_sdk_client

	.bss : {
		*(.bss* .sbss* .scommon COMMON)
	} > esp_sdk_client

	/* Only esp_sdk's startup sets up thread-local storage and runs
	 * ESP_SYSTEM_INIT_FN and SOC_RESERVE_MEMORY_REGION entries. */
	.esp_sdk_client_unsupported : {
		KEEP(*(.esp_sys_init_fn* .reserved_memory_address))
		*(.tdata* .tbss*)
	} > esp_sdk_client
	ASSERT(SIZEOF(.esp_sdk_client_unsupported) == 0,
		"esp_sdk_client.ld: No TLS, ESP_SYSTEM_INIT_FN or SOC_RESERVE_MEMORY_REGION in a client.")

	/DISCARD/ : {
		*(.eh_frame*)
	}

	.riscv.attributes 0 : {
		*(.riscv.attributes)
	}
}
