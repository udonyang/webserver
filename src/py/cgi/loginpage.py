def run(rfile, wfile):
  index_file = open('../html/index.html')
  index = ''.join(index_file.readlines())
  index_file.close()
  wfile.write(rfile.readlines())
  return 200
