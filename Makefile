NAME = ircserv
BOT = ircbot

CPPC = c++
CPPCFLAGS = -Wall -Wextra -Werror -std=c++98

SRCS = src/main.cpp \
       src/Server.cpp \
       src/Client.cpp \
       src/Channel.cpp \
       src/CommandHandler.cpp \
       src/commands/registration.cpp \
       src/commands/channel.cpp \
       src/commands/messaging.cpp \
       src/commands/misc.cpp

OBJS = $(SRCS:.cpp=.o)

BOT_SRC = bot/main.cpp bot/Bot.cpp
BOT_OBJ = $(BOT_SRC:.cpp=.o)

all : $(NAME) $(BOT)

$(NAME) : $(OBJS)
	$(CPPC) $(CPPCFLAGS) $(OBJS) -o $(NAME)

$(BOT): $(BOT_OBJ)
	$(CPPC) $(CPPCFLAGS) $(BOT_OBJ) -o $(BOT)

%.o : %.cpp
	$(CPPC) $(CPPCFLAGS) -c $< -o $@

clean :
	rm -rf $(OBJS) $(BOT_OBJ)

fclean : clean
	rm -rf $(NAME) $(BOT)

re : fclean all

.PHONY : all clean fclean re