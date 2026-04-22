/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   monitor.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: blemrabe <blemrabe@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/17 10:46:14 by blemrabe          #+#    #+#             */
/*   Updated: 2026/04/22 15:15:48 by blemrabe         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "coders.h"
#include <pthread.h>

static int	all_complete(t_sim	*sim)
{
	int	i;

	i = 0;
	while (i < sim->data[NBR_CDRS])
	{
		if (sim->coders[i].compile_count < sim->data[REQ_CMP])
			return (0);
	}
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

	sim = (t_sim *)arg;
	while (1)
	{
		i = 0;
		while (i < sim->data[NBR_CDRS])
		{
			if (get_time() - sim->coders[i].last_compile > sim->data[TT_BRNT])
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
