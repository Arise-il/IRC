#include "Bot.hpp"

Bot::Bot(const std::string& ip, int port, const std::string& password)
    : _sock(-1), _ip(ip), _port(port), _password(password), _nickname("bot")
{
}

Bot::~Bot()
{
    if (_sock != -1)
        close(_sock);
}

void Bot::connectToServer()
{
    _sock = socket(AF_INET, SOCK_STREAM, 0);
    if (_sock < 0)
    {
        std::cerr << "Socket creation failed\n";
        exit(1);
    }

    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(_port);
    addr.sin_addr.s_addr = inet_addr(_ip.c_str());

    if (connect(_sock, (sockaddr*)&addr, sizeof(addr)) < 0)
    {
        std::cerr << "Connection failed\n";
        exit(1);
    }

    std::cout << "Bot connected to server\n";
}

void Bot::registerBot()
{
    sendMessage("PASS " + _password + "\r\n");
    sendMessage("NICK " + _nickname + "\r\n");
    sendMessage("USER bot 0 * :bot\r\n");
}

void Bot::run()
{
    char buffer[512];

    while (true)
    {
        ssize_t bytes = recv(_sock, buffer, sizeof(buffer) - 1, 0);
        if (bytes <= 0)
            break;

        buffer[bytes] = '\0';
        _buffer += buffer;

        processBuffer();
    }
}

void Bot::processBuffer()
{
    size_t pos;

    while ((pos = _buffer.find("\r\n")) != std::string::npos
        || (pos = _buffer.find('\n')) != std::string::npos)
    {
        std::string msg = _buffer.substr(0, pos);
        _buffer.erase(0, pos + 2);

        handleMessage(msg);
    }
}

void Bot::handleMessage(const std::string& msg)
{
    if (msg.find(" 464 ") != std::string::npos)
    {
        std::cerr << "Authentication failed: wrong password" << std::endl;
        close(_sock);
        _sock = -1;
        exit(1);
    }
    std::cout << "Received: " << msg << std::endl;

    if (msg.find("PRIVMSG") == std::string::npos)
        return;

    std::string sender = extractSender(msg);
    std::string target = extractTarget(msg);
    std::string text   = extractText(msg);

    if (target != _nickname)
        return;

    if (text.empty() || text[0] != '!')
        return;

    executeCommand(sender, text);
}

std::string Bot::extractSender(const std::string& msg)
{
    if (msg.empty() || msg[0] != ':')
        return "";

    size_t end = msg.find('!');
    if (end == std::string::npos)
        return "";

    return msg.substr(1, end - 1);
}

std::string Bot::extractTarget(const std::string& msg)
{
    size_t pos = msg.find("PRIVMSG");
    if (pos == std::string::npos)
        return "";

    pos += 8;
    size_t end = msg.find(' ', pos);

    if (end == std::string::npos)
        return "";

    return msg.substr(pos, end - pos);
}

std::string Bot::extractText(const std::string& msg)
{
    size_t pos = msg.find(" :");
    if (pos == std::string::npos)
        return "";

    return msg.substr(pos + 2);
}


void Bot::executeCommand(const std::string& sender, const std::string& msg)
{
    if (msg == "!hello")
        cmdHello(sender);

    else if (msg.find("!echo ") == 0)
        cmdEcho(sender, msg);

    else if (msg == "!time")
        cmdTime(sender);

    else if (msg == "!help")
        cmdHelp(sender);
}


void Bot::cmdHello(const std::string& sender)
{
    sendMessage("PRIVMSG " + sender + " :Hello " + sender + " 👋\r\n");
}

void Bot::cmdEcho(const std::string& sender, const std::string& msg)
{
    std::string text = msg.substr(6);
    sendMessage("PRIVMSG " + sender + " :" + text + "\r\n");
}

void Bot::cmdTime(const std::string& sender)
{
    time_t now = time(NULL);
    std::string t = ctime(&now);

    if (!t.empty() && t[t.size() - 1] == '\n')
        t.erase(t.size() - 1);

    sendMessage("PRIVMSG " + sender + " :Time: " + t + "\r\n");
}

void Bot::cmdHelp(const std::string& sender)
{
    sendMessage("PRIVMSG " + sender +
        " :Commands: !hello !echo !time !help\r\n");
}


void Bot::sendMessage(const std::string& msg)
{
    send(_sock, msg.c_str(), msg.length(), 0);
}