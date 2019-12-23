import socket

example_req="GET /~prs/15-441-F15/ HTTP/1.1\r\nHost: www.baidu.com\r\n\r\n"
s=socket.socket(socket.AF_INET,socket.SOCK_STREAM)
s.connect(("localhost",15441))
s.send(example_req.encode("utf-8"))
print(example_req)
print(s.recv(1024).decode('utf-8'))
s.close()

# 为什么liso会陷入死循环。。。