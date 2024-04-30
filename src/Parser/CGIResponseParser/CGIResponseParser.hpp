/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGIResponseParser.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tchoquet <tchoquet@student.42tokyo.jp>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/02/15 15:53:02 by tchoquet          #+#    #+#             */
/*   Updated: 2024/02/19 16:12:14 by tchoquet         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CGIPARSER_HPP
# define CGIPARSER_HPP

#include <map>
#include <string>
#include <vector>

#include "Utils/Types.hpp"
#include "Parser/HTTPHeaderParser/HTTPHeaderParser.hpp"
#include "Parser/HTTPBodyParser/HTTPBodyParser.hpp"

namespace webserv
{

class CGIResponseParser
{
private:
    enum status
    {
        _headers       = 0,
        _body          = 2,
        _parseComplete = 3,
        _badResponse   = 4,
    };

public:
    CGIResponseParser(std::map<std::string, std::string>& headersDst);

    Byte* getBuffer();
    void parse(uint32 len);
    void continueParsing();
    inline void setBodyParser(const UniPointer<HTTPBodyParser>& parser) { m_bodyParser = parser; }

    inline bool isHeaderComplete() { return m_status  > _headers;       };
    inline bool isComplete()       { return m_status >= _parseComplete; };
    inline bool isBadResponse()    { return m_status == _badResponse;    };

private:
    int m_status;

    HTTPHeaderParser m_headerParser;
    UniPointer<HTTPBodyParser> m_bodyParser;

    std::vector<Byte> m_buffer;
    std::vector<Byte>::iterator m_curr;
};

}

#endif
