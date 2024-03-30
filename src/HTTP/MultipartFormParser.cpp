#include "HTTP/MultipartFormParser.hpp"
#include "HTTP/HeaderParser.hpp"

namespace webserv
{

MultipartFormParser::MultipartFormParser(void)
	:m_idx(0),
	m_isBadRequest(false),
	m_isEndFlag(false)
{};

bool MultipartFormParser::searchBoundary(const std::string& requestBody, const std::string boundary)
{
	std::string::size_type	find = requestBody.find(boundary, m_idx);

	if (find == std::string::npos)
		return (false);
	m_idx = find + boundary.size();
	return (true);
};

void	MultipartFormParser::setDataInfo(const std::string::size_type dataStartPos, const std::string::size_type dataEndPos)
{
	if (m_data.size() == 0)
		return ;
	m_data.back().dataStartPos = dataStartPos;
	m_data.back().lenData = dataEndPos - dataStartPos;
};

void	MultipartFormParser::parseHeader(const std::string& requestBody)
{
	std::map<std::string, std::string>	header;
	webserv::HeaderParser		headerParser(header);
	MultipartFormData			newData;

	while (m_idx < requestBody.size() && !headerParser.isComplete() && !headerParser.isBadRequest())
	{
		headerParser.parse(static_cast<Byte>(requestBody[m_idx]));
		if (headerParser.isBadRequest())
		{
			m_isBadRequest = true;
			log << "[error] MultipartFormParser: Invalid header";
			return ;
		}
		++m_idx;
	}
	if (header.find("content-type") != header.end())
		newData.contentType = header["content-type"];
	if (header.find("content-transfer-encoding") != header.end())
		newData.TransferEncoding = header["content-transfer-encoding"];
	if (header.find("content-disposition") != header.end())
	{
		std::vector<std::string>	splitedStr;
		splitedStr = splitByChars(header["content-disposition"], " ;");
		if (splitedStr.size())
		{
			newData.dispositionType = splitedStr[0];
			for (std::string::size_type i = 1; i < splitedStr.size(); ++i)
			{
				std::vector<std::string>	params = splitByChars(splitedStr[i], "=");
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
	m_data.push_back(newData);
};

bool	MultipartFormParser::isCRLF(const std::string& requestBody)
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
			log << "[error] MultipartFormParser: Invalid disposition type or name";
		}
	}
}

std::vector<MultipartFormData>	MultipartFormParser::parse(const std::string& requestBody, const std::string& boundary)
{
	std::string::size_type	dataStartPos = 0;
	std::string::size_type	nextBoundaryStartPos = 0;

	while (m_idx < requestBody.size() && !m_isEndFlag && !m_isBadRequest)
	{
		if (searchBoundary(requestBody, "--" + boundary) == false)
		{
			m_isBadRequest = true;
			log << "[error] MultipartFormParser: Expected boudary dosen't exist";
			return (m_data);
		}
		if (m_idx + 1 < requestBody.size() && std::strncmp(&requestBody[m_idx], "--", 2) == 0)
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
	if (m_isEndFlag == false || m_data.size() == 0)
	{
		m_isBadRequest = true;
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