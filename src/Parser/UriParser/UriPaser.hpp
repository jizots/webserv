/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   UriPaser.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sotanaka <sotanaka@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/17 15:34:35 by sotanaka          #+#    #+#             */
/*   Updated: 2024/04/17 16:55:27 by sotanaka         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef URIPARSER_HPP
# define URIPARSER_HPP

# include <string>

# include "Utils/Utils.hpp"

namespace webserv
{

class UriParser
{
private:
    enum status
    {
        _uri            = 1,
        _params         = 2,
        _query          = 3,
        _parseComplete  = 4,
        _badRequest     = 5,
    };

public:
    UriParser(std::string& uriDst, std::string& paramsDst, std::string& queryDst);

    void parse(Byte c);
    void parseString(const std::string& uri);

    inline bool isComplete()   { return m_status >= _parseComplete; }
    inline bool isBadRequest() { return m_status == _badRequest;    }

private:
    void decodeHex(Byte c, std::string& dst);

    std::string* m_uri;
    std::string* m_params;
    std::string* m_query;

    std::string m_hex;
    status m_status;

private:
    UriParser(void);
};

}

#endif