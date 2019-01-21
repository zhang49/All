# -*- coding:utf8 -*-
import struct
import threading
import hashlib
import socket
import base64
import time


global clients
clients = {}


# 通知客户端
def notify(message):
    for connection in clients.values():
        msgLen = len(message)
        backMsgList = []
        backMsgList.append(struct.pack('B', 129))
        if msgLen <= 125:
            backMsgList.append(struct.pack('b', msgLen))
        elif msgLen <= 65535:
            backMsgList.append(struct.pack('b', 126))
            backMsgList.append(struct.pack('>h', msgLen))
        elif msgLen <= (2 ^ 64 - 1):
            backMsgList.append(struct.pack('b', 127))
            backMsgList.append(struct.pack('>h', msgLen))
        else:
            print("the message is too long to send in a time")
            return
        message_byte = bytes()
        print(type(backMsgList[0]))
        for c in backMsgList:
            # if type(c) != bytes:
            # print(bytes(c, encoding="utf8"))
            message_byte += c
        message_byte += bytes(message, encoding="utf8")
        # print("message_str : ", str(message_byte))
        # print("message_byte : ", bytes(message_str, encoding="utf8"))
        # print(message_str[0], message_str[4:])
        # self.connection.send(bytes("0x810x010x63", encoding="utf8"))
        connection.send(message_byte)


# 客户端处理线程
class websocket_thread(threading.Thread):
    def __init__(self, connection, username):
        super(websocket_thread, self).__init__()
        self.connection = connection
        self.username = username

    def run(self):
        print('new websocket client joined!')
        data = self.connection.recv(1024)
        headers = self.parse_headers(data)
        token = self.generate_token(headers['Sec-WebSocket-Key'])
        self.connection.send(b'\
HTTP/1.1 101 WebSocket Protocol Hybi-10\r\n\
Upgrade: WebSocket\r\n\
Connection: Upgrade\r\n\
Sec-WebSocket-Accept: %s\r\n\r\n' % token)
        sendData = b'{\
        "type" : "GetDoorConfig",\
        "user_data" : ""\
        "data" : ""\
        }'
        while True:
            data=self.parse_data(sendData)
            notify(data)

    def recvProcess(self):
        try:
            data = self.connection.recv(1024)
        except socket.error as e:
            print("unexpected error: ", e)
            clients.pop(self.username)
        data = self.parse_data(data)
        if len(data) == 0:
            return 0
        # message = self.username + ": " + data
        notify(data)
    #data is bytes
    def parse_data(self, data):
        #print("str:%s",bytes.decode(msg))
        msg_len = data[1] & 127
        if msg_len == 126:
            mask = data[4:8]
            content = data[8:]
        elif msg_len == 127:
            mask = data[10:14]
            content = data[14:]
        else:
            mask = data[2:6]
            content = data[6:]
        raw_str = ''
        print('content:',bytes.decode(content))
        for i,d in enumerate(content):
            raw_str += chr( d ^ mask[i % 4])
        return raw_str

    def parse_headers(self, msg):
        headers = {}
        header, data = msg.split(b'\r\n\r\n', 1)
        print('recv header pairs')
        for line in header.split(b'\r\n')[1:]:
            key, value = line.split(b': ', 1)
            headers[bytes.decode(key)] = bytes.decode(value)
        headers['data'] = data
        return headers

    def generate_token(self, msg):
        key = msg + '258EAFA5-E914-47DA-95CA-C5AB0DC85B11'
        #bb=bytes(key,encoding = "utf8")

        ser_key = hashlib.sha1(bytes(key,encoding="utf-8")).digest()
        return base64.b64encode(ser_key)


# 服务端
class websocket_server(threading.Thread):
    def __init__(self, port):
        super(websocket_server, self).__init__()
        self.port = port

    def run(self):
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        sock.bind(('192.168.20.2', self.port))
        sock.listen(5)
        print('websocket server started!')
        while True:
            connection, address = sock.accept()
            try:
                username = "ID" + str(address[1])
                thread = websocket_thread(connection, username)
                thread.start()
                clients[username] = connection
            except socket.timeout:
                print('websocket connection timeout!')


if __name__ == '__main__':
    server = websocket_server(3380)
    server.start()