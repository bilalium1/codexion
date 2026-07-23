/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   dongle.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: blemrabe <blemrabe@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/17 10:47:31 by blemrabe          #+#    #+#             */
/*   Updated: 2026/06/17 10:22:54 by blemrabe         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "coders.h"

static long	priority_of(t_coder *cdr)
{
	long	deadline;

	if (cdr->sim->data[SCH] == 0)
	{
		pthread_mutex_lock(&cdr->cmutex);
		deadline = cdr->last_compile + cdr->sim->data[TT_BRNT];
		pthread_mutex_unlock(&cdr->cmutex);
		return (deadline);
	}
	return (get_time());
}

static void	enqueue(t_dongle *d, t_coder *cdr)
{
	int	dongle_id;

	dongle_id = (int)(d - cdr->sim->dongles);
	if (d->size == 0)
		d->queue[0] = cdr;
	else if (priority_of(cdr) < priority_of(d->queue[0]))
	{
		d->queue[1] = d->queue[0];
		d->queue[0] = cdr;
	}
	else
		d->queue[1] = cdr;
	d->size++;
}

static void	remove_from_queue(t_dongle *d, t_coder *cdr, int f)
{
	if (f)
	{
		d->queue[0] = d->queue[1];
		d->queue[1] = NULL;
		d->size--;
		return;
	}
	if (d->size == 0)
		return ;
	if (d->queue[0] == cdr)
	{
		d->queue[0] = d->queue[1];
		d->size--;
	}
	else if (d->size == 2 && d->queue[1] == cdr)
		d->size--;
}

static void	timed_wait_until(t_dongle *d, long until_ms)
{
	struct timespec	ts;

	ts.tv_sec = until_ms / 1000;
	ts.tv_nsec = (until_ms % 1000) * 1000000L;
	pthread_cond_timedwait(&d->cond, &d->mutex, &ts);
}

static int	request_dongle(t_dongle *d, t_coder *cdr)
{
	int	dongle_id;

	dongle_id = (int)(d - cdr->sim->dongles);
	pthread_mutex_lock(&d->mutex);
	enqueue(d, cdr);
	while (1)
	{
		if (is_stopped(cdr->sim))
		{
			remove_from_queue(d, cdr, 0);
			pthread_mutex_unlock(&d->mutex);
			return (0);
		}
		if (!d->in_use && get_time() >= d->available_at && d->queue[0] == cdr)
			break ;
		if (d->available_at > get_time())
			timed_wait_until(d, d->available_at);
		else
			pthread_cond_wait(&d->cond, &d->mutex);
	}
	//printf("[DONGLE %d] GRANTED to coder %d\n", dongle_id, cdr->id + 1);
	remove_from_queue(d, NULL, 1);
	d->in_use = 1;
	return (1);
}

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
	if (!request_dongle(first, cdr))
		return (0);
	if (!request_dongle(second, cdr))
	{
		release_dongle(first, cdr->sim->data[CLDOWN]);
		return (0);
	}
	pthread_mutex_unlock(&first->mutex);
	pthread_mutex_unlock(&second->mutex);
	log_action(cdr->sim, cdr->id, "has taken a dongle");
	log_action(cdr->sim, cdr->id, "has taken a dongle");
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
