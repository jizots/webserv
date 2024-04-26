/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   BuiltinCGIUpload.cpp                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sotanaka <sotanaka@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/17 15:58:50 by tchoquet          #+#    #+#             */
/*   Updated: 2024/04/26 15:19:52 by sotanaka         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "RequestHandler/BuiltinCGIUpload.hpp"
#include "Parser/MultipartFormParser/MultipartFormParser.hpp"
#include "RequestHandler/RequestHandler.hpp"

#include <unistd.h>
#include <vector>
#include <fcntl.h>
#include <cstdlib>

# define BUFFER_SIZE 1024

namespace webserv
{

struct HTTPFieldValue
{
    std::string valName;
    std::map<std::string, std::string> parameters;
};

int parseSingleFieldVal(const std::string& fieldVal, HTTPFieldValue& result)
{
	std::vector<std::string> strs = splitByChars(fieldVal, "; ");

	if (strs.size() >= 1)
		result.valName = strs[0];
	for (size_t i = 1; i < strs.size(); ++i)
	{
		std::vector<std::string> param = splitByChars(strs[i], "=");
		if (param.size() != 2)
		{
			log << "parseSingleFieldVal(): Invalid field value: " << fieldVal << "\n";
			return -1;
			break;
		}
		else
			result.parameters.insert(std::make_pair(stringToLower(param[0]), dequote(param[1])));
	}
	return 0;
};

int parseContentType(const std::string& fieldLine, HTTPFieldValue& result)
{
	if (parseSingleFieldVal(fieldLine, result) != 0)
	    return -1;
    
    result.valName = stringToLower(result.valName);
	return 0;
};

static bool getStdinToByteVector(std::vector<Byte>& body)
{
    Byte buffer[BUFFER_SIZE];
    ssize_t byteRead;

    while(true)
    {
        byteRead = read(FileDescriptor::stdin(), buffer, BUFFER_SIZE);
        if (byteRead <= 0)
        {
            if (byteRead == -1)
            {
                log << "builtinCGIUpload(): read() error" << "\n";
                return false;
            }
            else
                break;
        }
        body.insert(body.end(), buffer, buffer + byteRead);
    }
    return true;
}

static std::string getBoundaryFromValue(const std::string& contentType)
{
    HTTPFieldValue httpFieldValue;

    if (parseContentType(contentType, httpFieldValue) != 0)
        return ("");
    if (httpFieldValue.valName == "multipart/form-data")
    {
        std::map<std::string, std::string>::const_iterator boundaryIt = httpFieldValue.parameters.find("boundary");
        if (boundaryIt == httpFieldValue.parameters.end())
        {
            log << "builtinCGIUpload(): boundary not found\n"; 
            return ("");
        }
        return (boundaryIt->second);
    }
    else
    {
        log << "builtinCGIUpload(): Content-Type is not multipart/form-data\n";
        return ("");
    }
}

static bool isAccessableDirectory(const std::string& dir)
{
    if (access(RMV_LAST_SLASH(dir).c_str(), F_OK) != 0)
    {
        log << "Directory does not exist: " << dir << '\n';
        return (false);
    }
    if (access(RMV_LAST_SLASH(dir).c_str(), W_OK) != 0)
    {
        log << "No write access to directory: " << dir << '\n';
        return (false);
    }
    return (true);
}

static void createNewFile(const std::string& filePath, const char* data, size_t lenData)
{
    int fd = ::open(filePath.c_str(), O_WRONLY | O_CREAT | O_TRUNC, S_IWUSR | S_IRUSR | S_IRGRP | S_IROTH | O_NONBLOCK);
    if (fd < 0)
    {
        log << "builtinCGIUpload(): open(): " + std::string(std::strerror(errno));
        return ;
    }
    ssize_t writeLen = ::write(fd, data, lenData);
    if (writeLen == -1 || (size_t)writeLen != lenData)
        log << "builtinCGIUpload(): write() failed\n";
    if (::close(fd) < 0)
        log << "builtinCGIUpload(): close(): " + std::string(std::strerror(errno)) << "\n";
}

void builtinCGIUpload(const std::map<std::string, std::string>& headers)
{
    std::vector<Byte>   body;
    std::string         boundary;
    std::string         dir;

    if (!getStdinToByteVector(body))
        return ;

    std::map<std::string, std::string>::const_iterator contentTypeIt = headers.find("CONTENT_TYPE");
    if (contentTypeIt != headers.end())
        boundary = getBoundaryFromValue(contentTypeIt->second);
    else
    {
        log << "builtinCGIUpload(): CONTENT_TYPE not found\n";
        return ;
    }
    if (boundary.empty())
        return ;


    std::map<std::string, std::string>::const_iterator pathIt = headers.find("UPLOAD_PATH");
    if (pathIt != headers.end() && !isAccessableDirectory(pathIt->second))
        return ;
    else if (pathIt == headers.end())
    {
        log << "builtinCGIUpload(): UPLOAD_PATH not found\n";
        return ;
    }
    dir = pathIt->second;

    MultipartFormParser MultipartParser;
    std::vector<MultipartFormData> multipartDatas = MultipartParser.parse(body, boundary);
    bool isFileExist = false; 

    if (MultipartParser.isBadRequest())
        return ;
    for (std::vector<MultipartFormData>::size_type i = 0; i < multipartDatas.size(); ++i)
    {
        std::map<std::string, std::string>::iterator itr = multipartDatas[i].dispositionParams.find("filename");
        if (itr != multipartDatas[i].dispositionParams.end())
        {
            if (itr->second.empty())
                continue ;
            //open and write to new file
            createNewFile(dir + "/" + itr->second, reinterpret_cast<const char *>(&body[multipartDatas[i].dataStartPos]), multipartDatas[i].lenData);
            isFileExist = true;
        }
        else
        {
            log << "builtinCGIUpload(): filename not found\n";
            return ;
        }
    }
    if (!isFileExist)
    {
        std::cout << "status: 200\r\n";
        std::cout << "content-type: text/html\r\n";
        std::cout << "\r\n";
        std::cout << BUILT_IN_ERROR_PAGE(200, "No attached files");
        std::exit(0);
    }
    std::cout << "status: 201\r\n";
    std::cout << "content-type: text/html\r\n";
    std::cout << "\r\n";
    std::cout << BUILT_IN_ERROR_PAGE(201, "Upload success");
    std::exit(0);
}

}
