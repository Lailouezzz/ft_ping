/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   icmp.h                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ale-boud <ale-boud@student.42lehavre.fr>   +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/17 10:46:49 by ale-boud          #+#    #+#             */
/*   Updated: 2026/09/17 10:51:10 by ale-boud         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef ICMP_H
# define ICMP_H

# include <stddef.h>
# include <stdint.h>
# include <stdbool.h>

# include "utils.h"

# define ICMP_ECHO_REQUEST	8
# define ICMP_ECHO_REPLY	0
# define ICMP_MAX_PATTERN_LEN	16
# define ICMP_DEFAULT_DATALEN	56
# define ICMP_PACKET_LEN(datalen)	(sizeof(t_icmp_hdr) + (datalen))

typedef struct PACKED s_icmp_hdr
{
	uint8_t		type;
	uint8_t		code;
	uint16_t	checksum;
	uint16_t	id;
	uint16_t	seq;
}	t_icmp_hdr;

/**
 * @brief Internet checksum (RFC 1071).
 * @param data Buffer to sum.
 * @param len Buffer size.
 * @return Checksum value.
 */
uint16_t	icmp_checksum(const void *data, size_t len);

/**
 * @brief Fill echo header and its checksum.
 * @param buf Packet buffer.
 * @param len Total packet size.
 * @param id Echo identifier.
 * @param seq Echo sequence.
 */
void		icmp_build_echo(void *buf, size_t len, uint16_t id, uint16_t seq);

/**
 * @brief Skip the raw socket's IPv4 header.
 * @param buf Received buffer.
 * @param len Bytes received.
 * @param out_len Remaining length.
 * @return Pointer to ICMP part, or NULL.
 */
uint8_t		*icmp_strip_ip_header(uint8_t *buf, size_t len, size_t *out_len);

/**
 * @brief Decode a -p hex pattern ("aabb").
 * @param text Hex string.
 * @param out Output bytes.
 * @param out_len Decoded length.
 * @return false on bad hex.
 */
bool		icmp_decode_pattern(const char *text, uint8_t *out, size_t *out_len);

/**
 * @brief Fill payload, repeating pattern or sequential bytes.
 * @param payload Buffer to fill.
 * @param payload_len Buffer size.
 * @param pattern Bytes to repeat, or NULL.
 * @param pattern_len Pattern size.
 */
void		icmp_fill_payload(uint8_t *payload, size_t payload_len,
				const uint8_t *pattern, size_t pattern_len);

#endif
