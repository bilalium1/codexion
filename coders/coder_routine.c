/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   coder_routine.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: blemrabe <blemrabe@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/17 10:47:52 by blemrabe          #+#    #+#             */
/*   Updated: 2026/04/26 12:50:35 by blemrabe         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "coders.h"
#include <pthread.h>

static void	compile(t_coder *cdr)
{
	pthread_mutex_lock(&cdr->cmutex);
	cdr->last_compile = get_time();
	cdr->compile_count++;
	pthread_mutex_unlock(&cdr->cmutex);
	log_action(cdr->sim, cdr->id, "is compiling");
	ft_sleep(cdr->sim->data[TT_CMPL], cdr->sim);
	cool_dongles(cdr);
}

static void	post_compile(t_coder *cdr)
{
	log_action(cdr->sim, cdr->id, "is debugging");
	ft_sleep(cdr->sim->data[TT_DEBG], cdr->sim);
	log_action(cdr->sim, cdr->id, "is refactoring");
	ft_sleep(cdr->sim->data[TT_RFCT], cdr->sim);
}

static void	*handle_single(t_coder *cdr)
{
	log_action(cdr->sim, cdr->id, "is waiting");
	ft_sleep(cdr->sim->data[TT_BRNT], cdr->sim);
	return (NULL);
}

void	*coder_routine(void *arg)
{
	t_coder	*cdr;

	cdr = (t_coder *)arg;
	pthread_mutex_lock(&cdr->cmutex);
	cdr->last_compile = get_time();
	pthread_mutex_unlock(&cdr->cmutex);
	if (cdr->sim->data[NBR_CDRS] == 1)
		return (handle_single(cdr));
	while (!is_stopped(cdr->sim))
	{
		if (!take_dongles(cdr))
			break ;
		compile(cdr);
		post_compile(cdr);
	}
	return (NULL);
}
