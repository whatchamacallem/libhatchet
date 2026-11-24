// SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
// SPDX-License-Identifier: MIT
// This file is licensed under the MIT license found in the LICENSE.md file.

#include "esp_worm_file_server.hpp"
#include "esp_worm_tunnel.hpp"

#include <hx/hxfile.hpp>
#include <hx/hxutility.h>

#include "lwip/apps/tftp_common.h"

extern "C" {
#include "lwip/apps/tftp_server.h"
}

#include <errno.h>
#include <stdio.h>

namespace {

// lwIP's private TFTP_MAX_PAYLOAD_SIZE.
constexpr u16_t c_block_size = 512u;

// The one transfer. Its address is lwIP's handle.
class esp_worm_file_transfer {
public:
	hxfile card_file;
	// lwIP refuses a name longer than TFTP_MAX_FILENAME_LEN.
	char file_path[sizeof ESP_WORM_CARD_ROOT + 1u + TFTP_MAX_FILENAME_LEN];
	bool write_finished;
};

esp_worm_file_transfer s_file_transfer;

void s_file_server_close(void* transfer_handle) {
	esp_worm_file_transfer& file_transfer = *static_cast<esp_worm_file_transfer*>(transfer_handle);
	file_transfer.card_file.close();
	if(!file_transfer.write_finished) {
		const int remove_status = ::remove(file_transfer.file_path);
		hxassertf(remove_status == 0, "s_file_server_close: remove %s", file_transfer.file_path);
		(void)remove_status;
	}
}

// lwIP calls it without a null check.
void s_file_server_error(void* transfer_handle, int error_code, const char* error_message,
		int message_size) {
	(void)transfer_handle; (void)error_code; (void)error_message; (void)message_size;
}

void* s_file_server_open(const char* file_name, const char* transfer_mode, u8_t is_write) {
	hxfile& card_file = s_file_transfer.card_file;
	hxassertf(!card_file.is_open(), "s_file_server_open: Second transfer.");

	if(lwip_stricmp(transfer_mode, "octet") != 0) {
		return hxnil;
	}

	const int path_length = ::snprintf(s_file_transfer.file_path,
		sizeof s_file_transfer.file_path, ESP_WORM_CARD_ROOT "/%s", file_name);
	hxassertf(static_cast<size_t>(path_length) < sizeof s_file_transfer.file_path,
		"s_file_server_open: Name %s too long.", file_name);
	(void)path_length;

	const bool name_exists = card_file.open(hxfile::open_mode_in, "%s",
		s_file_transfer.file_path);
	if(is_write == 0u) {
		if(!name_exists) {
			return hxnil;
		}
		// A read starts finished, so its close removes nothing.
		s_file_transfer.write_finished = true;
		return &s_file_transfer;
	}
	if(name_exists) {
		card_file.close();
		return hxnil;
	}
	// Opening for writing truncates, and a card error does not mean the name is
	// free.
	if(errno != ENOENT
			|| !card_file.open(hxfile::open_mode_out, "%s", s_file_transfer.file_path)) {
		return hxnil;
	}
	s_file_transfer.write_finished = false;
	return &s_file_transfer;
}

int s_file_server_read(void* transfer_handle, void* block_bytes, int byte_count) {
	hxfile& card_file = static_cast<esp_worm_file_transfer*>(transfer_handle)->card_file;
	const size_t block_size = static_cast<size_t>(byte_count);
	const size_t bytes_read = card_file.read(block_bytes, block_size, block_size);
	// A short read is the last block only at the end of the file.
	if(bytes_read != block_size && !card_file.eof()) {
		return -1;
	}
	return static_cast<int>(bytes_read);
}

int s_file_server_write(void* transfer_handle, pbuf* block_buffer) {
	esp_worm_file_transfer& file_transfer = *static_cast<esp_worm_file_transfer*>(transfer_handle);
	for(const pbuf* block_part = block_buffer; block_part != hxnull; block_part = block_part->next) {
		const size_t part_size = block_part->len;
		if(file_transfer.card_file.write(block_part->payload, part_size) != part_size) {
			return -1;
		}
	}
	if(block_buffer->tot_len < c_block_size) {
		file_transfer.write_finished = true;
	}
	return 0;
}

constexpr tftp_context c_file_server_callbacks = {
	s_file_server_open,
	s_file_server_close,
	s_file_server_read,
	s_file_server_write,
	s_file_server_error
};

} // namespace

bool esp_worm_file_server_start(void) {
	if(tftp_init_server(&c_file_server_callbacks) != ERR_OK) {
		return false;
	}
	// lwIP keeps the server's PCB to itself, and udp_bind put it first. Nothing
	// arrives before it is bound because this runs on lwIP's thread.
	udp_pcb& server_control = *udp_pcbs;
	hxassert_hard(server_control.local_port == TFTP_PORT, "esp_worm_file_server_start: Port %u",
		static_cast<unsigned int>(server_control.local_port));
	esp_worm_tunnel_bind(server_control);
	return true;
}
