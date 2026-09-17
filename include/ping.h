/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ping.h                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ale-boud <ale-boud@student.42lehavre.fr>   +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/17 10:01:12 by ale-boud          #+#    #+#             */
/*   Updated: 2026/09/17 10:02:17 by ale-boud         ###   ########.fr       */
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

/**
 * @brief Global runtime context populated by argv.
 */
typedef struct s_ping_ctx
{
	const char	*host;	/**< Target host/IP, positional argument. */
}	t_ping_ctx;

/**
 * @brief Global context
 */
extern t_ping_ctx	g_ctx;

/**
 * @brief Parse command-line arguments with getopt_long and fill g_ctx.
 * @param argc argc from main.
 * @param argv argv from main.
 */
void	parse_args(int argc, char **argv);

/**
 * @brief Print the ft_ping short usage line on stdout.
 */
void	print_usage(void);

/**
 * @brief Print the full ft_ping help text on stdout, then exit successfully.
 */
void	print_help(void);

#endif
