import os
import sys
import BaseHTTPServer
import cgi.loginpage
import subprocess
import jinja2 
import urlparse
import MySQLdb
import MySQLdb.cursors

tmpl_env = jinja2.Environment(loader=jinja2.PackageLoader('html', '.'))
reload(sys)
sys.setdefaultencoding('utf-8')

class HearthStoneHandler:
  def __init__(self, ctx, qs):
    self.qs = qs
    self.ctx = ctx

  def mysql(self, sql, args = dict()):
    db = MySQLdb.connect('localhost', 'dwylkz', 'forconnect', 'dwylkz',
        cursorclass=MySQLdb.cursors.DictCursor)
    c = db.cursor()
    escaped_args = dict()
    for item in args.iteritems():
      escaped_args[item[0]] = db.escape_string(item[1][0])
    self.ctx.log_message(str.format('escaped=({args})', **{'args': repr(escaped_args)}));
    sql = str.format(sql, **escaped_args)
    c.execute(sql)
    self.ctx.log_message('sql=(%s)'%sql);
    result = c.fetchall()
    self.ctx.log_message('result=(%s)'%repr(result));
    return result

  def list(self):
    sql = 'select * from hearthstone'
    if self.qs.has_key('player'):
      sql += ' where player="{player}"'
    result = self.mysql(sql, self.qs)
    return (tmpl_env.get_template('hearthstone/list.html').render(result=result), 200)

  def add(self):
    self.mysql(
        '''
        insert hearthstone
          (player, player_deck, opponent_deck, is_win)
        values
          ("{player}", "{player_deck}", "{opponent_deck}", {is_win})
        ''',
        self.qs)
    return self.list()

class Handler(BaseHTTPServer.BaseHTTPRequestHandler):
  def do_GET(self):
    form = urlparse.urlparse(self.path)
    qs = urlparse.parse_qs(form.query)
    self.log_message(form.path)
    self.log_message(repr(qs))

    if form.path == '/':
      self.log_message('redirect to action=list')
      qs['action'] = ['list']

    retcode = 404
    if qs.has_key('action'):
      try:
        output = 'page not found'
        if 'list' in qs['action']:
          (output, retcode) = HearthStoneHandler(self, qs).list()
        elif 'add' in qs['action']:
          (output, retcode) = HearthStoneHandler(self, qs).add()
      except Exception as e:
        self.log_message(repr(e))
        output = '<p>suck it deep</p>'
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
