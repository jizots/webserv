/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   SharedPtr.tpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tchoquet <tchoquet@student.42tokyo.jp>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/02/13 18:31:06 by tchoquet          #+#    #+#             */
/*   Updated: 2024/03/31 21:29:30 by tchoquet         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SHAREDPTR_TPP
# define SHAREDPTR_TPP

#include "Utils/SharedPtr.hpp"

namespace webserv
{

template<typename T>
SharedPtr<T>::SharedPtr() : m_pointer(NULL), m_refCount(NULL)
{
}

template<typename T>
SharedPtr<T>::SharedPtr(const SharedPtr& cp) : m_pointer(cp.m_pointer), m_refCount(cp.m_refCount)
{
    *m_refCount += 1;
}

template<typename T>
template<typename Y> SharedPtr<T>::SharedPtr(const SharedPtr<Y>& cp) : m_pointer(cp.m_pointer), m_refCount(cp.m_refCount)
{
    *m_refCount += 1;
}

template<typename T>
SharedPtr<T>::SharedPtr(T* ptr) : m_pointer(ptr), m_refCount(new uint32(1))
{
}

template<typename T>
void SharedPtr<T>::clear()
{
    if (m_refCount == NULL)
        return;
    
    *m_refCount = *m_refCount > 0 ? *m_refCount - 1 : 0;

    if (*m_refCount == 0)
    {
        delete m_pointer;
        delete m_refCount;
    }

    m_pointer = NULL;
    m_refCount = NULL;

    return;
}

template<typename T>
template<typename Y> inline SharedPtr<Y> SharedPtr<T>::dynamicCast() const
{
    SharedPtr<Y> newPtr;

    Y* castedPtr = dynamic_cast<Y*>(m_pointer);
    if (castedPtr != NULL)
    {
        newPtr.m_pointer = castedPtr;
        newPtr.m_refCount = m_refCount;
        *newPtr.m_refCount += 1;
    }
    return newPtr;
}

template<typename T>
SharedPtr<T>::~SharedPtr()
{
    clear();
}

template<typename T>
SharedPtr<T>& SharedPtr<T>::operator = (const SharedPtr& cp)
{
    if (this != &cp)
    {
        clear();
        m_pointer = cp.m_pointer;
        m_refCount = cp.m_refCount;
        *m_refCount += 1;
    }
    return *this;
}

}

#endif // SHAREDPTR_TPP