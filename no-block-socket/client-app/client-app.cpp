// client-app.cpp : 定義主控台應用程式的進入點。
//

#include "stdafx.h"

#include "../socket-lib/socket-client.h"


int _tmain(int argc, _TCHAR* argv[])
{
	SocketClient client;
	SocketAddr addr("127.0.0.1", 5500);
	client.work(addr);
	return 0;
}

