#include <cstdio>
#include <cstdlib>
#include <stdarg.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

#include "util.h"
#include "code.h"

namespace udon
{
namespace util
{

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

int Write(const int& fd, const std::string& buf)
{
  int ret = code::OK;
  int writepos = 0;
  while (writepos < buf.size())
  {
    ret = write(fd, buf.data()+writepos, buf.size()-writepos);
    if (ret == 0)
    {
      break;
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

}
}
