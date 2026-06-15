/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   dongle.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: blemrabe <blemrabe@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/17 10:47:31 by blemrabe          #+#    #+#             */
/*   Updated: 2026/06/15 00:08:43 by blemrabe         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "coders.h"

/*
** DESIGN (no keys, no heap)
**
** Each dongle has exactly two possible claimants: its left neighbour
** and its right neighbour. So queue[2] is enough — no priority queue
** needed, just a 2-slot array kept in priority order.
**
** priority_of(cdr): the value used to decide ordering.
**   FIFO: arrival time at the dongle (we use get_time() when we enqueue)
**   EDF : burnout deadline = last_compile + TT_BRNT (smaller = more urgent)
** Smaller value = higher priority, in both cases.
**
** enqueue(d, cdr): insert cdr into d->queue, keeping queue[0] as the
** highest-priority (smallest value) entry. Insertion sort on 2 elements
** is just one comparison.
**
** request_dongle(d, cdr): enqueue, then sleep until:
**   - cdr is queue[0]
**   - !in_use
**   - cooldown has passed
** Returns with d->mutex LOCKED on success, UNLOCKED if sim stopped.
**
** dequeue_front(d): shift queue[1] -> queue[0], decrement size.
**
** release_dongle(d, cooldown): mark free, set cooldown, broadcast.
*/

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

/*
** enqueue: place cdr into d->queue[], ordered by priority.
** Must be called with d->mutex LOCKED. d->size must be < 2.
*/
static void	enqueue(t_dongle *d, t_coder *cdr)
{
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

/*
** remove_from_queue: take cdr out of d->queue[] (used when the sim
** stops while cdr was still waiting). Shifts queue[1] into queue[0]
** if cdr was at the front.
*/
static void	remove_from_queue(t_dongle *d, t_coder *cdr)
{
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

/*
** dequeue_front: the coder at queue[0] has claimed the dongle.
** Shift queue[1] into queue[0] (if any) and shrink size.
*/
static void	dequeue_front(t_dongle *d)
{
	d->queue[0] = d->queue[1];
	d->queue[1] = NULL;
	d->size--;
}

/*
** timed_wait_until: sleep on d->cond until woken or 'until_ms' elapses.
** Needed so cooldown expiry self-wakes a waiter even if no further
** broadcast occurs. Called with d->mutex LOCKED.
*/
static void	timed_wait_until(t_dongle *d, long until_ms)
{
	struct timespec	ts;

	ts.tv_sec = until_ms / 1000;
	ts.tv_nsec = (until_ms % 1000) * 1000000L;
	pthread_cond_timedwait(&d->cond, &d->mutex, &ts);
}

/*
** request_dongle: enqueue cdr and sleep until it is granted the dongle.
**
** Returns 1 with d->mutex LOCKED  (caller holds the dongle).
** Returns 0 with d->mutex UNLOCKED (sim stopped while waiting).
*/
static int	request_dongle(t_dongle *d, t_coder *cdr)
{
	pthread_mutex_lock(&d->mutex);
	enqueue(d, cdr);
	while (1)
	{
		if (is_stopped(cdr->sim))
		{
			remove_from_queue(d, cdr);
			pthread_mutex_unlock(&d->mutex);
			return (0);
		}
		if (!d->in_use && get_time() >= d->available_at
			&& d->queue[0] == cdr)
			break ;
		if (d->available_at > get_time())
			timed_wait_until(d, d->available_at);
		else
			pthread_cond_wait(&d->cond, &d->mutex);
	}
	dequeue_front(d);
	d->in_use = 1;
	return (1);
}

/*
** release_dongle: free the dongle, apply cooldown, wake all waiters.
** Must be called with d->mutex LOCKED. Unlocks before returning.
*/
static void	release_dongle(t_dongle *d, int cooldown)
{
	d->in_use = 0;
	d->available_at = get_time() + cooldown;
	pthread_cond_broadcast(&d->cond);
	pthread_mutex_unlock(&d->mutex);
}

/*
** take_dongles: acquire both of cdr's dongles in deadlock-safe order.
**
** request_dongle() returns with the dongle mutex LOCKED. We keep the
** first dongle's mutex locked while acquiring the second so no other
** coder can see it "free" mid-acquisition (the phantom-lock problem).
**
** Odd/even ordering prevents circular-wait deadlock:
**   even coders: left then right
**   odd  coders: right then left
*/
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

/*
** cool_dongles: release both dongles with cooldown after compile().
** Dongles are NOT held (mutexes unlocked) here — re-lock to release.
*/
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