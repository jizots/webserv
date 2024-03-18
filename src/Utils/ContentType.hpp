/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ContentType.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sotanaka <sotanaka@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/03/09 14:11:47 by tchoquet          #+#    #+#             */
/*   Updated: 2024/03/18 14:16:17 by sotanaka         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CONTENTTYPE_HPP
# define CONTENTTYPE_HPP

#include <string>

namespace webserv
{

class ContentType
{
public:
    enum Id { none, html, gif, ico };

public:
    ContentType(Id id = none);
    ContentType(const std::string& ext);

private:
    Id m_id;
    std::string m_str;

public:
    ContentType& operator = (const ContentType& rhs);
    ContentType& operator = (const Id& id);
    inline bool operator == (const ContentType& rhs) { return m_id == rhs.m_id; }
    inline operator const std::string& () const { return m_str; }
};

}

#endif // CONTENTTYPE_HPP