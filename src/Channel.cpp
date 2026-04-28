/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: iel-ghou <iel-ghou@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/28 22:47:15 by iel-ghou          #+#    #+#             */
/*   Updated: 2026/04/28 22:47:16 by iel-ghou         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/Channel.hpp"
#include "../includes/Client.hpp"
#include "../includes/Server.hpp"
#include <algorithm>

Channel::Channel(const std::string& name, Server& server)
    : _name(name),
      _topic(""),
      _key(""),
      _userLimit(0),
      _inviteOnly(false),
      _topicRestricted(false),
      _server(server)
{}

Channel::~Channel() {}

const std::string& Channel::getName() const { return _name; }
const std::string& Channel::getTopic() const { return _topic; }
const std::string& Channel::getKey() const { return _key; }
int  Channel::getUserLimit() const { return _userLimit; }
bool Channel::isInviteOnly() const { return _inviteOnly; }
bool Channel::isTopicRestricted() const { return _topicRestricted; }
std::map<int, Client*>& Channel::getMembers() { return _members; }

void Channel::setTopic(const std::string& topic) { _topic = topic; }
void Channel::setKey(const std::string& key) { _key = key; }
void Channel::setUserLimit(int limit) { _userLimit = limit; }
void Channel::setInviteOnly(bool value) { _inviteOnly = value; }
void Channel::setTopicRestricted(bool value) { _topicRestricted = value; }

void Channel::addMember(Client* client) {
    _members[client->getFd()] = client;
}

void Channel::removeMember(int fd) {
    _members.erase(fd);
    _operators.erase(fd);
}

bool Channel::hasMember(int fd) const {
    return _members.find(fd) != _members.end();
}

int Channel::getMemberCount() const {
    return _members.size();
}

void Channel::addOperator(int fd) { _operators[fd] = true; }
void Channel::removeOperator(int fd) { _operators.erase(fd); }
bool Channel::isOperator(int fd) const {
    return _operators.find(fd) != _operators.end();
}

void Channel::addInvited(int fd) { _invited.push_back(fd); }
bool Channel::isInvited(int fd) const {
    return std::find(_invited.begin(), _invited.end(), fd) != _invited.end();
}
void Channel::removeInvited(int fd) {
    _invited.erase(std::remove(_invited.begin(), _invited.end(), fd), _invited.end());
}

void Channel::broadcast(const std::string& message, int excludeFd) {
    for (std::map<int, Client*>::iterator it = _members.begin();
         it != _members.end(); ++it) {
        if (it->first != excludeFd)
            _server.sendToClient(it->first, message);
    }
}

std::string Channel::getMemberList() const {
    std::string list;
    for (std::map<int, Client*>::const_iterator it = _members.begin();
         it != _members.end(); ++it) {
        if (!list.empty()) list += " ";
        if (_operators.find(it->first) != _operators.end())
            list += "@";
        list += it->second->getNickname();
    }
    return list;
}