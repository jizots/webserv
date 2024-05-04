# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    cookie_test.py                                     :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: tchoquet <tchoquet@student.42tokyo.jp>     +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2024/05/04 13:15:26 by tchoquet          #+#    #+#              #
#    Updated: 2024/05/04 14:59:07 by tchoquet         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

import json
import sys
import Cookie
import os
import cgi
import uuid

from matplotlib import use

DATABASE_PATH = "../../database.json"

def loadDatabase():
    databaseFile = open(DATABASE_PATH, "r")
    database = json.load(databaseFile)
    databaseFile.close()
    return database

def saveDatabase(database):
    databaseFile = open(DATABASE_PATH, "w")
    json.dump(database, databaseFile, indent=4)
    databaseFile.close()

def indexForUsername(database, username):
    if username:
        for i, user in enumerate(database):
            if username == user["username"]:
                return i
    return None

def indexForCookie(database, cookie):
    for i, user in enumerate(database):
        if cookie in user["cookies"]:
            return i
    return None

def deleteCookie(user, cookie):
    for i, c in enumerate(user["cookies"]):
        if c == cookie:
            del user["cookies"][i]

pathInfo = os.environ.get("PATH_INFO")
cookies = Cookie.SimpleCookie()
database = loadDatabase()

if "HTTP_COOKIE" in os.environ:
    cookies.load(os.environ.get("HTTP_COOKIE"))

if pathInfo == "/login":
    print("Location: /login.html\r")
    print("\r")
    exit()

if pathInfo == "/account":
    form = cgi.FieldStorage()
    userIndex = None

    if "session" in cookies:
        userIndex = indexForCookie(database, cookies["session"].value)
    
    if userIndex == None:

        userIndex = indexForUsername(database, form.getvalue("username", None))
        if userIndex != None and database[userIndex]["password"] == form.getvalue("password", None):
            newSessionCookie = str(uuid.uuid4())
            database[userIndex]["cookies"].append(newSessionCookie)
            saveDatabase(database)
            print("Status: 302 Found\r")
            print("Location: /cgi-bin/cookie_test.py/account\r")
            print("Set-Cookie: session=" + newSessionCookie + "\r")
            print("Content-Type: text/plain\r")
            print("\r")
            print("redirection")
            exit()
        else:
            userIndex = None

    if userIndex == None:
        print("Status: 302 Found\r")
        print("Location: /cgi-bin/cookie_test.py/login\r")
        print("Content-Type: text/plain\r")
        print("\r")
        print("redirection")

    else:
        print("Content-Type: text/html\r")
        print("\r")
        print("<h1>Hello " +  database[userIndex]["username"] + "<button onclick=\"window.location.href='/cgi-bin/cookie_test.py/logout'\">Logout</button>")
    
    exit()

if pathInfo == "/register":
    print("Location: /register.html\r")
    print("\r")
    exit()

if pathInfo == "/create_user":
    
    try:
        requestData = json.loads(sys.stdin.read())
    except:
        requestData = None

    if requestData == None or not "username" in requestData or not "password" in requestData:
        print("Status: " + str(400) + " " + "Bad Request" + "\r")
        print("Content-Type: text/html\r")
        print("\r")
        print("<html>\n")
        print("   <head><title>" + str(400) + " " + "Bad Request" + "</title></head>\n")
        print("   <body>\n")
        print("       <center><h1>" + str(400) + " " + "Bad Request" + "</h1></center>\n")
        print("       <hr><center>webserv</center>\n")
        print("   </body>\n")
        print("</html>")
        exit()
    else:
        database = loadDatabase()
        if indexForUsername(database, requestData["username"]) == None:
            database.append({
                "username": requestData["username"], 
                "fullname": requestData.get("fullname", ""),
                "password": requestData["password"], 
                "cookies": []
            })
            saveDatabase(database)
            print("Status: 200\r")
            print("Content-Type: text/json\r")
            print("\r")
            print(json.dumps({}))

        else:
            print("Status: 400\r")
            print("Content-Type: text/json\r")
            print("\r")
            print(json.dumps({}))

if pathInfo == "/logout":
    userIndex = None

    if "session" in cookies:
        userIndex = indexForCookie(database, cookies["session"].value)

    if userIndex != None:
        deleteCookie(database[userIndex], cookies["session"].value)
        saveDatabase(database)
        print("Status: 302 Found\r")
        print("Location: /cgi-bin/cookie_test.py/login\r")
        print("Content-Type: text/plain\r")
        print("\r")
        print("redirection")
else:
    print("Status: " + str(404) + " " + "Not Found" + "\r")
    print("Content-Type: text/html\r")
    print("\r")
    print("<html>\n")
    print("   <head><title>" + str(404) + " " + "Not Found" + "</title></head>\n")
    print("   <body>\n")
    print("       <center><h1>" + str(404) + " " + "Not Found" + "</h1></center>\n")
    print("       <hr><center>webserv</center>\n")
    print("   </body>\n")
    print("</html>")
    exit()
