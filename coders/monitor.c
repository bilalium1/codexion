/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   monitor.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: blemrabe <blemrabe@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/17 10:46:14 by blemrabe          #+#    #+#             */
/*   Updated: 2026/06/15 02:09:01 by blemrabe         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "coders.h"
#include <pthread.h>

static int	all_complete(t_sim	*sim)
{
	int	i;
	int	count;

	i = 0;
	while (i < sim->data[NBR_CDRS])
	{
		pthread_mutex_lock(&sim->coders[i].cmutex);
		count = sim->coders[i].compile_count;
		pthread_mutex_unlock(&sim->coders[i].cmutex);
		if (count < sim->data[REQ_CMP])
			return (0);
		i++;
	}
	return (1);
}

static void	set_stop(t_sim *sim, int value)
{
	int	n;
	int	i;

	pthread_mutex_lock(&sim->stop_mutex);
	sim->stop = value;
	pthread_mutex_unlock(&sim->stop_mutex);
	n = sim->data[NBR_CDRS];
	i = 0;
	while (i < n)
	{
		pthread_mutex_lock(&sim->dongles[i].mutex);
		pthread_cond_broadcast(&sim->dongles[i].cond);
		pthread_mutex_unlock(&sim->dongles[i].mutex);
		i++;
	}
}

static int	check_burnout(t_sim *sim)
{
	int		i;
	int		count;
	long	last_comp;

	i = 0;
	while (i < sim->data[NBR_CDRS])
	{
		pthread_mutex_lock(&sim->coders[i].cmutex);
		last_comp = sim->coders[i].last_compile;
		count = sim->coders[i].compile_count;
		pthread_mutex_unlock(&sim->coders[i].cmutex);
		if (count < sim->data[REQ_CMP]
			&& get_time() - last_comp > sim->data[TT_BRNT])
			return (i);
		i++;
	}
	return (-1);
}

void	*monitor_routine(void *arg)
{
	t_sim	*sim;
	int		burned;

	sim = (t_sim *)arg;
	while (1)
	{
		burned = check_burnout(sim);
		if (burned >= 0)
			return (log_action(sim, sim->coders[burned].id,
					"burned out"), set_stop(sim, 2), NULL);
		if (all_complete(sim))
			return (set_stop(sim, 1), NULL);
		usleep(500);
	}
}
