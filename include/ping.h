/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ping.h                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ale-boud <ale-boud@student.42lehavre.fr>   +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/17 10:01:12 by ale-boud          #+#    #+#             */
/*   Updated: 2026/09/17 10:46:39 by ale-boud         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

/**
 * @file ping.h
 * @brief ft_ping public types, global context and function declarations.
 */

#ifndef PING_H
# define PING_H

# include <stdbool.h>
# include <stddef.h>
# include <stdint.h>
# include <stdnoreturn.h>

# include "icmp.h"

/** Global runtime context populated by argv. */
typedef struct s_ping_ctx
{
	const char	*host;	/**< Target host/IP, positional argument. */
	size_t		count;	/**< -c/--count, 0 = unlimited */
	uint8_t		pattern[ICMP_MAX_PATTERN_LEN];	/**< -p/--pattern bytes */
	size_t		pattern_len;	/**< 0 = no pattern, default sequential fill */
}	t_ping_ctx;

extern t_ping_ctx	g_ctx;

noreturn void	ft_ping();

#endif
