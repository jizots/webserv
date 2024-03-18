/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGIWriteTask.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tchoquet <tchoquet@student.42tokyo.jp>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/03/06 16:16:17 by tchoquet          #+#    #+#             */
/*   Updated: 2024/03/06 17:29:58 by tchoquet         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CGIWRITETASK_HPP
# define CGIWRITETASK_HPP

#include "IO/IOTask.hpp"

#include "HTTP/HTTPRequest.hpp"

namespace webserv
{

class CGIWriteTask : public IWriteTask
{
public:
    CGIWriteTask(int writeFd, const HTTPRequest& request);

    int fd() /*override*/;
    void write() /*override*/;

    ~CGIWriteTask() /*override*/;

private:
    int m_writeFd;
    std::vector<Byte> m_buffer;
    uint64 m_idx;
};

}

#endif // CGIWRITETASK_HPP