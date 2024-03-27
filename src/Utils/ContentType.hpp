/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ContentType.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tchoquet <tchoquet@student.42tokyo.jp>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/03/09 14:11:47 by tchoquet          #+#    #+#             */
/*   Updated: 2024/03/25 15:28:51 by tchoquet         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CONTENTTYPE_HPP
# define CONTENTTYPE_HPP

#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <map>
#include <string>

namespace webserv
{

class ContentType
{
public:
    enum Id {
		ico, gzip, pdf, bmp,
		gif, tiff, zip, rtf,
		png, jpeg, html, xml, 
        none
	};

public:
    ContentType(Id id = none);
    ContentType(const std::string& filePath);

private:
    Id m_id;
    std::string m_str;

public:
    ContentType& operator = (const ContentType& rhs);
    ContentType& operator = (const Id& id);
    inline bool operator == (const ContentType& rhs) const { return m_id == rhs.m_id; }
    inline bool operator != (const ContentType& rhs) const { return !(*this == rhs); }
    inline operator const std::string& () const { return m_str; }
};

}

#endif // CONTENTTYPE_HPP
