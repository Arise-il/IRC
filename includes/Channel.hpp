/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: iel-ghou <iel-ghou@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/28 22:46:46 by iel-ghou          #+#    #+#             */
/*   Updated: 2026/04/28 22:47:00 by iel-ghou         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include <string>
#include <map>
#include <vector>

class Client;
class Server;

class Channel {
private:
    std::string             _name;
    std::string             _topic;
    std::string             _key;
    int                     _userLimit;
    bool                    _inviteOnly;
    bool                    _topicRestricted;

    std::map<int, Client*>  _members;
    std::map<int, bool>     _operators;
    std::vector<int>        _invited;

    Server&                 _server;

public:
    Channel(const std::string& name, Server& server);
    ~Channel();

    // Getters
    const std::string&      getName() const;
    const std::string&      getTopic() const;
    const std::string&      getKey() const;
    int                     getUserLimit() const;
    bool                    isInviteOnly() const;
    bool                    isTopicRestricted() const;
    std::map<int, Client*>& getMembers();

    // Setters
    void setTopic(const std::string& topic);
    void setKey(const std::string& key);
    void setUserLimit(int limit);
    void setInviteOnly(bool value);
    void setTopicRestricted(bool value);

    // Members
    void addMember(Client* client);
    void removeMember(int fd);
    bool hasMember(int fd) const;
    int  getMemberCount() const;

    // Operators
    void addOperator(int fd);
    void removeOperator(int fd);
    bool isOperator(int fd) const;

    // Invite
    void addInvited(int fd);
    bool isInvited(int fd) const;
    void removeInvited(int fd);

    // Broadcast to all members
    void broadcast(const std::string& message, int excludeFd = -1);

    // Get member list string for RPL_NAMREPLY
    std::string getMemberList() const;
};

#endif