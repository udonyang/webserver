import os
import sys
import BaseHTTPServer

class Handler(BaseHTTPServer.BaseHTTPRequestHandler):
  def do_GET(self):
    index_file = open('html/index.html')
    index = ''.join(index_file.readlines())
    index_file.close()
    self.wfile.write(index)
    self.send_response(200)
    return 0

def main(argv):
  if len(argv) < 2:
    return -2
  port = argv[1]
  server = BaseHTTPServer.HTTPServer(("localhost", int(port)), Handler)
  server.serve_forever()
  return 0

if __name__ == '__main__':
  main(sys.argv)
