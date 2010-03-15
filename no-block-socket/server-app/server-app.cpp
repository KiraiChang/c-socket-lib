// server-app.cpp : 定義主控台應用程式的進入點。
//

#include "stdafx.h"
#include "../socket-lib/socket-server.h"

int _tmain(int argc, _TCHAR* argv[])
{
	SocketServer server;
	SocketAddr addr("127.0.0.1", 5500);
	server.server_work(addr);
	system("pause");
	return 0;
}

