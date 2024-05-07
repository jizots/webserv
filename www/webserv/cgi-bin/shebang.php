#!/usr/bin/php
<?php

// HTTPヘッダを明示的に出力
echo "Content-Type: text/html\r\n";
echo "Status: 200 OK\r\n";
echo "\r\n";  // ヘッダの終わりを示す空行

// HTMLコンテンツを変数に格納
$responseBody = <<<HTML
<!DOCTYPE html>
<html>
<head>
    <title>Shebang</title>
    <style>
        body {
            font-family: Arial, sans-serif;
            text-align: center;
            padding: 50px;
            font-size: 26px;
            margin-bottom: 20px;
        }
    </style>
</head>
<body>
    <h1>Hello Shebang</h1>
</body>
</html>
HTML;

// レスポンスボディを出力
echo $responseBody;
?>