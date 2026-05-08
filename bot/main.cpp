#include "Bot.hpp"

int isNumber(std::string nbr) {
    for (size_t i = 0; i < nbr.length(); i++)
    {
        if (!isdigit(nbr[i]))
            return (0);
    }
    return (1);
}

int isValidPort(const std::string &prt) {
    if (isNumber(prt)) {
        int port = std::atoi(prt.c_str());
        if (port < 1024 || port > 65535)
            return (0);
    }
    else
        return (0);
    return (1);
}

int hasSpace(const std::string &str) {
    size_t i = 0;
    while (i < str.length())
    {
        if (std::isspace(static_cast<unsigned char>(str[i])))
            return (1);
        i++;
    }
    
    return (0);
}

int main(int ac, char** av)
{
    if (ac != 3)
    {
        std::cout << "/Usage: ./ircbot <port> <password>" << std::endl;
        return (1);
    }

    if (!isValidPort(av[1]))
    {
        std::cerr << "Invalid Port" << std::endl;
        return (1);
    }

    int         port = atoi(av[1]);
    std::string password = av[2];

    if (password.empty() || hasSpace(password)) {
        std::cerr << "Invalid Password" << std::endl;
        return (1);
    }

    Bot bot("127.0.0.1", port, password);

    bot.connectToServer();
    bot.registerBot();
    bot.run();

    return 0;
}