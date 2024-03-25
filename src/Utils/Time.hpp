/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Time.hpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tchoquet <tchoquet@student.42tokyo.jp>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/03/24 12:04:21 by tchoquet          #+#    #+#             */
/*   Updated: 2024/03/25 14:23:13 by tchoquet         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef TIME_HPP
# define TIME_HPP

#include <ctime>
#include <iostream>
#include <climits>

#include "Utils/Types.hpp"

namespace webserv
{

struct Timestamp
{
    std::time_t time;

    friend std::ostream& operator << (std::ostream& os, const Timestamp& dr);
};

struct Duration
{
    uint64 s;

    inline static Duration minutes(uint16 min) { return (Duration){ (uint64)min * 60  }; }
    inline static Duration seconds(uint16 s)   { return (Duration){ (uint64)s         }; }
    inline static Duration infinity()          { return (Duration){ (uint64)ULONG_MAX }; }

    friend std::ostream& operator << (std::ostream& os, const Duration& dr);
    inline bool operator  < (const Duration& rhs) { return s < rhs.s; }
    inline bool operator  > (const Duration& rhs) { return s > rhs.s; }
    inline bool operator == (const Duration& rhs) { return s == rhs.s; }
    inline bool operator <= (const Duration& rhs) { return *this < rhs || *this == rhs; }
    inline bool operator >= (const Duration& rhs) { return *this > rhs || *this == rhs; }
};

class Time
{
public:
    static void init();
    static inline Time& shared() { return *s_instance; }
    static void terminate();

    inline Timestamp now() { return (Timestamp){ .time = std::time(NULL) }; }
    inline Duration since(const Timestamp& timestamp) { return (Duration){ (uint64)std::difftime(now().time, timestamp.time) }; }

private:
    inline Time() {}
    inline ~Time() {}

    static Time* s_instance;

};

}

#endif // TIME_HPP