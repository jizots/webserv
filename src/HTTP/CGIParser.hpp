/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGIParser.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tchoquet <tchoquet@student.42tokyo.jp>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/02/15 15:53:02 by tchoquet          #+#    #+#             */
/*   Updated: 2024/02/19 16:12:14 by tchoquet         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CGIPARSER_HPP
# define CGIPARSER_HPP

#include <string>
#include <vector>
#include <sstream>

#include "Utils/Utils.hpp"
#include "HTTP/BodyParser.hpp"
#include "HTTP/HeaderParser.hpp"

namespace webserv
{

class CGIParser
{
private:
    enum status
    {
        _header          = 0,
        _headerParseDone = 1,
        _requestBody     = 2,
        _parseComplete   = 3,
        _badRequest      = 4,
    };

public:
    CGIParser(std::map<std::string, std::string>& headers, std::vector<Byte>& body);

    Byte* getBuffer();
    void parse(uint32 len);

    inline bool isComplete() { return (m_status >= _parseComplete); };
    inline bool isBadRequest() { return m_status == _badRequest; };

private:
    const std::map<std::string, std::string>& m_headers;

    HeaderParser m_headerParser;
    BodyParser m_bodyParser;
    
    std::vector<Byte> m_buffer;

    int m_status;
    uint64 m_idx;
    std::string m_hex;
};

}

#endif
