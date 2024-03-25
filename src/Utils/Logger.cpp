/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Logger.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tchoquet <tchoquet@student.42tokyo.jp>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/02/11 15:54:51 by tchoquet          #+#    #+#             */
/*   Updated: 2024/03/24 17:37:17 by tchoquet         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Utils/Logger.hpp"

#include <exception>
#include <cstring>

namespace webserv
{

Logger* Logger::s_instance = NULL;
    
void Logger::init(const std::string& filepath)
{
    if (s_instance != NULL)
        return;
    s_instance = new Logger(filepath);
}

void Logger::terminate()
{
    delete s_instance;
    s_instance = NULL;
}

} // namespace webserv
