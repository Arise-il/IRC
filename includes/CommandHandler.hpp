/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CommandHandler.hpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: iel-ghou <iel-ghou@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/28 22:57:22 by iel-ghou          #+#    #+#             */
/*   Updated: 2026/04/28 23:04:11 by iel-ghou         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef COMMANDHANDLER_HPP
#define COMMANDHANDLER_HPP

#include <map>
#include <string>
#include <vector>

class Client;
class Server;

typedef void (*CommandFn)(Client*, const std::vector<std::string>&, Server&);

class CommandHandler {
public:
    CommandHandler(Server& server);
    ~CommandHandler();

    void handle(Client* client, const std::string& rawMessage);

private:
    Server&                          _server;
    std::map<std::string, CommandFn> _commands;

    struct ParsedMessage {
        std::string              prefix;
        std::string              command;
        std::vector<std::string> args;
    };

    ParsedMessage parse(const std::string& raw);
};

#endif