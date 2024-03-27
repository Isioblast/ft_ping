/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ping_lib.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tde-vlee <tde-vlee@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/03/25 14:17:20 by tde-vlee          #+#    #+#             */
/*   Updated: 2024/03/26 11:06:06 by tde-vlee         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <time.h>
#include <errno.h>

// msleep(): Sleep for the requested number of milliseconds.
int msleep(const size_t msec)
{
	struct timespec ts;
	int res;

	ts.tv_sec = msec / 1000;
	ts.tv_nsec = (msec % 1000) * 1000000;
	res = nanosleep(&ts, NULL);
	return res;
}
