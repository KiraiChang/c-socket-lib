#ifndef CSOCKET_ADDR_H_
#define CSOCKET_ADDR_H_
//#pragma comment(lib, "WS2_32.lib")
#include<winsock2.h>

class CSocketAddr
{
	struct sockaddr_in m_addr;
public:
	CSocketAddr(const char *szIP = NULL, int i_port = -1);
	~CSocketAddr();
	void set_ip(const char *szIP);
	void set_port(int i_port);
	sockaddr_in get_addr()															{return m_addr;}
};

CSocketAddr::CSocketAddr(const char *szIP, int i_port)
{
	m_addr.sin_family = AF_INET;
	set_ip(szIP);
	set_port(i_port);
}

CSocketAddr::~CSocketAddr()
{

}

void CSocketAddr::set_ip(const char *szIP)
{
	if(szIP)
	{
		m_addr.sin_addr.s_addr=inet_addr(szIP);
	}
	else
	{
		m_addr.sin_addr.s_addr=htonl(INADDR_ANY);
	}
}
void CSocketAddr::set_port(int i_port)
{
	m_addr.sin_port = htons(i_port);
}
#endif //CSOCKET_ADDR_H_