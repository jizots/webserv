/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HTTPBodyParser.hpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tchoquet <tchoquet@student.42tokyo.jp>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/26 17:52:59 by tchoquet          #+#    #+#             */
/*   Updated: 2024/04/26 18:00:50 by tchoquet         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTPBODYPARSER_HPP
# define HTTPBODYPARSER_HPP

#include "Utils/Utils.hpp"

namespace webserv
{

class HTTPBodyParser
{
protected:
    enum status
    {
        _body          = 1,
        _parseComplete = 2,
        _badRequest    = 3,
    };

public:
    HTTPBodyParser(std::vector<Byte>& bodyDst, uint64 contentLength);
    HTTPBodyParser(std::vector<Byte>& bodyDst);

    virtual void parse(Byte c);
    virtual void parseEOF();

    inline bool isComplete()   { return m_status >= _parseComplete; }
    inline bool isBadRequest() { return m_status == _badRequest;    }

    inline virtual ~HTTPBodyParser() {}

protected:
    std::vector<Byte>* m_body;

    status m_status;

    uint64 m_contentLength;
};

}

#endif // HTTPBODYPARSER_HPP