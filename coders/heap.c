/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   heap.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: blemrabe <blemrabe@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/17 10:47:19 by blemrabe          #+#    #+#             */
/*   Updated: 2026/04/25 12:13:37 by blemrabe         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "coders.h"

static void	swap_waiters(t_waiter *a, t_waiter *b)
{
	t_waiter	tmp;

	tmp = *a;
	*a = *b;
	*b = tmp;
}

/*
** heap_push_at: sift element at index i upward to restore heap order.
** Used both by heap_push (i = last slot) and by remove_waiter (i = gap).
*/
void	heap_push_at(t_dongle *d, int i)
{
	int	parent;

	while (i > 0)
	{
		parent = (i - 1) / 2;
		if (d->queue[parent].key <= d->queue[i].key)
			break ;
		swap_waiters(&d->queue[parent], &d->queue[i]);
		i = parent;
	}
}

void	heap_push(t_dongle *d, t_waiter *w)
{
	d->queue[d->size] = *w;
	heap_push_at(d, d->size);
	d->size++;
}

void	heap_pop(t_dongle *d)
{
	int	i;
	int	left;
	int	right;
	int	smallest;

	if (d->size == 0)
		return ;
	d->size--;
	d->queue[0] = d->queue[d->size];
	i = 0;
	while (1)
	{
		left = 2 * i + 1;
		right = 2 * i + 2;
		smallest = i;
		if (left < d->size && d->queue[left].key < d->queue[smallest].key)
			smallest = left;
		if (right < d->size && d->queue[right].key < d->queue[smallest].key)
			smallest = right;
		if (smallest == i)
			break ;
		swap_waiters(&d->queue[i], &d->queue[smallest]);
		i = smallest;
	}
}

void	cleanup(t_sim *sim)
{
	int	i;
	int	n;

	n = sim->data[NBR_CDRS];
	i = 0;
	while (i < n)
	{
		pthread_join(sim->coders[i].thread, NULL);
		i++;
	}
	i = 0;
	while (i < n)
	{
		pthread_mutex_destroy(&sim->coders[i].cmutex);
		pthread_cond_destroy(&sim->dongles[i].cond);
		pthread_mutex_destroy(&sim->dongles[i].mutex);
		i++;
	}
	pthread_mutex_destroy(&sim->log_mutex);
	pthread_mutex_destroy(&sim->stop_mutex);
	free(sim->coders);
	free(sim->dongles);
}