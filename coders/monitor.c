/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   monitor.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: blemrabe <blemrabe@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/17 10:46:14 by blemrabe          #+#    #+#             */
/*   Updated: 2026/04/24 16:28:37 by blemrabe         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "coders.h"
#include <pthread.h>

static int	all_complete(t_sim	*sim)
{
	int	i;
	int count;

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
	pthread_mutex_lock(&sim->stop_mutex);
	sim->stop = value;
	pthread_mutex_unlock(&sim->stop_mutex);
}

void	*monitor_routine(void *arg)
{
	t_sim	*sim;
	int		i;
	int		last_comp;

	sim = (t_sim *)arg;
	while (1)
	{
		i = 0;
		while (i < sim->data[NBR_CDRS])
		{
			pthread_mutex_lock(&sim->coders[i].cmutex);
			last_comp = sim->coders[i].last_compile;
			pthread_mutex_unlock(&sim->coders[i].cmutex);
			if (get_time() - last_comp > sim->data[TT_BRNT])
			{
				log_action(sim, sim->coders[i].id, "burned out");
				set_stop(sim, 1);
				return (NULL);
			}
			i++;
		}
		if (all_complete(sim))
		{
			set_stop(sim, 1);
			return (NULL);
		}
		usleep(500);
	}
}
