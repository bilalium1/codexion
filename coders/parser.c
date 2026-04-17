/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parser.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: blemrabe <blemrabe@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/17 08:30:36 by blemrabe          #+#    #+#             */
/*   Updated: 2026/04/17 09:35:09 by blemrabe         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "coders.h"

static int	is_digit(char *str)
{
	while (*str)
	{
		if (*str > '9' || *str < '0' || *str == '-')
			return (0);
		str++;
	}
	return (1);
}

int	*parser(int ac, char **av)
{
	int	i;
	int	*parsed;

	if (ac != 9)
		return (NULL);

	i = 1;
	while (i < ac - 1)
	{
		if (!is_digit(av[i]))
			return (NULL);
		i++;
	}
	if (strcmp(av[8], "fifo") && strcmp(av[8], "edf"))
		return (NULL);

	parsed = malloc(32);
	i = 0;
	while (i < 7)
	{
		parsed[i] = atoi(av[i + 1]);
		i++;
	}
	parsed[7] = strcmp(av[8], "edf");
	return (parsed);
}
