/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   misc.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: iel-ghou <iel-ghou@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/28 22:58:38 by iel-ghou          #+#    #+#             */
/*   Updated: 2026/05/06 16:58:09 by iel-ghou         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

//(PING, QUIT)#include "../../includes/commands/misc.hpp"
#include "../../includes/Client.hpp"
#include "../../includes/Server.hpp"
#include "../../includes/Channel.hpp"

void cmdPing(Client* client, const std::vector<std::string>& args, Server& server) {
    if (args.empty()) {
        server.sendToClient(client->getFd(), ":ircserv 409 " + client->getNickname() + " :No origin specified");
        return;
    }
    server.sendToClient(client->getFd(), ":ircserv PONG ircserv :" + args[0]);
}

void cmdQuit(Client* client, const std::vector<std::string>& args, Server& server) {
    std::string reason = args.empty() ? "Client quit" : args[0];
    std::string nick   = client->getNickname().empty() ? "*" : client->getNickname();

    std::map<std::string, Channel*>& channels = server.getChannels();
    std::vector<std::string> toRemove;

    for (std::map<std::string, Channel*>::iterator it = channels.begin();
         it != channels.end(); ++it) {
        Channel* ch = it->second;
        if (ch->hasMember(client->getFd())) {
            ch->broadcast(":" + nick + "!" + client->getUsername() + "@localhost QUIT :" + reason, client->getFd());
            ch->removeMember(client->getFd());
            if (ch->getMemberCount() == 0)
                toRemove.push_back(ch->getName());
        }
    }

    // Remove empty channels AFTER the loop
    for (size_t i = 0; i < toRemove.size(); i++)
        server.removeChannel(toRemove[i]);

    server.sendToClient(client->getFd(), ":ircserv ERROR :Closing link (" + nick + ") :" + reason);
    server.disconnectClient(client->getFd());
}