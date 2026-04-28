#include "../includes/Server.hpp"
#include "../includes/Client.hpp"
#include "../includes/Channel.hpp"
#include "../includes/CommandHandler.hpp"

Server::Server(int port, const std::string& password)
    : _port(port),
      _password(password),
      _listenFd(-1)
{
    _commandHandler = new CommandHandler(*this);//ilyas
}

Server::~Server()
{
    delete _commandHandler;//ilyas
    for (std::map<int, Client*>::iterator it = _clients.begin();
         it != _clients.end(); ++it)
    {
        close(it->first);
        delete it->second;
    }
    _clients.clear();

    for (std::map<std::string, Channel*>::iterator it = _channels.begin();//ilyas
         it != _channels.end(); ++it)
        delete it->second;
    _channels.clear();

    if (_listenFd != -1)
        close(_listenFd);
}

void Server::setupSocket()
{
    _listenFd = socket(AF_INET, SOCK_STREAM, 0);
    if (_listenFd < 0)
    {
        std::cerr << "socket() failed\n";
        exit(1);
    }

    // Allow fast restart
    int opt = 1;
    if (setsockopt(_listenFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
    {
        std::cerr << "setsockopt() failed\n";
        exit(1);
    }

    sockaddr_in addr;
    std::memset(&addr, 0, sizeof(addr));

    addr.sin_family = AF_INET;
    addr.sin_port = htons(_port);
    addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(_listenFd, (sockaddr*)&addr, sizeof(addr)) < 0)
    {
        std::cerr << "bind() failed\n";
        exit(1);
    }

    if (listen(_listenFd, SOMAXCONN) < 0)
    {
        std::cerr << "listen() failed\n";
        exit(1);
    }

    pollfd pfd;
    pfd.fd = _listenFd;
    pfd.events = POLLIN;
    pfd.revents = 0;

    _pollFds.push_back(pfd);

    std::cout << "Server listening on port " << _port << std::endl;
}

void Server::acceptNewClient()
{
    sockaddr_in clientAddr;
    socklen_t addrLen = sizeof(clientAddr);

    int clientFd = accept(_listenFd, (sockaddr*)&clientAddr, &addrLen);
    if (clientFd < 0)
    {
        std::cerr << "accept() failed\n";
        return;
    }

    Client* newClient = new Client(clientFd);

    //newClient->setRegistered(true); //ilyas

    _clients[clientFd] = newClient;

    pollfd pfd;
    pfd.fd = clientFd;
    pfd.events = POLLIN;
    pfd.revents = 0;

    _pollFds.push_back(pfd);

    std::cout << "New client connected (fd = "
              << clientFd << ")\n";
}

void Server::processClientBuffer(Client* client)
{
    std::string& buffer = client->getBuffer();

    size_t pos;

    while ((pos = buffer.find("\r\n")) != std::string::npos)
    {
        std::string message = buffer.substr(0, pos);

        buffer.erase(0, pos + 2);

        std::cout << "Complete message: " << message << std::endl;

        _commandHandler->handle(client, message);//ilyas
    }
}

void Server::disconnectClient(int fd)
{
    std::cout << "Client disconnected (fd = " << fd << ")" << std::endl;

    for (size_t i = 0; i < _pollFds.size(); ++i)
    {
        if (_pollFds[i].fd == fd)
        {
            _pollFds.erase(_pollFds.begin() + i);
            break;
        }
    }

    std::map<int, Client*>::iterator it = _clients.find(fd);
    if (it != _clients.end())
    {
        delete it->second;
        _clients.erase(it);
    }

    close(fd);
}

void Server::receiveFromClient(int clientFd)
{
    char buffer[1024];

    ssize_t bytesRead = recv(clientFd, buffer, sizeof(buffer) - 1, 0);

    if (bytesRead <= 0)
    {
        disconnectClient(clientFd);
        return;
    }

    buffer[bytesRead] = '\0';

    std::string data(buffer);

    std::cout << "Received from fd " << clientFd << ": " << data;

    std::map<int, Client*>::iterator it = _clients.find(clientFd);
    if (it != _clients.end())
    {
        it->second->getBuffer() += data;
        processClientBuffer(it->second);
    }
}

void Server::sendToClient(int fd, const std::string& message)
{
    std::string fullMsg = message + "\r\n";

    ssize_t sent = send(fd, fullMsg.c_str(), fullMsg.size(), 0);

    if (sent < 0)
    {
        std::cerr << "send() failed for fd " << fd << std::endl;
        disconnectClient(fd);
    }
}

void Server::sendNumericReply(int fd, int code,
                             const std::string& nick,
                             const std::string& msg)
{
    std::ostringstream oss;

    oss << ":ircserv "
        << std::setw(3) << std::setfill('0') << code
        << " " << nick
        << " :" << msg;

    sendToClient(fd, oss.str());
}

void Server::start()
{
    while (true)
    {
        int ready = poll(&_pollFds[0], _pollFds.size(), -1);
        if (ready < 0)
        {
            std::cerr << "poll() error\n";
            continue;
        }

        size_t count = _pollFds.size();

        for (size_t i = 0; i < count; ++i)
        {
            if (_pollFds[i].revents == 0)
                continue;

            int fd = _pollFds[i].fd;

            if (fd == _listenFd)
            {
                acceptNewClient();
            }
            else
            {
                if (_pollFds[i].revents & POLLIN)
                    receiveFromClient(fd);
            }
        }
    }
}
//ilyas all of this
const std::string& Server::getPassword() const {
    return _password;
}

std::map<int, Client*>& Server::getClients() {
    return _clients;
}

Client* Server::getClientByNick(const std::string& nick) {
    for (std::map<int, Client*>::iterator it = _clients.begin();
         it != _clients.end(); ++it) {
        if (it->second->getNickname() == nick)
            return it->second;
    }
    return NULL;
}

Channel* Server::getChannel(const std::string& name) {
    std::map<std::string, Channel*>::iterator it = _channels.find(name);
    return it != _channels.end() ? it->second : NULL;
}

Channel* Server::createChannel(const std::string& name, Client* creator) {
    Channel* ch = new Channel(name, *this);
    ch->addMember(creator);
    ch->addOperator(creator->getFd());
    _channels[name] = ch;
    return ch;
}

void Server::removeChannel(const std::string& name) {
    std::map<std::string, Channel*>::iterator it = _channels.find(name);
    if (it != _channels.end()) {
        delete it->second;
        _channels.erase(it);
    }
}

std::map<std::string, Channel*>& Server::getChannels() {
    return _channels;
}