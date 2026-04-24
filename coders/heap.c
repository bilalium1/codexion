/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   heap.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: blemrabe <blemrabe@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/17 10:47:19 by blemrabe          #+#    #+#             */
/*   Updated: 2026/04/24 16:27:44 by blemrabe         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "coders.h"
#include <pthread.h>

void	cleanup(t_sim *sim)
{
	int	i;
	int	n;

	n = sim->data[NBR_CDRS];
	i = 0;
	while (i < n)
	{
		pthread_join(sim->coders[i].thread, NULL);
		i++;
	}
	i = 0;
	while (i < n)
	{
		pthread_mutex_destroy(&sim->dongles[i].mutex);
		pthread_mutex_destroy(&sim->coders[i].cmutex);
		i++;
	}
	pthread_mutex_destroy(&sim->log_mutex);
	pthread_mutex_destroy(&sim->stop_mutex);
	free(sim->coders);
	free(sim->dongles);
}
