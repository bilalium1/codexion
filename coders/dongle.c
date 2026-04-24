/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   dongle.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: blemrabe <blemrabe@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/17 10:47:31 by blemrabe          #+#    #+#             */
/*   Updated: 2026/04/24 15:23:25 by blemrabe         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "coders.h"

static void	wait_for_dongle(t_dongle *dongle, t_sim *sim)
{
	while (!is_stopped(sim))
	{
		if (get_time() >= dongle->available_at)
			break ;
		usleep(500);
	}
}

static int	take_dongles_even(t_coder *cdr)
{
	pthread_mutex_lock(&cdr->left->mutex);
	if (is_stopped(cdr->sim))
		return (pthread_mutex_unlock(&cdr->left->mutex), 0);
	wait_for_dongle(cdr->left, cdr->sim);
	pthread_mutex_lock(&cdr->right->mutex);
	if (is_stopped(cdr->sim))
	{
		pthread_mutex_unlock(&cdr->right->mutex);
		pthread_mutex_unlock(&cdr->left->mutex);
		return (0);
	}
	wait_for_dongle(cdr->right, cdr->sim);
	return (1);
}

static int	take_dongles_odd(t_coder *cdr)
{
	pthread_mutex_lock(&cdr->right->mutex);
	if (is_stopped(cdr->sim))
		return (pthread_mutex_unlock(&cdr->right->mutex), 0);
	wait_for_dongle(cdr->right, cdr->sim);
	pthread_mutex_lock(&cdr->left->mutex);
	if (is_stopped(cdr->sim))
	{
		pthread_mutex_unlock(&cdr->left->mutex);
		pthread_mutex_unlock(&cdr->right->mutex);
		return (0);
	}
	wait_for_dongle(cdr->left, cdr->sim);
	return (1);
}

int	take_dongles(t_coder *cdr)
{
	int	res;

	if (cdr->id % 2 == 0)
		res = take_dongles_even(cdr);
	else
		res = take_dongles_odd(cdr);
	if (res)
	{
		log_action(cdr->sim, cdr->id, "has taken two dongles");
	}
	return (res);
}

void	cool_dongles(t_coder *cdr)
{
	int	cooldown;

	cooldown = cdr->sim->data[CLDOWN];
	if (cdr->id % 2 == 0)
	{
		cdr->right->available_at = get_time() + cooldown;
		pthread_mutex_unlock(&cdr->right->mutex);
		cdr->left->available_at = get_time() + cooldown;
		pthread_mutex_unlock(&cdr->left->mutex);
	}
	else
	{
		cdr->left->available_at = get_time() + cooldown;
		pthread_mutex_unlock(&cdr->left->mutex);
		cdr->right->available_at = get_time() + cooldown;
		pthread_mutex_unlock(&cdr->right->mutex);
	}
}
