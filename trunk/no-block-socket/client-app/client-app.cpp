// client-app.cpp : �w�q�D���x���ε{�����i�J�I�C
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

