/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   misc.hpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: iel-ghou <iel-ghou@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/28 22:58:30 by iel-ghou          #+#    #+#             */
/*   Updated: 2026/04/28 22:58:32 by iel-ghou         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef MISC_HPP
#define MISC_HPP

#include <vector>
#include <string>

class Client;
class Server;

void cmdPing(Client* client, const std::vector<std::string>& args, Server& server);
void cmdQuit(Client* client, const std::vector<std::string>& args, Server& server);

#endif