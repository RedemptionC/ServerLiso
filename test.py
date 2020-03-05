import requests 
import threading
from bs4 import BeautifulSoup

url="http://localhost:15441"

def test_head(threadName):
    rsp=requests.head(url)
    print("{} : {}".format(threadName,rsp.status_code))

def test_get(threadName):
    rsp=requests.get(url)
    bsobj=BeautifulSoup(rsp.text,"lxml")
    print("{} : #{} {}".format(threadName,rsp.status_code,bsobj.h1))

class myThread (threading.Thread):
    def __init__(self, threadID, name):
        threading.Thread.__init__(self)
        self.threadID = threadID
        self.name = name
    def run(self):
        # test_head(self.name)
        test_get(self.name)


num=512
threads=list()
for i in range(num):
    threads.append(myThread(i,"thread {}".format(i)))

for i in threads:
    i.start()

for i in threads:
    i.join()
