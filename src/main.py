import os
import sys
import BaseHTTPServer
import cgi.loginpage
import subprocess
import jinja2 
import urlparse

tmpl_env = jinja2.Environment(loader=jinja2.PackageLoader('html', '.'))

class HearthStoneHandler:
  def __init__(self, qs):
    self.qs = qs

  def list(self):
    return (tmpl_env.get_template('hearthstone/list.html').render(name='Dwylkz'), 200)

  def add(self):
    return (tmpl_env.get_template('hearthstone/add.html').render(name='Dwylkz'), 200)

class Handler(BaseHTTPServer.BaseHTTPRequestHandler):
  def do_GET(self):
    form = urlparse.urlparse(self.path)
    qs = urlparse.parse_qs(form.query)
    self.log_message(form.path)
    self.log_message(repr(qs))

    retcode = 404
    if qs.has_key('action'):
      output = 'page not found'
      if 'list' in qs['action']:
        (output, retcode) = HearthStoneHandler(qs).list()
      elif 'add' in qs['action']:
        (output, retcode) = HearthStoneHandler(qs).add()
      self.wfile.write(output)

    self.send_response(retcode)
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
