/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ContentType.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tchoquet <tchoquet@student.42tokyo.jp>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/03/09 14:12:54 by tchoquet          #+#    #+#             */
/*   Updated: 2024/03/25 16:04:05 by tchoquet         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Utils/ContentType.hpp"

namespace webserv
{

ContentType::ContentType(Id id_) : m_id(none)
{
    *this = id_;
}


ContentType::ContentType(const std::string& filePath): m_id(none)
{
    std::ifstream inputFile(filePath.c_str(), std::ios::binary);
    if (!inputFile)
        return ;

    std::map<std::string, Id> magicNumber;

    magicNumber["000001"]     = ico;
    magicNumber["1F8B08"]     = gzip;
    magicNumber["25504446"]   = pdf;
    magicNumber["424D"]       = bmp;
    magicNumber["47494638"]   = gif;
    magicNumber["49492A00"]   = tiff;
    magicNumber["504B0304"]   = zip;
    magicNumber["7B5C727466"] = rtf;
    magicNumber["89504E47"]   = png;
    magicNumber["FFD8FFE0"]   = jpeg;

    char buffer;
    std::stringstream out;
    for (int i = 0; inputFile.read(&buffer, 1) && i < 8; i++)
        out << std::hex << std::setw(2) << std::setfill('0') << (0xff & static_cast<int>(buffer));
    
    inputFile.close();

    for (int i = 0; out.str()[i]; i++) out.str()[i] = std::toupper(out.str()[i]);

    for (std::map<std::string, Id>::iterator it = magicNumber.begin(); it != magicNumber.end(); it++)
    {
        if (out.str().compare(0, it->first.size(), it->first) == 0)
        {
            *this = it->second;
            return ;
        }
    }

    size_t ext_pos = filePath.rfind('.');
    if (ext_pos != std::string::npos)
    {
        std::string ext = filePath.substr(ext_pos, filePath.size() - ext_pos);

        if (ext == ".html")
        {
            *this = html;
            return;
        }

        if (ext == ".xml")
        {
            *this = xml;
            return;
        }
    }
    
    *this = none;
}

ContentType& ContentType::operator = (const ContentType& rhs)
{
    if (this != &rhs)
    {
        m_id = rhs.m_id;
        m_str = rhs.m_str;
    }
    return *this;
}

ContentType& ContentType::operator = (const Id& id)
{
    m_id = id;
    switch (m_id)
    {
    case ico:
        m_str = "image/vnd.microsoft.icon";
        break;

    case gzip:
        m_str = "application/gzip";
        break;

    case pdf:
        m_str = "application/pdf";
        break;

    case bmp:
        m_str = "image/bmp";
        break;

    case gif:
        m_str = "image/gif";
        break;

    case tiff:
        m_str = "image/tiff";
        break;

    case zip:
        m_str = "application/zip";
        break;

    case rtf:
        m_str = "application/rtf";
        break;

    case png:
        m_str = "image/png";
        break;

    case jpeg:
        m_str = "image/jpeg";
        break;

    case html:
        m_str = "text/html";
        break;

    case xml:
        m_str = "application/xml";
        break;

    default:
        m_str = "text/plain";
        break;
    }
    
    return *this;
}

}
