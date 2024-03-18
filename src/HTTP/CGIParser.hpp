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

// #include "Utils/Utils.hpp"
#include "HTTP/BodyParser.hpp"
#include "HTTP/HeaderParser.hpp"

namespace webserv
{


class CGIParser
{
private:
    // enum class was c++11 extension
    enum status{
        _header          = 0,
        _headerParseDone = 1,
        _requestBody     = 2,
        _parseComplete   = 3,
        _badRequest      = 4,
    };
public:
    CGIParser();

    Byte* getBuffer();
    void parse(uint32 len);

    inline const std::map<std::string, std::string>& header() {return m_header;}
    inline const std::vector<Byte>& body() {return m_body; }
    inline uint64 contentLength() { return m_contentLength; }

    inline bool isComplete() { return (m_status >= _parseComplete); };
    inline bool isBadRequest() { return m_status == _badRequest; };

private:
    std::map<std::string, std::string> m_header;
    std::vector<Byte> m_body;

    HeaderParser m_headerParser;
    BodyParser m_bodyParser;
    
    uint64 m_contentLength;
    int m_status;
    uint64 m_idx;
    std::string m_hex;
    std::vector<Byte> m_buffer;
};

}

#endif
