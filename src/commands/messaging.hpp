/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   messaging.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: iel-ghou <iel-ghou@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/28 22:59:12 by iel-ghou          #+#    #+#             */
/*   Updated: 2026/04/28 22:59:14 by iel-ghou         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef MESSAGING_HPP
#define MESSAGING_HPP

#include <vector>
#include <string>

class Client;
class Server;

void cmdPrivmsg(Client* client, const std::vector<std::string>& args, Server& server);
void cmdNotice(Client* client, const std::vector<std::string>& args, Server& server);

#endif