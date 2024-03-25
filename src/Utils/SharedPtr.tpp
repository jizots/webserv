/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   SharedPtr.tpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tchoquet <tchoquet@student.42tokyo.jp>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/02/13 18:31:06 by tchoquet          #+#    #+#             */
/*   Updated: 2024/03/24 16:53:02 by tchoquet         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SHAREDPTR_TPP
# define SHAREDPTR_TPP

#include "Utils/SharedPtr.hpp"

namespace webserv
{

template<typename T>
SharedPtr<T>::SharedPtr() : m_pointer(new T* (NULL)), m_refCount(new uint32(1))
{
}

template<typename T>
SharedPtr<T>::SharedPtr(const SharedPtr& cp) : m_pointer(cp.m_pointer), m_refCount(cp.m_refCount)
{
    *m_refCount += 1;
}

template<typename T>
SharedPtr<T>::SharedPtr(T* ptr) : m_pointer(new T* (ptr)), m_refCount(new uint32(1))
{
}

template<typename T>
void SharedPtr<T>::clear()
{
    *m_refCount = *m_refCount > 0 ? *m_refCount - 1 : 0;

    if (*m_refCount > 0)
        return;
    
    delete *m_pointer;
    delete m_pointer;
    delete m_refCount;
}

template<typename T>
template<typename Y> inline SharedPtr<Y> SharedPtr<T>::dynamicCast() const
{
    Y* res = dynamic_cast<Y*>(*m_pointer);
    if (res == NULL)
        return SharedPtr<Y>();
    return SharedPtr<Y>(*this);
}

template<typename T>
SharedPtr<T>::~SharedPtr()
{
    clear();
}

template<typename T>
template<typename Y> SharedPtr<T>::SharedPtr(const SharedPtr<Y>& cp) : m_pointer((T**)cp.m_pointer), m_refCount(cp.m_refCount)
{
    *m_refCount += 1;
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

template<typename T>
SharedPtr<T>& SharedPtr<T>::operator = (T* ptr)
{
    *m_pointer = ptr;
    return *this;
}

}

#endif // SHAREDPTR_TPP