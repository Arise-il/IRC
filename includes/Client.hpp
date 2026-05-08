#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <string>

class Client
{
private:
    int             _fd;
    std::string     _buffer;

    std::string     _nickname;
    std::string     _username;
    bool            _registered;
    bool            _passAccepted;//ilyas

public:
    Client(int fd);
    ~Client();

    int             getFd() const;
    std::string&    getBuffer();

    void            setNickname(const std::string& nick);
    const std::string& getNickname() const;

    void            setRegistered(bool value);
    bool            isRegistered() const;

    void               setUsername(const std::string& username);//ilyas
    const std::string& getUsername() const;//ilyas
    void               setPassAccepted(bool value);//ilyas
    bool               isPassAccepted() const;//ilyas
};

#endif
