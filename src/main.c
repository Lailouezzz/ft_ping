/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ale-boud <ale-boud@student.42lehavre.fr>   +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/17 10:01:09 by ale-boud          #+#    #+#             */
/*   Updated: 2026/09/17 10:23:39 by ale-boud         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

/**
 * @file main.c
 * @brief Entry point: argument parsing and program bootstrap.
 */

// ---
// Includes
// ---

#include <getopt.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdnoreturn.h>
#include <sysexits.h>

#include "ping.h"
#include "utils.h"

// ---
// Global variables
// ---

/**
 * @brief Global ping context, see ping.h.
 */
t_ping_ctx	g_ctx = {
	.host = NULL,
};

// ---
// Local variables
// ---

/**
 * @brief getopt_long long-option table.
 */
static struct option	long_opts[] = {
	{"help", no_argument, NULL, 'h'},
	{"verbose", no_argument, NULL, 'v'},
	{NULL, 0, NULL, 0},
};

// ---
// Static function definitions
// ---

void	print_usage(void)
{
	printf("Usage: ft_ping [OPTION...] HOST ...\n");
}

noreturn void	print_help(void)
{
	print_usage();
	printf("Send ICMP ECHO_REQUEST packets to network hosts.\n");
	printf("\n");
	printf("  -v, --verbose              verbose output\n");
	printf("  -?, --help                 give this help list\n");
	exit(EXIT_SUCCESS);
}

// ---
// Extern function definitions
// ---

int	main(
		int argc,
		char **argv) {
	set_pn(*argv);
	parse_args(argc, argv);

	return (EXIT_SUCCESS);
}

void	parse_args(int argc, char **argv)
{
	int			c;
	const char	*tok;

	opterr = 0;
	while ((c = getopt_long(argc, argv, "hv?", long_opts, NULL)) != -1)
	{
		switch (c) {
			case 'h':
				print_help();
			case 'v':
				set_verbose(true);
				break ;
			case '?':
				tok = argv[optind - 1];
				if (optopt == 0 && !(tok[0] == '-' && tok[1] == '-'))
					print_help();
				if (tok[0] == '-' && tok[1] == '-')
					error_msg("unrecognized option '%s'", tok);
				else
					error_msg("invalid option -- '%c'", optopt);
				fprintf(stderr, "Try '%s --help' for more information.\n", argv[0]);
				exit(EX_USAGE);
		}
	}
	if (optind >= argc)
	{
		error_msg("missing host operand");
		fprintf(stderr, "Try '%s --help' for more information.\n", argv[0]);
		exit(EX_USAGE);
	}
	g_ctx.host = argv[optind];
}
