/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tde-vlee <tde-vlee@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/01/29 03:23:08 by tde-vlee          #+#    #+#             */
/*   Updated: 2024/02/29 06:44:45 by tde-vlee         ###   ########.fr       */
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
	explicit_bzero(pdstaddr, sizeof(pdstaddr));
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
	icmphdr.un.echo.sequence = 0;
	icmphdr.checksum = 0;
	return (icmphdr);
}

struct sockaddr_in getlocaladrr(char *ifname)
{
	printf("getlocaladrr:\n");
	struct sockaddr_in	local_addr;
	struct ifaddrs		*ifap;
	struct ifaddrs		*ifa;

	memset(&local_addr, 0, sizeof(local_addr));
	if (getifaddrs(&ifap) < 0)
	{
		perror("getifaddrs");
		exit(EXIT_FAILURE);
	}
	ifa = ifap;
	while(ifa)
	{
		printf("ifa_name: %s\n", ifa->ifa_name);
		if (strcmp(ifname, ifa->ifa_name) == 0)
		{
		}
		ifa = ifa->ifa_next;
	}
	freeifaddrs(ifap);
	return local_addr;
}

int main(void)
{
	const char *const addr_str = "8.8.8.8";
	//struct sockaddr_in src_addr;
	struct sockaddr_in dest_addr;
	int raw_sock;
	int err;
	uint8_t recvbuf[DEFAULT_MTU];
	uint8_t sendbuf[84]; 
	struct iphdr *iphdr;
	struct icmphdr *icmphdr;

	explicit_bzero(recvbuf, sizeof(recvbuf));
	explicit_bzero(sendbuf, sizeof(sendbuf));
	iphdr = (struct iphdr *)sendbuf;
	icmphdr = (struct icmphdr *)&sendbuf[sizeof(*iphdr)]; //TODO find a better syntax

	printf("Printing lenght: %lu\n", (size_t)((size_t)icmphdr - (size_t)iphdr)); //TODO debug

	err = inet_pton(AF_INET, addr_str, &dest_addr.sin_addr);
	if (err <= 0)
	{
		if (err == 0)
			fprintf(stderr, "Network adress is not in presentation format.");
		else
			perror("inet_pton");
		exit(EXIT_FAILURE);
	}

	// TODO debug
	printaddr(AF_INET, &dest_addr.sin_addr, "destination adress");
	// end debug

	//src_addr = getlocaladrr("eth0");
	//TODO debug
	// err = inet_pton(AF_INET, "10.0.2.15", &src_addr.sin_addr);
	// if (err <= 0)
	// {
	// 	if (err == 0)
	// 		fprintf(stderr, "Network adress is not in presentation format.");
	// 	else
	// 		perror("inet_pton");
	// 	exit(EXIT_FAILURE);
	// }
	//end debug

	dest_addr.sin_family = AF_INET;
	explicit_bzero(dest_addr.sin_zero, sizeof(dest_addr.sin_zero));

	raw_sock = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
	if (raw_sock < 0)
	{
		perror("socket");
		exit(EXIT_FAILURE);
	}

	// int on;

	// on = 1;
	// err = setsockopt(raw_sock, IPPROTO_IP, IP_HDRINCL, &on, sizeof(on));
	// if (err < 0)
	// {
	// 	perror("setsockopt");
	// 	exit(EXIT_FAILURE);
	// }

	// iphdr->version = 4;
	// iphdr->ihl = 5;
	// iphdr->tos = 0;
	// iphdr->tot_len = htons(sizeof(sendbuf));
	// iphdr->id = htons(391);
	// iphdr->frag_off = htons(1 << 14); //mettre df a 1
	// iphdr->ttl = DEFAULT_TTL;
	// iphdr->protocol = IPPROTO_ICMP;
	// iphdr->check = 0;
	// //iphdr->saddr = (*(struct sockaddr_in *)result->ai_addr).sin_addr.s_addr;
	// iphdr->saddr = src_addr.sin_addr.s_addr;
	// iphdr->daddr = dest_addr.sin_addr.s_addr;

	// iphdr->check = chksum((uint16_t *)iphdr, (iphdr->ihl << 2));

	*icmphdr = initicmp();
	icmphdr->checksum = chksum((uint16_t *)iphdr, sizeof(sendbuf) - sizeof(*iphdr));

	printaddr(AF_INET, (struct in_addr *)&iphdr->saddr, "buffer source adress");
	printaddr(AF_INET, (struct in_addr *)&iphdr->daddr, "buffer destination adress");

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
