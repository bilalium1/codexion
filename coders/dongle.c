/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   dongle.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: blemrabe <blemrabe@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/17 10:47:31 by blemrabe          #+#    #+#             */
/*   Updated: 2026/04/25 12:12:37 by blemrabe         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "coders.h"

static void	release_dongle(t_dongle *d, int cooldown)
{
	d->in_use = 0;
	d->available_at = get_time() + cooldown;
	pthread_cond_broadcast(&d->cond);
	pthread_mutex_unlock(&d->mutex);
}

int	take_dongles(t_coder *cdr)
{
	t_dongle	*first;
	t_dongle	*second;

	if (cdr->id % 2 == 0)
	{
		first = cdr->left;
		second = cdr->right;
	}
	else
	{
		first = cdr->right;
		second = cdr->left;
	}
	if (!acquire_dongle(first, cdr))
		return (0);
	log_action(cdr->sim, cdr->id, "has taken a dongle");
	pthread_mutex_unlock(&first->mutex);
	if (!acquire_dongle(second, cdr))
	{
		pthread_mutex_lock(&first->mutex);
		release_dongle(first, cdr->sim->data[CLDOWN]);
		return (0);
	}
	log_action(cdr->sim, cdr->id, "has taken a dongle");
	return (pthread_mutex_unlock(&second->mutex), 1);
}

void	cool_dongles(t_coder *cdr)
{
	int	cooldown;

	cooldown = cdr->sim->data[CLDOWN];
	if (cdr->id % 2 == 0)
	{
		pthread_mutex_lock(&cdr->right->mutex);
		release_dongle(cdr->right, cooldown);
		pthread_mutex_lock(&cdr->left->mutex);
		release_dongle(cdr->left, cooldown);
	}
	else
	{
		pthread_mutex_lock(&cdr->left->mutex);
		release_dongle(cdr->left, cooldown);
		pthread_mutex_lock(&cdr->right->mutex);
		release_dongle(cdr->right, cooldown);
	}
}