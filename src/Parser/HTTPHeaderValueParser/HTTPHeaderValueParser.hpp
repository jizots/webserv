/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HTTPHeaderValueParser.hpp                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sotanaka <sotanaka@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/18 15:47:47 by sotanaka          #+#    #+#             */
/*   Updated: 2024/04/18 16:42:32 by sotanaka         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTPHEADERVALUEPARSER_HPP
# define HTTPHEADERVALUEPARSER_HPP

# include <string>

# include "Utils/Utils.hpp"
# include "HTTP/HTTPRequest.hpp"

namespace webserv
{

class HTTPHeaderValueParser
{
public:
    inline HTTPHeaderValueParser(void) : m_isBadRequest(false) {};

    template <typename T>
    T parseHeaderValue(const std::string& fieldLine, T (HTTPHeaderValueParser::*func)(const std::string&))
    {
        if (hasCommonCharacter(fieldLine, m_InvalidFieldVal))
        {
            log << "parseHeaderValue(): Invalid field value: " << fieldLine << "\n";
            return (T());
        }
        return ((this->*func)(fieldLine));
    }

    HTTPHostValue   parseHost(const std::string& fieldLine);
    uint64          parseContentLength(const std::string& fieldLine);
    HTTPFieldValue  parseContentType(const std::string& fieldLine);
    std::vector<HTTPFieldValue> parseTransferEncoding(const std::string& fieldLine);
    inline std::vector<HTTPFieldValue> parseConnection(const std::string& fieldLine) { return (parseTransferEncoding(fieldLine)); };

    inline bool isBadRequest(void) { return (m_isBadRequest); };

private:
    static const std::string m_InvalidFieldVal;
    bool    m_isBadRequest;

private:
    HTTPFieldValue              parseSingleFieldVal(const std::string& fieldVal);
    std::vector<HTTPFieldValue> parseMultiFieldVal(const std::string& fieldVals);

};

}

#endif
