/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Time.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tchoquet <tchoquet@student.42tokyo.jp>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/03/24 12:04:26 by tchoquet          #+#    #+#             */
/*   Updated: 2024/03/24 18:04:36 by tchoquet         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Utils/Time.hpp"

namespace webserv
{

Time* Time::s_instance = NULL;

std::ostream& operator << (std::ostream& os, const Timestamp& stamp)
{
    std::tm* tm = std::localtime(&stamp.time);

    os << tm->tm_year + 1900 << "/" << tm->tm_mon << "/" << tm->tm_mday << " ";

    if (tm->tm_hour < 10)
        os << '0';
    os << tm->tm_hour << ":";

    if (tm->tm_min < 10)
        os << '0';
    os << tm->tm_min << ":";

    if (tm->tm_sec < 10)
        os << '0';
    os << tm->tm_sec;
        
    return os;
}

std::ostream& operator << (std::ostream& os, const Duration& dr)
{
    uint64 h   = dr.s / 3600;
    uint64 min = dr.s / 60 - h * 60;
    uint64 s   = dr.s - (h * 3600 + min * 60);

    os << h << "hs " << min << "min " << s << "s";
    return os;
}

void Time::init()
{
    if (s_instance != NULL)
        return;
    s_instance = new Time();
}

void Time::terminate()
{
    delete s_instance;
    s_instance = NULL;
}

}