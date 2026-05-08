#ifndef BOT_HPP
#define BOT_HPP

#include <string>
#include <iostream>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <ctime>
#include <cstdlib>

class Bot
{
private:
    int         _sock;
    std::string _ip;
    int         _port;
    std::string _password;

    std::string _buffer;
    std::string _nickname;

public:
    Bot(const std::string& ip, int port, const std::string& password);
    ~Bot();

    void connectToServer();
    void registerBot();
    void run();

    void processBuffer();
    void handleMessage(const std::string& msg);

    std::string extractSender(const std::string& msg);
    std::string extractTarget(const std::string& msg);
    std::string extractText(const std::string& msg);

    void executeCommand(const std::string& sender, const std::string& message);

    void cmdHello(const std::string& sender);
    void cmdEcho(const std::string& sender, const std::string& msg);
    void cmdTime(const std::string& sender);
    void cmdHelp(const std::string& sender);
    //void cmdUsers(const std::string& sender);

    void sendMessage(const std::string& msg);
};

#endif