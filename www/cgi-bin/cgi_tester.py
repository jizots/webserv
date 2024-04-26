# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    cgi_tester.py                                      :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: tchoquet <tchoquet@student.42tokyo.jp>     +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2024/04/22 18:07:21 by tchoquet          #+#    #+#              #
#    Updated: 2024/04/22 22:30:39 by tchoquet         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

import sys
import os

print("Content-Type: text/plain\r")
print("\r")

if os.environ.get('QUERY_STRING', '') != '':
    print(os.environ['QUERY_STRING'])

body = sys.stdin.read()

if len(body) > 0:
    print(body)