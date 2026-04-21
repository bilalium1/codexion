/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   monitor.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: blemrabe <blemrabe@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/17 10:46:14 by blemrabe          #+#    #+#             */
/*   Updated: 2026/04/21 15:24:22 by blemrabe         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "coders.h"
#include <pthread.h>

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
				pthread_mutex_lock(&sim->stop_mutex);
				sim->stop = 1;
				pthread_mutex_unlock(&sim->stop_mutex);	
			}
		}
	}
}