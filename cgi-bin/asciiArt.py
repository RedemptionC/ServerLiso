#!/usr/bin/python3

import os
import art
import sys
import cgi

string=os.getenv("QUERY_STRING")
form=cgi.FieldStorage()
s=form.getfirst('input',"")
content="<p>{}</p>".format(art.text2art(s))
print("Connection: close\r\n")
print("Content-length: {}\r\n".format(len(content)))
print("Content-type: text/html\r\n\r\n")
print(content,flush=True)

sys.exit()