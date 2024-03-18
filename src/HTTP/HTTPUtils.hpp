/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HTTPUtils.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: emukamada <emukamada@student.42.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/03/09 15:51:05 by ekamada           #+#    #+#             */
/*   Updated: 2024/03/11 22:21:55 by emukamada        ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Utils/Utils.hpp"



namespace webserv
{

bool isUnReserved(Byte c);
bool isReserved(Byte c);
bool isPchar(Byte c);
bool isToken(Byte c);
bool isPrintableAscii(Byte c);
std::string trimOptionalSpace(const std::string& str);

}
