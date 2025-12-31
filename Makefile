NAME = codexion

SRC = coders/main.c \
	coders/args.c \
	coders/init.c \
	coders/coder.c \
	coders/monitor.c \
	coders/dongle.c \
	coders/heap.c \
	coders/time.c \
	coders/log.c

OBJ = $(SRC:.c=.o)

CC = cc
CFLAGS = -Wall -Wextra -Werror -pthread
RM = rm -f

all: $(NAME)

$(NAME): $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) -o $(NAME)

%.o: %.c coders/codexion.h
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	$(RM) $(OBJ)

fclean: clean
	$(RM) $(NAME)

re: fclean all

.PHONY: all clean fclean re
