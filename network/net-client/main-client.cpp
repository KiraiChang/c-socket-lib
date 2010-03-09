#include"..\net-lib\CSocket.h"

int main()
{
	CSocket *p_client = new CSocket();
	CSocketAddr addr("127.0.0.1", 5500);
	char buf[256];
	ZeroMemory(buf, sizeof(buf));
	sprintf_s(buf, sizeof(buf),"Client is connectting");
	//TCP
	(*p_client).Connect(addr);
	(*p_client).Send(buf, sizeof(buf));
	////UDP
	//(*p_client).SendTo(buf, sizeof(buf), addr);
	delete p_client;
	system("pause");
	return 0;
}