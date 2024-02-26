/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Logger.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tchoquet <tchoquet@student.42tokyo.jp>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/02/11 15:54:51 by tchoquet          #+#    #+#             */
/*   Updated: 2024/02/13 19:34:34 by tchoquet         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Utils/Logger.hpp"

#include <exception>

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

Logger::Logger(const std::string& filepath) : m_ofstream(filepath.c_str(), std::ios::out | std::ios::app)
{
    // If the open() call returns a null pointer, sets setstate(failbit) -> https://en.cppreference.com/w/cpp/io/basic_ofstream/basic_ofstream
    if (m_ofstream.rdstate() == std::ios_base::failbit)
        throw std::runtime_error("error while opening " + filepath + ": " + strerror(errno));
    if (m_ofstream.good() == false)
        throw std::runtime_error("unknow error");
}

Logger::~Logger()
{
}

} // namespace webserv
