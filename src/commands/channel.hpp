/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   channel.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: iel-ghou <iel-ghou@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/28 22:59:42 by iel-ghou          #+#    #+#             */
/*   Updated: 2026/04/28 22:59:44 by iel-ghou         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CHANNEL_HPP_COMMANDS
#define CHANNEL_HPP_COMMANDS

#include <vector>
#include <string>

class Client;
class Server;

void cmdJoin(Client* client, const std::vector<std::string>& args, Server& server);
void cmdPart(Client* client, const std::vector<std::string>& args, Server& server);
void cmdTopic(Client* client, const std::vector<std::string>& args, Server& server);
void cmdKick(Client* client, const std::vector<std::string>& args, Server& server);
void cmdInvite(Client* client, const std::vector<std::string>& args, Server& server);
void cmdMode(Client* client, const std::vector<std::string>& args, Server& server);

#endif