/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Logger.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tchoquet <tchoquet@student.42tokyo.jp>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/02/11 15:51:43 by tchoquet          #+#    #+#             */
/*   Updated: 2024/03/04 08:37:02 by tchoquet         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef LOGGER_HPP
# define LOGGER_HPP

#include <iostream>
#include <string>
#include <fstream>
#include <cstring>

namespace webserv
{

class Logger
{
public:
    static void init(const std::string& filepath);
    static inline Logger& shared() { return *s_instance; }
    static void terminate();

    ~Logger();

private:
    Logger()/* = delete*/; 
    Logger(const Logger&)/* = delete*/;
    Logger(const std::string& filepath);

    static Logger* s_instance; 
    std::ofstream m_ofstream;
    std::string m_filepath;

public:
    template<typename T> Logger& operator << (const T& data)
    {
        // std::cout << data;
        m_ofstream.open(m_filepath.c_str(), std::ios::out | std::ios::app);
        if (m_ofstream.rdstate() == std::ios_base::failbit)
            throw std::runtime_error("error while opening " + m_filepath + ": " + std::strerror(errno));
        if (m_ofstream.good() == false)
            throw std::runtime_error("unknow error");
        m_ofstream << data /*<< std::flush*/;
        m_ofstream.close();
        return *this;
    }
};

} // namespace webserv

#endif // LOGGER_HPP