/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   init.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: blemrabe <blemrabe@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/17 10:47:06 by blemrabe          #+#    #+#             */
/*   Updated: 2026/06/17 09:47:38 by blemrabe         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "coders.h"

static int	init_dongles(t_sim *sim, int n)
{
	int	i;

	i = 0;
	while (i < n)
	{
		pthread_mutex_init(&sim->dongles[i].mutex, NULL);
		pthread_cond_init(&sim->dongles[i].cond, NULL);
		sim->dongles[i].available_at = 0;
		sim->dongles[i].in_use = 0;
		sim->dongles[i].size = 0;
		sim->dongles[i].queue[0] = NULL;
		sim->dongles[i].queue[1] = NULL;
		i++;
	}
	return (1);
}

static int	init_coders(t_sim *sim, int n)
{
	int	i;

	i = 0;
	while (i < n)
	{
		sim->coders[i].id = i;
		sim->coders[i].compile_count = 0;
		sim->coders[i].last_compile = sim->st;
		sim->coders[i].sim = sim;
		sim->coders[i].left = &sim->dongles[i];
		sim->coders[i].right = &sim->dongles[(i + 1) % n];
		pthread_mutex_init(&sim->coders[i].cmutex, NULL);
		i++;
	}
	return (1);
}

int	init_codex(t_sim *sim)
{
	int	n;
	int	i;

	n = sim->data[NBR_CDRS];
	sim->coders = malloc(n * sizeof(t_coder));
	sim->dongles = malloc(n * sizeof(t_dongle));
	if (!sim->coders || !sim->dongles)
		return (0);
	pthread_mutex_init(&sim->log_mutex, NULL);
	pthread_mutex_init(&sim->stop_mutex, NULL);
	sim->st = get_time();
	sim->stop = 0;
	init_dongles(sim, n);
	init_coders(sim, n);
	i = 0;
	while (i < n)
	{
		pthread_create(&sim->coders[i].thread, NULL,
			coder_routine, &sim->coders[i]);
		i++;
	}
	return (1);
}
