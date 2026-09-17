/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ping.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ale-boud <ale-boud@student.42lehavre.fr>   +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/17 10:43:57 by ale-boud          #+#    #+#             */
/*   Updated: 2026/09/17 11:52:00 by ale-boud         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

// ---
// Includes
// ---

#include <arpa/inet.h>
#include <math.h>
#include <netdb.h>
#include <netinet/ip.h>
#include <signal.h>
#include <stdlib.h>
#include <stdnoreturn.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sysexits.h>
#include <unistd.h>

#include "ping.h"
#include "icmp.h"
#include "utils.h"

// ---
// Local defines
// ---

# define PING_RECV_TIMEOUT_SEC	1
# define PING_DEFAULT_INTERVAL_SEC	1

// ---
// Local variables
// ---

static volatile sig_atomic_t	g_stop = 0;

typedef struct s_stats
{
	size_t	sent;
	size_t	recv;
	double	rtt_min;
	double	rtt_max;
	double	rtt_sum;
	double	rtt_sum_sq;
}	t_stats;

// ---
// Static function declarations
// ---

static void	on_sigint(int sig);
static double	timeval_diff_ms(struct timeval t0, struct timeval t1);
static bool	resolve_host(
				const char *host,
				struct sockaddr_in *dest,
				char *ip_str,
				size_t ip_str_len);
static int	open_socket(void);
static void	send_request(int sock, struct sockaddr_in *dest,
				uint8_t *packet, size_t packet_len);
static void	recv_reply(int sock, struct timeval t0,
				uint16_t id, size_t seq, t_stats *stats);
static bool	is_error_for_us(const uint8_t *icmp_data, size_t icmp_len, uint16_t id);
static void	print_statistics(const char *host, const t_stats *stats);
static void	pace(struct timeval t0);

// ---
// Extern function definitions
// ---

noreturn void	ft_ping() {
	struct sockaddr_in	dest;
	char				ip_str[INET_ADDRSTRLEN];
	int					sock;
	uint8_t				packet[ICMP_PACKET_LEN(g_ctx.data_len)];
	uint16_t			id;
	size_t				seq;
	t_stats				stats;
	struct timeval		t0;

	setvbuf(stdout, NULL, _IOLBF, 0);
	if (!resolve_host(g_ctx.host, &dest, ip_str, sizeof(ip_str)))
		exit(EXIT_FAILURE);
	sock = open_socket();
	id = getpid() & 0xffff;
	memset(&stats, 0, sizeof(stats));
	stats.rtt_min = -1;
	signal(SIGINT, on_sigint);

	printf("PING %s (%s): %zu data bytes", g_ctx.host, ip_str, g_ctx.data_len);
	if (is_verbose())
		printf(", id 0x%04x = %u", id, id);
	printf("\n");

	seq = 0;
	while (!g_stop && (g_ctx.count == 0 || seq < g_ctx.count))
	{
		icmp_fill_payload(packet + sizeof(t_icmp_hdr), g_ctx.data_len,
			g_ctx.pattern_len ? g_ctx.pattern : NULL, g_ctx.pattern_len);
		icmp_build_echo(packet, sizeof(packet), id, seq);
		gettimeofday(&t0, NULL);
		send_request(sock, &dest, packet, sizeof(packet));
		stats.sent++;
		recv_reply(sock, t0, id, seq, &stats);
		seq++;
		if (!g_stop && (g_ctx.count == 0 || seq < g_ctx.count))
			pace(t0);
	}
	print_statistics(g_ctx.host, &stats);
	close(sock);
	exit(stats.recv > 0 || stats.sent == 0 ? EX_OK : EXIT_FAILURE);
}

// ---
// Static function definitions
// ---

static void	on_sigint(int sig)
{
	UNUSED(sig);
	g_stop = 1;
}

static double	timeval_diff_ms(struct timeval t0, struct timeval t1)
{
	return ((t1.tv_sec - t0.tv_sec) * 1000.0
		+ (t1.tv_usec - t0.tv_usec) / 1000.0);
}

static bool	resolve_host(
				const char *host,
				struct sockaddr_in *dest,
				char *ip_str,
				size_t ip_str_len) {
	struct addrinfo	hints;
	struct addrinfo	*res;
	int				err;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	err = getaddrinfo(host, NULL, &hints, &res);
	if (err != 0)
	{
		error_msg("%s: %s", host, gai_strerror(err));
		return (false);
	}
	*dest = *(struct sockaddr_in *)res->ai_addr;
	inet_ntop(AF_INET, &dest->sin_addr, ip_str, ip_str_len);
	freeaddrinfo(res);
	return (true);
}

static int	open_socket(void)
{
	int				sock;
	struct timeval	timeout;

	sock = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
	if (sock < 0)
	{
		perror_msg("socket");
		exit(EXIT_FAILURE);
	}
	if (g_ctx.ttl != -1)
		setsockopt(sock, IPPROTO_IP, IP_TTL, &g_ctx.ttl, sizeof(g_ctx.ttl));
	timeout.tv_sec = PING_RECV_TIMEOUT_SEC;
	timeout.tv_usec = 0;
	setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
	return (sock);
}

static void	send_request(int sock, struct sockaddr_in *dest,
				uint8_t *packet, size_t packet_len) {
	if (sendto(sock, packet, packet_len, 0,
			(struct sockaddr *)dest, sizeof(*dest)) < 0)
		perror_msg("sendto");
}

static void	recv_reply(int sock, struct timeval t0,
				uint16_t id, size_t seq, t_stats *stats) {
	uint8_t			rbuf[IP_MAXPACKET];
	uint8_t			*icmp_data;
	size_t			icmp_len;
	t_icmp_hdr		*reply;
	struct sockaddr_in	from;
	socklen_t		from_len;
	char			from_str[INET_ADDRSTRLEN];
	struct timeval	now;
	double			rtt;
	ssize_t			n;

	while (true)
	{
		from_len = sizeof(from);
		n = recvfrom(sock, rbuf, sizeof(rbuf), 0,
				(struct sockaddr *)&from, &from_len);
		if (n <= 0)
			return ;
		icmp_data = icmp_strip_ip_header(rbuf, (size_t)n, &icmp_len);
		if (!icmp_data || icmp_len < sizeof(t_icmp_hdr))
			continue ;
		reply = (t_icmp_hdr *)icmp_data;
		if (reply->type == ICMP_ECHO_REQUEST)
			continue ;	/* our own request looping back (e.g. on lo) */
		inet_ntop(AF_INET, &from.sin_addr, from_str, sizeof(from_str));
		if (reply->type != ICMP_ECHO_REPLY)
		{
			if (is_verbose() && is_error_for_us(icmp_data, icmp_len, id))
				printf("%zu bytes from %s: icmp_type=%u icmp_code=%u\n",
					icmp_len, from_str, reply->type, reply->code);
			continue ;
		}
		if (ntohs(reply->id) != id || ntohs(reply->seq) != seq)
			continue ;
		gettimeofday(&now, NULL);
		break ;
	}
	rtt = timeval_diff_ms(t0, now);
	stats->recv++;
	if (stats->rtt_min < 0 || rtt < stats->rtt_min)
		stats->rtt_min = rtt;
	if (rtt > stats->rtt_max)
		stats->rtt_max = rtt;
	stats->rtt_sum += rtt;
	stats->rtt_sum_sq += rtt * rtt;
	if (!g_ctx.quiet)
		printf("%zu bytes from %s: icmp_seq=%zu ttl=%d time=%.3f ms\n",
			icmp_len, from_str, seq, ((struct ip *)rbuf)->ip_ttl, rtt);
}

/* An ICMP error embeds the original IP+ICMP header that caused it. */
static bool	is_error_for_us(const uint8_t *icmp_data, size_t icmp_len, uint16_t id)
{
	const uint8_t	*orig;
	size_t			orig_len;
	uint8_t			*orig_icmp_data;
	size_t			orig_icmp_len;
	t_icmp_hdr		*orig_icmp;

	if (icmp_len <= sizeof(t_icmp_hdr))
		return (false);
	orig = icmp_data + sizeof(t_icmp_hdr);
	orig_len = icmp_len - sizeof(t_icmp_hdr);
	orig_icmp_data = icmp_strip_ip_header((uint8_t *)orig, orig_len, &orig_icmp_len);
	if (!orig_icmp_data || orig_icmp_len < sizeof(t_icmp_hdr))
		return (false);
	orig_icmp = (t_icmp_hdr *)orig_icmp_data;
	return (orig_icmp->type == ICMP_ECHO_REQUEST && ntohs(orig_icmp->id) == id);
}

static void	pace(struct timeval t0)
{
	struct timeval	now;
	double			elapsed_ms;
	double			remaining_ms;

	gettimeofday(&now, NULL);
	elapsed_ms = timeval_diff_ms(t0, now);
	remaining_ms = PING_DEFAULT_INTERVAL_SEC * 1000.0 - elapsed_ms;
	if (remaining_ms > 0)
		usleep((useconds_t)(remaining_ms * 1000));
}

static void	print_statistics(const char *host, const t_stats *stats)
{
	size_t	loss_pct;
	double	avg;
	double	variance;

	loss_pct = 0;
	if (stats->sent)
		loss_pct = 100 * (stats->sent - stats->recv) / stats->sent;
	printf("--- %s ping statistics ---\n", host);
	printf("%zu packets transmitted, %zu packets received, %zu%% packet loss\n",
		stats->sent, stats->recv, loss_pct);
	if (stats->recv > 0)
	{
		avg = stats->rtt_sum / (double)stats->recv;
		variance = stats->rtt_sum_sq / (double)stats->recv - avg * avg;
		if (variance < 0)
			variance = 0;
		printf("round-trip min/avg/max/stddev = %.3f/%.3f/%.3f/%.3f ms\n",
			stats->rtt_min, avg, stats->rtt_max, sqrt(variance));
	}
}
