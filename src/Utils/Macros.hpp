/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Macros.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tchoquet <tchoquet@student.42tokyo.jp>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/02/11 16:20:12 by tchoquet          #+#    #+#             */
/*   Updated: 2024/03/01 09:39:08 by tchoquet         ###   ########.fr       */
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
"<!DOCTYPE html>"\
"<head>"\
"    <title>"+to_string(code)+" "+description+"</title>"\
"    <style>"\
"        body {"\
"            font-family: Arial, sans-serif;"\
"            color: #000000;"\
"            background-color: #f5f5f5;"\
"            text-align: center;"\
"            margin-top: 60px;"\
"        }"\
"    </style>"\
"</head>"\
"<body>"\
"    <h1>"+to_string(code)+" "+description+"</h1>"\
"    <hr>"\
"    <p>webserv</p>"\
"</body>"\
"</html>"
#endif