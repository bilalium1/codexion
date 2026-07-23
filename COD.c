/* MAIN OBJECTIVE:

    WRITE CODEXION IN ONE FILE
 */

#include "coders/coders.h"
#include <string.h>

int is_digit(char *s)
{
    while (*s)
    {
        if (*s < '0' || *s > '9')
            return 0;
    }
    return 1;
}

int* parser(int ac, char **av){
    int i;
    int *parsed;

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
        return(NULL);
    parsed = malloc(32);
    i = 0;
    while (i < 7)
    {
        parsed[i] = atoi(av[i + 1]);
        i++;
    }
    if (parsed[5] == 0 || parsed[0] == 0)
        return (0);
}

int main(int ac, char **av)
{
    // PARSING WORKS
    if (ac < 9)
    {
        printf("wrong args");
        return 0;
    }
    int *info_table = parser(ac, av);

    int i = 0;
    while (i < 8){
        printf("info : %d\n", info_table[i]);
        i++;
    }


}
