# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    10seconds.py                                       :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: tchoquet <tchoquet@student.42tokyo.jp>     +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2024/04/22 20:28:41 by tchoquet          #+#    #+#              #
#    Updated: 2024/04/22 20:35:03 by tchoquet         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

import time

responseBody=\
'''\
<!DOCTYPE html>
<html>
<head>
    <title>10 seconds</title>
    <style> 
        body
        { 
            font-family:   Arial, sans-serif;
            text-align:    center;
            padding:       50px;
            font-size:     26px;
            margin-bottom: 20px;
        }
    </style>
</head>
<body>
    <h1>omataseshimashita</h1>
</body>
</html>
'''

time.sleep(10)

print('Status: 200 OK\r')
print('Content-Type: text/html\r')
print('\r')
print(responseBody)
