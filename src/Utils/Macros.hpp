/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Macros.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tchoquet <tchoquet@student.42tokyo.jp>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/02/11 16:20:12 by tchoquet          #+#    #+#             */
/*   Updated: 2024/02/22 18:39:21 by tchoquet         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef log
    #define log webserv::Logger::shared()
#endif // log

#ifdef NDEBUG
    #ifdef assert
        #undef assert
    #endif
    #define assert(x) (void)(x)
#else
    #ifdef assert
        #undef assert
    #endif
    #define assert(x) if ((x) == false) { std::abort(); }
#endif // NDEBUG

#ifdef max
    #undef max
#endif // max
#define max(a, b) a > b ? a : b 

#ifndef BUFFER_SIZE
    #define BUFFER_SIZE 1024
#endif