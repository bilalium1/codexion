NAME=codexion
CC=cc
SRCS=coders/coder_routine.c coders/parser.c coders/monitor.c coders/heap.c coders/dongle.c coders/utils.c coders/scheduler.c coders/init.c coders/main.c coders/sim.c
CFLAGS= -Wall -Wextra -Werror
all : $(NAME)

$(NAME): $(SRCS)
	$(CC) $^ $(CFLAGS) -pthread -o $@

clean :
	rm $(NAME)

fclean : clean

re : clean all

run : $(NAME)
	./$(NAME) 5 800 200 200 200 4 20 fifo

.PHONE : all clean fclean re
