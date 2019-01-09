import threading
import hashlib
import socket
import base64

global clients
clients = {}

def notify(message):
    for connection in clients.values():connection.send('%c%c%s' % (0x81, len(message), message))

class websocket_thread(threading.Thread):
    def __init__(self, connection, username):
        super(websocket_thread, self).__init__()
        self.connection = connection
        self.username = username

    def run(self):
        print('new websocket client joined!')
        data = self.connection.recv(1024)
        headers = self.parse_headers(data)
        token = self.generate_token(headers['Sec-Websocket-Key'])
        self.connection.send('\
HTTP/1.1 101 WebSocket Protocol Hybi-10\r\n\
Upgrade: WebSocket\r\n\
Connection: Upgrade\r\n\
Sec-WebSocket-Accept: %s\r\n\r\n' % token)
        while True:
            try:
                data = self.connection.recv(1024)
            except socket.error as e:
                print('unexpected error:',e)
                clients.pop(self.username)
                break
            data = self.parse_data(data)
            if len(data) == 0:
                continue
            message = self.username + ":" + data
            notify(message)
def parse_data(self, msg):
    v = ord(msg[1]) & 0x7f
    if v == 0x7e:
        p = 4
    elif v == 0x7f:
        p = 10
    else:
        p=2
    mask = msg[p:p+4]
    data = msg[p+4:]
    return ''.join([chr(ord(v) ^ ord(mask[k%4])) for k, v in enumerate(data)])
def parse_headers(self,msg):
    headers = {}
    header, data = msg.split('\r\n\r\n', 1)
    for line in header.split('\r\n')[1:]:
        key, value = line.split(':', 1)
        headers[key] = value
    headers['data'] = data
    return headers
def generate_token(self,msg):
    key = msg + '258EAFA5-E914-47DA-95CA-C5AB0DC85B11'
    ser_key = hashlib.sha1(key).digest()
    return base64.b64encode(ser_key)














































