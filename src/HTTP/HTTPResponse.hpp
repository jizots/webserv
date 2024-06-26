/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HTTPResponse.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tchoquet <tchoquet@student.42tokyo.jp>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/03/06 16:27:06 by tchoquet          #+#    #+#             */
/*   Updated: 2024/04/06 14:33:02 by tchoquet         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTPRESPONSE_HPP
# define HTTPRESPONSE_HPP

#include "HTTP/HTTPBase.hpp"
#include "Utils/SharedPtr.hpp"

#define BUILT_IN_ERROR_PAGE(code, description)                           \
"<html>\n"                                                               \
"   <head><title>"+to_string(code)+" "+description+"</title></head>\n"   \
"   <body>\n"                                                            \
"       <center><h1>"+to_string(code)+" "+description+"</h1></center>\n" \
"       <hr><center>webserv</center>\n"                                  \
"   </body>\n"                                                           \
"</html>"

namespace webserv
{

struct HTTPResponse : public HTTPBase
{
    HTTPResponse(int code = 200);
    
    uint32 statusCode;
    std::string statusDescription;
    bool isComplete;

    void setStatusCode(uint32 code);
    void makeBuiltInResponse(uint32 code);
    void getRaw(std::vector<Byte>&) const;
};
typedef SharedPtr<HTTPResponse> HTTPResponsePtr;

}

#endif // HTTPRESPONSE_HPP