# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: tde-vlee <tde-vlee@student.42.fr>          +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2024/01/30 02:48:49 by tde-vlee          #+#    #+#              #
#    Updated: 2024/03/18 11:10:10 by tde-vlee         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

EXEC = ft_ping

CC = gcc

CFLAGS = -Wall -Wextra -Werror -pedantic-errors

LDFLAGS =

HEADER =

SRCS =	./src/ping.c \
		./src/ping_common.c \
		./src/ping_network.c \
		./src/ping_common.c 

OBJS = $(SRCS:.c=.o)

all: $(EXEC)

$(EXEC): $(OBJS)
	$(CC) $(LDFLAGS) $^ -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -rf $(OBJS)

fclean: clean
	rm -rf $(EXEC)

re: fclean all

.PHONY: all $(EXEC) clean fclean re
