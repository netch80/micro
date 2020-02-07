#!/usr/bin/env python3

import socket

ls = socket.socket(socket.AF_INET, socket.SOCK_STREAM, socket.IPPROTO_SCTP)
ls.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
ls.bind(('127.0.0.1', 5210))
ls.listen(1)
while True:
    server, addr = ls.accept()
    print("Connection from {}".format(addr))
    server.send(b"hi")
    server.close()
