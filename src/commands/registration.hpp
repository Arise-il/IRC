/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   registration.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: iel-ghou <iel-ghou@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/28 22:57:54 by iel-ghou          #+#    #+#             */
/*   Updated: 2026/04/28 22:57:56 by iel-ghou         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef REGISTRATION_HPP
#define REGISTRATION_HPP

#include <vector>
#include <string>

class Client;
class Server;

void cmdPass(Client* client, const std::vector<std::string>& args, Server& server);
void cmdNick(Client* client, const std::vector<std::string>& args, Server& server);
void cmdUser(Client* client, const std::vector<std::string>& args, Server& server);

#endif