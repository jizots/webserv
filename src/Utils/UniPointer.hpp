/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   UniPointer.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tchoquet <tchoquet@student.42tokyo.jp>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/02/11 14:31:09 by tchoquet          #+#    #+#             */
/*   Updated: 2024/04/01 18:20:46 by tchoquet         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef UNIPOINTER_HPP
# define UNIPOINTER_HPP

#include <cstddef>

namespace webserv
{

template<typename T>
class UniPointer
{
public:
    UniPointer();
    UniPointer(const UniPointer& mv);
    UniPointer(T* ptr);

    ~UniPointer();

private:
    T* m_pointer;

public:
    UniPointer& operator = (const UniPointer& mv);
    UniPointer& operator = (T* ptr);

    inline T& operator  * () const { return *m_pointer; }
    inline T* operator -> () const { return  m_pointer; }

    inline bool operator == (const UniPointer& rhs) const { return m_pointer == rhs.m_pointer; }
    inline bool operator != (const UniPointer& rhs) const { return m_pointer != rhs.m_pointer; }

    inline bool operator < (const UniPointer& rhs) const { return m_pointer < rhs.m_pointer; }

    inline operator bool () const { return m_pointer != NULL; }
};

} // namespace webserv

#ifndef UNIPOINTER_TPP
    #include "Utils/UniPointer.tpp"
#endif // UNIPOINTER_TPP

#endif // UNIPOINTER_HPP