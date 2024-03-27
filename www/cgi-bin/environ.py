import os

responseBody = '''<!DOCTYPE html>
<html>
<head>
    <title>Hello World</title>
    <style> 
        body { 
            font-family: Arial, sans-serif;
        }
        h1 {
            color: #333;
            text-align: center;
        }
        ul {
            list-style-type: none;
            padding: 3;
        }
        li {
            margin-bottom: 10px;
        }
        li span {
            font-weight: bold;
        }
    </style>
</head>
<body>
    <h1>Environment Variables:</h1>
    <ul> 
        <li><span>AUTH_TYPE:</span> '''         + os.environ.get('AUTH_TYPE', '')         + '''</li>
        <li><span>CONTENT_LENGTH:</span> '''    + os.environ.get('CONTENT_LENGTH', '')    + '''</li>
        <li><span>CONTENT_TYPE:</span> '''      + os.environ.get('CONTENT_TYPE', '')      + '''</li>
        <li><span>GATEWAY_INTERFACE:</span> ''' + os.environ.get('GATEWAY_INTERFACE', '') + '''</li>
        <li><span>PATH_INFO:</span> '''         + os.environ.get('PATH_INFO', '')         + '''</li>
        <li><span>PATH_TRANSLATED:</span> '''   + os.environ.get('PATH_TRANSLATED', '')   + '''</li>
        <li><span>QUERY_STRING:</span> '''      + os.environ.get('QUERY_STRING', '')      + '''</li>
        <li><span>REMOTE_ADDR:</span> '''       + os.environ.get('REMOTE_ADDR', '')       + '''</li>
        <li><span>REMOTE_HOST:</span> '''       + os.environ.get('REMOTE_HOST', '')       + '''</li>
        <li><span>REMOTE_IDENT:</span> '''      + os.environ.get('REMOTE_IDENT', '')      + '''</li>
        <li><span>REMOTE_USER:</span> '''       + os.environ.get('REMOTE_USER', '')       + '''</li>
        <li><span>REQUEST_METHOD:</span> '''    + os.environ.get('REQUEST_METHOD', '')    + '''</li>
        <li><span>SCRIPT_NAME:</span> '''       + os.environ.get('SCRIPT_NAME', '')       + '''</li>
        <li><span>SERVER_NAME:</span> '''       + os.environ.get('SERVER_NAME', '')       + '''</li>
        <li><span>SERVER_PORT:</span> '''       + os.environ.get('SERVER_PORT', '')       + '''</li>
        <li><span>SERVER_PROTOCOL:</span> '''   + os.environ.get('SERVER_PROTOCOL', '')   + '''</li>
        <li><span>SERVER_SOFTWARE:</span> '''   + os.environ.get('SERVER_SOFTWARE', '')   + '''</li>
    </ul>
</body>
</html>'''

print('status: 200 OK\r\n'),
print('Content-Type: text/html\r\n'),
print('\r\n'),
print(responseBody)
