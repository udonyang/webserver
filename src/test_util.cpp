#include <stdlib.h>
#include <iostream>

#include "util.h"
#include "code.h"

using namespace std;
using namespace udon;

int Http(util::Int2StrList& args)
{
  http::Http httpmgr(args['i'][0], strtoul(args['p'][0].c_str(), NULL, 10));
  for (int i = 0; i < args['k'].size(); i++)
  {
    httpmgr.AddReqHeader(args['k'][i], args['v'][i]);
  }
  cout << "ret: " << httpmgr.Request(args['m'][0], args['t'][0], (args['b'].size()? args['b'][0]: "")) << endl;
  cout << httpmgr.status() << endl;
  for (const auto& header: httpmgr.rspheaders())
  {
    cout << header.first << ": " << header.second << endl;
  }
  cout << httpmgr.rspbody() << endl;
  return 0;
}

int main(int argc, char** argv)
{
  int ret = 0;

  util::Int2StrList args;
  ret = util::ParseArgs(argc, argv, "f:t:i:p:m:k:v:b:", &args);
  if (ret == code::OK)
  {
    if (args['f'].size() == 0)
    {
      ret = -2;
    }
    else
    {
      if (strcasecmp(args['f'][0].c_str(), "http") == 0)
      {
        ret = Http(args);
      }
      else
      {
        ret = -2;
      }
    }
  }

  if (ret != 0)
  {
    cout
        << "usage: "<< argv[0] << " <cmd>" << endl
        << " <cmd> = -f http -i <ip> -p <port> -m <method> -u <url> -k <header key> -v <header value> -b <body>" << endl
        << endl;
  }
  return 0;
}
