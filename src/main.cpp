#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include <string>

#include "code.h"
#include "util.h"
#include "konst.h"

using namespace std;
using namespace udon;

int main(int argc, char** argv)
{
  int ret = code::OK;

  if (argc < 3)
  {
    util::LogErr("%s <listen-ip> <listen-port> [<daemon>]", argv[0]);
    return code::E_ARG;
  }

  if (argc == 4)
  {
    ret = util::ForkAsDaemon();
    if (ret == code::I_MULTI_FATHER)
    {
      return code::OK;
    }
    if (ret != code::OK)
    {
      util::LogErr("ERR:%d: ForkAsDaemon failed", ret);
      return ret;
    }
  }

  const char* listen_ip = argv[1];
  short listen_port = strtol(argv[2], nullptr, 10);

  int listenfd = socket(PF_INET, SOCK_STREAM, getprotobyname("tcp")->p_proto);
  if (listenfd == -1)
  {
    util::LogErr("ERR:%d: socket failed: msg=(%s)", errno,
                 strerror(errno));
    return code::E_SOCKET_CREATE;
  }
  util::ScopedClose _listenfd(listenfd, close);

  struct sockaddr_in listen_sockaddr;
  listen_sockaddr.sin_family = AF_INET;
  listen_sockaddr.sin_port = htons(listen_port);
  ret = inet_aton(listen_ip, &listen_sockaddr.sin_addr);
  if (ret != 1)
  {
    util::LogErr("ERR:%d: inet_ntoa failed: ip=(%s) msg=(%s)", errno, 
                 listen_ip, strerror(errno));
    return code::E_INET_ATON;
  }

  ret = bind(listenfd, (struct sockaddr*)&listen_sockaddr, sizeof(struct sockaddr));
  if (ret != 0)
  {
    util::LogErr("ERR:%d: bind failed: msg=(%s)", errno,
                 strerror(errno));
    return code::E_SOCKET_BIND;
  }

  ret = listen(listenfd, 16);
  if (ret != 0)
  {
    util::LogErr("ERR:%d: listen failed: msg=(%s)", errno,
                 strerror(errno));
    return code::E_SOCKET_LISTEN;
  }

  util::LogInf("INFO: listening %s:%d", listen_ip, listen_port);

  while (true)
  {
    struct sockaddr_in accept_sockaddr;
    socklen_t accept_sockaddrlen;
    int acceptfd = accept(listenfd, (struct sockaddr*)&accept_sockaddr, &accept_sockaddrlen);
    if (acceptfd < 0)
    {
      util::LogErr("ERR:%d: accept failed: msg=(%s)", errno,
                   strerror(errno));
      if (errno == EINTR)
      {
        continue;
      }

      return code::E_SOCKET_ACCEPT;
    }
    util::ScopedClose _acceptfd(acceptfd, &close);

    util::LogInf("INFO: accept %s:%d", inet_ntoa(accept_sockaddr.sin_addr), ntohs(accept_sockaddr.sin_port));

    string inpkg;
    char readbuf[konst::kBufferSize];
    while (true)
    {
      ret = read(acceptfd, readbuf, konst::kBufferSize);
      if (ret <= 0)
      {
        if (errno == EINTR || errno == EAGAIN)
        {
          continue;
        }

        util::LogErr("ERR:%d: read failed: msg=(%s)", errno,
                     strerror(errno));
        break;
      }

      if (ret == 0 || ret < konst::kBufferSize)
      {
        break;
      }

      inpkg.append(readbuf, ret);
      if (inpkg.size() >= konst::kBufferLimit)
      {
        util::LogErr("ERR:%d: read too much: inpkg_size=%zu", inpkg.size());
        break;
      }
    }

    util::LogInf("INFO: inpkg_size=%zu", inpkg.size());

    string outpkg;
    outpkg.append("HTTP/1.1 200 OK\r\n");
    outpkg.append("\r\n");
    outpkg.append("<html><title>udonyang</title><p>hello world</p><html>");

    ret = util::Write(acceptfd, outpkg);
    if (ret != code::OK)
    {
      util::LogErr("ERR:%d: write failed", ret);
      break;
    }

    util::LogInf("INFO: outpkg_size=%zu", outpkg.size());
  }

  return code::OK;
}
