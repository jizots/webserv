/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HTTPRequestParser.hpp                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tchoquet <tchoquet@student.42tokyo.jp>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/02/15 15:53:02 by tchoquet          #+#    #+#             */
/*   Updated: 2024/03/28 18:36:59 by tchoquet         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTPREQUESTPARSER_HPP
# define HTTPREQUESTPARSER_HPP

#include <string>
#include <vector>
#include <sstream>

#include "Utils/Utils.hpp"
#include "HTTP/HTTPRequest.hpp"
#include "HTTP/HeaderParser.hpp"

namespace webserv
{

class HTTPRequestParser
{
private:
    enum status
    {
        _requestMethod  = 1,
        _slash          = 2,
        _uri            = 3,
        _params         = 4,
        _query          = 5,
        _HTTP           = 6,
        _verMajor       = 7,
        _dot            = 8,
        _verMinor       = 9,
        _requestLineEnd = 10,
        _header         = 11,
        _requestBody    = 12,
        _endUnchunk     = 13,
        _parseComplete  = 14,
        _badRequest     = 15,
    };

public:
    HTTPRequestParser(const HTTPRequestPtr& request);

    Byte* getBuffer();
    void parse(uint32 len);
    void continueParsing();
    void parseMultipart();

    void nextRequest(const HTTPRequestPtr& request);

    inline bool isRequestLineComplete() { return (m_status  > _requestLineEnd); }
    inline bool isHeaderComplete()      { return (m_status  > _header);         }
    inline bool isBodyComplete()        { return (m_status >= _parseComplete);  }
    inline bool isBadRequest()          { return (m_status == _badRequest);     }

private:
    void requestLineParse(Byte c);
    void parseChunk(Byte c);

    void checkCRLF(Byte c, int successStatus);
    void checkCRLFChunk(Byte c, int successStatus);
    void decodeHex(Byte c, std::string& dst);
    uint64 hexStringToUint64(const std::string& str);

    HTTPRequestPtr m_request;

    HeaderParser m_headerParser;
    
    std::vector<Byte> m_buffer;

    int m_status;
    std::string m_hex;
    bool m_foundCR;
    std::string m_protocol;
    bool m_isChunkLen;
	std::string m_chunkLenStr;
    uint64 m_contentLength;
};

}

#endif
