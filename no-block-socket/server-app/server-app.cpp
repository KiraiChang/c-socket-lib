// server-app.cpp : �w�q�D���x���ε{�����i�J�I�C
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

