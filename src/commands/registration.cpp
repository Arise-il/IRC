/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   registration.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: iel-ghou <iel-ghou@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/28 22:58:09 by iel-ghou          #+#    #+#             */
/*   Updated: 2026/04/28 23:02:26 by iel-ghou         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

//(PASS, NICK, USER)
#include "registration.hpp"
#include "../../includes/Client.hpp"
#include "../../includes/Server.hpp"
#include <cctype>

static bool isValidNick(const std::string& nick) {
    if (nick.empty() || nick.size() > 9)
        return false;
    std::string special = "-_[]{}\\|";
    if (!isalpha(nick[0]) && special.find(nick[0]) == std::string::npos)
        return false;
    for (size_t i = 1; i < nick.size(); i++) {
        if (!isalnum(nick[i]) && special.find(nick[i]) == std::string::npos)
            return false;
    }
    return true;
}

void cmdPass(Client* client, const std::vector<std::string>& args, Server& server) {
    if (client->isRegistered()) {
        server.sendToClient(client->getFd(), ":ircserv 462 " + client->getNickname() + " :You may not reregister");
        return;
    }
    if (args.empty()) {
        std::cout << "here1" << std::endl;
        server.sendToClient(client->getFd(), ":ircserv 461 * PASS :Not enough parameters");
        return;
    }
    if (args[0] != server.getPassword()) {
        server.sendToClient(client->getFd(), ":ircserv 464 * :Password incorrect");
        return;
    }
    client->setPassAccepted(true);
}

void cmdNick(Client* client, const std::vector<std::string>& args, Server& server) {
    if (!client->isPassAccepted()) {
        server.sendToClient(client->getFd(), ":ircserv 464 * :Password required");
        return;
    }
    if (args.empty()) {
        server.sendToClient(client->getFd(), ":ircserv 431 * :No nickname given");
        return;
    }

    std::string newNick = args[0];

    if (!isValidNick(newNick)) {
        server.sendToClient(client->getFd(), ":ircserv 432 " + newNick + " :Erroneous nickname");
        return;
    }

    Client* existing = server.getClientByNick(newNick);
    if (existing && existing->getFd() != client->getFd()) {
        server.sendToClient(client->getFd(), ":ircserv 433 * " + newNick + " :Nickname is already in use");
        return;
    }

    std::string oldNick = client->getNickname().empty() ? "*" : client->getNickname();
    client->setNickname(newNick);
    server.sendToClient(client->getFd(), ":" + oldNick + " NICK " + newNick);
}

void cmdUser(Client* client, const std::vector<std::string>& args, Server& server) {
    if (client->isRegistered()) {
        server.sendToClient(client->getFd(), ":ircserv 462 " + client->getNickname() + " :You may not reregister");
        return;
    }
    if (!client->isPassAccepted()) {
        server.sendToClient(client->getFd(), ":ircserv 464 * :Password required");
        return;
    }
    if (client->getNickname().empty()) {
        server.sendToClient(client->getFd(), ":ircserv 451 * :You have not registered");
        return;
    }

    client->setUsername(args[0]);
    client->setRegistered(true);

    std::string nick = client->getNickname();
    server.sendToClient(client->getFd(), ":ircserv 001 " + nick + " :Welcome to the IRC server " + nick + "!" + client->getUsername() + "@localhost");
    server.sendToClient(client->getFd(), ":ircserv 002 " + nick + " :Your host is ircserv, running version 1.0");
    server.sendToClient(client->getFd(), ":ircserv 003 " + nick + " :This server was created today");
    server.sendToClient(client->getFd(), ":ircserv 004 " + nick + " :ircserv 1.0 o o");
}