/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   coders.h                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: blemrabe <blemrabe@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/17 08:31:20 by blemrabe          #+#    #+#             */
/*   Updated: 2026/06/15 01:31:46 by blemrabe         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CODERS_H
# define CODERS_H

# include <stdlib.h>
# include <stdio.h>
# include <unistd.h>
# include <pthread.h>
# include <string.h>
# include <sys/time.h>

# define NBR_CDRS	0
# define TT_BRNT	1
# define TT_CMPL	2
# define TT_DEBG	3
# define TT_RFCT	4
# define REQ_CMP	5
# define CLDOWN		6
# define SCH		7

/*
** t_dongle: shared U
SB dongle.
**
** Each dongle has a queue of AT MOST 2 coders (its only two neighbours
** can ever want it). No heap, no priority keys — the queue is kept
** ordered at insertion time by request_dongle().
**
**   queue[0] = the coder currently entitled to take the dongle next
**              (or currently holding it, if in_use == 1)
**   queue[1] = the other neighbour, waiting behind queue[0]
**   size     = 0, 1, or 2 -> how many of the two slots are occupied
*/
typedef struct s_dongle
{
	pthread_mutex_t	mutex;
	pthread_cond_t	cond;
	int				in_use;
	long			available_at;
	struct s_coder	*queue[2];
	int				size;
}	t_dongle;

typedef struct s_coder
{
	int				id;
	int				compile_count;
	long			last_compile;
	pthread_mutex_t	cmutex;
	pthread_t		thread;
	t_dongle		*left;
	t_dongle		*right;
	struct s_sim	*sim;
}	t_coder;

typedef struct s_sim
{
	int				*data;	/* parsed argv */
	int				stop;	/* stop flag, guarded by stop_mutex */
	long			st;		/* simulation start time */
	pthread_mutex_t	log_mutex;
	pthread_mutex_t	stop_mutex;
	t_dongle		*dongles;
	t_coder			*coders;
}	t_sim;

/* parser / init / cleanup */
int		*parser(int ac, char **av);
int		init_codex(t_sim *sim);
void	cleanup(t_sim *sim);

/* dongle */
int		take_dongles(t_coder *cdr);
void	cool_dongles(t_coder *cdr);

/* threads */
void	*coder_routine(void *arg);
void	*monitor_routine(void *arg);

/* utils */
int		is_stopped(t_sim *sim);
long	get_time(void);
void	ft_sleep(long duration, t_sim *sim);
void	log_action(t_sim *sim, int id, char *msg);

#endif