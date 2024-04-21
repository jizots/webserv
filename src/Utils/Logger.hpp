/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Logger.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tchoquet <tchoquet@student.42tokyo.jp>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/02/11 15:51:43 by tchoquet          #+#    #+#             */
/*   Updated: 2024/04/20 15:13:55 by tchoquet         ###   ########.fr       */
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

#ifndef log
    #define log webserv::Logger::shared()
#endif // log

namespace webserv
{

class Logger
{
public:
    static void init(const std::string& filepath);
    static inline Logger& shared() { return *s_instance; }
    static void terminate();

    inline void setIsChildProcess(bool val) { m_isChildProcess = val; } 

private:
    Logger()/* = delete*/; 
    Logger(const Logger&)/* = delete*/;
    inline Logger(const std::string& filepath) : m_filepath(filepath), m_isChildProcess(false) {}
    inline ~Logger() {}

    static Logger* s_instance; 
    std::string m_filepath;
    std::stringstream m_lineStream;
    bool m_isChildProcess;

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
            
            m_ofstream << Time::shared().now() << (m_isChildProcess ? " [CHILD] " : " ") << m_lineStream.str().substr(0, m_lineStream.str().find_first_of('\n')) << std::endl;
            m_ofstream.close();

            m_lineStream.str(m_lineStream.str().substr(m_lineStream.str().find_first_of('\n') + 1));
        }
        return *this;
    }
};

} // namespace webserv

#endif // LOGGER_HPP