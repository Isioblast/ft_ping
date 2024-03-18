/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ping.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tde-vlee <tde-vlee@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/01/29 03:23:08 by tde-vlee          #+#    #+#             */
/*   Updated: 2024/03/18 10:45:44 by tde-vlee         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <arpa/inet.h>
#include <netinet/ip_icmp.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <argp.h>

#include "ping.h"

void printaddr(int family, struct in_addr *addr, char *msg) //debug
{
	char pdstaddr[255];
	memset(pdstaddr, 0, sizeof(pdstaddr));
	const char *dst = inet_ntop(family, addr, pdstaddr, sizeof(pdstaddr));
	if (!dst)
	{
		perror("ping");
	}
	printf("%s: %s\n", msg, pdstaddr);
}

static error_t parse_opt (int key, char *arg __attribute_maybe_unused__, struct argp_state *state __attribute_maybe_unused__)
{
	switch (key)
	{
		case 'v':
			break;
		default:
			return ARGP_ERR_UNKNOWN;
	}
	return 0;
}

static const char doc[] =
	"Send ICMP ECHO_REQUEST packets to network hosts."
	"\vOptions marked with (root only) are available only to superuser.";

int main(int argc, char **argv)
{
	t_ping ping;

	struct argp_option argp_opt[] =
		{
			{"verbose", 'v', NULL, 0, "verbose output", 0},
			{NULL, 0, NULL, 0, NULL, 0}
		};

	struct argp argp = {argp_opt, parse_opt, "HOST ...", doc, NULL, NULL, NULL};

	int parse_idx;
	parse_idx = 0;
	argp_parse(&argp, argc, argv, 0, &parse_idx, 0);

	if (ping_init(&ping))
	{
		perror("ping");
		exit(EXIT_FAILURE);
	}
	ping.hdr.checksum = chksum((uint16_t *)&ping.hdr, sizeof(ping.hdr)); //TODO delete and call it before sendto()


	struct sockaddr_in dest_addr;
	init_dest_addr(AF_INET, argv[parse_idx], &dest_addr);

	ssize_t bytes_sent = sendto(ping.fd, &ping.hdr, (sizeof(ping.hdr)),
								0, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
	if (bytes_sent < 0)
	{
		perror("sendto");
		exit(EXIT_FAILURE);
	}

	uint8_t recvbuf[PING_DEFAULT_MTU];
	struct sockaddr from;
	socklen_t addrlen;
	memset(recvbuf, 0, sizeof(recvbuf));
	memset(&from, 0, sizeof(from));
	addrlen = 0;
	ssize_t bytes_recv = recvfrom(ping.fd, recvbuf, sizeof(recvbuf), 0, &from, &addrlen);
	if (bytes_recv < 0)
	{
		perror("recvfrom");
		exit(EXIT_FAILURE);
	}
	printf("Receive bytes : %ld\n", bytes_recv);

	if (close(ping.fd) < 0)
	{
		perror("close");
		exit(EXIT_FAILURE);
	}
	return (0);
}
