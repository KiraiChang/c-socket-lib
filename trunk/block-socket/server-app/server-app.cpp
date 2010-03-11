// server-app.cpp : 定義主控台應用程式的進入點。
//

#include "stdafx.h"
#include "../socket-lib/socket-server.h"

int _tmain(int argc, _TCHAR* argv[])
{
	char buf_send[256];
	char buf_recv[256];
	memset(buf_send, 0, sizeof(buf_send));
	memset(buf_recv, 0, sizeof(buf_recv));
	SocketServer server;
	CSocketAddr addr("127.0.0.1", 5500);
	server.WSA_setup();
	server.bind(addr);
	server.listen();
	server.accept();
	server.recevice_line(buf_recv);
	std::cout<<"Client Send: "<<buf_recv;
	strcpy_s(buf_send,sizeof(buf_send),"Hello Client...\n");
	server.send_line(buf_send);
	std::cout<<"Send to client OK!!\n";
	system("pause");
	return 0;
}

