#include "Parser/MultipartFormParser/MultipartFormParser.hpp"
#include "Parser/HTTPHeaderParser/HTTPHeaderParser.hpp"

namespace webserv
{

MultipartFormParser::MultipartFormParser(void)
	:m_idx(0),
	m_isBadRequest(false),
	m_isEndFlag(false)
{};

bool MultipartFormParser::searchStrFromByteVec(const std::vector<Byte>& byteVec, const std::string& little)
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

bool MultipartFormParser::searchBoundary(const std::vector<Byte>& byteVec, const std::string boundary)
{
	if (searchStrFromByteVec(byteVec, boundary) == false)
	{
		m_isBadRequest = true;
		log << "[error] MultipartFormParser: Expected boudary dosen't exist" << "\n";
		return (false);
	}
	return (true);
};

void	MultipartFormParser::setDataInfo(const std::string::size_type dataStartPos, const std::string::size_type dataEndPos)
{
	if (m_data.size() == 0)
		return ;
	m_data.back().dataStartPos = dataStartPos;
	m_data.back().lenData = dataEndPos - dataStartPos;
};

void	MultipartFormParser::parseHeader(const std::vector<Byte>& requestBody)
{
	std::map<std::string, std::string>	header;
	webserv::HTTPHeaderParser		    headerParser(header, false);
	MultipartFormData			        newData;

	while (m_idx < requestBody.size() && !headerParser.isComplete() && !headerParser.isBadRequest())
	{
		headerParser.parse(requestBody[m_idx]);
		if (headerParser.isBadRequest())
		{
			m_isBadRequest = true;
			log << "[error] MultipartFormParser: Invalid header" << "\n";
			return ;
		}
		++m_idx;
	};
	if (header.find("content-type") != header.end())
		newData.contentType = header["content-type"];
	if (header.find("content-transfer-encoding") != header.end())
		newData.TransferEncoding = header["content-transfer-encoding"];
	if (header.find("content-disposition") != header.end())
	{
		std::vector<std::string>	splitedStr;
		splitedStr = splitQuotedStringByChars(header["content-disposition"], " ;");
		if (splitedStr.size())
		{
			newData.dispositionType = splitedStr[0];
			for (std::string::size_type i = 1; i < splitedStr.size(); ++i)
			{
				std::vector<std::string>	params = splitQuotedStringByChars(splitedStr[i], "=");
				if (params.size() < 2)
				{
					std::pair<std::string, std::string> data = std::make_pair(params[0], "");
					newData.dispositionParams.insert(data);
				}
				else
				{
					std::pair<std::string, std::string> data = std::make_pair(params[0], dequote(params[1]));
					newData.dispositionParams.insert(data);
				}
			}
		}
	}
	else
	{
		m_isBadRequest = true;
		log << "[error] MultipartFormParser: Content-Disposition not found" << "\n";
		return ;
	}
	m_data.push_back(newData);
};

bool	MultipartFormParser::isCRLF(const std::vector<Byte>& requestBody)
{
	if (m_idx + 1 < requestBody.size()
		&& requestBody[m_idx] == '\r' && requestBody[m_idx + 1] == '\n')
		return (true);
	return (false);
};

void	MultipartFormParser::checkDatas(void)
{
	for (std::string::size_type i = 0; i < m_data.size(); ++i)
	{
		if (m_data[i].dispositionType.empty()
			|| m_data[i].dispositionParams.find("name") == m_data[i].dispositionParams.end())
		{
			m_isBadRequest = true;
			log << "[error] MultipartFormParser: Invalid disposition type or name" << "\n";
		}
	}
}

std::vector<MultipartFormData>	MultipartFormParser::parse(const std::vector<Byte>& requestBody, const std::string& boundary)
{
	std::string::size_type	dataStartPos = 0;
	std::string::size_type	nextBoundaryStartPos = 0;

	while (m_idx < requestBody.size() && !m_isEndFlag && !m_isBadRequest)
	{
		if (searchBoundary(requestBody, "--" + boundary) == false)
			return (m_data);
		if (m_idx + 1 < requestBody.size() && std::strncmp(reinterpret_cast<const char*>(&requestBody[m_idx]), "--", 2) == 0)
			m_isEndFlag = true;
		nextBoundaryStartPos = m_idx - boundary.size() - 2;//-2 is '--' of head of boundary
		if (isCRLF(requestBody))
			m_idx += 2;
		if (m_idx < requestBody.size() && dataStartPos)
			setDataInfo(dataStartPos, nextBoundaryStartPos - 2);//-2 is CRLF of end of body
		if (m_idx < requestBody.size() && !m_isEndFlag)
			parseHeader(requestBody);
		dataStartPos = m_idx;
	}
	if (m_isEndFlag == false)
	{
		m_isBadRequest = true;
		log << "[error] MultipartFormParser: Ending boundary not found" << "\n";
		return (m_data);
	}
	checkDatas();
	return (m_data);
};

std::ostream& operator<<(std::ostream& os, const std::vector<MultipartFormData>& data)
{
	for (std::string::size_type i = 0; i < data.size(); ++i)
	{
		std::cout << "------------- Multipart Data#" << i << std::endl;
		std::cout << "contentType: " << data[i].contentType << std::endl;
		std::cout << "TransferEncoding: " << data[i].TransferEncoding << std::endl;
		std::cout << "dispositionType: " << data[i].dispositionType << std::endl;
		std::cout << "dispositionParams: " << std::endl;
		for (std::map<std::string, std::string>::const_iterator itr = data[i].dispositionParams.begin(); itr != data[i].dispositionParams.end(); ++itr)
			std::cout << itr->first << " | " << itr->second << std::endl;
		std::cout << "dataStartPos: " << data[i].dataStartPos << std::endl;
		std::cout << "lenData: " << data[i].lenData << std::endl;
		std::cout << std::endl;
	}
	return (os);
}

}