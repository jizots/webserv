/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   BodyParser.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tchoquet <tchoquet@student.42tokyo.jp>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/03/09 14:27:25 by ekamada           #+#    #+#             */
/*   Updated: 2024/03/17 17:59:46 by tchoquet         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef BODYPARSER_HPP
#define BODYPARSER_HPP

#include <map>
#include <vector>
#include <string>

#include "Utils/Utils.hpp"
#include "ConfigParser/Utils.hpp"
#include "HTTPUtils.hpp"
namespace webserv
{


class BodyParser
{
private:
    enum status{
        _requestBody,
        _parseComplete,
        _badRequest
    };
public:
    BodyParser(std::vector<Byte>& body);

    void setContentLength(uint64 len);

    void parse(Byte c);

    inline bool isComplete() {return (m_status == _parseComplete);};
    inline bool isBadRequest() {return (m_status == _badRequest);};
    
private:
	std::vector<Byte>& m_body;

    int m_status;
	uint64 m_contentLength;
};

}


#endif
