/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   dongle.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: blemrabe <blemrabe@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/17 10:47:31 by blemrabe          #+#    #+#             */
/*   Updated: 2026/06/17 14:00:00 by blemrabe         ###   ########.fr       */
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

static void	dq_enqueue(t_dongle *d, t_coder *cdr)
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

static void	dq_remove(t_dongle *d, t_coder *cdr)
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

static void	wait_for_turn(t_dongle *d)
{
	struct timespec	ts;

	if (d->available_at > get_time())
	{
		ts.tv_sec = d->available_at / 1000;
		ts.tv_nsec = (d->available_at % 1000) * 1000000L;
		pthread_cond_timedwait(&d->cond, &d->mutex, &ts);
	}
	else
		pthread_cond_wait(&d->cond, &d->mutex);
}

int	request_dongle(t_dongle *d, t_coder *cdr)
{
	pthread_mutex_lock(&d->mutex);
	dq_enqueue(d, cdr);
	while (1)
	{
		if (is_stopped(cdr->sim))
		{
			dq_remove(d, cdr);
			pthread_mutex_unlock(&d->mutex);
			return (0);
		}
		if (!d->in_use && get_time() >= d->available_at && d->queue[0] == cdr)
			break ;
		wait_for_turn(d);
	}
	d->queue[0] = d->queue[1];
	d->queue[1] = NULL;
	d->size--;
	d->in_use = 1;
	return (1);
}
