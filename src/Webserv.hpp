/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Webserv.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tchoquet <tchoquet@student.42tokyo.jp>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/02/13 15:35:41 by tchoquet          #+#    #+#             */
/*   Updated: 2024/02/21 18:34:25 by tchoquet         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef WEBSERV_HPP
# define WEBSERV_HPP

#include <vector>
#include <map>

#include "Utils/Utils.hpp"
#include "ConfigParser/ConfigParser.hpp"

namespace webserv
{

class Webserv
{

public:
    Webserv(const std::vector<ServerConfig>& configs);

    void run();
    inline void stop() { m_running = false; };

private:
    bool m_running;
};

}

#endif // WEBSERV_HPP