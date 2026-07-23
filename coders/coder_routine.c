/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   coder_routine.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: blemrabe <blemrabe@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/17 10:47:52 by blemrabe          #+#    #+#             */
/*   Updated: 2026/06/15 02:12:08 by blemrabe         ###   ########.fr       */
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
	if (is_stopped(cdr->sim))
	{
		cool_dongles(cdr);
		return ;
	}
	log_action(cdr->sim, cdr->id, "is compiling");
	ft_sleep(cdr->sim->data[TT_CMPL], cdr->sim);
	cool_dongles(cdr);
}

static void	post_compile(t_coder *cdr)
{
	if (is_stopped(cdr->sim) == 2)
		return;
	log_action(cdr->sim, cdr->id, "is debugging");
	ft_sleep(cdr->sim->data[TT_DEBG], cdr->sim);
	if (is_stopped(cdr->sim) == 2)
		return;
	log_action(cdr->sim, cdr->id, "is refactoring");
	ft_sleep(cdr->sim->data[TT_RFCT], cdr->sim);
}

static void	*handle_single(t_coder *cdr)
{
	log_action(cdr->sim, cdr->id, "is waiting");
	ft_sleep(cdr->sim->data[TT_BRNT], cdr->sim);
	return (NULL);
}

static int done_compiling(t_coder *cdr)
{
    int compiles = 0;
    pthread_mutex_lock(&cdr->cmutex);
    compiles = cdr->compile_count;
    pthread_mutex_unlock(&cdr->cmutex);
    return (compiles >= cdr->sim->data[REQ_CMP]);
}

void	*coder_routine(void *arg)
{
	t_coder	*cdr;

	cdr = (t_coder *)arg;
	int compiles = 0;
	pthread_mutex_lock(&cdr->cmutex);
	cdr->last_compile = get_time();
	compiles = cdr->compile_count;
	pthread_mutex_unlock(&cdr->cmutex);
	if (cdr->sim->data[NBR_CDRS] == 1)
		return (handle_single(cdr));
	if (compiles >= cdr->sim->data[REQ_CMP])
	{
        log_action(cdr->sim, cdr->id, "FINISHED 2");
	}
	while (!is_stopped(cdr->sim) && !done_compiling(cdr))
	{
		if (!take_dongles(cdr))
			break ;
		compile(cdr);
		post_compile(cdr);
	}
	pthread_mutex_lock(&cdr->cmutex);
	cdr->last_compile = get_time();
	pthread_mutex_unlock(&cdr->cmutex);
	return (NULL);
}
