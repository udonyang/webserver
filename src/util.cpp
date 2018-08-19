#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <stdarg.h>
#include <errno.h>
#include <string>
#include <sstream>
#include <unistd.h>
#include <netdb.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include "util.h"
#include "code.h"

namespace udon
{

using namespace std;

namespace util
{

std::string Trim(const std::string& str)
{
  size_t sppos = 0;
  while (sppos < str.size() && isspace(str[sppos])) sppos++;
  size_t splastpos = str.size();
  while (splastpos > 0 && isspace(str[splastpos-1])) splastpos--;
  return str.substr(sppos, splastpos-sppos);
}

void LogInf(const char* format, ...)
{
  va_list ap;
  va_start(ap, format);
  vfprintf(stdout, format, ap);
  fprintf(stdout, "\n");
  va_end(ap);
}

void LogErr(const char* format, ...)
{
  va_list ap;
  va_start(ap, format);
  vfprintf(stderr, format, ap);
  fprintf(stderr, "\n");
  va_end(ap);
}

int ForkAsDaemon()
{
  pid_t pid = fork();
  if (pid < 0)
  {
    util::LogErr("ERR:%d: fork failed: msg=(%s)", errno,
                 strerror(errno));
    return code::E_MULTI_FORK;
  }
  if (pid > 0)
  {
    util::LogInf("INFO: father process exit: pid=%d sid=%d", 
                 getpid(), getsid(0));
    return code::I_MULTI_FATHER;
  }

  setsid();
  util::LogInf("INFO: fork as daemon: pid=%d sid=%d",
               getpid(), getsid(0));
  return code::OK;
}

int Close(int* fd)
{
  if (fd == nullptr) return code::E_ARG;
  return close(*fd);
}

int Connect(const std::string& ip, const int& port)
{
  int ret = 0;

  int fd = socket(PF_INET, SOCK_STREAM, getprotobyname("tcp")->p_proto);
  if (fd == -1)
  {
    util::LogErr("ERR:%d: socket failed: msg=(%s)", errno,
                 strerror(errno));
    return code::E_SOCKET_CREATE;
  }
  util::ScopedClosePtr fdowner(&fd, Close);

  struct sockaddr_in sockaddr;
  sockaddr.sin_family = AF_INET;
  sockaddr.sin_port = htons(port);
  ret = inet_aton(ip.c_str(), &sockaddr.sin_addr);
  if (ret != 1)
  {
    util::LogErr("ERR:%d: inet_ntoa failed: ip=(%s) msg=(%s)", errno, 
                 ip.c_str(), strerror(errno));
    return code::E_INET_ATON;
  }

  do
  {
    ret = connect(fd, (struct sockaddr*)&sockaddr, sizeof(sockaddr));
    if (ret != 0)
    {
      util::LogErr("ERR:%d: connect failed: fd[%d] msg[%s] ip[%s] port[%d]", ret,
                   fd, strerror(errno), ip.c_str(), port);
      if (errno == EINTR) 
      {
        continue;
      }
      else
      {
        return code::E_SOCKET_CONNECT;
      }
    }
  } while (0);

  fdowner.Release();
  return fd;
}

int Read(const int& fd, const int& limit, std::string* buf)
{
  int ret = code::OK;
  char readbuf[konst::kBufferSize] = "";
  for (;;)
  {
    ret = read(fd, readbuf, konst::kBufferSize);
    if (ret < 0)
    {
      if (errno == EINTR || errno == EAGAIN)
      {
        continue;
      }

      util::LogErr("ERR:%d: read failed: msg=(%s)", errno,
                   strerror(errno));
      return code::E_IO_READ;
    }

    util::LogInf("INFO: fuck ret[%d] readbuf[%s]", ret, readbuf);

    if (ret == 0)
    {
      break;
    }

    buf->append(readbuf, ret);
    if (buf->size() > konst::kBufferLimit)
    {
      util::LogErr("ERR: read too much: buf.size=%zu", buf->size());
      return code::E_IO_LMIIT;
    }

    // if (ret < konst::kBufferSize || ret < limit)
    // {
    //   break;
    // }
  }

  return code::OK;
}

int Write(const int& fd, const std::string& buf)
{
  int ret = code::OK;
  int writepos = 0;
  while (writepos < buf.size())
  {
    ret = write(fd, buf.data()+writepos, buf.size()-writepos);
    if (ret == 0)
    {
      continue;
    }
    if (ret < 0)
    {
      if (errno == EINTR || errno == EAGAIN)
      {
        continue;
      }

      util::LogErr("ERR:%d: write failed: msg=(%s)", errno,
                   strerror(errno));
      return code::E_IO_WRITE;
    }
    writepos += ret;
  }
  return code::OK;
}

Singleton::Singleton()
{
}

Singleton::~Singleton() 
{
}

void Singleton::operator = (const Singleton&) 
{
}


int ParseArgs(int argc, char** argv, const char* optstr, Int2StrList* kv)
{
  int ret = 0;
  while ((ret = getopt(argc, argv, optstr)) != -1)
  {
    if (ret == '?')
    {
      util::LogErr("ERR: unknow failed: opt[%c]", optopt);
      return code::E_OPT_UNKNOW;
    }
    else if (ret == ':')
    {
      util::LogErr("ERR: need value: opt[%c]", optopt);
      return code::E_OPT_NEED_VALUE;
    }
    else
    {
      (*kv)[ret].push_back(optarg);
    }
  }
  return code::OK;
}

}

namespace http
{

Http::Http(const std::string& ip, const int& port)
{
  ip_ = ip;
  port_ = port;
  statuscode_ = 200;
}

Http::~Http()
{
}

int Http::Request(const std::string& method, const std::string& target, const std::string& body)
{
  static const std::string& SP = " ";
  static const std::string& CRLF = "\r\n";
  static const std::string& COLON = ":";
  static const std::string& PROTOCOL = "HTTP/1.1";

  int ret = code::OK;
  stringstream ss;

  ss << method << SP << target << SP << PROTOCOL << CRLF;
  for (const auto& header: reqheaders_)
  {
    ss << header.first << COLON << SP << header.second << CRLF;
  }

  ss << CRLF;
  ss << body;

  util::LogInf("INFO: requestp[%s]", ss.str().c_str());

  ret = util::Connect(ip_.c_str(), port_);
  if (ret < 0)
  {
    util::LogErr("ERR:%d: Connect failed: ip[%s] port[%d]", ret,
                 ip_.c_str(), port_);
    return ret;
  }
  int fd = ret;
  util::ScopedClose fdowner(fd, close);

  ret = util::Write(fd, ss.str());
  if (ret != code::OK)
  {
    util::LogErr("ERR:%d: Write failed: req[%s]", ret,
                 ss.str().c_str());
    return ret;
  }

  string rsp;
  ret = util::Read(fd, 0, &rsp);
  if (ret != code::OK)
  {
    util::LogErr("ERR:%d: Read failed", ret);
    return ret;
  }

  util::LogInf("INFO: rsp[%s]", rsp.c_str());

  /* start-line */
  size_t lastpos = rsp.find(CRLF);
  if (lastpos == string::npos)
  {
    util::LogErr("ERR: invalid response: rsp[%s]", rsp.c_str());
    return code::E_HTTP_INVALID_RSP;
  }
  status_ = rsp.substr(0, lastpos);
  ret = sscanf(status_.c_str(), "%*s %d", &statuscode_);
  if (ret != 1)
  {
    util::LogErr("ERR:%d: invalid status: status[%s]", ret,
                 status_.c_str());
    return code::E_HTTP_INVALID_RSP;
  }
  // util::LogInf("INFO: status[%s] statuscode[%d]", status_.c_str(), statuscode_);
  lastpos += CRLF.size();
  /* headers */
  for (;;)
  {
    size_t tmppos = rsp.find(CRLF, lastpos);
    if (tmppos == string::npos || tmppos == lastpos)
    {
      break;
    }
    // util::LogInf("INFO: tmppos[%zu] lastpos[%zu]", tmppos, lastpos);
    size_t colonpos = rsp.find(COLON, lastpos);
    if (colonpos >= tmppos)
    {
      util::LogErr("ERR: invalid header: headerp[%s]",
                   rsp.substr(lastpos, tmppos-lastpos).c_str());
      return code::E_HTTP_INVALID_RSP;
    }
    string key = rsp.substr(lastpos, colonpos-lastpos);
    string value = rsp.substr(colonpos+COLON.size(), tmppos-(colonpos+COLON.size()));
    rspheaders_.insert(make_pair(util::Trim(key), util::Trim(value)));
    lastpos = tmppos+CRLF.size();
    // util::LogInf("INFO: key[%s] value[%s]", key.c_str(), value.c_str());
  }
  lastpos += CRLF.size();
  /* message body */
  rspbody_ = rsp.substr(lastpos, rsp.size()-lastpos);
  return code::OK;
}

void Http::AddReqHeader(const std::string& key, const std::string& value)
{
  reqheaders_.insert(make_pair(key, value));
}

const std::string& Http::GetRspHeader(const std::string& key) const
{
  static const std::string VOID = "";
  const auto& it = rspheaders_.find(key);
  if (it != rspheaders_.end())
  {
    return it->second;
  }
  return VOID;
}

}
}
