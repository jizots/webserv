/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HTTPRequestParser.hpp                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tchoquet <tchoquet@student.42tokyo.jp>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/02/15 15:53:02 by tchoquet          #+#    #+#             */
/*   Updated: 2024/04/20 13:11:28 by tchoquet         ###   ########.fr       */
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

namespace webserv
{

class HTTPRequestParser
{
private:
    class RequestLineParser
    {
    private:
        enum status
        {
            _requestMethod  = 1,
            _slash          = 2,
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
        std::string* m_uri;
        uint8* m_verMajor;
        uint8* m_verMinor;

        status m_status;

        std::string m_hex;
        bool m_foundCR;
        std::string m_protocol;
    };

    class BodyParser
    {
    protected:
        enum status
        {
            _body          = 1,
            _parseComplete = 2,
            _badRequest    = 3,
        };

    public:
        BodyParser(std::vector<Byte>& bodyDst, uint64 contentLength);
        BodyParser(std::vector<Byte>& bodyDst);

        virtual void parse(Byte c);

        inline bool isComplete()   { return m_status >= _parseComplete; }
        inline bool isBadRequest() { return m_status == _badRequest;    }

        inline virtual ~BodyParser() {}

    protected:
        std::vector<Byte>* m_body;

        status m_status;

        uint64 m_contentLength;
    };

    class ChunkedBodyParser : public BodyParser
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
    inline void clearBuffer() { m_buffer.clear(); }

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
    UniPointer<BodyParser> m_bodyParser;
    
    std::vector<Byte> m_buffer;
    std::vector<Byte>::iterator m_curr;
};

}

#endif
