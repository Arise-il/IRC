/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   messaging.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: iel-ghou <iel-ghou@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/28 22:59:18 by iel-ghou          #+#    #+#             */
/*   Updated: 2026/04/28 23:02:52 by iel-ghou         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

//(PRIVMSG, NOTICE)
#include "messaging.hpp"
#include "../../includes/Client.hpp"
#include "../../includes/Server.hpp"
#include "../../includes/Channel.hpp"

static void sendMessage(Client* client, const std::vector<std::string>& args,
                        Server& server, bool isPrivmsg) {
    std::string cmd  = isPrivmsg ? "PRIVMSG" : "NOTICE";
    std::string nick = client->getNickname();

    // Need target and message
    if (args.size() < 2) {
        if (isPrivmsg)
            server.sendToClient(client->getFd(), ":ircserv 411 " + nick + " :No recipient given (PRIVMSG)");
        return;
    }

    std::string target  = args[0];
    std::string message = args[1];

    // Target is a channel
    if (target[0] == '#' || target[0] == '&') {
        Channel* ch = server.getChannel(target);
        if (!ch) {
            if (isPrivmsg)
                server.sendToClient(client->getFd(), ":ircserv 403 " + nick + " " + target + " :No such channel");
            return;
        }
        if (!ch->hasMember(client->getFd())) {
            if (isPrivmsg)
                server.sendToClient(client->getFd(), ":ircserv 404 " + nick + " " + target + " :Cannot send to channel");
            return;
        }
        ch->broadcast(":" + nick + "!" + client->getUsername() + "@localhost " + cmd + " " + target + " :" + message, client->getFd());
        return;
    }

    // Target is a user
    Client* target_client = server.getClientByNick(target);
    if (!target_client) {
        if (isPrivmsg)
            server.sendToClient(client->getFd(), ":ircserv 401 " + nick + " " + target + " :No such nick");
        return;
    }
    server.sendToClient(target_client->getFd(), ":" + nick + "!" + client->getUsername() + "@localhost " + cmd + " " + target + " :" + message);
}

void cmdPrivmsg(Client* client, const std::vector<std::string>& args, Server& server) {
    sendMessage(client, args, server, true);
}

void cmdNotice(Client* client, const std::vector<std::string>& args, Server& server) {
    sendMessage(client, args, server, false);
}