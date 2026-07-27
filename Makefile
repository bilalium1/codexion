NAME=codexion
CC=cc
SRCS=coders/coder_routine.c coders/parser.c coders/monitor.c coders/dongle.c coders/dongle2.c coders/utils.c coders/utils2.c coders/init.c coders/main.c
CFLAGS= -Wall -Wextra -Werror
all : $(NAME)

$(NAME): $(SRCS)
	$(CC) $^  -pthread -o $@

clean :
	rm $(NAME)

fclean : clean

re : clean all

.PHONE : all clean fclean re
