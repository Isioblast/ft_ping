/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ping_common.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tde-vlee <tde-vlee@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/03/12 04:07:23 by tde-vlee          #+#    #+#             */
/*   Updated: 2024/03/21 08:11:24 by tde-vlee         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <error.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include "ping.h"

uint16_t chksum(uint16_t *addr, int len)
{
	int nleft = len;
	int sum = 0;
	uint16_t *w = addr;
	uint16_t answer = 0;
	while (nleft > 1)
	{
		sum += *w++;
		nleft -= 2;
	}
	if (nleft == 1)
	{
		*(uint8_t *)(&answer) = *(uint8_t *)w;
		sum += answer;
	}
	sum = (sum >> 16) + (sum & 0xffff);
	sum += (sum >> 16);
	answer = ~sum;
	return (answer);
}

struct icmphdr icmp_init()
{
	struct icmphdr icmphdr;

	memset(&icmphdr, 0 , sizeof(icmphdr));
	icmphdr.type = ICMP_ECHO;
	icmphdr.un.echo.id = getpid();
	return (icmphdr);
}

int ping_init(t_ping *ping)
{
	memset(ping, 0, sizeof(*ping));
    ping->fd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
	if (ping->fd < 0)
	{
		if (errno != EPERM && errno != EACCES)
		{
			perror("ping");
			return (-1);
		}
		errno = 0;
		ping->fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_ICMP);
		if (ping->fd < 0)
		{
            if (errno == EPERM || errno == EACCES || errno == EPROTONOSUPPORT)
            {
                fprintf(stderr, "ping: Lacking privilege for icmp socket.\n");
            }
            else
            {
                perror("ping");
            }
            return (-1);
        }
	}
    ping->hdr = icmp_init();
    ping->data = NULL;
    ping->interval = PING_DEFAULT_INTERVAL;
    return (0);
}

int init_dest_addr(const int af, const char *const src, struct sockaddr_in *const dest)
{
	int err_code;

	memset(dest, 0, sizeof(*dest));
	dest->sin_family = af;
	err_code = inet_pton(af, src, &dest->sin_addr);
	if (err_code <= 0)
	{
		if (err_code < 0)
		{
			perror("ping");
			exit(EXIT_FAILURE);
		}
		err_code = lookup_host(src, dest);
		if (err_code != 0)
		{
			fprintf(stderr, "ping: unknown host\n");
			exit(EXIT_FAILURE);
		}
	}
	return (err_code);
}
