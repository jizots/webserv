# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    cgi_tester.py                                      :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: tchoquet <tchoquet@student.42tokyo.jp>     +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2024/04/22 18:07:21 by tchoquet          #+#    #+#              #
#    Updated: 2024/04/30 14:46:58 by tchoquet         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

import cgi

print("Content-Type: text/html\r")
print("\r")

cgi.print_form(cgi.parse())
cgi.print_directory()
cgi.print_environ()