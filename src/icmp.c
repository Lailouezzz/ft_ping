/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   icmp.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ale-boud <ale-boud@student.42lehavre.fr>   +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/17 10:48:52 by ale-boud          #+#    #+#             */
/*   Updated: 2026/09/17 10:48:55 by ale-boud         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

/**
 * @file icmp.c
 * @brief ICMP echo packet building
 */

// ---
// Includes
// ---

#include <arpa/inet.h>
#include <netinet/ip.h>
#include <stdio.h>

#include "icmp.h"

// ---
// Extern function definitions
// ---

uint16_t	icmp_checksum(const void *data, size_t len)
{
	const uint16_t	*buf;
	uint32_t		sum;
	uint16_t		last;

	buf = data;
	sum = 0;
	while (len > 1)
	{
		sum += *buf++;
		len -= 2;
	}
	if (len == 1)
	{
		last = 0;
		*(uint8_t *)&last = *(const uint8_t *)buf;
		sum += last;
	}
	sum = (sum >> 16) + (sum & 0xffff);
	sum += (sum >> 16);
	return ((uint16_t)~sum);
}

void	icmp_build_echo(void *buf, size_t len, uint16_t id, uint16_t seq)
{
	t_icmp_hdr	*hdr;

	hdr = buf;
	hdr->type = ICMP_ECHO_REQUEST;
	hdr->code = 0;
	hdr->id = htons(id);
	hdr->seq = htons(seq);
	hdr->checksum = 0;
	hdr->checksum = icmp_checksum(buf, len);
}

uint8_t	*icmp_strip_ip_header(uint8_t *buf, size_t len, size_t *out_len)
{
	struct ip	*ip_hdr;
	size_t		ip_len;

	if (len < sizeof(struct ip))
		return (NULL);
	ip_hdr = (struct ip *)buf;
	ip_len = ip_hdr->ip_hl << 2;
	if (ip_len >= len)
		return (NULL);
	if (out_len)
		*out_len = len - ip_len;
	return (buf + ip_len);
}

bool	icmp_decode_pattern(const char *text, uint8_t *out, size_t *out_len)
{
	size_t	i;
	int		byte;
	int		consumed;

	i = 0;
	while (text[0] && i < ICMP_MAX_PATTERN_LEN)
	{
		if (sscanf(text, "%2x%n", &byte, &consumed) != 1)
			return (false);
		out[i] = (uint8_t)byte;
		text += consumed;
		i++;
	}
	*out_len = i;
	return (true);
}

void	icmp_fill_payload(uint8_t *payload, size_t payload_len,
			const uint8_t *pattern, size_t pattern_len)
{
	size_t	i;

	i = 0;
	while (i < payload_len)
	{
		if (pattern && pattern_len)
			payload[i] = pattern[i % pattern_len];
		else
			payload[i] = (uint8_t)i;
		i++;
	}
}
