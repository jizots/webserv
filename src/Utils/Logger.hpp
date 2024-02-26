/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Logger.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tchoquet <tchoquet@student.42tokyo.jp>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/02/11 15:51:43 by tchoquet          #+#    #+#             */
/*   Updated: 2024/02/21 18:37:35 by tchoquet         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef LOGGER_HPP
# define LOGGER_HPP

#include <iostream>
#include <string>
#include <fstream>

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

public:
    template<typename T> Logger& operator << (const T& data)
    {
        // std::cout << data;
        m_ofstream << data << std::flush;
        return *this;
    }
};

} // namespace webserv

#endif // LOGGER_HPP