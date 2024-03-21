/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ping.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tde-vlee <tde-vlee@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/01/29 03:23:08 by tde-vlee          #+#    #+#             */
/*   Updated: 2024/03/21 09:37:54 by tde-vlee         ###   ########.fr       */
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

static error_t parse_opt (int key, char *arg __attribute_maybe_unused__, struct argp_state *state __attribute_maybe_unused__)
{
	switch (key)
	{
		case 'c':
			((t_ping_opt *)state->input)->count = 1;
			break;
		case ARG_TTL:
			((t_ping_opt *)state->input)->ttl = 255;
			break;
		case 'v':
			break;
		default:
			return ARGP_ERR_UNKNOWN;
	}
	return 0;
}

static const char doc[] =
	"Send ICMP ECHO_REQUEST packets to network hosts.";

struct argp_option argp_opt[] = {
#define GROUP 0
	{NULL, 0, NULL, 0, "Options valid for all request types:", GROUP},
	{"count", 'c', "NUMBER", 0, "stop after sending NUMBER packets", GROUP+1},
	{"ttl", ARG_TTL, "NUMBER", 0, "specify N as time-to-live", GROUP+1},
	{"verbose", 'v', NULL, 0, "verbose output", GROUP+1},
	{NULL, 0, NULL, 0, NULL, 0}
#undef GROUP
};

struct argp argp = {argp_opt, parse_opt, "HOST ...", doc, NULL, NULL, NULL};

int main(int argc, char **argv)
{
	t_ping		ping;
	t_ping_opt	ping_opt;

	int parse_idx;
	parse_idx = 0;
	memset(&ping_opt, 0, sizeof(ping_opt));
	argp_parse(&argp, argc, argv, 0, &parse_idx, &ping_opt);

	if (ping_init(&ping))
	{
		perror("ping test");
		exit(EXIT_FAILURE);
	}
	ping.hdr.checksum = chksum((uint16_t *)&ping.hdr, sizeof(ping.hdr)); //TODO delete and call it before sendto()

	init_dest_addr(AF_INET, argv[parse_idx], &ping.dest);
	ping.hostname = argv[parse_idx];

	ping_loop(&ping);

	if (close(ping.fd) < 0)
	{
		perror("close");
		exit(EXIT_FAILURE);
	}
	return (0);
}
