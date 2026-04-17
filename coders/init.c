/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   init.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: blemrabe <blemrabe@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/17 10:47:06 by blemrabe          #+#    #+#             */
/*   Updated: 2026/04/17 18:18:37 by blemrabe         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "coders.h"
#include <pthread.h>

void	init_codex(t_sim *sim, int *info)
{
	int	i;

	i = 0;
	sim->coders = malloc(info[NBR_CDRS] * sizeof(t_coder));
	sim->dongles = malloc(info[NBR_CDRS] * sizeof(t_dongle));
	while (i < info[NBR_CDRS])
	{
		sim->coders[i].id = i;
		sim->coders[i].compile_count = 0;
		sim->coders[i].last_compile = 0;
		sim->coders[i].right = NULL;
		sim->coders[i].left = NULL;
		sim->coders[i].sim = sim;
		pthread_create(&sim->coders[i].thread, NULL, coder_routine, NULL);
	}
}
