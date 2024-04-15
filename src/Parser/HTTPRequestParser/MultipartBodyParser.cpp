/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   MultipartBodyParser.cpp                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tchoquet <tchoquet@student.42tokyo.jp>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/01 16:58:38 by tchoquet          #+#    #+#             */
/*   Updated: 2024/04/08 17:45:46 by tchoquet         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Parser/HTTPRequestParser/HTTPRequestParser.hpp"

namespace webserv
{

HTTPRequestParser::MultipartBodyParser::MultipartBodyParser(std::vector<Byte>& bodyDst, std::vector<MultipartFormData>& multipartDatasDst, uint64 contentLength, const std::string& boundary)
    : BodyParser(bodyDst, contentLength), m_multipartDatas(&multipartDatasDst), m_boundary(boundary), m_idx(0), m_isEndFlag(false)
{
}

void HTTPRequestParser::MultipartBodyParser::parse(Byte c)
{
    BodyParser::parse(c);
    if (isComplete() == false)
        return;

    std::string::size_type dataStartPos = 0;
    std::string::size_type nextBoundaryStartPos = 0;

    while (m_idx < m_body->size() && m_isEndFlag == false && isBadRequest() == false)
    {
        if (searchBoundary(*m_body, "--" + m_boundary) == false)
            return (void)(m_status = _badRequest);
        if (m_idx + 1 < m_body->size() && std::strncmp(reinterpret_cast<const char*>(&(*m_body)[m_idx]), "--", 2) == 0)
            m_isEndFlag = true;
        nextBoundaryStartPos = m_idx - m_boundary.size() - 2;//-2 is '--' of head of boundary
        if (isCRLF(*m_body))
            m_idx += 2;
        if (m_idx < m_body->size() && dataStartPos)
            setDataInfo(dataStartPos, nextBoundaryStartPos - 2); //-2 is CRLF of end of body
        if (m_idx < m_body->size() && !m_isEndFlag)
            parseHeader(*m_body);
        dataStartPos = m_idx;
    }
    if (m_isEndFlag == false || m_multipartDatas->size() == 0)
        return (void)(m_status = _badRequest);

    checkDatas();
}

bool HTTPRequestParser::MultipartBodyParser::searchBoundary(const std::vector<Byte>& byteVec, const std::string boundary)
{
	if (searchStrFormByteVec(byteVec, boundary) == false)
	{
		m_status = _badRequest;
		log << "[error] MultipartFormParser: Expected boudary dosen't exist";
		return (false);
	}
	return (true);
};

bool HTTPRequestParser::MultipartBodyParser::searchStrFormByteVec(const std::vector<Byte>& byteVec, const std::string& little)
{
    size_t  foundingPos = 0;

    if (byteVec.size() < little.size() || little.size() == 0)
        return (false);
    while (foundingPos <= byteVec.size() - little.size())
	{
		if (std::memcmp(&byteVec[m_idx + foundingPos], little.c_str(), little.size()) == 0)
		{
			m_idx += foundingPos + little.size();
			return (true);
		}
		++foundingPos;
	}
    return (false);
};

void HTTPRequestParser::MultipartBodyParser::setDataInfo(const std::string::size_type dataStartPos, const std::string::size_type dataEndPos)
{
    if (m_multipartDatas->size() == 0)
        return;
    m_multipartDatas->back().dataStartPos = dataStartPos;
    m_multipartDatas->back().lenData = dataEndPos - dataStartPos;
};

void HTTPRequestParser::MultipartBodyParser::parseHeader(const std::vector<Byte>& requestBody)
{
    std::map<std::string, std::string> header;
    HTTPHeaderParser headerParser(header);
    MultipartFormData newData;
        
    while (m_idx < requestBody.size() && !headerParser.isComplete() && !headerParser.isBadRequest())
    {
        headerParser.parse(requestBody[m_idx]);
        if (headerParser.isBadRequest())
        {
            log << "[error] MultipartFormParser: Invalid header\n";
            return (void)(m_status = _badRequest);
        }
        ++m_idx;
    }
    if (header.find("content-type") != header.end())
        newData.contentType = header["content-type"];

    if (header.find("content-transfer-encoding") != header.end())
        newData.TransferEncoding = header["content-transfer-encoding"];

    if (header.find("content-disposition") != header.end())
    {
        std::vector<std::string> splitedStr;
        splitedStr = splitByChars(header["content-disposition"], " ;");
        if (splitedStr.size())
        {
            newData.dispositionType = splitedStr[0];
            for (std::string::size_type i = 1; i < splitedStr.size(); ++i)
            {
                std::vector<std::string> params = splitByChars(splitedStr[i], "=");
                if (params.size() < 2)
                {
                    std::pair<std::string, std::string> data = std::make_pair(params[0], "");
                    newData.dispositionParams.insert(data);
                }
                std::pair<std::string, std::string> data = std::make_pair(params[0], dequote(params[1]));
                newData.dispositionParams.insert(data);
            }
        }
    }

    m_multipartDatas->push_back(newData);
};

bool HTTPRequestParser::MultipartBodyParser::isCRLF(const std::vector<Byte>& requestBody)
{
    if (m_idx + 1 < requestBody.size() && requestBody[m_idx] == '\r' && requestBody[m_idx + 1] == '\n')
        return (true);
    return (false);
};

void HTTPRequestParser::MultipartBodyParser::checkDatas(void)
{
    for (std::string::size_type i = 0; i < m_multipartDatas->size(); ++i)
    {
        if ((*m_multipartDatas)[i].dispositionType.empty() || (*m_multipartDatas)[i].dispositionParams.find("name") == (*m_multipartDatas)[i].dispositionParams.end())
        {
            log << "[error] MultipartFormParser: Invalid disposition type or name\n";
            return (void)(m_status = _badRequest);
        }
    }
}

}