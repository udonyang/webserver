#pragma once

namespace udon
{
namespace code
{

enum Code
{
  /* [-600, -500) multi thread/process */
  E_HTTP_INVALID_RSP = -501,

  /* [-500, -400) multi thread/process */
  E_MULTI_FORK = -401,

  /* [-400, -300) I/O error */
  E_IO_LMIIT = -303,
  E_IO_READ = -302,
  E_IO_WRITE = -301,

  /* [-300, -200) INET error */
  E_INET_ATON = -201,

  /* [-200, -100) socket error */
  E_SOCKET_CONNECT = -105,
  E_SOCKET_ACCEPT = -104,
  E_SOCKET_LISTEN = -103,
  E_SOCKET_BIND = -102,
  E_SOCKET_CREATE = -101,

  /* [-100, 0) common error */
  E_OPT_UNKNOW = -4,
  E_OPT_NEED_VALUE = -3,
  E_ARG = -2,
  E_SYS = -1,

  /* [-oo, 0) error */
  OK = 0,
  /* [0, +oo] info */

  /* [400, 500) multi thread/process */
  I_MULTI_FATHER = 400,

};

}
}
