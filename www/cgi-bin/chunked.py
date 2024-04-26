#!/usr/bin/env python
# -*- coding: utf-8 -*-

import sys
import time

def send_chunk(data):
    # データサイズを16進数で出力し、その後にデータ本体とCRLFを出力
    sys.stdout.write("{0:X}\r\n{1}\r\n".format(len(data), data))

def end_chunks():
    # チャンクエンコーディングの終了を示す0サイズのチャンク
    sys.stdout.write("0\r\n\r\n")

# ヘッダを出力
print('Status: 200 OK\r')
print ('Content-Type: text/plain\r')
print ('Transfer-Encoding: chunked\r')
print ('\r')

# チャンク化されたデータの送信
send_chunk("Hello, ")
time.sleep(0.3)
send_chunk("World!\n")
time.sleep(0.3)
send_chunk("This is a chunked response from a CGI script.\n")
time.sleep(0.3)

# チャンクの終了
end_chunks()
