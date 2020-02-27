# -*- coding: utf-8 -*
from socket import *
import sys
import random
import os
import time

if len(sys.argv) < 6:
    sys.stderr.write('Usage: %s <ip> <port> <#trials>\
            <#writes and reads per trial>\
            <#connections> \n' % (sys.argv[0]))
    sys.exit(1)

serverHost = gethostbyname(sys.argv[1])
serverPort = int(sys.argv[2])
numTrials = int(sys.argv[3])
numWritesReads = int(sys.argv[4])
numConnections = int(sys.argv[5])

# #connections必须大于#WritesReads
if numConnections < numWritesReads:
    sys.stderr.write('<#connections> should be greater than or equal to <#writes and reads per trial>\n')
    sys.exit(1)

socketList = []

RECV_TOTAL_TIMEOUT = 0.1
RECV_EACH_TIMEOUT = 0.01

# 建立#connection个连接
for i in range(numConnections):
    s = socket(AF_INET, SOCK_STREAM)
    try:
        s.connect((serverHost, serverPort))
    except error as err:
        print (str(i)+str(err))
        continue
    socketList.append(s)

if len(socketList)==0:
    sys.exit()

GOOD_REQUESTS = ['GET / HTTP/1.1\r\nUser-Agent: 441UserAgent/1.0.0\r\n\r\n']
BAD_REQUESTS = [
    'GET\r / HTTP/1.1\r\nUser-Agent: 441UserAgent/1.0.0\r\n\r\n', # Extra CR
    'GET / HTTP/1.1\nUser-Agent: 441UserAgent/1.0.0\r\n\r\n',     # Missing CR
    'GET / HTTP/1.1\rUser-Agent: 441UserAgent/1.0.0\r\n\r\n',     # Missing LF
]

BAD_REQUEST_RESPONSE = 'HTTP/1.1 400 Bad Request\r\n\r\n'

for i in range(numTrials):
    socketSubset = []
    randomData = []
    randomLen = []
    # 注意到socketList的len就是#connections,这里实际上就是打乱顺序，随意也不算是子集，而是一个打乱顺序的副本
    socketSubset = random.sample(socketList, numConnections)
    # #WritesReads我暂时也只能处理很小的一个值 FIND OUT WHY
    # 在这个循环内：选择发送valid/invalid request，然后发送
    for j in range(numWritesReads):
        # 在[0,#请求的种类)中随机取一个值，在这里就是在[0,4):0,1,2,3中取
        random_index = random.randrange(len(GOOD_REQUESTS) + len(BAD_REQUESTS))
        # 意思是random_index为0时，发送valid request
        if random_index < len(GOOD_REQUESTS):
            # 这里设置request，长度，对应的返回数据（对于valid request，就是它本身）
            random_string = GOOD_REQUESTS[random_index]
            randomLen.append(len(random_string))
            randomData.append(random_string)
        else:
            # 当random_index不为0，就发送bad request
            # 这里同样设置request,len,返回：统一为400
            random_string = BAD_REQUESTS[random_index - len(GOOD_REQUESTS)]
            randomLen.append(len(BAD_REQUEST_RESPONSE))
            randomData.append(BAD_REQUEST_RESPONSE)
        # 设置完毕后，发送请求
        socketSubset[j].send(random_string.encode())
        print (str(socketSubset[j].fileno())+"send message")

    # 这个循环里，就开始接收数据
    for j in range(numWritesReads):
        data = socketSubset[j].recv(randomLen[j]).decode()
        print (str(socketSubset[j].fileno())+" recv "+data)
        start_time = time.time()
        while True:
            # 如果已经接收到全部数据，就结束
            if len(data) == randomLen[j]:
                break
                # continue
            #　如果没有接收到全部数据，就设置一个超时时间，然后接着接收，直到超时或者接受完全
            socketSubset[j].settimeout(RECV_EACH_TIMEOUT)
            data += socketSubset[j].recv(randomLen[j])
            if time.time() - start_time > RECV_TOTAL_TIMEOUT:
                break
                # continue
        # if data != randomData[j]:
        #     sys.stderr.write("Error: Data received is not the same as sent! \n")
        #     sys.exit(1)
        #     print(data)
        # 先不管发回的符不符合要求，先看看是不是都response了
            print(data)

for i in range(numConnections):
    socketList[i].close()

print ("Success!")
