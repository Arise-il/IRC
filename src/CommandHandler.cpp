/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CommandHandler.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: iel-ghou <iel-ghou@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/28 22:57:01 by iel-ghou          #+#    #+#             */
/*   Updated: 2026/05/06 16:21:26 by iel-ghou         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/CommandHandler.hpp"
#include "../includes/Client.hpp"
#include "../includes/Server.hpp"
#include "../src/commands/registration.hpp"
#include "../src/commands/channel.hpp"
#include "../src/commands/messaging.hpp"
#include "../src/commands/misc.hpp"

CommandHandler::CommandHandler(Server& server) : _server(server) {
    _commands["PASS"]    = cmdPass;
    _commands["NICK"]    = cmdNick;
    _commands["USER"]    = cmdUser;
    _commands["JOIN"]    = cmdJoin;
    _commands["PART"]    = cmdPart;
    _commands["TOPIC"]   = cmdTopic;
    _commands["KICK"]    = cmdKick;
    _commands["INVITE"]  = cmdInvite;
    _commands["MODE"]    = cmdMode;
    _commands["PRIVMSG"] = cmdPrivmsg;
    _commands["privmsg"] = cmdPrivmsg;
    _commands["NOTICE"]  = cmdNotice;
    _commands["PING"]    = cmdPing;
    _commands["QUIT"]    = cmdQuit;
}

CommandHandler::~CommandHandler() {}

void CommandHandler::handle(Client* client, const std::string& rawMessage) {
    ParsedMessage msg = parse(rawMessage);

    if (msg.command.empty())
        return;

    if (!client->isRegistered()) {
        if (msg.command != "PASS" && msg.command != "NICK" && msg.command != "USER") {
            _server.sendToClient(client->getFd(), ":ircserv 451 * :You have not registered");
            return;
        }
    }

    std::map<std::string, CommandFn>::iterator it = _commands.find(msg.command);
    if (it != _commands.end())
        it->second(client, msg.args, _server);
    else
        _server.sendToClient(client->getFd(), ":ircserv 421 " + client->getNickname() + " " + msg.command + " :Unknown command");
}

CommandHandler::ParsedMessage CommandHandler::parse(const std::string& raw) {
    ParsedMessage msg;
    std::string line = raw;
    size_t pos = 0;

    // Optional prefix
    if (!line.empty() && line[0] == ':') {
        size_t end = line.find(' ');
        if (end == std::string::npos) return msg;
        msg.prefix = line.substr(1, end - 1);
        pos = end + 1;
    }

    // Command
    size_t end = line.find(' ', pos);
    msg.command = line.substr(pos, end - pos);
    if (end == std::string::npos) return msg;
    pos = end + 1;

    // Args
    while (pos < line.size()) {
        if (line[pos] == ':') {
            msg.args.push_back(line.substr(pos + 1));
            break;
        }
        end = line.find(' ', pos);
        if (end == std::string::npos) {
            msg.args.push_back(line.substr(pos));
            break;
        }
        msg.args.push_back(line.substr(pos, end - pos));
        pos = end + 1;
    }

    return msg;
}