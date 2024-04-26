# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    404_image.py                                       :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: tchoquet <tchoquet@student.42tokyo.jp>     +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2024/03/21 17:14:00 by tchoquet          #+#    #+#              #
#    Updated: 2024/04/22 20:27:34 by tchoquet         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

import random

random.seed()

print('Location: /img/' + str(random.randint(1, 6)) + '-sorry.gif\r')
print('\r')