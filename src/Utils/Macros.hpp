/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Macros.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tchoquet <tchoquet@student.42tokyo.jp>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/02/11 16:20:12 by tchoquet          #+#    #+#             */
/*   Updated: 2024/04/08 11:26:49 by tchoquet         ###   ########.fr       */
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

// #ifdef max
//     #undef max
// #endif // max
// #define max(a, b) a > b ? a : b 

#ifndef BUFFER_SIZE
    #define BUFFER_SIZE 1024
#endif

#ifndef BUILT_IN_ERROR_PAGE
    #define BUILT_IN_ERROR_PAGE(code, description)\
"<html>"\
"<head><title>"+to_string(code)+" "+description+"</title></head>"\
"<body>"\
"<center><h1>"+to_string(code)+" "+description+"</h1></center>"\
"<hr><center>webserv</center>"\
"</body>"\
"</html>"
#endif

#define READ_END 0
#define WRITE_END 1

#define RMV_LAST_SLASH(str) (*(--str.end()) == '/' ? str.substr(0, str.length() - 1) : str)

#define IS_IN(str, c)         (std::string(str).find(c) != std::string::npos)
#define IS_PRINTABLE_ASCII(c) (c >= ' ' && c <= '~')
#define IS_UN_RESERVED(c)     (std::isalpha(c) || std::isdigit(c) || IS_IN("!*`(),$-_."                   , c))
#define IS_RESERVED(c)        (std::isalpha(c) || std::isdigit(c) || IS_IN("!*`(),$-_." ";/?:@&=+"        , c))
#define IS_PCHAR(c)           (std::isalpha(c) || std::isdigit(c) || IS_IN("!*`(),$-_." ":@&=+/"          , c))
#define IS_TOKEN(c)           (std::isalpha(c) || std::isdigit(c) || IS_IN(             "!#$%&'*+-.^_`|~" , c))