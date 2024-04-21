/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   SharedPtr.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tchoquet <tchoquet@student.42tokyo.jp>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/02/13 18:27:47 by tchoquet          #+#    #+#             */
/*   Updated: 2024/02/13 18:29:04 by tchoquet         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SHAREDPTR_HPP
# define SHAREDPTR_HPP

#include <cstddef>
#include <ostream>

#include "Utils/Types.hpp"

namespace webserv
{

template<typename T>
class SharedPtr
{
template<typename Y> friend class SharedPtr;
public:
    SharedPtr();
    SharedPtr(const SharedPtr&);
    template<typename Y> SharedPtr(const SharedPtr<Y>&);

    SharedPtr(T* ptr);

    void clear();
    template<typename Y> SharedPtr<Y> dynamicCast() const;
    
    ~SharedPtr();

protected:
    T* m_pointer;
    uint32* m_refCount;

private:

public:
    SharedPtr& operator = (const SharedPtr&);

    inline T& operator  * () const { return *m_pointer; }
    inline T* operator -> () const { return  m_pointer; }

    inline bool operator == (const SharedPtr& rhs) const { return m_pointer == rhs.m_pointer; }
    inline bool operator == (T* rhs) const { return m_pointer == rhs; }
    inline bool operator != (const SharedPtr& rhs) const { return m_pointer != rhs.m_pointer; }

    inline bool operator < (const SharedPtr& rhs) const { return m_pointer < rhs.m_pointer; }

    inline operator bool () const { return m_pointer != NULL; }

    friend std::ostream& operator << (std::ostream& os, const SharedPtr& ptr)
    {
        os << (void*)ptr.m_pointer << " | ref count: " << *ptr.m_refCount;
        return os;
    }
};

}

#ifndef SHAREDPTR_TPP
    #include "Utils/SharedPtr.tpp"
#endif // SHAREDPTR_TPP

#endif // SHAREDPTR_HPP