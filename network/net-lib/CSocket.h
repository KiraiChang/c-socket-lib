#ifndef CSOCKET_H_
#define CSOCKET_H_
#pragma comment(lib, "WS2_32.lib")
#include<winsock2.h>
#include"CSocketAddr.h"

#define CMD

#ifdef CMD
#include<iostream>
using namespace std;
#endif

class CSocket
{
	SOCKET m_socket;

	fd_set readmask,writemask,exceptmask;
	struct timeval timeout;
public:
	CSocket(void);
	CSocket(SOCKET socket);
	~CSocket(void);
	void WSASetup(void);
	//---------------------Both---------------------------
	void ErrorDetected(const char *msg);
	void Send(const char *data, int len);
	void Receive(char *data, int len);
	void SendTo(const char *data, int len, CSocketAddr &addr);
	void ReceiveFrom(char *data, int len);
	void Close(void);
	//-------------------Server---------------------------
	void Bind(CSocketAddr &addr);
	void Listen(void);
	CSocket *Accept(void);

	//-------------------Client-----------------------------
	void Connect(CSocketAddr &addr);
};

CSocket::CSocket(void)
{
	WSASetup(); 
	SOCKET sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);//TCP
	//SOCKET sockfd = socket(AF_INET, SOCK_DGRAM, 0);//UDP
	if(INVALID_SOCKET == sockfd)
	{
#ifdef CMD
		cout<<"Socket fails!!\n";
#endif
		WSACleanup();
		return;
	}
#ifdef CMD
	else
		cout<<"Socket Success\n";
#endif
	m_socket = sockfd;
}

CSocket::CSocket(SOCKET socket)
{
	WSASetup();
	m_socket = socket;
}

CSocket::~CSocket(void)
{
	Close();
}

void CSocket::WSASetup(void)
{
	WORD wVersionRequested;
	WSADATA wsaData;
	int nErrCode;
	wVersionRequested = MAKEWORD(2, 2);
	nErrCode = WSAStartup(wVersionRequested, &wsaData);
	if(nErrCode != 0)
	{
#ifdef CMD
	cout<<"WSAStart fails!!\n";
#endif
		return;
	}
#ifdef CMD
	else
		cout<<"WSAStart Success\n";
#endif
	if(LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2)
	{
#ifdef CMD
	cout<<"Version fails!!\n";
#endif
		WSACleanup();
		return;
	}
#ifdef CMD
	else
		cout<<"Version is right\n";
#endif
}

//---------------------Both---------------------------
void CSocket::ErrorDetected(const char *meg)
{
#ifdef CMD
	cout<<meg<<endl;
#endif
	closesocket(m_socket);
	WSACleanup();
}

void CSocket::Send(const char *data, int len)
{
	int ret_val;
	ret_val = send(m_socket, data, len, 0);
	if(SOCKET_ERROR == ret_val)
	{
		ErrorDetected("Send Error");
		return;
	}
#ifdef CMD
	else
		cout<<"Send OK!!\n";
#endif
}

void CSocket::Receive(char *data, int len)
{
	int ret_val;
	FD_ZERO(&readmask);
	FD_ZERO(&writemask);
	FD_ZERO(&exceptmask);
	FD_SET(m_socket,&readmask);
	FD_SET(m_socket,&writemask);
	FD_SET(m_socket,&exceptmask);
	select(FD_SETSIZE,&readmask,&writemask,&exceptmask,&timeout);
	if(FD_ISSET(m_socket,&readmask))
	{
		ret_val = recv(m_socket, data, len, 0);
		if(SOCKET_ERROR == ret_val)
		{
			ErrorDetected("Recevice Error");
			return;
		}
#ifdef CMD
		else
			cout<<"Receive OK!!\n";
#endif
	}
}

void CSocket::SendTo(const char *data, int len, CSocketAddr &addr)
{
	if(sendto(m_socket, data, len, 0, (SOCKADDR *)&addr.get_addr(), sizeof(addr.get_addr())) == SOCKET_ERROR)
	{
		ErrorDetected("Send Error");
		return;
	}
#ifdef CMD
	cout<<"Send OK!!\n";
#endif
}

void CSocket::ReceiveFrom(char *data, int len)
{
	SOCKADDR_IN from_addr;
	int n_client_addr = sizeof(from_addr);
	if(recvfrom(m_socket, data, len, 0, (SOCKADDR *)&from_addr, &n_client_addr) == SOCKET_ERROR)
	{
		ErrorDetected("Recevice Error");
		return;
	}
#ifdef CMD
	cout<<"Recevice OK!!\n";
#endif
}

void CSocket::Close(void)
{
	closesocket(m_socket);
	WSACleanup();
#ifdef CMD
	cout<<"Socket close!!\n";
#endif
}

//-------------------Server---------------------------

void CSocket::Bind(CSocketAddr &addr)
{
	int nErrCode;
	nErrCode = bind(m_socket, (SOCKADDR *)&addr.get_addr(), sizeof(addr));
	if(SOCKET_ERROR == nErrCode)
	{
		ErrorDetected("Bind Error");
		return;
	}
#ifdef CMD
	else
		cout<<"Bind Success!\n";
#endif
}

void CSocket::Listen(void)
{
	int nErrCode;
	nErrCode = listen(m_socket, 3);
	if(SOCKET_ERROR == nErrCode )
	{
		ErrorDetected("Listen Error");
		return;
	}
#ifdef CMD
	else
		cout<<"Listen Start...\n";
#endif
}

CSocket *CSocket::Accept(void)
{
	SOCKET s_accept;
	sockaddr_in addr_client;
	int addr_client_len = sizeof(addr_client);
	s_accept = accept(m_socket, (SOCKADDR *)&addr_client, &addr_client_len);
	if(INVALID_SOCKET == s_accept)
	{
		ErrorDetected("Accept Error");
		return NULL;
	}
	else
	{
#ifdef CMD
		cout<<"Accept Success!!\n";
#endif
		CSocket *client_socket = new CSocket(s_accept);
		return client_socket;
	}
}

//-------------------Client---------------------------

void CSocket::Connect(CSocketAddr &addr)
{
	int nErrCode;
	nErrCode = connect(m_socket,  (sockaddr *)&addr.get_addr(), sizeof(sockaddr));
	if(SOCKET_ERROR == nErrCode)
	{
		ErrorDetected("Connect Error");
		return;
	}
	else
	{
#ifdef CMD
		cout<<"Connect now!!\n";
#endif
	}
};
#endif //CSOCKET_H_