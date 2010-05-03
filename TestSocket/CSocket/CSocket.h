#ifndef _CSOCKET_H_
#define _CSOCKET_H_

#pragma comment (lib, "WS2_32.lib")
#include <winsock2.h>
#include <assert.h>

class CSocketAddr
{
private:
	SOCKADDR_IN m_addr;
public:
	CSocketAddr(const char *szIP = NULL, int iPort = -1)
	{
		m_addr.sin_family = AF_INET;
		SetIP(szIP);
		SetPort(iPort);
	}
	virtual ~CSocketAddr(void)																{}
	void SetIP(const char *szIP)
	{
		if(szIP)
			m_addr.sin_addr.s_addr=inet_addr(szIP);
		else
			m_addr.sin_addr.s_addr=htonl(INADDR_ANY);
	}
	void SetPort(int iPort)
	{
		m_addr.sin_port = htons(iPort);
	}
	int GetPort(void)const							{return m_addr.sin_port;}
	SOCKADDR_IN GetAddr(void)const {return m_addr;}
};

class CSocket
{
private:
	enum STATE{OFFLINE = 0, TCP_ONLINE, UDP_ONLINE};

	STATE m_State;
	FD_SET m_SocketSet;
	//FD_SET m_WriteSocket;
	//FD_SET m_ReadSocket;
	SOCKET m_CurSocket;
	//enum {RECV_TMP_BUF_SIZE = 1024};
	fd_set InitBufferSocketSet(void);
	sockaddr_in SetAddress(const char *strAddress,WORD port);

	BOOL Open(BOOL bUsingTCP);
	BOOL Open(const char *strAddress,WORD port, BOOL bUsingTCP);
	BOOL Listen(void);
	void Connect(const char *strAddress,WORD port);//TCP
public:
	CSocket(void);
	//CSocket(SOCKET socket):m_CurSocket(socket)					{}
	virtual ~CSocket(void)																{Release();}
	BOOL IsConnect(void)															{if(m_State == TCP_ONLINE)return TRUE;else return FALSE;}
	BOOL StartListen(const char *strAddress,WORD port);
	void StartConnect(const char *strAddress,WORD port);//TCP
	void StartUDP(WORD port);//UDP
	//SOCKET Accept(void);

	int ProcAccept(void *pBufRecv, int bufSize);

	BOOL ProcSend(const void* pBufSend, int bufSize);
	//void Send(const void *pData, int dataSize);//TCP
	//int Recv(void *pBufRecv, int bufSize);//TCP

	void SendTo(CSocketAddr addr, const void *pData, int dataSize);//UDP
	int RecvFrom(void *pBufRecv, int bufSize);//UDP

	void CloseSocket(void);
	void Release(void);
};

#endif //_CSOCKET_H_