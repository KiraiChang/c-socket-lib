// TestSocket.cpp : 定義主控台應用程式的進入點。
//

#include "stdafx.h"
#include "..\CSocket\CSocket.h"
#include "packet.h"

#include <iostream>

using namespace std;

#define USING_TCP

int _tmain(int argc, _TCHAR* argv[])
{
	CSocket s1;
	Packet p2;

#ifdef USING_TCP
	//TCP
	s1.StartListen("127.0.0.1", 5500);
	//s1.ProcAccept((void *)&p2, sizeof(p2));
	//CSocket sc1(s1.Accept());
	while(1)
	{
		s1.ProcAccept(&p2, sizeof(p2));
		if(p2.message[0] != 0)
			cout<<"P2: "<<p2.message<<endl;
		if(strcmp(p2.message, "quit") == 0)
			break;
		p2.Init();
	}
	//c1.Recv((void *)&p2, sizeof(p2));
#else
	//UDP
	s1.StartUDP(5500);
	c1.StartUDP(NULL);
	c1.SendTo(CSocketAddr("127.0.0.1",5500),(const void*)&p1, sizeof(p1));
	s1.RecvFrom((void *)&p2, sizeof(p2));
#endif

	cout<<"\nP2: "<<p2.message<<endl;
	system("pause");
	return 0;
}

