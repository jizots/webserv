#include "RequestHandler/RequestHandler.hpp"

namespace webserv
{

const std::string RequestHandler::m_InvalidFieldVal = "\r\n\0";

int RequestHandler::parseHost(const std::string& fieldLine)
{
	std::vector<std::string> strs = splitByChars(fieldLine, ":");

	m_request->host.port = 80;
	if (strs.size() == 0 || 2 < strs.size())
	{
		log << "parseHost(): Invalid Host field: " << fieldLine << "\n";
        return (400);
	}
	else
	{
		m_request->host.hostname = stringToLower(strs[0]);
		if (strs.size() == 2 && is<uint16>(strs[1]))
			m_request->host.port = to<uint16>(strs[1]);
	}
	return (0);
};

int RequestHandler::parseContentLength(const std::string& fieldLine)
{
	if (!is<uint64>(fieldLine))
		return (400);
	else
		m_request->contentLength = to<uint64>(fieldLine);
	return (0);
};

int RequestHandler::parseContentType(const std::string& fieldLine)
{
	m_request->m_HTTPFieldValue = parseSingleFieldVal(fieldLine);
    m_request->m_HTTPFieldValue.valName = stringToLower(m_request->m_HTTPFieldValue.valName);
	return (0);
};

int RequestHandler::parseTransferEncoding(const std::string& fieldLine)
{
	m_request->m_HTTPFieldValues = parseMultiFieldVal(fieldLine);

	for (std::vector<HTTPFieldValue>::size_type i = 0; i < m_request->m_HTTPFieldValues.size(); ++i)
	{
		m_request->m_HTTPFieldValues[i].valName = stringToLower(m_request->m_HTTPFieldValues[i].valName);
		if (!m_request->m_HTTPFieldValues[i].parameters.empty())
		{
			log << "parseTransferEncoding(): Invalid Transfer-Encoding field: " << fieldLine << "\n";
            return (400);
		}
	}
	return (0);
};

HTTPFieldValue RequestHandler::parseSingleFieldVal(const std::string& fieldVal)
{
	HTTPFieldValue result;
	std::vector<std::string> strs = splitByChars(fieldVal, "; ");

	if (strs.size() >= 1)
		result.valName = strs[0];
	for (size_t i = 1; i < strs.size(); ++i)
	{
		std::vector<std::string> param = splitByChars(strs[i], "=");
		if (param.size() != 2)
		{
			log << "parseSingleFieldVal(): Invalid field value: " << fieldVal << "\n";
			break;
		}
		else
			result.parameters.insert(std::make_pair(stringToLower(param[0]), dequote(param[1])));
	}
	return (result);
};

std::vector<HTTPFieldValue> RequestHandler::parseMultiFieldVal(const std::string& fieldVals)
{
	std::vector<HTTPFieldValue> result;
	HTTPFieldValue tmp;

	std::vector<std::string> strs = splitByChars(fieldVals, ", ");
	for (size_t i = 0; i < strs.size(); ++i)
	{
		tmp = parseSingleFieldVal(strs[i]);
		result.push_back(tmp);
	}
	return (result);
};

}