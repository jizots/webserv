/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Logger.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tchoquet <tchoquet@student.42tokyo.jp>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/02/11 15:51:43 by tchoquet          #+#    #+#             */
/*   Updated: 2024/03/24 18:03:56 by tchoquet         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef LOGGER_HPP
# define LOGGER_HPP

#include <iostream>
#include <string>
#include <fstream>
#include <cstring>
#include <sstream>

#include "Utils/Time.hpp"

namespace webserv
{

class Logger
{
public:
    static void init(const std::string& filepath);
    static inline Logger& shared() { return *s_instance; }
    static void terminate();


private:
    Logger()/* = delete*/; 
    Logger(const Logger&)/* = delete*/;
    inline Logger(const std::string& filepath) : m_filepath(filepath) {}
    inline ~Logger() {}

    static Logger* s_instance; 
    std::string m_filepath;
    std::stringstream m_lineStream;

public:
    template<typename T> Logger& operator << (const T& data)
    {
        m_lineStream << data;

        while (true)
        {
            if (m_lineStream.str().find_first_of('\n') == std::string::npos)
                break;
            
            std::ofstream m_ofstream(m_filepath.c_str(), std::ios::out | std::ios::app);

            if (m_ofstream.rdstate() == std::ios_base::failbit)
                throw std::runtime_error("error while opening " + m_filepath + ": " + std::strerror(errno));

            if (m_ofstream.good() == false)
                throw std::runtime_error("unknow error");

            m_ofstream << Time::shared().now() << " " << m_lineStream.str().substr(0, m_lineStream.str().find_first_of('\n')) << std::endl;
            m_ofstream.close();

            m_lineStream.str(m_lineStream.str().substr(m_lineStream.str().find_first_of('\n') + 1));
        }
        return *this;
    }
};

} // namespace webserv

#endif // LOGGER_HPP