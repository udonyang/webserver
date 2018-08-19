#pragma once

#include <string>
#include <map>
#include <vector>

#include "konst.h"

namespace udon
{

namespace util
{

typedef std::map<std::string, std::string> Str2Str;
typedef std::map<int, std::vector<std::string>> Int2StrList;

std::string Trim(const std::string& str);

void LogInf(const char* format, ...);

void LogErr(const char* format, ...);

int Connect(const std::string& ip, const int& port);

int Read(const int& fd, const int& limit, std::string* buf);

inline int Read(const int& fd, std::string* buf)
{
  return Read(fd, konst::kBufferLimit, buf);
}

int Write(const int& fd, const std::string& buf);

int ForkAsDaemon();

int Close(int* fd);

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
    needfree_ = true;
  }

  virtual ~ScopedFree() 
  {
    if (needfree_)
    {
      free_(data_);
    }
  }

  void Release()
  {
    needfree_ = false;
  }

 protected:
  T data_;
  F free_;
  bool needfree_;
};

typedef ScopedFree<int, int(*)(int)> ScopedClose;
typedef ScopedFree<int*, int(*)(int*)> ScopedClosePtr;

int ParseArgs(int argc, char** argv, const char* optstr, Int2StrList* kv);

}

namespace http
{

class Http
{
 public:
  Http(const std::string& ip, const int& port);

  ~Http();

  int Request(const std::string& method, const std::string& target, const std::string& body);

  void AddReqHeader(const std::string& key, const std::string& value);

  const std::string& GetRspHeader(const std::string& key) const;

 public:
  inline const int statuscode() const {return statuscode_;}

  inline const std::string& status() const {return status_;}

  inline const std::string& rspbody() const {return rspbody_;}

  inline const util::Str2Str rspheaders() const {return rspheaders_;}

 protected:
  std::string ip_;
  int port_;
  int statuscode_;
  std::string status_;
  std::string rspbody_;
  util::Str2Str reqheaders_;
  util::Str2Str rspheaders_;
};

}

}
