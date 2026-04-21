/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   coder_routine.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: blemrabe <blemrabe@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/17 10:47:52 by blemrabe          #+#    #+#             */
/*   Updated: 2026/04/21 15:17:15 by blemrabe         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "coders.h"

static int	take_dongles(t_coder *cdr)
{
	pthread_mutex_lock(&cdr->left->mutex);
	if (is_stopped(cdr->sim))
	{
		pthread_mutex_unlock(&cdr->left->mutex);
		return (0);
	}
	pthread_mutex_lock(&cdr->right->mutex);
	if (is_stopped(cdr->sim))
	{
		pthread_mutex_unlock(&cdr->right->mutex);
		pthread_mutex_unlock(&cdr->left->mutex);
		return (0);
	}
	return (1);
}

static void	compile(t_coder *cdr)
{
	cdr->last_compile = get_time();
	cdr->compile_count++;
	log_action(cdr->sim, cdr->id, "is compiling...");
	ft_sleep(cdr->sim->data[TT_CMPL], cdr->sim);
}

static void	post_compile(t_coder *cdr)
{
	pthread_mutex_unlock(&cdr->left->mutex);
	pthread_mutex_unlock(&cdr->right->mutex);
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
