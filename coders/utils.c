/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: blemrabe <blemrabe@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/17 10:46:50 by blemrabe          #+#    #+#             */
/*   Updated: 2026/04/26 12:59:10 by blemrabe         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "coders.h"

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
		if (get_time() - start >= duration)
			break ;
		usleep(500);
	}
}

void	log_action(t_sim *sim, int id, char *msg)
{
	pthread_mutex_lock(&sim->log_mutex);
	printf("%ld %d %s\033[0m\n", get_time() - sim->st, id + 1, msg);
	pthread_mutex_unlock(&sim->log_mutex);
}
