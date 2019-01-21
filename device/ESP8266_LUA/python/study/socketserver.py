#!/usr/bin/python
# -*- coding: UTF-8 -*-
# 文件名：server.py

import socket              # 导入 socket 模块
import threading
import simplejson

class client_Thread(threading.Thread):
    def __init__(self,sck):
        super(client_Thread,self).__init__()
        self.sck=sck

    def run(self):
        self.sck.send(str.encode("Connection Successful!"));
        print("running")
        while True:
            try:
                recvData = self.sck.recv(1024)
            except IOError:
                print("IOError.Maybe Client Disconnected")
                break;
            if recvData:
                print("recv Length:",int.from_bytes(recvData[:4], byteorder = 'big'))
                #print("recv Bytes:", recvData)
                motion = simplejson.loads(bytes.decode(recvData[6:]))
                print(motion)

s = socket.socket()         # 创建 socket 对象
host = "192.168.3.108" # 获取本地主机名
port = 8888                # 设置端口
s.bind((host, port))        # 绑定端口
s.listen(5)                 # 等待客户端连接
while True:
    sck, addr = s.accept()     # 建立客户端连接。
    print ('连接来自：', addr)
    client_Thread(sck).start()