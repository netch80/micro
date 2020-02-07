#!/usr/bin/env python3

import socket

cs = socket.socket(socket.AF_INET, socket.SOCK_STREAM, socket.IPPROTO_SCTP)
cs.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
cs.connect(('127.0.0.1', 5210))
while True:
    data = cs.recv(65536)
    print("Got data ({}): {!r}".format(len(data), data))
    if not data:
        break
