#pragma once

#include <string>

#include "konst.h"

namespace udon
{
namespace util
{

void LogInf(const char* format, ...);

void LogErr(const char* format, ...);

int Read(const int& fd, const int& limit, std::string* buf);

inline int Read(const int& fd, std::string* buf)
{
  return Read(fd, konst::kBufferLimit, buf);
}

int Write(const int& fd, const std::string& buf);

int ForkAsDaemon();

class Singleton
{
 public:
  Singleton();

  virtual ~Singleton();

 private:
  void operator = (const Singleton&);
};

template<typename T, typename F>
class ScopedFree: public Singleton
{
 public:
  ScopedFree(T data, F free)
  {
    typedef char type_test[sizeof(T)? 0: -1];
    (void)sizeof(type_test);

    data_ = data;
    free_ =  free;
  }

  virtual ~ScopedFree() 
  {
    free_(data_);
  }

 protected:
  T data_;
  F free_;
};

typedef ScopedFree<int, int(*)(int)> ScopedClose;

}
}
