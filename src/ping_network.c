/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ping_network.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tde-vlee <tde-vlee@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/03/12 05:24:46 by tde-vlee          #+#    #+#             */
/*   Updated: 2024/03/26 16:21:35 by tde-vlee         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netdb.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <signal.h>
#include <unistd.h>
#include <sys/select.h>
#include <errno.h>

#include "ping.h"

static int run = 1;

static void sig_int(int signal __attribute_maybe_unused__)
{
	printf("signal is set...\n");
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
		dest->sin_addr = ((struct sockaddr_in *)ai_begin->ai_addr)->sin_addr;
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
	printf("--- %s ping statistics ---\n", stats->hostname);
	printf("%zu packets transmitted, ", stats->emit_nb);
	printf("%zu packets received, ", stats->receive_nb);
	printf("%d%% packet loss", (int)(((stats->emit_nb - stats->receive_nb) * 100) / stats->emit_nb));
	printf("\n");
}

static int msleepstep(const size_t msec, const size_t mstep)
{
	struct timeval current;
	struct timeval timeout;

	if (gettimeofday(&current, NULL) != 0)
	{
		perror("ping");
		exit(EXIT_FAILURE);
	}
	timeout.tv_sec = current.tv_sec + (msec / 1000);
	timeout.tv_usec = current.tv_usec + ((msec % 1000) * 1000);
	while (run && (current.tv_sec <= timeout.tv_sec && current.tv_usec < timeout.tv_usec))
	{
		if (gettimeofday(&current, NULL) != 0)
		{
			perror("ping sleepstep");
			exit(EXIT_FAILURE);
		}
		msleep(mstep);
	}
	return (0);
}

static int send_echo(t_ping *ping, t_ping_stat *stats)
{
	uint8_t sendbuf[100];
	struct icmphdr *sendhdr;

	memset(sendbuf, 0, sizeof(sendbuf));
	ping->hdr.checksum = 0;
	memcpy(sendbuf, &ping->hdr, sizeof(ping->hdr));
	sendhdr = (struct icmphdr *)sendbuf;
	sendhdr->checksum = chksum((uint16_t *)sendhdr, sizeof(ping->hdr) + PING_DEFAULT_DATALEN);
	ssize_t bytes_sent = sendto(ping->fd, sendbuf, (sizeof(ping->hdr) + PING_DEFAULT_DATALEN),
								0, (struct sockaddr *)&ping->dest, sizeof(ping->dest));
	if (bytes_sent < 0)
	{
		perror("ping send");
		return (-1);
	}
	stats->emit_nb += 1;
	printf("Sended %zu bytes of data, sequence = %d\n", sizeof(ping->hdr) + ping->datalen, ping->hdr.un.echo.sequence);
	return (bytes_sent);
}

static int receiv_reply(t_ping *ping, t_ping_stat *stats)
{
	uint8_t recvbuf[PING_DEFAULT_MTU];
	struct sockaddr from;
	socklen_t addrlen;
	ssize_t bytes_recv;

	memset(recvbuf, 0, sizeof(recvbuf));
	memset(&from, 0, sizeof(from));
	addrlen = 0;
	while (run)
	{
		// printf("pre recvfrom\n");
		bytes_recv = recvfrom(ping->fd, recvbuf, sizeof(recvbuf), 0, &from, &addrlen);
		// printf("post recvfrom\n");
		if (bytes_recv < 0)
		{
			if (errno != EAGAIN)
			{
				fprintf(stderr, "Error num = %d\n", errno);
				perror("ping");
				exit(EXIT_FAILURE);
			}
			// if -c is set, check timeout of 10 sec max and break if necessary;
			if (ping->count)
			{
				//if timeout PING_DEFAULT_COUNT_TIMEOUT
				//break;
			}
		}
		else
		{
			stats->receive_nb += 1;
			//TODO add time and print it
			char addrbufrecv[16];
			memset(addrbufrecv, 0, sizeof(addrbufrecv));
			printf("Received %zu bytes from \n", bytes_recv);
			// check error return like for ttl exceed
			break;
		}
	}
	return (bytes_recv);
}

int ping_loop(t_ping *ping, t_ping_opt *opt)
{
	char addrbuf[16];
	t_ping_stat stats;
	struct timeval start_time;
	struct timeval current_time;
	int64_t sleeptime;

	printf("Ping loop\n");
	if (signal(SIGINT, sig_int) == SIG_ERR)
	{
		perror("ping");
		return (-1);
	}
	memset(&stats, 0, sizeof(stats));
	memset(&addrbuf, 0, sizeof(addrbuf));
	inet_ntop(ping->dest.sin_family, &ping->dest.sin_addr, addrbuf, sizeof(addrbuf));
	printf("PING %s (%s): %zu data bytes\n", ping->hostname, addrbuf, ping->datalen);
	while (run)
	{
		gettimeofday(&start_time, NULL);
		send_echo(ping, &stats);
		receiv_reply(ping, &stats);
		ping->hdr.un.echo.sequence += 1;
		if (opt->count && stats.emit_nb >= opt->count)
		{
			break;
		}
		gettimeofday(&current_time, NULL);
		sleeptime = ping->interval - (((current_time.tv_sec - start_time.tv_sec) * 1000) + (current_time.tv_usec - start_time.tv_usec) / 1000);
		printf("Should Sleep for %lu.\n", sleeptime);
		if (sleeptime > 0)
		{
			msleepstep(sleeptime, 10);
		}
	}
	stats.hostname = ping->hostname;
	print_stats(&stats);
	return (0);
}
