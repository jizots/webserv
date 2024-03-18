/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HTTPResponse.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tchoquet <tchoquet@student.42tokyo.jp>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/03/06 16:27:06 by tchoquet          #+#    #+#             */
/*   Updated: 2024/03/11 13:09:31 by tchoquet         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTPRESPONSE_HPP
# define HTTPRESPONSE_HPP

#include "HTTP/HTTPBase.hpp"

namespace webserv
{

struct HTTPResponse : public HTTPBase
{
    HTTPResponse(int code = 200);
    
    uint32 statusCode;
    std::string statusDescription;
    bool isComplete;

    void setStatusCode(uint32 code);
    void makeBuiltInBody();
    void getRaw(std::vector<Byte>&) const;
};
typedef SharedPtr<HTTPResponse> HTTPResponsePtr;

}

#endif // HTTPRESPONSE_HPP