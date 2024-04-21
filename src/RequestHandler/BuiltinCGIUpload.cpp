/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   BuiltinCGIUpload.cpp                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tchoquet <tchoquet@student.42tokyo.jp>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/17 15:58:50 by tchoquet          #+#    #+#             */
/*   Updated: 2024/04/20 08:58:22 by tchoquet         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "RequestHandler/BuiltinCGIUpload.hpp"
#include "Parser/HTTPHeaderValueParser/HTTPHeaderValueParser.hpp"
#include "Parser/MultipartFormParser/MultipartFormParser.hpp"
#include "RequestHandler/RequestHandler.hpp"

#include <unistd.h>
#include <vector>
#include <fcntl.h>
#include <cstdlib>

# define BUFFER_SIZE 1024

namespace webserv
{

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
                log << "builtinCGIUpload(): read() error" << "/n";
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
    HTTPHeaderValueParser headerValueParser;
    HTTPFieldValue        httpFieldValue;

    httpFieldValue = headerValueParser.parseHeaderValue(contentType, &HTTPHeaderValueParser::parseContentType);
    if (headerValueParser.isBadRequest())
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
    int fd = ::open(filePath.c_str(), O_WRONLY | O_CREAT | O_TRUNC, S_IWUSR | S_IRUSR | S_IRGRP | S_IROTH);
    if (fd < 0)
    {
        log << "builtinCGIUpload(): open(): " + std::string(std::strerror(errno));
        return ;
    }
    ssize_t writeLen = ::write(fd, data, lenData);
    if (writeLen > 0 && (size_t)writeLen != lenData)
        log << "builtinCGIUpload(): write() failed\n";
    if (::close(fd) < 0)
        log << "builtinCGIUpload(): close(): " + std::string(std::strerror(errno));
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
    std::cout << "status: 200\r\n";
    std::cout << "content-type: text/html\r\n";
    std::cout << "\r\n";
    std::cout << BUILT_IN_ERROR_PAGE(201, "Upload success");
    std::exit(0);
}

}
