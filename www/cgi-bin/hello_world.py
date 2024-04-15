
responseBody=\
'''\
<!DOCTYPE html>
<html>
<head>
    <title>Hello World</title>
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
    <h1>Hello World</h1>
</body>
</html>
'''

print('Status: 200 OK\r')
print('Content-Type: text/html\r')
print('\r')
print(responseBody)
