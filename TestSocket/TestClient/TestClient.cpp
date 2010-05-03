// TestClient.cpp : 定義主控台應用程式的進入點。
//


#include "stdafx.h"
#include<iostream>
#include "..\CSocket\CSocket.h"
#include "..\TestSocket\Packet.h"

using namespace std;

int _tmain(int argc, _TCHAR* argv[])
{
	CSocket c1;

	Packet p1;

	//p1.size = 100;
	//strcpy_s(p1.message,sizeof("Hello"), "Hello");

	c1.StartConnect("127.0.0.1", 5500);
	while(c1.IsConnect())
	{
		//if(c1.ProcSend(&p1, sizeof(p1)))
		//	break;
		cin>>p1.message;
		c1.ProcSend(&p1, sizeof(p1));
		if(strcmp(p1.message, "quit") == 0)
			break;
		p1.Init();
	}
	return 0;
}

