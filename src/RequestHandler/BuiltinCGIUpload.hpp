/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   BuiltinCGIUpload.hpp                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sotanaka <sotanaka@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/17 15:58:40 by tchoquet          #+#    #+#             */
/*   Updated: 2024/04/18 14:05:12 by sotanaka         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef BUILTINCGIUPLOAD_HPP
# define BUILTINCGIUPLOAD_HPP

# include <map>
# include <string>
namespace webserv
{

void builtinCGIUpload(const std::map<std::string, std::string>&);

}

#endif // BUILTINCGIUPLOAD_HPP