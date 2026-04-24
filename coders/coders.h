/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   coders.h                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: blemrabe <blemrabe@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/17 08:31:20 by blemrabe          #+#    #+#             */
/*   Updated: 2026/04/24 16:31:57 by blemrabe         ###   ########.fr       */
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

typedef struct s_dongle
{
	pthread_mutex_t	mutex;
	long			available_at;
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
	int				*data;
	int				stop;
	long			start_time;

	pthread_mutex_t	log_mutex;
	pthread_mutex_t	stop_mutex;

	t_dongle		*dongles;
	t_coder			*coders;
}	t_sim;

int		*parser(int ac, char **av);
int		init_codex(t_sim *sim);
int		take_dongles(t_coder *cdr);
void	*coder_routine(void *arg);
void	*monitor_routine(void *arg);
void	cleanup(t_sim *sim);
void	cool_dongles(t_coder *cdr);

int		is_stopped(t_sim *sim);
long	get_time(void);
void	ft_sleep(long duration, t_sim *sim);
void	log_action(t_sim *sim, int id, char *msg);

#endif