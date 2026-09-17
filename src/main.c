/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ale-boud <ale-boud@student.42lehavre.fr>   +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/17 10:01:09 by ale-boud          #+#    #+#             */
/*   Updated: 2026/09/17 11:08:45 by ale-boud         ###   ########.fr       */
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
// Local defines
// ---

enum { OPT_TTL = 256 };

// ---
// Global variables
// ---

/**
 * @brief Global ping context, see ping.h.
 */
t_ping_ctx	g_ctx = {
	.host = NULL,
	.count = 0,
	.pattern_len = 0,
	.data_len = ICMP_DEFAULT_DATALEN,
	.ttl = -1,
	.quiet = false,
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
	{"count", required_argument, NULL, 'c'},
	{"pattern", required_argument, NULL, 'p'},
	{"size", required_argument, NULL, 's'},
	{"ttl", required_argument, NULL, OPT_TTL},
	{"quiet", no_argument, NULL, 'q'},
	{NULL, 0, NULL, 0},
};

// ---
// Static function declarations
// ---

static void			print_usage();

noreturn static void	print_help();

static size_t		ping_cvt_number(
						const char *arg,
						size_t maxval,
						bool allow_zero);

static void			parse_args(
						int argc,
						char **argv);

// ---
// Extern function definitions
// ---

int	main(
		int argc,
		char **argv) {
	set_pn(*argv);
	parse_args(argc, argv);
	ft_ping();
}

// ---
// Static function definitions
// ---

static void	print_usage()
{
	printf("Usage: ft_ping [OPTION...] HOST ...\n");
}

noreturn static void	print_help()
{
	print_usage();
	printf("Send ICMP ECHO_REQUEST packets to network hosts.\n");
	printf("\n");
	printf("  -c, --count=NUMBER         stop after sending NUMBER packets\n");
	printf("  -p, --pattern=PATTERN      fill ICMP packet with given pattern (hex)\n");
	printf("  -s, --size=NUMBER          send NUMBER data octets\n");
	printf("      --ttl=N                specify N as time-to-live\n");
	printf("  -q, --quiet                quiet output\n");
	printf("  -v, --verbose              verbose output\n");
	printf("  -?, --help                 give this help list\n");
	exit(EXIT_SUCCESS);
}

static size_t	ping_cvt_number(const char *arg, size_t maxval, bool allow_zero)
{
	char			*end;
	unsigned long	n;

	n = strtoul(arg, &end, 0);
	if (*end)
	{
		error_msg("invalid value (`%s' near `%s')", arg, end);
		exit(EXIT_FAILURE);
	}
	if (n == 0 && !allow_zero)
	{
		error_msg("option value too small: %s", arg);
		exit(EXIT_FAILURE);
	}
	if (maxval && n > maxval)
	{
		error_msg("option value too big: %s", arg);
		exit(EXIT_FAILURE);
	}
	return (n);
}

static void	parse_args(int argc, char **argv)
{
	int			c;
	const char	*tok;

	opterr = 0;
	while ((c = getopt_long(argc, argv, "hvqc:p:s:?", long_opts, NULL)) != -1)
	{
		switch (c) {
			case 'h':
				print_help();
			case 'v':
				set_verbose(true);
				break ;
			case 'q':
				g_ctx.quiet = true;
				break ;
			case 'c':
				g_ctx.count = ping_cvt_number(optarg, 0, true);
				break ;
			case 'p':
				if (!icmp_decode_pattern(optarg, g_ctx.pattern, &g_ctx.pattern_len))
				{
					error_msg("error in pattern near %s", optarg);
					exit(EXIT_FAILURE);
				}
				break ;
			case 's':
				g_ctx.data_len = ping_cvt_number(optarg, ICMP_MAX_DATALEN, true);
				break ;
			case OPT_TTL:
				g_ctx.ttl = ping_cvt_number(optarg, 255, false);
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
