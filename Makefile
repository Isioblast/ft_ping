# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: tde-vlee <tde-vlee@student.42.fr>          +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2024/01/30 02:48:49 by tde-vlee          #+#    #+#              #
#    Updated: 2024/04/02 15:00:21 by tde-vlee         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

EXEC = ft_ping

CC = gcc

CFLAGS = -Wall -Wextra -Werror -pedantic-errors

LDFLAGS =

HEADER = ./src/ping.h

SRCS =	./src/ping.c \
		./src/ping_common.c \
		./src/ping_network.c \
		./src/ping_common.c \
		./src/ping_lib.c

OBJS = $(SRCS:.c=.o)

all: $(EXEC)

$(EXEC): $(OBJS)
	$(CC) $(LDFLAGS) $^ -o $@

%.o: %.c $(HEADER)
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -rf $(OBJS)

fclean: clean
	rm -rf $(EXEC)

re: fclean all

.PHONY: all $(EXEC) clean fclean re
