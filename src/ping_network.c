/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ping_network.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tde-vlee <tde-vlee@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/03/12 05:24:46 by tde-vlee          #+#    #+#             */
/*   Updated: 2024/03/21 10:58:04 by tde-vlee         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <signal.h>
#include <unistd.h>
#include <sys/select.h>
#include <errno.h>

#include "ping.h"

static int run = 1;

static void sig_int (int signal __attribute_maybe_unused__)
{
	run = 0;
}

int lookup_host(const char *const hostname, struct sockaddr_in *const dest)
{
	int errcode;
	struct addrinfo hints;
	struct addrinfo *ai_begin;

	ai_begin = NULL;
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = 0;
	hints.ai_protocol = IPPROTO_ICMP;
	hints.ai_flags = 0;
	errcode = getaddrinfo(hostname, NULL, &hints, &ai_begin);
	if (errcode != 0)
	{
		return (errcode);
	}
	if (ai_begin)
	{
		dest->sin_addr = ((struct sockaddr_in *) ai_begin->ai_addr)->sin_addr;
	}
	else
	{
		return (-1);
	}
	freeaddrinfo(ai_begin);
	return (errcode);
}

static void print_stats(t_ping_stat *stats)
{
	fflush(stdout);
	printf ("--- %s ping statistics ---\n", stats->hostname);
  	printf ("%zu packets transmitted, ", stats->emit_nb);
  	printf ("%zu packets received, ", stats->receive_nb);
	printf ("%d%% packet loss", (int) (((stats->emit_nb - stats->receive_nb) * 100) / stats->emit_nb));
	printf ("\n");
}

int ping_loop(t_ping *ping)
{
	char		addrbuf[16];
	t_ping_stat	stats;

	if (signal(SIGINT, sig_int) == SIG_ERR)
	{
		perror("ping");
		return (-1);
	}
	memset(&stats, 0, sizeof(stats));
	memset(&addrbuf, 0, sizeof(addrbuf));
	inet_ntop(ping->dest.sin_family, &ping->dest.sin_addr, addrbuf, sizeof(addrbuf));
	printf("PING %s (%s): %zu data bytes\n", ping->hostname, addrbuf, ping->datalen);


	struct timeval tv;
	tv.tv_sec = 1;
	tv.tv_usec = 0;
	if (setsockopt(ping->fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0) {
		perror("setsockopt");
	}

	while (run)
	{
		printf("Sending %zu bytes of data to %s\n", sizeof(ping->hdr) + ping->datalen, addrbuf);
		ssize_t bytes_sent = sendto(ping->fd, &ping->hdr, (sizeof(ping->hdr) + ping->datalen),
									0, (struct sockaddr *)&ping->dest, sizeof(ping->dest));
		if (bytes_sent < 0)
		{
			perror("ping send");
			return (-1);
		}
		stats.emit_nb += 1;
		printf("Sended %zu bytes of data\n", bytes_sent);

		uint8_t recvbuf[PING_DEFAULT_MTU];
		struct sockaddr from;
		socklen_t addrlen;
		memset(recvbuf, 0, sizeof(recvbuf));
		memset(&from, 0, sizeof(from));
		addrlen = 0;
		ssize_t bytes_recv = recvfrom(ping->fd, recvbuf, sizeof(recvbuf), 0, &from, &addrlen);
		if (bytes_recv < 0)
		{
			if (errno != EAGAIN)
			{
				perror("recvfrom");
				return (-1);
			}
		}
		else
		{
			stats.receive_nb += 1;
			char addrbufrecv[16];
			memset(addrbufrecv, 0, sizeof(addrbufrecv));
			printf("Received %zu bytes from \n", bytes_recv);
		}
		sleep(1);
	}

	stats.hostname = ping->hostname;
	fflush(stdout);
	print_stats(&stats);
	return (0);
}

int receiv_reply()
{
	return (0);
}