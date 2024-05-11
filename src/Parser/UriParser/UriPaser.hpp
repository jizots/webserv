/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   UriPaser.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tchoquet <tchoquet@student.42tokyo.jp>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/17 15:34:35 by sotanaka          #+#    #+#             */
/*   Updated: 2024/05/09 15:09:03 by tchoquet         ###   ########.fr       */
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
        _uri    = 1,
        _params = 2,
        _query  = 3,
        _badURI = 4,
    };

public:
    UriParser(std::string& uriDst, std::string& paramsDst, std::string& queryDst);

    void appendParsed(const std::vector<Byte>& buff);
    void parseString(const std::string& str);

    inline bool isBadURI() { return m_status == _badURI;    }

private:
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