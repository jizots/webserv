/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HTTPRequest.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tchoquet <tchoquet@student.42tokyo.jp>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/03/06 16:25:33 by tchoquet          #+#    #+#             */
/*   Updated: 2024/03/30 13:03:40 by tchoquet         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTPREQUEST_HPP
# define HTTPREQUEST_HPP

#include "HTTP/HTTPBase.hpp"

namespace webserv
{

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

    std::string host;
    uint64 contentLength;
    bool isChunk;
    bool isMultipart;
    std::string boundary;
};
typedef SharedPtr<HTTPRequest> HTTPRequestPtr;

}

#endif // HTTPREQUEST_HPP
