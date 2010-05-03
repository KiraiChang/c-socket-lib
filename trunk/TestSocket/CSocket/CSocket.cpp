#include "CSocket.h"
#include <iostream>

//Using in class function

fd_set CSocket::InitBufferSocketSet(void)
{
	fd_set temp;
	FD_ZERO(&temp);
	temp = m_SocketSet;
	return temp;
}

sockaddr_in CSocket::SetAddress(const char *strAddress,WORD port)
{
		sockaddr_in tcp_address ;
		int size = sizeof (tcp_address) ;
		memset (&tcp_address, 0, sizeof (tcp_address)) ;

		tcp_address.sin_family = AF_INET ;
		if (strAddress == NULL)
			tcp_address.sin_addr.s_addr = htonl(INADDR_ANY) ;
		else
			tcp_address.sin_addr.s_addr = inet_addr(strAddress) ;
		tcp_address.sin_port = htons (port) ;

		return tcp_address;
}

BOOL CSocket::Open(BOOL bUsingTCP)
{
	if(INVALID_SOCKET == m_CurSocket)
	{
		if(bUsingTCP)
			m_CurSocket = socket(AF_INET, SOCK_STREAM, 0);
		else
			m_CurSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
		if(INVALID_SOCKET == m_CurSocket)
		{
			closesocket (m_CurSocket) ;
			m_CurSocket = INVALID_SOCKET ;
			assert("[Error]Socket setup fail!");
			return FALSE;
		}
	}
	else
	{
		assert(0);
	}
	return TRUE;
}

BOOL CSocket::Open(const char *strAddress,WORD port, BOOL bUsingTCP)
{
	if(Open(bUsingTCP))
	{
		sockaddr_in tcp_address = SetAddress(strAddress, port);
		if(port != 0)//UDP Client;
			if(SOCKET_ERROR == bind (m_CurSocket, (LPSOCKADDR)&tcp_address, sizeof(SOCKADDR_IN)))
			{
				assert(!"[Error]open socket fails");
				Release();
				return FALSE;
			}
	}
	else
	{
			assert(!"[Error]bind fails");
			Release();
			return FALSE;
	}
	return TRUE;
}

BOOL CSocket::Listen(void)
{
	assert (m_CurSocket != INVALID_SOCKET) ;
	unsigned long ul =1;
	if(SOCKET_ERROR == ioctlsocket(m_CurSocket, FIONBIO, &ul))//set no-block
	{
		Release();
		return FALSE;
	}
	if(SOCKET_ERROR == listen(m_CurSocket, SOMAXCONN))
	{
		assert(!"[Error]listen fails");
		Release();
		return FALSE;
	}
	m_State = TCP_ONLINE;
	FD_SET(m_CurSocket, &m_SocketSet);//把Server的ListenSocket放到fd_set中
	return TRUE;
}

void CSocket::Connect(const char *strAddress,WORD port)
{
	sockaddr_in tcp_address = SetAddress(strAddress, port);
	int nRet = connect(m_CurSocket, (LPSOCKADDR)&tcp_address, sizeof(SOCKADDR_IN));
	if(SOCKET_ERROR == nRet)
	{
		assert("[Error]Connect fail!");
		Release();
	}
	else
	{
		m_State = TCP_ONLINE;
		FD_SET(m_CurSocket, &m_SocketSet);//把Clinet的SOCKET放到fd_set中
	}
}

//Constructure

CSocket::CSocket(void):m_State(OFFLINE),m_CurSocket(INVALID_SOCKET)
{
		WSADATA wsaData;
		WSAStartup(MAKEWORD(2, 2), &wsaData);
		FD_ZERO(&m_SocketSet);
		//FD_ZERO(&m_ReadSocket);
		//FD_ZERO(&m_WriteSocket);
}

//Public using function

BOOL CSocket::StartListen(const char *strAddress,WORD port)
{
	if(Open(strAddress, port, TRUE))//using tcp
		return Listen();
	else
	{
		assert("[Error]Open socket fail");
		return FALSE;
	}
}

void CSocket::StartConnect(const char *strAddress,WORD port)
{
	if(Open(TRUE))//using tcp
	{
		Connect(strAddress, port);
	}
	else
	{
		assert("[Error]connect fail");
		Release();
	}
}

void CSocket::StartUDP(WORD port)
{
	Release();
	if(Open(NULL, port, FALSE))//using udp
		m_State = UDP_ONLINE;
}

/*
SOCKET CSocket::Accept(void)
{
	SOCKET sclient;
	SOCKADDR_IN addr;
	int addr_size = sizeof(addr);
	sclient = accept(m_CurSocket, (LPSOCKADDR)&addr, &addr_size);
	if(INVALID_SOCKET == sclient)
	{
		assert("[Error]accept fails");
		Release();
	}
	return sclient;
}
*/

int CSocket::ProcAccept(void *pBufRecv, int bufSize)
{
	if(m_State == TCP_ONLINE)
	{
		fd_set readfd = InitBufferSocketSet();
		fd_set writefd = InitBufferSocketSet();
		timeval timeV ;
		timeV.tv_sec = 0 ;
		timeV.tv_usec = 1 ;
		if(0 < select(0, &readfd, &writefd, NULL, NULL))
		{
			for(int i = 0; i < (int)m_SocketSet.fd_count; i++)
			{
				if(FD_ISSET(m_SocketSet.fd_array[i], &readfd))
				{
					if(m_SocketSet.fd_array[i] == m_CurSocket)
					{
						SOCKET acceptSocket;
						SOCKADDR_IN addr;
						int addr_size = sizeof(addr);
						acceptSocket = accept(m_CurSocket, (LPSOCKADDR)&addr, &addr_size);
						if(INVALID_SOCKET == acceptSocket)
						{
							assert("[Error]accept fails");
							Release();
						}
						FD_SET(acceptSocket, &m_SocketSet);
					}
					else
					{
						int recvSize;
						recvSize = recv(m_SocketSet.fd_array[i],(char *)pBufRecv, bufSize,0);
						if(SOCKET_ERROR == recvSize)
						{
							assert("[Error]send fails");
						}
						else if(recvSize > 0 && recvSize == bufSize)
						{
							return recvSize;
						}
					}
				}
			}
		}
	}
	return 0;
}

int CSocket::ProcSend(const void* pBufSend, int bufSize)
{
	if(m_State == TCP_ONLINE)
	{
		fd_set readfd = InitBufferSocketSet();
		fd_set writefd = InitBufferSocketSet();
		timeval timeV ;
		timeV.tv_sec = 0 ;
		timeV.tv_usec = 10 ;
		if(0 < select(0, &readfd, &writefd, NULL, NULL))
		{
			for(int i = 0; i < m_SocketSet.fd_count; i++)
				if(FD_ISSET(m_SocketSet.fd_array[i], &writefd))
				{
					if( SOCKET_ERROR == send(m_SocketSet.fd_array[i],(char *)pBufSend, bufSize, 0))
						assert("[Error]sent falis");
					else
						return TRUE;
				}
		}
	}
	//else if(m_State == CLIENT_ONLINE)
	//{
	//	fd_set readfd = InitBufferSocketSet();
	//	fd_set writefd = InitBufferSocketSet();
	//	timeval timeV ;
	//	timeV.tv_sec = 0 ;
	//	timeV.tv_usec = 10 ;
	//	if(0 < select(0, &readfd, &writefd, NULL, NULL))
	//	{
	//		for(int i = 0; i < m_SocketSet.fd_count; i++)
	//			if(FD_ISSET(m_SocketSet.fd_array[i], &writefd))
	//			{
	//				if( SOCKET_ERROR == send(m_CurSocket,(char *)pBufSend, bufSize, 0))
	//					assert("[Error]sent falis");
	//				else
	//					return TRUE;
	//			}
	//	}
	//}
	return FALSE;
}

/*
void CSocket::Send(const void *pData, int dataSize)
{
	if(INVALID_SOCKET != m_CurSocket)
	{
		if( SOCKET_ERROR == send(m_CurSocket,(char *)pData, dataSize, 0))
			assert("[Error]sent falis");
	}
	else
		assert("[Error]Socket has error");		
}


int CSocket::Recv(void *pBufRecv, int bufSize)
{
	if(INVALID_SOCKET != m_CurSocket)
	{
		int recvSize;
		recvSize = recv(m_CurSocket,(char *)pBufRecv, bufSize,0);
		if(SOCKET_ERROR == recvSize)
		{
			assert("[Error]send fails");
			return 0;
		}
		else if(recvSize > 0 && recvSize == bufSize)
			return recvSize;
	}
	else
		assert("[Error]Socket has error");
	return 0;
}
*/

void CSocket::SendTo(CSocketAddr addr, const void *pData, int dataSize)//UDP use.
{
	if(INVALID_SOCKET != m_CurSocket)
	{
		if(SOCKET_ERROR == sendto(m_CurSocket,(char *)pData, dataSize, 0,(LPSOCKADDR)&addr.GetAddr(), sizeof(SOCKADDR_IN)))
			assert("[Error]sendto falis");
	}
	else
	{
		assert("[Error]when snedto socket has error!");
	}
}

int CSocket::RecvFrom(void *pBuf, int bufSize)//UDP use.
{
	if(INVALID_SOCKET != m_CurSocket)
	{
		sockaddr_in fromAddr;
		int fromAddrSize = sizeof(fromAddr);
		int recvSize = recvfrom(m_CurSocket,(char *)pBuf, bufSize, 0,(LPSOCKADDR)&fromAddr, &fromAddrSize);
		if(SOCKET_ERROR == recvSize)
		{
			assert("[Error]recvfrom fails");
			return 0;
		}
		else if(recvSize > 0 && recvSize == bufSize)
			return recvSize;
	}
	else
	{
		assert("[Error]when recvfrom socket has error!");
		return 0;
	}
	return 0;
}

void CSocket::CloseSocket(void)
{
	Release();
}

void CSocket::Release(void)
{
	if(m_CurSocket != INVALID_SOCKET)
	{
		shutdown (m_CurSocket, SD_SEND) ;
		closesocket (m_CurSocket) ;
		m_CurSocket = INVALID_SOCKET ;
	}
}