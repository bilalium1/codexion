/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   dongle2.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: blemrabe <blemrabe@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/17 14:00:00 by blemrabe          #+#    #+#             */
/*   Updated: 2026/06/17 14:00:00 by blemrabe         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "coders.h"

void	release_dongle(t_dongle *d, int cooldown)
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
	if (!request_dongle(first, cdr))
		return (0);
	if (!request_dongle(second, cdr))
		return (release_dongle(first, cdr->sim->data[CLDOWN]), 0);
	pthread_mutex_unlock(&first->mutex);
	pthread_mutex_unlock(&second->mutex);
	log_action(cdr->sim, cdr->id, "has taken two dongles");
	return (1);
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
