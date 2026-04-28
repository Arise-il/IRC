#include "../includes/Client.hpp"

Client::Client(int fd)
    : _fd(fd),
      _buffer(""),
      _nickname(""),
      _username(""),
      _realname(""),
      _registered(false),
      _passAccepted(false)
{}

Client::~Client()
{
}

int Client::getFd() const
{
    return _fd;
}

std::string& Client::getBuffer()
{
    return _buffer;
}

void Client::setNickname(const std::string& nick)
{
    _nickname = nick;
}

const std::string& Client::getNickname() const
{
    return _nickname;
}

void Client::setRegistered(bool value)
{
    _registered = value;
}

bool Client::isRegistered() const
{
    return _registered;
}

void               Client::setUsername(const std::string& username) { _username = username; }//ilyas
const std::string& Client::getUsername() const { return _username; }//ilyas

void               Client::setRealname(const std::string& realname) { _realname = realname; }//ilyas
const std::string& Client::getRealname() const { return _realname; }//ilyas

void               Client::setPassAccepted(bool value) { _passAccepted = value; }//ilyas
bool               Client::isPassAccepted() const { return _passAccepted; }//ilyas