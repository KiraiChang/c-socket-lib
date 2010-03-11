// clinet-app.cpp : 定義主控台應用程式的進入點。
//

#include "stdafx.h"
#include "../socket-lib/socket-client.h"

int _tmain(int argc, _TCHAR* argv[])
{
	char buf_send[256];
	char buf_recv[256];
	memset(buf_send, 0, sizeof(buf_send));
	memset(buf_recv, 0, sizeof(buf_recv));
	SocketClient client;
	CSocketAddr addr("127.0.0.1", 5500);
	client.WSA_setup();
	client.connect(addr);
	strcpy_s(buf_send,sizeof(buf_send),"Hello Server...\n");
	client.send_line(buf_send);
	std::cout<<"Send to server OK!!\n";
	client.recevice_line(buf_recv);
	std::cout<<"Server Send: "<<buf_recv;
	system("pause");
	return 0;
}

