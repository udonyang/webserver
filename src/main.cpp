#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include <string>
#include <sstream>

#include "code.h"
#include "util.h"
#include "konst.h"

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

    struct timeval recvtimeout;
    recvtimeout.tv_sec = 0;
    recvtimeout.tv_usec = 10000;
    ret = setsockopt(acceptfd, SOL_SOCKET, SO_RCVTIMEO,
                     (const void*)&recvtimeout, sizeof(struct timeval));
    if (ret != 0)
    {
      util::LogErr("ERR:%d: setsockopt failed: msg=(%s)", errno,
                   strerror(errno));
      break;
    }

    util::LogInf("INFO: accept %s:%d", inet_ntoa(accept_sockaddr.sin_addr), ntohs(accept_sockaddr.sin_port));

    std::string inpkg;
    ret = util::Read(acceptfd, &inpkg);
    if (ret != code::OK)
    {
      util::LogErr("ERR:%d: Read failed", ret);
      break;
    }

    util::LogInf("INFO: inpkg_size=%zu", inpkg.size());

    std::string content = 
        "<html>"
        "<title>geta8key test</title>"
        "<p>http</p>"
        "<a href='http://mp.weixin.qq.com/s/bfRkhkplm-jSExIkwROLBg'>case1. has session</a><br>"
        "<a href='http://mp.weixin.qq.com/s?__biz=MzAwNDc3NzAxMQ==&mid=2665718962&idx=1&sn=a1f029d40b6c9e3dddd663657731106c'>case2. has session</a><br>"
        "<a href='http://mp.weixin.qq.com/mp/appmsg/show?__biz=MzAwNDc3NzAxMQ==&mid=2665718962&idx=1&sn=a1f029d40b6c9e3dddd663657731106c'>case3. has session</a><br>"
        "<a href='http://mp.weixin.qq.com/mp/profile_ext?__biz=MjM5NDE1ODg2MQ=='>case4. no session</a><br>"
        "<a href='http://mp.weixin.qq.com/s/bfRkhkplm-jSExIkwROLBg'>case5. has session</a><br>"
        "<p>https</p>"
        "<a href='https://mp.weixin.qq.com/s/bfRkhkplm-jSExIkwROLBg'>case1. has session</a><br>"
        "<a href='https://mp.weixin.qq.com/s?__biz=MzAwNDc3NzAxMQ==&mid=2665718962&idx=1&sn=a1f029d40b6c9e3dddd663657731106c'>case2. has session</a><br>"
        "<a href='https://mp.weixin.qq.com/mp/appmsg/show?__biz=MzAwNDc3NzAxMQ==&mid=2665718962&idx=1&sn=a1f029d40b6c9e3dddd663657731106c'>case3. has session</a><br>"
        "<a href='https://mp.weixin.qq.com/mp/profile_ext?__biz=MjM5NDE1ODg2MQ=='>case4. no session</a><br>"
        "<a href='https://mp.weixin.qq.com/s/bfRkhkplm-jSExIkwROLBg'>case5. has session</a><br>"
        "<a href='https://www.baidu.com'>case6. baidu, has session</a><br>"
        "</html>"
        ;
    std::stringstream outpkg;
    outpkg
        << "HTTP/1.1 200 OK\r\n"
        << "Connection: close\r\n"
        << "Content-Length: " << content.size() << "\r\n"
        << "\r\n"
        << content
        ;

    ret = util::Write(acceptfd, outpkg.str());
    if (ret != code::OK)
    {
      util::LogErr("ERR:%d: Write failed", ret);
      break;
    }

    util::LogInf("INFO: outpkg_size=%zu", outpkg.str().size());
  }

  return code::OK;
}
