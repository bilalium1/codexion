/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: blemrabe <blemrabe@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/17 10:46:50 by blemrabe          #+#    #+#             */
/*   Updated: 2026/04/30 20:57:37 by blemrabe         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "coders.h"
#include <stdio.h>

int	is_stopped(t_sim *sim)
{
	int	val;

	pthread_mutex_lock(&sim->stop_mutex);
	val = sim->stop;
	pthread_mutex_unlock(&sim->stop_mutex);
	return (val);
}

long	get_time(void)
{
	struct timeval	tv;

	gettimeofday(&tv, NULL);
	return ((tv.tv_sec * 1000) + (tv.tv_usec / 1000));
}

void	ft_sleep(long duration, t_sim *sim)
{
	long	start;

	start = get_time();
	while (!is_stopped(sim))
	{
		//printf("time : %ld\n", get_time() - start);
		if (get_time() - start >= duration)
			break ;
		usleep(500);
	}
}

const char *get_color(char *msg)
{
	if (!strcmp(msg, "is debugging"))
		return "\033[48;2;0;0;80m";       // blue bg
	if (!strcmp(msg, "is compiling"))
		return "\033[48;2;0;80;0m";       // green bg
	if (!strcmp(msg, "is refactoring"))
		return "\033[48;2;80;80;0m";     // yellow bg
	if (!strcmp(msg, "has taken a dongle"))
		return "\033[48;2;80;0;80m";     // magenta bg
	if (!strcmp(msg, "burned out"))
	    return "\033[80;2;150;0;80m";
	return "\033[0m"; // default
}

void	log_action(t_sim *sim, int id, char *msg)
{
	pthread_mutex_lock(&sim->log_mutex);

	const char *color = get_color(msg);

	printf("%s%ld | %d %s\033[0m\n",
		color,
		get_time() - sim->st,
		id + 1,
		msg
	);

	pthread_mutex_unlock(&sim->log_mutex);
}
