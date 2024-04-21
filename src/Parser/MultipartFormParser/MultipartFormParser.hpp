/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   MultipartFormParser.hpp                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sotanaka <sotanaka@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/18 13:17:12 by sotanaka          #+#    #+#             */
/*   Updated: 2024/04/18 18:47:41 by sotanaka         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef MULTIPARTFORMPARSER_HPP
# define MULTIPARTFORMPARSER_HPP

# include <string>
# include <map>

# include "Utils/Utils.hpp"

namespace webserv
{

struct MultipartFormData
{
	std::string		contentType;
	std::string		TransferEncoding;
	std::string		dispositionType;
	std::map<std::string, std::string>	dispositionParams;
	std::string::size_type	dataStartPos;
	std::string::size_type	lenData;
};

class MultipartFormParser
{
public:
	MultipartFormParser(void);
	inline ~MultipartFormParser(void){};
	std::vector<MultipartFormData> parse(const std::vector<Byte>& requestBody, const std::string& boundary);
	inline bool isBadRequest(void) {return (m_isBadRequest);};

private:
	size_t	m_idx;
	bool	m_isBadRequest;
	bool	m_isEndFlag;
	std::vector<MultipartFormData>	m_data;

private:
	bool isCRLF(const std::vector<Byte>& requestBody);
	bool searchBoundary(const std::vector<Byte>& requestBody, const std::string boundary);
	bool searchStrFromByteVec(const std::vector<Byte>& byteVec, const std::string& little);
	void parseHeader(const std::vector<Byte>& requestBody);
	void setDataInfo(const size_t potentialDataPos, const size_t boundarySize);
	void checkDatas(void);
};

std::ostream& operator<<(std::ostream& os, const std::vector<MultipartFormData>& data);

}

#endif