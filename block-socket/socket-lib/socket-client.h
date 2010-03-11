#ifndef SOCKET_CLIENT_H_
#define SOCKET_CLIENT_H_

#include "socket-base.h"
#include "CSocketAddr.h"

enum {CLIENT_API_ERROR = 1};
class SocketClient:public SocketBase
{
public:
	SocketClient();
	~SocketClient();
	BOOL recevice_line(char * buf);
	BOOL send_line(char * buf);
	int connect(CSocketAddr addr);
};

SocketClient::SocketClient()
{
	SocketBase::inti_member();
}

SocketClient::~SocketClient()
{
	SocketBase::exit_client(0);
}

BOOL SocketClient::recevice_line(char * buf)
{
	return SocketBase::recevice_line(s_main, buf);
}

BOOL SocketClient::send_line(char * buf)
{
	return SocketBase::send_line(s_main, buf);
}
int SocketClient::connect(CSocketAddr addr)
{
	int ret_val = ::connect(s_main, (LPSOCKADDR)&addr.get_addr(), sizeof(SOCKADDR_IN));
	if(SOCKET_ERROR == ret_val)
	{
		SocketBase::show_socket_msg("Connect Error!!");
		return SocketBase::exit_client(CLIENT_API_ERROR);
	}
	else
	{
		b_conning = TRUE;
		return 0;
	}
}

#endif //SOCKET_CLIENT_H_