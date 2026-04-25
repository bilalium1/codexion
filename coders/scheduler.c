/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   scheduler.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: blemrabe <blemrabe@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/17 10:46:25 by blemrabe          #+#    #+#             */
/*   Updated: 2026/04/25 12:17:40 by blemrabe         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "coders.h"

/*
** Why one shared cond per dongle instead of one per waiter:
**
** pthread_cond_t must never be copied after pthread_cond_init().
** The heap swaps t_waiter entries — if cond were inside t_waiter, every
** swap would copy an initialised cond_t (undefined behaviour: the internal
** futex address becomes stale). One shared d->cond avoids this entirely.
**
** On release: pthread_cond_broadcast wakes ALL waiters. Each re-checks
** whether it is at heap[0] and the dongle is free. Only one passes;
** the rest loop back into cond_wait. Standard Mesa-style monitor.
*/

static long	get_key(t_coder *cdr)
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

static int	find_my_slot(t_dongle *d, long key)
{
	int	i;

	i = 0;
	while (i < d->size)
	{
		if (d->queue[i].key == key)
			return (i);
		i++;
	}
	return (-1);
}

static void	remove_my_slot(t_dongle *d, long key)
{
	int	i;

	i = find_my_slot(d, key);
	if (i < 0)
		return ;
	d->queue[i] = d->queue[d->size - 1];
	d->size--;
	heap_push_at(d, i);
}

/*
** acquire_dongle: enqueue, sleep, and claim the dongle when granted.
**
** Called with d->mutex UNLOCKED.
** Returns 1 with d->mutex LOCKED  (caller holds dongle, in_use == 1).
** Returns 0 with d->mutex UNLOCKED (sim stopped while waiting).
*/
int	acquire_dongle(t_dongle *d, t_coder *cdr)
{
	t_waiter	w;
	int			slot;

	w.key = get_key(cdr);
	pthread_mutex_lock(&d->mutex);
	heap_push(d, &w);
	while (1)
	{
		if (is_stopped(cdr->sim))
		{
			remove_my_slot(d, w.key);
			pthread_mutex_unlock(&d->mutex);
			return (0);
		}
		slot = find_my_slot(d, w.key);
		if (!d->in_use && get_time() >= d->available_at && slot == 0)
			break ;
		pthread_cond_wait(&d->cond, &d->mutex);
	}
	heap_pop(d);
	d->in_use = 1;
	return (1);
}