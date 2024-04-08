/******************************************************************************/
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HTTPRequest.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hotph <hotph@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/03/06 16:25:33 by tchoquet          #+#    #+#             */
/*   Updated: 2024/04/08 11:34:50 by hotph            ###   ########.fr       */
/*                                                                            */
/******************************************************************************/

#ifndef HTTPREQUEST_HPP
# define HTTPREQUEST_HPP

#include "HTTP/HTTPBase.hpp"

namespace webserv
{

struct HTTPHostValue{
    std::string hostname;
    uint16      port;
};

struct HTTPFieldValue{
    std::string                         valName;
    std::map<std::string, std::string>  parameters;
};

struct MultipartFormData
{
	std::string		contentType;
	std::string		TransferEncoding;
	std::string		dispositionType;
	std::map<std::string, std::string>	dispositionParams;
	std::string::size_type	dataStartPos;
	std::string::size_type	lenData;
};

struct HTTPRequest : public HTTPBase
{
    inline HTTPRequest() : HTTPBase(), contentLength(0), isChunk(false), isMultipart(false) {};

    std::string method;
    std::string uri;
    std::string query;
    std::string params; // ?
    std::vector<MultipartFormData> m_multipartFormDatas;
    HTTPFieldValue              m_HTTPFieldValue;
    std::vector<HTTPFieldValue> m_HTTPFieldValues;

    HTTPHostValue host;
    uint64 contentLength;
    bool isChunk;
    bool isMultipart;
    std::string boundary;
};
typedef SharedPtr<HTTPRequest> HTTPRequestPtr;

}

#endif // HTTPREQUEST_HPP
