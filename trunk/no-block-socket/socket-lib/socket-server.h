#ifndef SERVER_H_
#define SERVER_H_

#pragma comment(lib, "WS2_32.lib")
#include<winsock2.h>
#include<iostream>
#include <winbase.h>

#include "server-client.h"
#include "socket-addr.h"

using namespace std;

enum{SERVER_SETUP_FAIL=1, 
		  TIMEFOR_THREAD_EXIT = 5000, 
		  TIMEFOR_THREAD_HELP = 1500, 
		  TIMEFOR_THREAD_SLEEP=500
		 };

class SocketServer
{
	HANDLE mh_thread_accept;
	HANDLE mh_thread_help;
	HANDLE mh_server_event;
	SOCKET ms_main;
	BOOL mb_server_running;
	CRITICAL_SECTION mcs_client_list;
	SOCKET_SERVER_CLIENT m_client_list;
public:
	SocketServer(void)											{init_member();}
	~SocketServer(void)											{}
	int server_work(SocketAddr addr);
	void init_member(void);
	BOOL init_socket(SocketAddr addr);
	BOOL start_service(void);
	BOOL create_helper_and_accept_thread(void);
	void	show_tip_msg(BOOL b_first_input);
	void	show_server_start_msg(BOOL bSuc);
	void	show_server_exit_msg(void);
	void stop_services(void);

	static DWORD	__stdcall	helper_thread(void *pParam);
	static DWORD	__stdcall 	accept_thread(void *pParam);

	void exit_server(void);
};

int SocketServer::server_work(SocketAddr addr)
{
	if(!init_socket(addr))
	{
		exit_server();
		return SERVER_SETUP_FAIL;
	}
	if(!start_service())
	{
		show_server_start_msg(FALSE);
		exit_server();
		return SERVER_SETUP_FAIL;
	}
	stop_services();
	exit_server();
	return 0;
}

void SocketServer::init_member(void)
{
	InitializeCriticalSection(&mcs_client_list);
	mh_server_event = CreateEvent(NULL, TRUE, FALSE, NULL);
	mh_thread_help = NULL;
	mh_thread_accept = NULL;
	mb_server_running = FALSE;
	m_client_list.clear();
}

BOOL SocketServer::init_socket(SocketAddr addr)
{
	int ret_val;
	//��l��Windows Sockets DLL
	WSADATA wsaData;
	ret_val = WSAStartup(MAKEWORD(2, 2), &wsaData);
	ms_main = socket(AF_INET, SOCK_STREAM, 0);
	if(INVALID_SOCKET == ms_main)
		return FALSE;
	unsigned long ul = 1;
	ret_val = ioctlsocket(ms_main, FIONBIO, (unsigned long *)&ul);
	if(INVALID_SOCKET == ms_main)
		return FALSE;
	ret_val = bind (ms_main, (LPSOCKADDR)&addr.get_addr(), sizeof(SOCKADDR_IN));
	if(SOCKET_ERROR == ret_val)
		return FALSE;
	ret_val = listen(ms_main, SOMAXCONN);
	if(SOCKET_ERROR == ret_val)
		return FALSE;
	return TRUE;
}

BOOL SocketServer::start_service(void)
{
	BOOL ret_val = TRUE;
	show_tip_msg(TRUE);
	char c_input;
	do
	{
		cin>>c_input;
		if('s' == c_input || 'S' == c_input)
		{
			if(create_helper_and_accept_thread())
			{
				show_server_start_msg(TRUE);
			}
			else
			{
				ret_val = FALSE;
			}
			break;
		}
		else
		{
			show_tip_msg(TRUE);
		}
	}
	while(c_input != 's' && c_input !='S');
	return ret_val;
}

BOOL SocketServer::create_helper_and_accept_thread(void)
{
	mb_server_running = TRUE;
	unsigned long ul_threas_ID;
	mh_thread_help = CreateThread(NULL, 0, helper_thread, this, 0, &ul_threas_ID);
	if(NULL == mh_thread_help)
	{
		mb_server_running = FALSE;
		return FALSE;
	}
	else
	{
		CloseHandle(mh_thread_help);
	}
	mh_thread_accept = CreateThread(NULL, 0, accept_thread, this, 0, &ul_threas_ID);
	if(NULL == mh_thread_accept)
	{
		mb_server_running = FALSE;
		return FALSE;
	}
	else
	{
		CloseHandle(mh_thread_accept);
	}
	return TRUE;
}

void SocketServer::stop_services(void)
{
	BOOL ret_val  = TRUE;
	show_tip_msg(FALSE);

	char c_input;
	for(;mb_server_running;)
	{
		cin >> c_input;
		if(c_input == 'e' || c_input == 'E')
		{
			if(IDOK == MessageBoxA(NULL, "Are you sure?", "Server", MB_OKCANCEL))
			{
				break;
			}
			else
			{
				Sleep(TIMEFOR_THREAD_EXIT);
			}
		}
		else
		{
			Sleep(TIMEFOR_THREAD_EXIT);
		}
	}
	mb_server_running = FALSE;
	show_server_exit_msg();
	Sleep(TIMEFOR_THREAD_EXIT);
	WaitForSingleObject(mh_server_event, INFINITE);
	return;
}


DWORD	__stdcall	SocketServer::helper_thread(void *pParam)
{
	SocketServer *p_server = (SocketServer *)pParam;
	for (;p_server->mb_server_running;)//���A�����b����
	{
		EnterCriticalSection(&p_server->mcs_client_list);//�i�J�{�ɰ�
		
		//�M�z�H���_���s�u�Ȥ�ݰO����Ŷ�
		SOCKET_SERVER_CLIENT::iterator iter = p_server->m_client_list.begin();		
		for (iter; iter != p_server->m_client_list.end();)
		{
			SocketSClient *p_client = (SocketSClient*)*iter;
			if (p_client->is_exit())			//�Ȥ�ݤw�g�h�X
			{
				p_server->m_client_list.erase(iter++);	//�R�����I
				delete p_client;				//����O����
				p_client = NULL;		
			}else
			{
				iter++;						//���Щ��U��
			}				
		}
		
		LeaveCriticalSection(&p_server->mcs_client_list);//���}�{�ɰ�
		
		Sleep(TIMEFOR_THREAD_HELP);
	}
	
	
	//���A������u�@
	if (!p_server->mb_server_running)
	{
		//���_�C�ӳs�u�A������h�X
		EnterCriticalSection(&p_server->mcs_client_list);
		SOCKET_SERVER_CLIENT::iterator iter = p_server->m_client_list.begin();		
		for (iter; iter != p_server->m_client_list.end();)
		{
			SocketSClient *p_client = (SocketSClient*)*iter;
			//�p�G�Ȥ�ݪ��s�u�٦s�b�h���椤�_�A������h�X
			if (p_client->is_conning())
			{
				p_client->dis_conning();
			}
			++iter;			
		}
		//���}�{�ɰ�
		LeaveCriticalSection(&p_server->mcs_client_list);
		
		//���Ȥ�ݰ�����ɶ��A�Ϩ�۰ʰh�X
		Sleep(TIMEFOR_THREAD_SLEEP);
		
		//�i�J�{�ɰ�
		EnterCriticalSection(&p_server->mcs_client_list);
		
		//�T�O�C�ӫȤ�ݪ��O����Ŷ�����^��
		while ( 0 != p_server->m_client_list.size())
		{
			iter = p_server->m_client_list.begin();		
			for (iter; iter != p_server->m_client_list.end();)
			{
				SocketSClient *p_client = (SocketSClient*)*iter;
				if (p_client->is_exit())			//�Ȥ�ݰ�����w�g�h�X
				{
					p_server->m_client_list.erase(iter++);	//�R�����I
					delete p_client;				//�O��O����Ŷ�
					p_client = NULL;
				}else{
					iter++;						//���ФU��
				}				
			}
			//���Ȥ�ݰ�����ɶ��A�Ϩ�۰ʰh�X
			Sleep(TIMEFOR_THREAD_SLEEP);			
		}		
		LeaveCriticalSection(&p_server->mcs_client_list);//���}�{�ɰ�
		
	}
	
	p_server->m_client_list.clear();		//�M���쵲
	
	SetEvent(p_server->mh_server_event);	//�q���D������h�X
	
	return 0;
}

DWORD	__stdcall 	SocketServer::accept_thread(void *pParam)
{
	SocketServer *p_server = (SocketServer*)pParam;	
	SOCKET socket_tmp;
	sockaddr_in addr_client;
	for(;p_server->mb_server_running;)
	{
		memset(&addr_client, 0, sizeof(sockaddr_in));
		int i_len_client = sizeof(sockaddr_in);
		socket_tmp = accept(p_server->ms_main, (LPSOCKADDR)&addr_client, &i_len_client);
		if(INVALID_SOCKET == socket_tmp)
		{
			int n_error_code = WSAGetLastError();
			if(n_error_code == WSAEWOULDBLOCK)//�L�k�����@�ӫD���ש�socket �ާ@
			{
				Sleep(TIMEFOR_THREAD_SLEEP);
				continue;//�~�򵥫�
			}
			else
			{
				return 0;
			}
		}
		else
		{
			SocketSClient *p_sclient = new SocketSClient(socket_tmp, addr_client);
			EnterCriticalSection(&p_server->mcs_client_list);
			p_server->m_client_list.push_back(p_sclient);
			LeaveCriticalSection(&p_server->mcs_client_list);
			p_sclient->start_running();
		}
	}
	return 0;
}



void	SocketServer::show_tip_msg(BOOL b_first_input)
{
	if (b_first_input)
	{
		cout << endl;
		cout << endl;
		cout << "**********************" << endl;
		cout << "*                    *" << endl;
		cout << "* s(S): Start server *" << endl;
		cout << "*                    *" << endl;
		cout << "**********************" << endl;
		cout << "Please input:" << endl;
		
	}else
	{
		cout << endl;
		cout << endl;
		cout << "**********************" << endl;
		cout << "*                    *" << endl;
		cout << "* e(E): Exit  server *" << endl;
		cout << "*                    *" << endl;
		cout << "**********************" << endl;
		cout << " Please input:" << endl;		
	}	

}

void	SocketServer::show_server_start_msg(BOOL bSuc)
{
	if (bSuc)
	{
		cout << "**********************" << endl;
		cout << "*                    *" << endl;
		cout << "* Server succeeded!  *" << endl;
		cout << "*                    *" << endl;
		cout << "**********************" << endl;
	}else
	{
		cout << "**********************" << endl;
		cout << "*                    *" << endl;
		cout << "* Server failed   !  *" << endl;
		cout << "*                    *" << endl;
		cout << "**********************" << endl;
	}
}

void	SocketServer::show_server_exit_msg(void)
{
	cout << "**********************" << endl;
	cout << "*                    *" << endl;
	cout << "* Server exit...     *" << endl;
	cout << "*                    *" << endl;
	cout << "**********************" << endl;
}

void SocketServer::exit_server(void)
{
	DeleteCriticalSection(&mcs_client_list);
	CloseHandle(mh_server_event);
	closesocket(ms_main);
	WSACleanup();
}
#endif //SERVER_H_