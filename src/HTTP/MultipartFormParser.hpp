#ifndef MULTIPART_FORM_PARSER_HPP
# define MULTIPART_FORM_PARSER_HPP

# include <map>
# include <vector>
# include <string>
# include "Utils/Utils.hpp"
# include "HTTP/HTTPRequest.hpp"

namespace webserv
{

class MultipartFormParser
{
public:
	MultipartFormParser(void);
	inline ~MultipartFormParser(void){};
	std::vector<MultipartFormData> parse(const std::string& requestBody, const std::string& boundary);
	inline bool isBadRequest(void) {return (m_isBadRequest);};

private:
	size_t		m_idx;
	bool		m_isBadRequest;
	bool		m_isEndFlag;
	std::vector<MultipartFormData>	m_data;

private:
	bool isCRLF(const std::string& requestBody);
	bool searchBoundary(const std::string& requestBody, const std::string boundary);
	void parseHeader(const std::string& requestBody);
	void setDataInfo(const size_t potentialDataPos, const size_t boundarySize);
	void checkDatas(void);
};

std::ostream& operator<<(std::ostream& os, const std::vector<MultipartFormData>& data);

}

#endif
