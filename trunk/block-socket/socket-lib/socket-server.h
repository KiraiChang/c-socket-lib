#ifndef SERVER_H_
#define SERVER_H_

#include "socket-base.h"
#include "CSocketAddr.h"


class SocketServer:public SocketBase
{
	SOCKET s_client;
public:
	SocketServer();
	virtual ~SocketServer();
	BOOL recevice_line(char * buf);
	BOOL send_line(char * buf);
	int exit_client(int n_exit);
	void bind(CSocketAddr &addr);
	void listen();
	int accept();
};

SocketServer::SocketServer()
{
	s_client = INVALID_SOCKET;
	SocketBase::inti_member();
}

SocketServer::~SocketServer()
{
	exit_client(0);
}

BOOL SocketServer::recevice_line(char * buf)
{
	return SocketBase::recevice_line(s_client, buf);
}

BOOL SocketServer::send_line(char * buf)
{
	return SocketBase::send_line(s_client, buf);
}

void SocketServer::bind(CSocketAddr &addr)
{
	int ret_val = ::bind(s_main, (LPSOCKADDR)&addr.get_addr(), sizeof(SOCKADDR_IN));
	if(SOCKET_ERROR == ret_val)
	{
		closesocket(s_main);
		handle_socket_error("Fail bind()!");
	}
}

void SocketServer::listen()
{
	int ret_val = ::listen(s_main, 1);
	if(SOCKET_ERROR == ret_val)
	{
		closesocket(s_main);
		handle_socket_error("Fail listen()!");
	}
	std::cout<<"Server succeeded!"<<std::endl;
	std::cout<<"Waiting for new client..."<<std::endl;
}

int SocketServer::accept()
{
	sockaddr_in client_addr;
	int client_addr_len = sizeof(client_addr);
	s_client = ::accept(s_main, (sockaddr FAR*)&client_addr, &client_addr_len);
	if(INVALID_SOCKET == s_client)
	{
		closesocket(s_main);
		return handle_socket_error("Fail accept()!!");
	}
	else
		b_conning =TRUE;
	char *p_client_IP = inet_ntoa(client_addr.sin_addr);
	u_short client_port = ntohs(client_addr.sin_port);
	std::cout<<"Accept a Client."<<std::endl;
	std::cout<<"IP: "<<p_client_IP<<std::endl;
	std::cout<<"Port: "<<client_port<<std::endl;
	return 0;
}

int SocketServer::exit_client(int n_exit)
{
	closesocket(s_client);
	return SocketBase::exit_client(n_exit);
}


#endif //SERVER_H_