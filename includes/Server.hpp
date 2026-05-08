#ifndef SERVER_HPP
#define SERVER_HPP

#include <string>
#include <map>
#include <vector>
#include <poll.h>
#include <netinet/in.h>
#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <sstream>
#include <iomanip>
#include <csignal>
#include <cerrno>
#include "Channel.hpp"
#include "CommandHandler.hpp"

class Client;
class Channel;

class Server
{
private:
    int                             _port;
    std::string                     _password;

    int                             _listenFd;
    std::vector<pollfd>             _pollFds;

    std::map<int, Client*>          _clients;

    std::map<std::string, Channel*> _channels;//ilyas
    CommandHandler*                 _commandHandler;//ilyas

    bool                            _running;

public:
    Server(int port, const std::string& password);
    ~Server();

    void setupSocket();
    void start();
    void acceptNewClient();
    void receiveFromClient(int clientFd);
    void processClientBuffer(Client* client);
    void sendToClient(int fd, const std::string &message);
    void sendNumericReply(int fd, int code, const std::string& nick, const std::string& msg);
    void disconnectClient(int fd);
    void stop();
    void cleanup();

    const std::string&               getPassword() const;//ilyas
    Client*                          getClientByNick(const std::string& nick);//ilyas
    std::map<int, Client*>&          getClients();//ilyas
    Channel*                         getChannel(const std::string& name);//ilyas
    Channel*                         createChannel(const std::string& name, Client* creator);//ilyas
    void                             removeChannel(const std::string& name);//ilyas
    std::map<std::string, Channel*>& getChannels();//ilyas
};

#endif
