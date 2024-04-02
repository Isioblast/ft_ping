/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ping.h                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tde-vlee <tde-vlee@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/02/06 06:56:21 by tde-vlee          #+#    #+#             */
/*   Updated: 2024/03/26 14:46:21 by tde-vlee         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#if !defined(PING_H)
#define PING_H

//   A summary of the contents of the internet header follows:
//     0                   1                   2                   3
//     0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
//    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//    |Version|  IHL  |Type of Service|          Total Length         |
//    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//    |         Identification        |Flags|      Fragment Offset    |
//    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//    |  Time to Live |    Protocol   |         Header Checksum       |
//    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//    |                       Source Address                          |
//    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//    |                    Destination Address                        |
//    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//    |                    Options                    |    Padding    |
//    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

// Echo or Echo Reply Message
//     0                   1                   2                   3
//     0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
//    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//    |     Type      |     Code      |          Checksum             |
//    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//    |           Identifier          |        Sequence Number        |
//    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//    |     Data ...
//    +-+-+-+-+-
#include <stdint.h>
#include <netinet/ip_icmp.h>

#define PING_DEFAULT_TTL 255
#define PING_DEFAULT_DATALEN 56
#define PING_DEFAULT_INTERVAL 1000
#define PING_DEFAULT_COUNT_TIMEOUT 10000
#define PING_DEFAULT_MTU 1500

#define ARG_TTL 260

typedef struct s_ping_opt
{
	uint8_t verbose;
	size_t count;
	int ttl;
} t_ping_opt;

typedef struct s_ping_stat
{
	char *hostname;
	size_t emit_nb;
	size_t receive_nb;
	int max_time;
	int min_time;
} t_ping_stat;

typedef struct s_ping
{
	char *hostname;
	int fd;
	struct sockaddr_in dest;
	struct icmphdr hdr;
	uint8_t data[PING_DEFAULT_DATALEN];
	size_t datalen;
	size_t interval;
	size_t count;
	uint8_t verbose;
} t_ping;

int ping_init(t_ping *ping, t_ping_opt *opt);
struct icmphdr icmp_init();
uint16_t chksum(uint16_t *addr, int len);
int init_dest_addr(const int af, const char *src, struct sockaddr_in *dest);
int lookup_host(const char *const hostname, struct sockaddr_in *const dest);
int ping_loop(t_ping *ping, t_ping_opt *opt);
int msleep(const size_t msec);
int tvtimeout(struct timeval start_tv, struct timeval current_tv, struct timeval timeout_tv);

#endif // PING_H
