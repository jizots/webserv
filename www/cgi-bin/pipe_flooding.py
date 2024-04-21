#  ************************************************************************** */
#                                                                             */
#                                                         :::      ::::::::   */
#    pipe_flooding.py                                     :+:      :+:    :+:   */
#                                                     +:+ +:+         +:+     */
#    By: tchoquet <tchoquet@student.42tokyo.jp>     +#+  +:+       +#+        */
#                                                 +#+#+#+#+#+   +#+           */
#    Created: 2024/04/18 13:50:11 by tchoquet          #+#    #+#             */
#                                                     ###   ########.fr       */
#                                                                             */
#  ************************************************************************** */

import sys
import errno

BUFFER_SIZE = 4096

data = "x" * BUFFER_SIZE
writeLen = 0

print("Content-Type: text/plain\r")
print("\r")

while True:
    try:
        sys.stdout.write(data)
        sys.stdout.flush()

    except IOError as e:
        if e.errno == errno.EAGAIN or e.errno == errno.EWOULDBLOCK:
            print("\nNon-blocking I/O failure")
        else:
            print("\nI/O error:", e)
        break

    writeLen += BUFFER_SIZE
    if writeLen >= (65535 * 2):
        print("\nNo error")
        break
