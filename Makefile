NAME=codexion
CC=cc
SRCS=coders/coder_routine.c coders/parser.c coders/logging.c coders/monitor.c coders/heap.c coders/dongle.c coders/utils.c coders/scheduler.c coders/init.c coders/main.c coders/sim.c
CFLAGS= -Werror -Wall -Wextra

all : $(NAME)

$(NAME): $(SRCS)
	$(CC) $^ -pthread -g -o $@

clean :
	rm $(NAME)

fclean : clean

re : clean all

.PHONE : all clean fclean re
