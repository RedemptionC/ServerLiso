import socket

reqs=["fuck /BAD/ HTTP/1.1\nHost: www.baidu.com\r\n\r\n","GET /GOOD/ HTTP/1.1\r\nHook: www.baidu.com\r\n\r\n"]
response=["HTTP/1.1 400 Bad Request\r\n\r\n",reqs[1]]
# 注意这种是每次一个socket，不能体现出并发
for i in range(10):
    example_req=reqs[(i)%2]
    s=socket.socket(socket.AF_INET,socket.SOCK_STREAM)
    s.connect(("localhost",15441))
    s.send(example_req.encode("utf-8"))
    # print(example_req)
    res=s.recv(1024).decode('utf-8')
    print(res==response[i%2])
    
    s.close()

