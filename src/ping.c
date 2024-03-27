/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ping.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tde-vlee <tde-vlee@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/01/29 03:23:08 by tde-vlee          #+#    #+#             */
/*   Updated: 2024/03/25 16:51:15 by tde-vlee         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <arpa/inet.h>
#include <netinet/ip_icmp.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <errno.h>
#include <error.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <argp.h>

#include "ping.h"

static error_t parse_opt(int key, char *arg __attribute_maybe_unused__, struct argp_state *state __attribute_maybe_unused__)
{
	char *endptr;
	t_ping_opt *opt;

	opt = ((t_ping_opt *)state->input);
	endptr = NULL;
	switch (key)
	{
		case 'c':
			opt->count = strtoul(arg, &endptr, 10);
			if (endptr != NULL && *endptr != '\0') // TODO refactor this if and the following if
			{
				error(EXIT_FAILURE, 0, "ping: invalid value (`%s' near `%s')", arg, endptr);
			}
			//TODO invalid argument like -c 0
			break;
		case ARG_TTL:
			opt->ttl = strtoul(arg, &endptr, 10);
			if (endptr != NULL && *endptr != '\0') // TODO refactor this if
			{
				error(EXIT_FAILURE, 0, "ping: invalid value (`%s' near `%s')", arg, endptr);
			}
			if (opt->ttl > 255)
			{
				error(EXIT_FAILURE, 0, "ping: option value too big: %s", arg);
			}
			break;
		case 'v':
			opt->verbose = 1;
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
	{"count", 'c', "NUMBER", 0, "stop after sending NUMBER packets", GROUP + 1},
	{"ttl", ARG_TTL, "NUMBER", 0, "specify N as time-to-live", GROUP + 1},
	{"verbose", 'v', NULL, 0, "verbose output", GROUP + 1},
	{NULL, 0, NULL, 0, NULL, 0}
#undef GROUP
};

struct argp argp = {argp_opt, parse_opt, "HOST ...", doc, NULL, NULL, NULL};

int main(int argc, char **argv)
{
	t_ping ping;
	t_ping_opt ping_opt;

	int parse_idx;
	parse_idx = 0;
	memset(&ping_opt, 0, sizeof(ping_opt));
	argp_parse(&argp, argc, argv, 0, &parse_idx, &ping_opt);

	if (ping_init(&ping, &ping_opt))
	{
		exit(EXIT_FAILURE);
	}

	init_dest_addr(AF_INET, argv[parse_idx], &ping.dest);
	ping.hostname = argv[parse_idx];

	ping_loop(&ping, &ping_opt);

	if (close(ping.fd) < 0)
	{
		perror("close");
		exit(EXIT_FAILURE);
	}
	return (0);
}
