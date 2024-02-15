# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: tde-vlee <tde-vlee@student.42.fr>          +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2024/01/30 02:48:49 by tde-vlee          #+#    #+#              #
#    Updated: 2024/02/15 05:29:04 by tde-vlee         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

EXEC = ft_ping

CC = gcc

CFLAGS = -Wall -Wextra -Werror -pedantic-errors

LDFLAGS =

HEADER = 

SRCS = ./src/main.c

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
