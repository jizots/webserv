/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HTTPHeaderParser.hpp                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tchoquet <tchoquet@student.42tokyo.jp>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/03/09 14:27:23 by ekamada           #+#    #+#             */
/*   Updated: 2024/04/06 18:38:39 by tchoquet         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HEADERPARSER_HPP
#define HEADERPARSER_HPP

#include <map>
#include <string>

#include "Utils/Utils.hpp"

namespace webserv
{

class HTTPHeaderParser
{
private:
    enum status
    {
        _headerKey     = 1,
        _headerValue   = 2,
        _parseComplete = 3,
        _badRequest    = 4
    };

public:
    HTTPHeaderParser(std::map<std::string, std::string>& header);

    void parse(Byte c);

    inline bool isComplete()   { return (m_status >= _parseComplete); }
    inline bool isBadRequest() { return (m_status == _badRequest);    }

private:
    void checkCRLF(Byte c, int successStatus);
    void clearKeyValue();

    std::map<std::string, std::string>* m_header;

    int m_status;
    bool m_foundCR;
    std::string m_key;
    std::string m_value;
};

}


#endif
