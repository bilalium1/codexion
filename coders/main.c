/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: blemrabe <blemrabe@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/17 08:28:01 by blemrabe          #+#    #+#             */
/*   Updated: 2026/04/24 16:05:49 by blemrabe         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "coders.h"
#include <stdio.h>

/*
This file has the main program, this program uses all the other files
to run the program.
*/

static void	display_parsed(int *info)
{
	int		i;
	char	**args;

	args = (char *[]){"number_coders", "time_to_burnout", "time_to_compile",
		"time_to_debug", "time_to_refactor", "number_of_compiles_required",
		"dongle_cooldown", "edf", "fifo"};
	i = 0;
	while (i < 7)
	{
		printf("[-//] %s : %d\n", args[i], info[i]);
		i++;
	}
	printf("[-//] scheduler : %s\n\n", args[7 + info[i]]);
}

int	main(int ac, char **av)
{
	int		*parse_info;
	t_sim	sim;

	printf("\n[+//] WELCOME TO CODEXION\n\n");

	parse_info = parser(ac, av);
	if (!parse_info)
		return (printf("[x//] Wrong Format : Try again."), 0);

	display_parsed(parse_info);
	sim.data = parse_info;

	if (init_codex(&sim) == 0)
		return (printf("[x//] Init Failed. \n"), 1);

	monitor_routine(&sim);
	cleanup(&sim);

	free(parse_info);
	return (0);
}
