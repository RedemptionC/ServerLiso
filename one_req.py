import socket

reqs=["fuck /BAD/ HTTP/1.1\nHost: www.baidu.com\r\n\r\n","GET /GOOD/ HTTP/1.1\r\nHook: www.baidu.com\r\n\r\n"]
# 注意这种是每次一个socket，不能体现出并发
for i in range(10):
    example_req=reqs[(i)%2]
    s=socket.socket(socket.AF_INET,socket.SOCK_STREAM)
    s.connect(("localhost",15441))
    s.send(example_req.encode("utf-8"))
    # print(example_req)
    print(s.recv(1024).decode('utf-8'))
    s.close()

# 为什么liso会陷入死循环。。。