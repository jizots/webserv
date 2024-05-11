/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   httpRequestLineParserTestCases.cpp                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tchoquet <tchoquet@student.42tokyo.jp>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/09 15:40:50 by tchoquet          #+#    #+#             */
/*   Updated: 2024/05/10 13:54:17 by tchoquet         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <gtest/gtest.h>

#include "Parser/HTTPRequestParser/HTTPRequestParser.hpp"

namespace webserv_test
{
struct TestParams
{
    std::string            request;

    bool                   isComplete = true;
    bool                   isBadRequest = false;
    bool                   didParseAllString = true;
    std::string::size_type index;
    std::string            method;
    std::string            uri;
    std::string            params;
    std::string            query;
    webserv::uint8         verMajor = 1;
    webserv::uint8         verMinor = 1;
};

std::ostream& operator << (std::ostream& os, const TestParams& testParam)
{
    os << "\"";
    for (auto& c : testParam.request)
    {
        switch (c)
        {
            case '\\':
                os << "\\\\";
                break;
            case '\n':
                os << "\\n";
                break;
            case '\r':
                os << "\\r";
                break;
            case '\t':
                os << "\\t";
                break;
            default:
                os << c;
                break;
        }
    }
    os << "\"";
    return os;
} 

class HTTPRequestLineParserTest : public testing::TestWithParam<TestParams>
{
protected:
    HTTPRequestLineParserTest() : m_parser(m_method, m_uri, m_params, m_query, m_verMajor, m_verMinor)
    {
    }

    void parse(const std::string& data)
    {
        for (m_index = 0; m_index < data.length(); m_index++)
        {
            m_parser.parse(data[m_index]);
            if (m_parser.isComplete())
            {
                m_isComplete = true;
                if (m_parser.isBadRequest())
                    m_isBadRequest = true;
                break;
            }
        }
        if (m_index == data.length() - 1 || m_index == data.length())
            m_didParseAllString = true;
    }

    std::string    m_method;
    std::string    m_uri;
    std::string    m_params;
    std::string    m_query;
    webserv::uint8 m_verMajor;
    webserv::uint8 m_verMinor;

    bool m_isComplete = false;
    bool m_isBadRequest = false;

    bool m_didParseAllString = false;
    std::string::size_type m_index = 0;

    webserv::HTTPRequestParser::RequestLineParser m_parser;
};

TEST_P(HTTPRequestLineParserTest,)
{
    parse(GetParam().request);

    EXPECT_EQ(m_isBadRequest, GetParam().isBadRequest);
    EXPECT_EQ(m_isComplete, GetParam().isComplete);
    EXPECT_EQ(m_didParseAllString, GetParam().didParseAllString) << "Stoped at index " << m_index << " (\'" << GetParam().request[m_index] << "\')" ;

    if (m_isBadRequest || GetParam().isBadRequest || m_isComplete || GetParam().isComplete == false)
        return;

    EXPECT_EQ(m_method,   GetParam().method);
    EXPECT_EQ(m_uri,      GetParam().uri);
    EXPECT_EQ(m_params,   GetParam().params);
    EXPECT_EQ(m_query,    GetParam().query);
    EXPECT_EQ(m_verMajor, GetParam().verMajor);
    EXPECT_EQ(m_verMinor, GetParam().verMinor);
}

INSTANTIATE_TEST_SUITE_P(, HTTPRequestLineParserTest, testing::Values(

    (TestParams){ .request="GET /index../",                                 .isComplete=false },
    (TestParams){ .request="GET /////////",                                 .isComplete=false },
    (TestParams){ .request="GET shouldBeBad",                               .isComplete=false },
    (TestParams){ .request="GET /../",                                      .isComplete=false },
    (TestParams){ .request="GET /%4K",                                      .isComplete=false },
    // (TestParams){ .request="GET /%4K HTTP/1.1",                             .isComplete=false }, // Ok, webserv do it during uri parsing
    (TestParams){ .request="GET /H%48 HTTP/1.1",                            .isComplete=false },
    // (TestParams){ .request="GET /%4K/ABC HTTP/1.1",                         .isComplete=false }, // Ok, webserv do it during uri parsing
    

    (TestParams){ .request="QWERTYHGFDS          /index.html HTTP/1.1\r\n", .method="QWERTYHGFDS", .uri="/index.html" },
    (TestParams){ .request="QWERTYHGFDS /index.html          HTTP/1.1\r\n", .method="QWERTYHGFDS", .uri="/index.html" },
    (TestParams){ .request="QWERTYHGFDS /index.html HTTP/1.1\r\n",          .method="QWERTYHGFDS", .uri="/index.html" },
    (TestParams){ .request="POST /cgi.py?user=thomas HTTP/1.1\r\n",         .method="POST",        .uri="/cgi.py",       .query="user=thomas" },
    (TestParams){ .request="GET /blah?lolo HTTP/1.1\r\n",                   .method="GET",         .uri="/blah",         .query="lolo" },
    (TestParams){ .request="A /index.html HTTP/1.1\r\n",                    .method="A",           .uri="/index.html" },
    (TestParams){ .request="GET /blah HTTP/1.1\r\n",                        .method="GET",         .uri="/blah" },
    (TestParams){ .request="GET /index.html HTTP/1.1\r\n",                  .method="GET",         .uri="/index.html" },
    (TestParams){ .request="GET / HTTP/1.1\r\n",                            .method="GET",         .uri="/" },
    (TestParams){ .request="GET /////////// HTTP/1.1\r\n",                  .method="GET",         .uri="///////////" },
    (TestParams){ .request="GET /??????? HTTP/1.1\r\n",                     .method="GET",         .uri="/" },
    (TestParams){ .request="GET /;?;?;?;?;?; HTTP/1.1\r\n",                 .method="GET",         .uri="/;", .query=";?;?;?;?;" },
    (TestParams){ .request="GET /%48%65%6c%6c%6f HTTP/1.1\r\n",             .method="GET",         .uri="/Hello"},
    (TestParams){ .request="GET /%48%65%6C%6C%6F HTTP/1.1\r\n",             .method="GET",         .uri="/Hello"},
    (TestParams){ .request="GET /%25%48%65%6c%6c%6f HTTP/1.1\r\n",          .method="GET",         .uri="/%Hello"},
    (TestParams){ .request="GET /%25%48%65%6C%6C%6F HTTP/1.1\r\n",          .method="GET",         .uri="/%Hello"},
    (TestParams){ .request="GET /H%48 HTTP/1.1\r\n",                        .method="GET",         .uri="/HH" },

    // (TestParams){ .request="GET /%2e%2e/ HTTP/1.1\r\n",                     .isBadRequest=true }, // OK, webserv do it after parsing
    (TestParams){ .request="BAD REQUEST@",                                  .isBadRequest=true },
    (TestParams){ .request="BAD REQUEST ",                                  .isBadRequest=true },
    // (TestParams){ .request="GET /../ HTTP/1.1\r\n",                         .isBadRequest=true }, // OK, webserv do it after parsing
    (TestParams){ .request="GeT / HTTP/1.1\r\n",                            .isBadRequest=true, .didParseAllString=false, .index=1 },
    (TestParams){ .request="GET f HTTP/1.1\r\n",                            .isBadRequest=true, .didParseAllString=false, .index=5 },
    // (TestParams){ .request="GET /%4K HTTP/1.1\r\n",                         .isBadRequest=true, .didParseAllString=true }, // Ok, webserv do it during uri parsing
    // (TestParams){ .request="GET /%4K/ABC HTTP/1.1\r\n",                     .isBadRequest=true, .didParseAllString=true }, // Ok, webserv do it during uri parsing

    
    (TestParams){ .request="GET /こんにちは HTTP/1.1\r\n", .method="GET", .uri="/こんにちは" }
));

}
