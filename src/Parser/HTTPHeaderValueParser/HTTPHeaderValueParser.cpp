/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HTTPHeaderValueParser.cpp                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sotanaka <sotanaka@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/18 16:01:19 by sotanaka          #+#    #+#             */
/*   Updated: 2024/04/18 16:21:50 by sotanaka         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Parser/HTTPHeaderValueParser/HTTPHeaderValueParser.hpp"

namespace webserv
{

const std::string HTTPHeaderValueParser::m_InvalidFieldVal = "\r\n\0";

HTTPHostValue HTTPHeaderValueParser::parseHost(const std::string& fieldLine)
{
	HTTPHostValue result;
	std::vector<std::string> strs = splitByChars(fieldLine, ":");

	result.port = 80;
	if (strs.size() == 0 || 2 < strs.size())
	{
		m_isBadRequest = true;
		log << "parseHost(): Invalid Host field: " << fieldLine << "\n";
        return (result);
	}
	else
	{
		result.hostname = stringToLower(strs[0]);
		if (strs.size() == 2 && is<uint16>(strs[1]))
			result.port = to<uint16>(strs[1]);
	}
	return (result);
};

uint64 HTTPHeaderValueParser::parseContentLength(const std::string& fieldLine)
{
	if (!is<uint64>(fieldLine))
	{
		m_isBadRequest = true;
		return (1);
	}
	return (to<uint64>(fieldLine));
};

HTTPFieldValue HTTPHeaderValueParser::parseContentType(const std::string& fieldLine)
{
	HTTPFieldValue result = parseSingleFieldVal(fieldLine);

	if (!m_isBadRequest)
	    result.valName = stringToLower(result.valName);
	return (result);
};

std::vector<HTTPFieldValue> HTTPHeaderValueParser::parseTransferEncoding(const std::string& fieldLine)
{
	std::vector<HTTPFieldValue> result = parseMultiFieldVal(fieldLine);

	for (std::vector<HTTPFieldValue>::size_type i = 0; i < result.size(); ++i)
	{
		result[i].valName = stringToLower(result[i].valName);
		if (!result[i].parameters.empty())
		{
			m_isBadRequest = true;
			log << "parseTransferEncoding(): Invalid Transfer-Encoding field: " << fieldLine << "\n";
            return (result);
		}
	}
	return (result);
};

HTTPFieldValue HTTPHeaderValueParser::parseSingleFieldVal(const std::string& fieldVal)
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
			m_isBadRequest = true;
			log << "parseSingleFieldVal(): Invalid field value: " << fieldVal << "\n";
			break;
		}
		else
			result.parameters.insert(std::make_pair(stringToLower(param[0]), dequote(param[1])));
	}
	return (result);
};

std::vector<HTTPFieldValue> HTTPHeaderValueParser::parseMultiFieldVal(const std::string& fieldVals)
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