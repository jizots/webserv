#! /usr/bin/python

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

print('Status: 200 OK\r\n'),
print('Content-Type: text/html\r\n'),
print('\r\n'),
print(responseBody),
