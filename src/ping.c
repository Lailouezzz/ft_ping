/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ping.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ale-boud <ale-boud@student.42lehavre.fr>   +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/17 10:43:57 by ale-boud          #+#    #+#             */
/*   Updated: 2026/09/17 10:52:03 by ale-boud         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

// ---
// Includes
// ---

#include <stdlib.h>
#include <stdnoreturn.h>
#include <sysexits.h>
#include <unistd.h>

#include "ping.h"
#include "icmp.h"

// ---
// Static function declarations
// ---



// ---
// Extern function definitions
// ---

noreturn void	ft_ping() {
	uint8_t		packet[ICMP_PACKET_LEN(g_ctx.data_len)];
	uint16_t	id;

	id = getpid() & 0xffff;
	icmp_fill_payload(packet + sizeof(t_icmp_hdr), g_ctx.data_len,
		g_ctx.pattern_len ? g_ctx.pattern : NULL, g_ctx.pattern_len);
	icmp_build_echo(packet, sizeof(packet), id, 0);

	// TODO: socket(AF_INET, SOCK_RAW, IPPROTO_ICMP), TTL, sendto/recvfrom
	// loop (-c g_ctx.count), icmp_strip_ip_header() on each reply.

	exit(EX_OK);
}

// ---
// Static function definitions
// ---
