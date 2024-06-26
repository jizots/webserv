/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HTTPRequestParser.hpp                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tchoquet <tchoquet@student.42tokyo.jp>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/02/15 15:53:02 by tchoquet          #+#    #+#             */
/*   Updated: 2024/05/09 17:41:15 by tchoquet         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTPREQUESTPARSER_HPP
# define HTTPREQUESTPARSER_HPP

#include <string>
#include <vector>
#include <sstream>

#include "Utils/Utils.hpp"
#include "HTTP/HTTPRequest.hpp"
#include "Parser/HTTPHeaderParser/HTTPHeaderParser.hpp"
#include "Parser/UriParser/UriPaser.hpp"
#include "Parser/HTTPBodyParser/HTTPBodyParser.hpp"

namespace webserv_test { class HTTPRequestLineParserTest; }

namespace webserv
{

class HTTPRequestParser
{
    friend class webserv_test::HTTPRequestLineParserTest;

private:
    class RequestLineParser
    {
    private:
        enum status
        {
            _requestMethod  = 1,
            // _slash          = 2,
            _uri            = 3,
            // _params         = 4,
            // _query          = 5,
            _HTTP           = 6,
            _verMajor       = 7,
            _dot            = 8,
            _verMinor       = 9,
            _requestLineEnd = 10,
            _parseComplete  = 11,
            _badRequest     = 12,
        };

    public:
        RequestLineParser(std::string& methodDst, std::string& uriDst, std::string& paramsDst, std::string& queryDst, uint8& verMajorDst, uint8& verMinorDst);

        void parse(Byte c);

        inline bool isComplete()   { return m_status >= _parseComplete; }
        inline bool isBadRequest() { return m_status == _badRequest;    }

    private:
        void checkCRLF(Byte c, status successStatus);

        UriParser m_uriParser;
    
        std::string* m_method;

        uint8* m_verMajor;
        uint8* m_verMinor;

        status m_status;

        std::string m_hex;
        bool m_foundCR;
        std::string m_protocolRaw;
        std::vector<Byte> m_uriBuff;
    };

    class ChunkedBodyParser : public HTTPBodyParser
    {
    public:
        ChunkedBodyParser(std::vector<Byte>& bodyDst);

        void parse(Byte c) /*override*/;

    private:
        void checkCRLF(Byte c, bool successStatus);
        uint64 hexStringToUint64(const std::string& str);

        bool m_isChunkLen;
	    std::string m_chunkLenStr;
        bool m_foundCR;
        bool m_endUnChunk;
    };

private:
    enum status
    {
        _requestLine    = 1,
        _requestHeaders = 2,
        _requestBody    = 3,
        _parseComplete  = 4,
        _badRequest     = 5,
    };

public:
    HTTPRequestParser(const HTTPRequestPtr& request);

    Byte* getBuffer();
    void parse(uint32 len);
    void continueParsing();

    void nextRequest(const HTTPRequestPtr& request);

    inline bool isRequestLineComplete() { return (m_status  > _requestLine);    }
    inline bool isHeaderComplete()      { return (m_status  > _requestHeaders); }
    inline bool isBodyComplete()        { return (m_status  > _requestBody);    }
    inline bool isBadRequest()          { return (m_status == _badRequest);     }

private:
    HTTPRequestPtr m_request;

    int m_status;

    RequestLineParser m_requestLineParser;
    HTTPHeaderParser m_headerParser;
    UniPointer<HTTPBodyParser> m_bodyParser;
    
    std::vector<Byte> m_buffer;
    std::vector<Byte>::iterator m_curr;
};

}

#endif
