# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    environ.py                                         :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: tchoquet <tchoquet@student.42tokyo.jp>     +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2024/04/22 20:29:17 by tchoquet          #+#    #+#              #
#    Updated: 2024/04/23 18:25:44 by tchoquet         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

import abc
import os

print('status: 200 OK\r')
print('Content-Type: text/plain\r')
print('\r')
for key in os.environ:
    print(key + "=" + os.environ[key])