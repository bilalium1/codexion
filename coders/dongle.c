/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   dongle.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: blemrabe <blemrabe@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/17 10:47:31 by blemrabe          #+#    #+#             */
/*   Updated: 2026/04/26 12:34:52 by blemrabe         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "coders.h"

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
** take_dongles: acquire both dongles atomically from the coder's perspective.
**
** KEY DESIGN: we compute ONE key for this entire acquisition attempt and
** use it for BOTH dongles. This ensures the coder has the same priority
** position in both heaps — critical for correctness.
**
** acquire_dongle() returns with the dongle mutex LOCKED and in_use=1.
** We keep the first dongle's mutex LOCKED while acquiring the second.
** This means between the two acquisitions:
**   - first dongle: mutex locked, in_use=1  -> fully protected
**   - second dongle: being acquired
** No other coder can slip in and see first as "free" during this window.
**
** After both are acquired, we unlock both and log. Then compile() runs
** and cool_dongles() will re-lock both to release them.
**
** Odd/even ordering prevents circular-wait deadlock:
**   even coders: left then right
**   odd  coders: right then left
*/
int	take_dongles(t_coder *cdr)
{
	t_dongle	*first;
	t_dongle	*second;
	long		key;

	key = get_key(cdr);
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
	if (!acquire_dongle(first, cdr, key))
		return (0);
	if (!acquire_dongle(second, cdr, key))
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
** Dongles are NOT held (mutexes unlocked) at this point — re-lock to release.
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