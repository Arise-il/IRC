/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   channel.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: iel-ghou <iel-ghou@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/28 22:59:48 by iel-ghou          #+#    #+#             */
/*   Updated: 2026/04/28 23:02:58 by iel-ghou         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

//(JOIN, PART, TOPIC, KICK, INVITE, MODE)
#include "channel.hpp"
#include "../../includes/Client.hpp"
#include "../../includes/Server.hpp"
#include "../../includes/Channel.hpp"
#include <sstream>

// ─── helpers ────────────────────────────────────────────────────────────────

static bool isValidChannelName(const std::string& name) {
    if (name.size() < 2) return false;
    if (name[0] != '#' && name[0] != '&') return false;
    for (size_t i = 1; i < name.size(); i++) {
        if (name[i] == ' ' || name[i] == ',' || name[i] == '\a')
            return false;
    }
    return true;
}

static void sendNames(Client* client, Channel* ch, Server& server) {
    std::string nick = client->getNickname();
    server.sendToClient(client->getFd(),
        ":ircserv 353 " + nick + " = " + ch->getName() + " :" + ch->getMemberList());
    server.sendToClient(client->getFd(),
        ":ircserv 366 " + nick + " " + ch->getName() + " :End of /NAMES list");
}

// ─── JOIN ────────────────────────────────────────────────────────────────────

void cmdJoin(Client* client, const std::vector<std::string>& args, Server& server) {
    if (args.empty()) {
        server.sendToClient(client->getFd(), ":ircserv 461 " + client->getNickname() + " JOIN :Not enough parameters");
        return;
    }

    std::string channelName = args[0];
    std::string key         = args.size() > 1 ? args[1] : "";

    if (!isValidChannelName(channelName)) {
        server.sendToClient(client->getFd(), ":ircserv 476 " + client->getNickname() + " " + channelName + " :Bad channel mask");
        return;
    }

    Channel* ch = server.getChannel(channelName);

    if (!ch) {
        // Create new channel, client becomes operator
        ch = server.createChannel(channelName, client);
    } else {
        // Channel exists — check restrictions
        if (ch->hasMember(client->getFd()))
            return;

        if (ch->isInviteOnly() && !ch->isInvited(client->getFd())) {
            server.sendToClient(client->getFd(), ":ircserv 473 " + client->getNickname() + " " + channelName + " :Cannot join channel (+i)");
            return;
        }

        if (!ch->getKey().empty() && ch->getKey() != key) {
            server.sendToClient(client->getFd(), ":ircserv 475 " + client->getNickname() + " " + channelName + " :Cannot join channel (+k)");
            return;
        }

        if (ch->getUserLimit() > 0 && ch->getMemberCount() >= ch->getUserLimit()) {
            server.sendToClient(client->getFd(), ":ircserv 471 " + client->getNickname() + " " + channelName + " :Cannot join channel (+l)");
            return;
        }

        ch->addMember(client);
        ch->removeInvited(client->getFd());
    }

    // Notify everyone in channel including the joiner
    std::string joinMsg = ":" + client->getNickname() + "!" + client->getUsername() + "@localhost JOIN " + channelName;
    ch->broadcast(joinMsg);

    // Send topic if set
    if (!ch->getTopic().empty())
        server.sendToClient(client->getFd(), ":ircserv 332 " + client->getNickname() + " " + channelName + " :" + ch->getTopic());
    else
        server.sendToClient(client->getFd(), ":ircserv 331 " + client->getNickname() + " " + channelName + " :No topic is set");

    sendNames(client, ch, server);
}

// ─── PART ────────────────────────────────────────────────────────────────────

void cmdPart(Client* client, const std::vector<std::string>& args, Server& server) {
    if (args.empty()) {
        server.sendToClient(client->getFd(), ":ircserv 461 " + client->getNickname() + " PART :Not enough parameters");
        return;
    }

    std::string channelName = args[0];
    std::string reason      = args.size() > 1 ? args[1] : client->getNickname();

    Channel* ch = server.getChannel(channelName);
    if (!ch) {
        server.sendToClient(client->getFd(), ":ircserv 403 " + client->getNickname() + " " + channelName + " :No such channel");
        return;
    }
    if (!ch->hasMember(client->getFd())) {
        server.sendToClient(client->getFd(), ":ircserv 442 " + client->getNickname() + " " + channelName + " :You're not on that channel");
        return;
    }

    std::string partMsg = ":" + client->getNickname() + "!" + client->getUsername() + "@localhost PART " + channelName + " :" + reason;
    ch->broadcast(partMsg);
    ch->removeMember(client->getFd());

    if (ch->getMemberCount() == 0)
        server.removeChannel(channelName);
}

// ─── TOPIC ───────────────────────────────────────────────────────────────────

void cmdTopic(Client* client, const std::vector<std::string>& args, Server& server) {
    if (args.empty()) {
        server.sendToClient(client->getFd(), ":ircserv 461 " + client->getNickname() + " TOPIC :Not enough parameters");
        return;
    }

    std::string channelName = args[0];
    Channel* ch = server.getChannel(channelName);

    if (!ch) {
        server.sendToClient(client->getFd(), ":ircserv 403 " + client->getNickname() + " " + channelName + " :No such channel");
        return;
    }
    if (!ch->hasMember(client->getFd())) {
        server.sendToClient(client->getFd(), ":ircserv 442 " + client->getNickname() + " " + channelName + " :You're not on that channel");
        return;
    }

    // No second arg — just viewing the topic
    if (args.size() < 2) {
        if (ch->getTopic().empty())
            server.sendToClient(client->getFd(), ":ircserv 331 " + client->getNickname() + " " + channelName + " :No topic is set");
        else
            server.sendToClient(client->getFd(), ":ircserv 332 " + client->getNickname() + " " + channelName + " :" + ch->getTopic());
        return;
    }

    // Setting the topic
    if (ch->isTopicRestricted() && !ch->isOperator(client->getFd())) {
        server.sendToClient(client->getFd(), ":ircserv 482 " + client->getNickname() + " " + channelName + " :You're not channel operator");
        return;
    }

    ch->setTopic(args[1]);
    ch->broadcast(":" + client->getNickname() + "!" + client->getUsername() + "@localhost TOPIC " + channelName + " :" + args[1]);
}

// ─── KICK ────────────────────────────────────────────────────────────────────

void cmdKick(Client* client, const std::vector<std::string>& args, Server& server) {
    if (args.size() < 2) {
        server.sendToClient(client->getFd(), ":ircserv 461 " + client->getNickname() + " KICK :Not enough parameters");
        return;
    }

    std::string channelName = args[0];
    std::string targetNick  = args[1];
    std::string reason      = args.size() > 2 ? args[2] : client->getNickname();

    Channel* ch = server.getChannel(channelName);
    if (!ch) {
        server.sendToClient(client->getFd(), ":ircserv 403 " + client->getNickname() + " " + channelName + " :No such channel");
        return;
    }
    if (!ch->hasMember(client->getFd())) {
        server.sendToClient(client->getFd(), ":ircserv 442 " + client->getNickname() + " " + channelName + " :You're not on that channel");
        return;
    }
    if (!ch->isOperator(client->getFd())) {
        server.sendToClient(client->getFd(), ":ircserv 482 " + client->getNickname() + " " + channelName + " :You're not channel operator");
        return;
    }

    Client* target = server.getClientByNick(targetNick);
    if (!target || !ch->hasMember(target->getFd())) {
        server.sendToClient(client->getFd(), ":ircserv 441 " + client->getNickname() + " " + targetNick + " " + channelName + " :They aren't on that channel");
        return;
    }

    ch->broadcast(":" + client->getNickname() + "!" + client->getUsername() + "@localhost KICK " + channelName + " " + targetNick + " :" + reason);
    ch->removeMember(target->getFd());

    if (ch->getMemberCount() == 0)
        server.removeChannel(channelName);
}

// ─── INVITE ──────────────────────────────────────────────────────────────────

void cmdInvite(Client* client, const std::vector<std::string>& args, Server& server) {
    if (args.size() < 2) {
        server.sendToClient(client->getFd(), ":ircserv 461 " + client->getNickname() + " INVITE :Not enough parameters");
        return;
    }

    std::string targetNick  = args[0];
    std::string channelName = args[1];

    Channel* ch = server.getChannel(channelName);
    if (!ch) {
        server.sendToClient(client->getFd(), ":ircserv 403 " + client->getNickname() + " " + channelName + " :No such channel");
        return;
    }
    if (!ch->hasMember(client->getFd())) {
        server.sendToClient(client->getFd(), ":ircserv 442 " + client->getNickname() + " " + channelName + " :You're not on that channel");
        return;
    }
    if (!ch->isOperator(client->getFd())) {
        server.sendToClient(client->getFd(), ":ircserv 482 " + client->getNickname() + " " + channelName + " :You're not channel operator");
        return;
    }

    Client* target = server.getClientByNick(targetNick);
    if (!target) {
        server.sendToClient(client->getFd(), ":ircserv 401 " + client->getNickname() + " " + targetNick + " :No such nick");
        return;
    }
    if (ch->hasMember(target->getFd())) {
        server.sendToClient(client->getFd(), ":ircserv 443 " + client->getNickname() + " " + targetNick + " " + channelName + " :is already on channel");
        return;
    }

    ch->addInvited(target->getFd());

    server.sendToClient(client->getFd(), ":ircserv 341 " + client->getNickname() + " " + targetNick + " " + channelName);
    server.sendToClient(target->getFd(), ":" + client->getNickname() + "!" + client->getUsername() + "@localhost INVITE " + targetNick + " :" + channelName);
}

// ─── MODE ────────────────────────────────────────────────────────────────────

void cmdMode(Client* client, const std::vector<std::string>& args, Server& server) {
    if (args.empty()) {
        server.sendToClient(client->getFd(), ":ircserv 461 " + client->getNickname() + " MODE :Not enough parameters");
        return;
    }

    std::string channelName = args[0];
    Channel* ch = server.getChannel(channelName);

    if (!ch) {
        server.sendToClient(client->getFd(), ":ircserv 403 " + client->getNickname() + " " + channelName + " :No such channel");
        return;
    }

    // No mode string — return current modes
    if (args.size() < 2) {
        std::string modes = "+";
        if (ch->isInviteOnly())      modes += "i";
        if (ch->isTopicRestricted()) modes += "t";
        if (!ch->getKey().empty())   modes += "k";
        if (ch->getUserLimit() > 0)  modes += "l";
        server.sendToClient(client->getFd(), ":ircserv 324 " + client->getNickname() + " " + channelName + " " + modes);
        return;
    }

    if (!ch->isOperator(client->getFd())) {
        server.sendToClient(client->getFd(), ":ircserv 482 " + client->getNickname() + " " + channelName + " :You're not channel operator");
        return;
    }

    std::string modeStr = args[1];
    bool adding = true;
    size_t argIdx = 2;

    for (size_t i = 0; i < modeStr.size(); i++) {
        char m = modeStr[i];

        if (m == '+') { adding = true; continue; }
        if (m == '-') { adding = false; continue; }

        if (m == 'i') {
            ch->setInviteOnly(adding);
            ch->broadcast(":" + client->getNickname() + "!" + client->getUsername() + "@localhost MODE " + channelName + " " + (adding ? "+i" : "-i"));
        }
        else if (m == 't') {
            ch->setTopicRestricted(adding);
            ch->broadcast(":" + client->getNickname() + "!" + client->getUsername() + "@localhost MODE " + channelName + " " + (adding ? "+t" : "-t"));
        }
        else if (m == 'k') {
            if (adding) {
                if (argIdx >= args.size()) {
                    server.sendToClient(client->getFd(), ":ircserv 461 " + client->getNickname() + " MODE :Not enough parameters");
                    return;
                }
                ch->setKey(args[argIdx++]);
                ch->broadcast(":" + client->getNickname() + "!" + client->getUsername() + "@localhost MODE " + channelName + " +k " + ch->getKey());
            } else {
                ch->setKey("");
                ch->broadcast(":" + client->getNickname() + "!" + client->getUsername() + "@localhost MODE " + channelName + " -k");
            }
        }
        else if (m == 'l') {
            if (adding) {
                if (argIdx >= args.size()) {
                    server.sendToClient(client->getFd(), ":ircserv 461 " + client->getNickname() + " MODE :Not enough parameters");
                    return;
                }
                std::istringstream ss(args[argIdx++]);
                int limit; ss >> limit;
                if (limit <= 0) {
                    server.sendToClient(client->getFd(), ":ircserv 461 " + client->getNickname() + " MODE :Invalid limit");
                    return;
                }
                ch->setUserLimit(limit);
                ch->broadcast(":" + client->getNickname() + "!" + client->getUsername() + "@localhost MODE " + channelName + " +l " + args[argIdx - 1]);
            } else {
                ch->setUserLimit(0);
                ch->broadcast(":" + client->getNickname() + "!" + client->getUsername() + "@localhost MODE " + channelName + " -l");
            }
        }
        else if (m == 'o') {
            if (argIdx >= args.size()) {
                server.sendToClient(client->getFd(), ":ircserv 461 " + client->getNickname() + " MODE :Not enough parameters");
                return;
            }
            std::string targetNick = args[argIdx++];
            Client* target = server.getClientByNick(targetNick);
            if (!target || !ch->hasMember(target->getFd())) {
                server.sendToClient(client->getFd(), ":ircserv 441 " + client->getNickname() + " " + targetNick + " " + channelName + " :They aren't on that channel");
                return;
            }
            if (adding) {
                ch->addOperator(target->getFd());
                ch->broadcast(":" + client->getNickname() + "!" + client->getUsername() + "@localhost MODE " + channelName + " +o " + targetNick);
            } else {
                ch->removeOperator(target->getFd());
                ch->broadcast(":" + client->getNickname() + "!" + client->getUsername() + "@localhost MODE " + channelName + " -o " + targetNick);
            }
        }
        else {
            server.sendToClient(client->getFd(), ":ircserv 472 " + client->getNickname() + " " + m + " :is unknown mode char to me");
        }
    }
}