#include <cstdio>
#include <stdarg.h>

#include "util.h"

namespace udon
{
namespace util
{

void loginf(const char* format, ...)
{
  va_list ap;
  va_start(ap, format);
  vfprintf(stdout, format, ap);
  fprintf(stdout, "\n");
  va_end(ap);
}

void logerr(const char* format, ...)
{
  va_list ap;
  va_start(ap, format);
  vfprintf(stderr, format, ap);
  fprintf(stderr, "\n");
  va_end(ap);
}

}
}
