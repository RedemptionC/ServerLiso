# Liso Server

## 文件说明
Login.html      cgi小程序之登录界面
Makefile        用于编译/运行/测试（obsolete，现使用浏览器测试） 服务器
README          本文件
algs4.jpg       用于测试的图片
calculator.html cgi小程序之计算器界面
cgi-bin         cgi程序目录
cgi-bin/calculator
cgi-bin/calculator.c    cgi小程序之计算器
cgi-bin/login           cgi小程序之简单登录
cgi-bin/login.c         
csapp.c
csapp.h                 包含一些helper函数，截取自CS:APP
example.c       测试parser
home.html       网站主界面
lexer.l         flex配置文件
lisod.c         服务器主要的代码
one_req.py      (obsolete) 测试代码
parse.c
parse.h
parser.y        parser配置文件
demo.html       html demo
sample_request_example
sample_request_realistic

## 程序运行基本流程
![control flow](./controFlow.png)

## 运行方式
切换到当前目录
make rebuild
make run
(默认端口为15441，可在makefile 里修改)
也可./ lisod <port>

