/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   UniPointer.tpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tchoquet <tchoquet@student.42tokyo.jp>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/02/11 15:02:34 by tchoquet          #+#    #+#             */
/*   Updated: 2024/03/20 18:10:48 by tchoquet         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef UNIPOINTER_TPP
# define UNIPOINTER_TPP

#include "Utils/UniPointer.hpp"

namespace webserv
{
    
template<typename T>
UniPointer<T>::UniPointer() : m_pointer(NULL)
{
}

template<typename T>
UniPointer<T>::UniPointer(const UniPointer& mv) : m_pointer(mv.m_pointer) 
{
    ((UniPointer&)mv).m_pointer = NULL;
}

template<typename T>
template<typename Y> UniPointer<T>::UniPointer(const UniPointer<Y>& mv) : m_pointer((T*)mv.m_pointer)
{
    ((UniPointer&)mv).m_pointer = NULL;
}

template<typename T>
UniPointer<T>::UniPointer(T* ptr) : m_pointer(ptr)
{
}

template<typename T>
template<typename Y> UniPointer<Y> UniPointer<T>::dynamicCast()
{
    Y* res = dynamic_cast<Y*>(m_pointer);
    if (res == NULL)
        return UniPointer<Y>();
    return UniPointer<Y>(*this);
}

template<typename T>
UniPointer<T>::~UniPointer()
{
    delete m_pointer;
}

template<typename T>
UniPointer<T>& UniPointer<T>::operator = (const UniPointer& mv)
{
    if (this != &mv)
    {
        delete m_pointer;
        m_pointer = mv.m_pointer;
        ((UniPointer&)mv).m_pointer = NULL;
    }
    return *this;
}

template<typename T>
UniPointer<T>& UniPointer<T>::operator = (T* ptr)
{
    if (m_pointer != ptr)
    {
        delete m_pointer;
        m_pointer = ptr;
    }
    return *this;
}

} // namespace webserv


#endif // UNIPOINTER_TPP