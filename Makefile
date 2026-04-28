NAME = ircserv

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

all : $(NAME)

$(NAME) : $(OBJS)
	$(CPPC) $(CPPCFLAGS) $(OBJS) -o $(NAME)

%.o : %.cpp
	$(CPPC) $(CPPCFLAGS) -c $< -o $@

clean :
	rm -rf $(OBJS)

fclean : clean
	rm -rf $(NAME)

re : fclean all

.PHONY : all clean fclean re