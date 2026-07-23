NAME=codexion
CC=cc
SRCS=coders/coder_routine.c coders/parser.c coders/monitor.c coders/dongle.c coders/utils.c coders/utils2.c coders/init.c coders/main.c
CFLAGS= -Wall -Wextra -Werror
all : $(NAME)

$(NAME): $(SRCS)
	$(CC) $^  -pthread -o $@

cod :
	$(CC) coders/coder_routine.c coders/parser.c coders/monitor.c coders/dongle.c coders/utils.c coders/utils2.c coders/init.c COD.c -pthread -o cod
	./cod 1 2 3 4 5 6 7 fifo

clean :
	rm $(NAME) cod

fclean : clean

re : clean all

run : $(NAME)
	./$(NAME) 5 800 200 200 200 4 20 fifo

.PHONE : all clean fclean re
