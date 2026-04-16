#ifndef CODEXION_H
#define CODEXION_H

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>

typedef struct parse_s
{
    int nb_coders;
    int burnout;
    int compile;
    int debug;
    int refactor;
    int cmp_req;
    int cooldown;
    int scheduler;
}   parse_t;

int* parser(int ac, char **av);

#endif CODEXION_H