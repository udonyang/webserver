#!/usr/bin/python

import sys
import socket

port = int(sys.argv[1])

so = socket.socket(
    socket.AF_INET,
    socket.SOCK_STREAM,
    socket.getprotobyname('tcp'))
so.bind(('localhost', port));
so_ac = so.listen(5)

while True:
  (so_ac, so_ac_addr) = so.accept()
  print so_ac_addr
  print so_ac.recv(1024)
  so_ac.shutdown(socket.SHUT_RD)
  so_ac.close();

so.shutdown(socket.SHUT_RDWR)
so.close()
