#include"..\net-lib\CSocket.h"

int main()
{
	CSocket *p_server = new CSocket();
	CSocketAddr addr("127.0.0.1", 5500);
	char buf[256];
	ZeroMemory(buf, sizeof(buf));
	p_server->Bind(addr);
	//TCP
	p_server->Listen();
	CSocket *p_client;
	p_client = p_server->Accept();
	(*p_client).Receive(buf, sizeof(buf));
	////UDP
	//(*p_server).ReceiveFrom(buf, sizeof(buf));
	cout<<buf<<endl;
	delete p_server;
	delete p_client;
	system("pause");
	return 0;
}