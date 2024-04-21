/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ClientSocketWriteTask.hpp                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tchoquet <tchoquet@student.42tokyo.jp>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/03/06 16:14:56 by tchoquet          #+#    #+#             */
/*   Updated: 2024/04/05 18:41:23 by tchoquet         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CLIENTSOCKETWRITETASK_HPP
# define CLIENTSOCKETWRITETASK_HPP

#include "IO/IOTask/IOTask.hpp"

#include "Socket/ClientSocket.hpp"
#include "HTTP/HTTPResponse.hpp"

namespace webserv 
{

class ClientSocketWriteTask : public IWriteTask
{
public:
    ClientSocketWriteTask(const ClientSocketPtr& clientSocket, const HTTPResponsePtr& resp);

    inline const FileDescriptor& fd() /*override*/ { return m_clientSocket->fileDescriptor(); };
    void write() /*override*/;
    
private:
    ClientSocketPtr m_clientSocket;
    std::vector<Byte> m_buffer;
    uint64 m_idx;
};

}

#endif // CLIENTSOCKETWRITETASK_HPP