/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tde-vlee <tde-vlee@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/01/29 03:23:08 by tde-vlee          #+#    #+#             */
/*   Updated: 2024/02/29 09:39:08 by tde-vlee         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <arpa/inet.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <ifaddrs.h>
#include <netdb.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

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

void printaddr(int family, struct in_addr *addr, char *msg)
{
	char pdstaddr[255];
	memset(pdstaddr, 0, sizeof(pdstaddr));
	const char *dst = inet_ntop(family, addr, pdstaddr, sizeof(pdstaddr));
	if (!dst)
	{
		perror("inet_ntop");
		exit(EXIT_FAILURE);
	}
	printf("%s: %s\n", msg, pdstaddr);
}

struct icmphdr initicmp()
{
	struct icmphdr icmphdr;

	icmphdr.type = ICMP_ECHO;
	icmphdr.code = 0;
	icmphdr.checksum = 0;
	icmphdr.un.echo.id = getpid();
	icmphdr.un.echo.sequence = 256;
	return (icmphdr);
}

int main(void)
{
	const char *const addr_str = "8.8.8.8";
	struct sockaddr_in dest_addr;
	int raw_sock;
	int err;
	uint8_t recvbuf[DEFAULT_MTU];
	uint8_t sendbuf[84]; 
	struct icmphdr *icmphdr;

	memset(recvbuf, 0, sizeof(recvbuf));
	memset(sendbuf, 0, sizeof(sendbuf));
	icmphdr = (struct icmphdr *)sendbuf;

	err = inet_pton(AF_INET, addr_str, &dest_addr.sin_addr);
	if (err <= 0)
	{
		if (err == 0)
			fprintf(stderr, "Network adress is not in presentation format.");
		else
			perror("inet_pton");
		exit(EXIT_FAILURE);
	}

	dest_addr.sin_family = AF_INET;
	memset(dest_addr.sin_zero, 0, sizeof(dest_addr.sin_zero));

	raw_sock = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
	if (raw_sock < 0)
	{
		perror("socket");
		exit(EXIT_FAILURE);
	}

	*icmphdr = initicmp();
	icmphdr->checksum = chksum((uint16_t *)icmphdr, sizeof(icmphdr));

	ssize_t bytes_sent = sendto(raw_sock, icmphdr, (sizeof(icmphdr)),
								0, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
	if (bytes_sent < 0)
	{
		perror("sendto");
		exit(EXIT_FAILURE);
	}

	struct sockaddr from;
	socklen_t addrlen;
	memset(&from, 0, sizeof(from));
	addrlen = 0;
	ssize_t bytes_recv = recvfrom(raw_sock, &icmphdr, 56, 0, &from, &addrlen);
	if (bytes_recv < 0)
	{
		perror("recvfrom");
		exit(EXIT_FAILURE);
	}

	printf("Receive bytes : %ld\n", bytes_recv);

	if (close(raw_sock) < 0)
	{
		perror("close");
		exit(EXIT_FAILURE);
	}
	return (0);
}
