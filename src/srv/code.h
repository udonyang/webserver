#pragma once

namespace udon
{
namespace code
{

enum Code
{
  /* [-300, -200) INET error */
  E_INET_ATON = -205,

  /* [-200, -100) socket error */
  E_SOCKET_ACCEPT = -106,
  E_SOCKET_LISTEN = -103,
  E_SOCKET_BIND = -102,
  E_SOCKET_CREATE = -101,

  /* [-100, 0) common error */
  E_ARG = -2,
  E_SYS = -1,

  /* [-oo, 0) error */
  OK = 0,
  /* [0, +oo] info */

};

}
}
