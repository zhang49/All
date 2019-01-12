# -*- coding: UTF-8 -*-

import pymysql
import socket
import simplejson
import threading
import time

db = pymysql.connect('119.23.207.135','root','123456','zzcdb')
cursor = db.cursor()
cursor.execute("SELECT * FROM DeviceToken")
data = cursor.fetchone()
print("Database version : %s ", data)
db.close()

def checkToken(queryData):
    db = pymysql.connect('119.23.207.135', 'root', '123456', 'zzcdb')
    cursor = db.cursor()
    cursor.execute("SELECT id FROM DeviceToken WHERE token =%s" , queryData)
    data = cursor.fetchone()
    db.close()
    if not data == None :
        return True
    return False

class client_thread(threading.Thread):
    def __init__(self, sck):
        super(client_thread, self).__init__()
        self.sck = sck


    def run(self):
            dd = {}
            dd["type"] = "Request_Token"
            dd["data"] = ""
            self.sck.send(str.encode(simplejson.dumps(dd)))
            data = self.sck.recv(1024)
            if data :
                root = simplejson.loads(bytes.decode(data))
                if root["type"]=="Response_Token" and checkToken(root["data"]["token"]) :
                    print('a client pass check token')
                    while True:
                        ret = {}
                        ret["type"] = "Hearbeat"
                        ret["data"] = ""
                        self.sck.send(str.encode(simplejson.dumps(ret)))
                        time.sleep(3)
                else:
                     print('error')



def startServer():
    s = socket.socket()         # 创建 socket 对象
    host = '192.168.43.92' # 获取本地主机名
    port = 8860                # 设置端口
    s.bind((host, port))        # 绑定端口
    s.listen(5)                 # 等待客户端连接
    while True:
        c, addr = s.accept()     # 建立客户端连接。
        print ('接受一个连接,ip ：', addr)
        client_thread(c).start()


startServer()