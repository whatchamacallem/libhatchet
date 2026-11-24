// SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
// SPDX-License-Identifier: MIT
// This file is licensed under the MIT license found in the LICENSE.md file.

#include "esp_worm_tunnel.hpp"

#include <hx/hxutility.h>

#include "lwip/ip.h"
#include "lwip/sys.h"

extern "C" {
#include "wireguard-platform.h"
#include "wireguardif.h"
}

// A fragment from the link could complete a datagram that came out of the
// tunnel, which a PCB bound to the tunnel would then take.
#if IP_REASSEMBLY
#error esp_worm_tunnel.cpp requires IP_REASSEMBLY=0.
#endif

// wireguard-lwip reads only the first pbuf of a datagram. Wi-Fi puts each frame
// in one.
namespace {

// The upper word of 2^62, the TAI64 label of 1970.
constexpr uint32_t c_label_high_word = 0x40000000u;

netif s_tunnel_interface;

} // namespace

void esp_worm_tunnel_bind(udp_pcb& udp_control) {
	// wireguard-lwip passes a decrypted packet to ip_input with the tunnel as its
	// input interface. A packet from the link addressed to the tunnel keeps the
	// link as its input interface. Output goes to wireguardif_output, which drops
	// what it has no session to encrypt with.
	udp_bind_netif(&udp_control, &s_tunnel_interface);
}

bool esp_worm_tunnel_start(void) {
	// Read only during netif_add.
	wireguardif_init_data tunnel_settings = {
		ESP_WORM_TUNNEL_PRIVATE_KEY,
		ESP_WORM_TUNNEL_PORT,
		hxnil
	};

	ip4_addr_t tunnel_address;
	ip4_addr_t tunnel_netmask;
	if(ip4addr_aton(ESP_WORM_TUNNEL_ADDRESS, &tunnel_address) == 0
			|| ip4addr_aton(ESP_WORM_TUNNEL_NETMASK, &tunnel_netmask) == 0) {
		return false;
	}

	// wireguardif_init fails on a private key that does not decode.
	if(netif_add(&s_tunnel_interface, &tunnel_address, &tunnel_netmask, hxnil,
			&tunnel_settings, &wireguardif_init, &ip_input) == hxnil) {
		return false;
	}

	// wireguard-lwip sets the link up with the host's first data packet and down
	// once no session is left.
	netif_set_up(&s_tunnel_interface);

	// Without an endpoint the device waits for the host's handshake and takes
	// the endpoint from it.
	wireguardif_peer host_peer;
	wireguardif_peer_init(&host_peer);
	host_peer.public_key = ESP_WORM_TUNNEL_PEER_KEY;
	if(ipaddr_aton(ESP_WORM_TUNNEL_PEER_ADDRESS, &host_peer.allowed_ip) == 0) {
		return false;
	}
	IP_ADDR4(&host_peer.allowed_mask, 255, 255, 255, 255);
	// Keepalives would have the device start handshakes of its own. Their
	// stamps count from boot, so after a reset the host drops them as replays.
	host_peer.keep_alive = 0u;

	// wireguardif_add_peer fails on a public key that does not decode.
	u8_t peer_index = WIREGUARDIF_INVALID_INDEX;
	return wireguardif_add_peer(&s_tunnel_interface, &host_peer, &peer_index) == ERR_OK;
}

extern "C" bool wireguard_is_under_load(void) {
	return false;
}

extern "C" uint32_t wireguard_sys_now(void) {
	return sys_now();
}

// The time since boot, with each word big endian.
extern "C" void wireguard_tai64n_now(uint8_t* timestamp_bytes) {
	const uint32_t elapsed_milliseconds = sys_now();
	const uint32_t stamp_words[] = {
		lwip_htonl(c_label_high_word),
		lwip_htonl(elapsed_milliseconds / 1000u),
		lwip_htonl((elapsed_milliseconds % 1000u) * 1000000u)
	};
	::memcpy(timestamp_bytes, stamp_words, sizeof stamp_words);
}
