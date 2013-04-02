#ifndef WSOCKET_H
#define WSOCKET_H
/*=========================================================================*\
* Socket compatibilization module for Win32
* LuaSocket toolkit
*
* RCS ID: $Id: wsocket.h,v 1.3 2004/02/11 03:31:53 diego Exp $
\*=========================================================================*/

/*=========================================================================*\
* WinSock include files
\*=========================================================================*/
#include <winsock.h>

typedef int socklen_t;
typedef SOCKET t_sock;
typedef t_sock *p_sock;

#define SOCK_INVALID (INVALID_SOCKET)

#endif /* WSOCKET_H */
