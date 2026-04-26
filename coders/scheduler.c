/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   scheduler.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: blemrabe <blemrabe@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/17 10:46:25 by blemrabe          #+#    #+#             */
/*   Updated: 2026/04/26 12:32:18 by blemrabe         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "coders.h"

/*
** DESIGN NOTES:
**
** 1. ONE shared cond per dongle (d->cond).
**    pthread_cond_t must never be copied — embedding it in t_waiter and
**    swapping entries in the heap is undefined behaviour. One shared cond
**    sidesteps this entirely.
**
** 2. acquire_dongle() returns with d->mutex LOCKED.
**    take_dongles() holds the first dongle's mutex while acquiring the second.
**    This eliminates the "phantom lock" bug where in_use=1 but mutex unlocked
**    caused other coders to queue on a dongle that would never be released
**    (because cool_dongles only fires after compile(), long after take_dongles).
**
** 3. Cooldown handled with pthread_cond_timedwait.
**    release_dongle() broadcasts immediately after setting available_at.
**    Waiters that wake before available_at use timedwait to sleep until
**    exactly available_at — so they don't miss the window when no further
**    broadcast will come. Plain cond_wait + broadcast-only would leave
**    a waiter sleeping forever if it wakes during the cooldown window
**    and no subsequent release triggers another broadcast.
**
** 4. Key computed once per take_dongles() call and shared for both dongles.
**    Recomputing gives different timestamps -> different heap positions ->
**    incorrect priority and potential collision in find_my_slot().
**
** 5. Key = timestamp * 1000 + coder_id for uniqueness.
**    Without the id suffix, two coders at the same millisecond get equal keys,
**    find_my_slot() may return the wrong slot, and the heap check slot==0
**    can pass for the wrong coder.
*/

long	get_key(t_coder *cdr)
{
	long	deadline;

	if (cdr->sim->data[SCH] == 0)
	{
		pthread_mutex_lock(&cdr->cmutex);
		deadline = cdr->last_compile + cdr->sim->data[TT_BRNT];
		pthread_mutex_unlock(&cdr->cmutex);
		return (deadline * 1000 + cdr->id);
	}
	return (get_time() * 1000 + cdr->id);
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
** timed_wait_until: sleep on d->cond until woken or until 'until_ms' elapses.
** Converts a millisecond timestamp to the timespec format required by
** pthread_cond_timedwait. Called with d->mutex LOCKED.
*/
static void	timed_wait_until(t_dongle *d, long until_ms)
{
	struct timespec	ts;

	ts.tv_sec = until_ms / 1000;
	ts.tv_nsec = (until_ms % 1000) * 1000000L;
	pthread_cond_timedwait(&d->cond, &d->mutex, &ts);
}

/*
** acquire_dongle: enqueue on the heap, sleep until granted.
**
** Uses timedwait so that cooldown expiry wakes the coder even when
** no subsequent release fires a broadcast.
**
** Returns 1 with d->mutex LOCKED.
** Returns 0 with d->mutex UNLOCKED (sim stopped).
*/
int	acquire_dongle(t_dongle *d, t_coder *cdr, long key)
{
	t_waiter	w;
	int			slot;

	w.key = key;
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
		if (d->available_at > get_time())
			timed_wait_until(d, d->available_at);
		else
			pthread_cond_wait(&d->cond, &d->mutex);
	}
	heap_pop(d);
	d->in_use = 1;
	return (1);
}