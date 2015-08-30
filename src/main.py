import os
import sys
import BaseHTTPServer
import cgi.loginpage

def hearthstone_list():
  pass

class Handler(BaseHTTPServer.BaseHTTPRequestHandler):
  def do_GET(self):
    index_file = open('html/index.html')
    index = ''.join(index_file.readlines())
    index_file.close()
    self.log_message(self.path);
    self.wfile.write(index)
    self.send_response(200)
    return 0

def main(argv):
  if len(argv) < 3:
    return -2
  host = argv[1]
  port = int(argv[2])
  server = BaseHTTPServer.HTTPServer((host, port), Handler)
  server.serve_forever()
  return 0

if __name__ == '__main__':
  main(sys.argv)
