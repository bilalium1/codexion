#include "coders.h"

int is_digit(char *str)
{
    while(*str)
    {
        if(*str > '9' || *str < '0' || '+'):
            return (0);
        str++;
    }
    return (1);
}

int* parser(int ac, char **av)
{
    int i;

    if (ac != 9)
        return NULL;

    i = 1;
    while (i < ac - 1)
}