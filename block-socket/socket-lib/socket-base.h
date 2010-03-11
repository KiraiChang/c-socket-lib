#ifndef SOCKET_BASE_H_H
#define SOCKET_BASE_H_H

#include <iostream>
#include <WINSOCK2.H>
#pragma comment(lib, "wsock32.lib")

enum{SERVER_DLL_ERROR = 1, SERVER_API_ERROR};

class SocketBase
{
protected:
	SOCKET s_main;//主要socket
	WORD	wVersionRequested;	//指定準備載入Windows Sockets DLL的版本
	WSADATA	wsaData;//Windows sockets DLL版本資訊
	bool b_conning;//與客戶連線狀態
	//int ret_val;//執行後回傳訊息
public:
	SocketBase();
	virtual ~SocketBase();
	void inti_member(void);
	int WSA_setup(void);
	BOOL recevice_line(SOCKET s, char *buf);
	BOOL send_line(SOCKET s,const char *buf);
	void get_last_error(void);
	void show_socket_msg(const char *msg);
	int handle_socket_error(const char * msg);
	int exit_client(int n_exit);
};

SocketBase::SocketBase()
{
	inti_member();
}

SocketBase::~SocketBase()
{
	exit_client(0);
}
void SocketBase::inti_member(void)
{
	s_main = INVALID_SOCKET;
	b_conning = FALSE;
	//ret_val = 0;
}

int SocketBase::WSA_setup(void)
{
	wVersionRequested = MAKEWORD(1,1);
	int ret_val = WSAStartup(wVersionRequested, &wsaData);
	if(0 != ret_val)
	{
		show_socket_msg("Can not find a usable Winodws Sockets dll!");
		return SERVER_DLL_ERROR;
	}
	if(LOBYTE(wsaData.wVersion) != 1 || HIBYTE(wsaData.wVersion) != 1)
	{
		show_socket_msg("Can not find a usable Winodws Sockets dll!");
		WSACleanup();
		return SERVER_DLL_ERROR;
	}
	s_main = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if(INVALID_SOCKET == s_main)
	{
		return handle_socket_error("Failed socket()!");
	}
	return 0;
}

BOOL SocketBase::recevice_line(SOCKET s, char *buf)
{
	BOOL ret_val = TRUE;
	BOOL b_line_end = FALSE;
	int n_read_len = 0;
	int n_data_len = 0;

	while(!b_line_end && b_conning)
	{
		n_read_len = recv(s, buf + n_data_len, 1, 0);
		if(SOCKET_ERROR == n_read_len)
		{
			get_last_error();
			ret_val = FALSE;
			break;
		}
		if(0 == n_read_len)//客戶端關閉
		{
			ret_val = FALSE;
			break;
		}
		if('\n' == *(buf + n_data_len))//換行字元
		{
			b_line_end = true;//接收資料結束
		}
		else
		{
			n_data_len += n_read_len;//增加資料長度
		}
	}
	return ret_val;
}

BOOL SocketBase::send_line(SOCKET s, const char *buf)
{
	int ret_val = send(s, buf, strlen(buf), 0);
	if(SOCKET_ERROR == ret_val)
	{
		get_last_error();
		return FALSE;
	}
	return TRUE;
}

void SocketBase::get_last_error(void)
{
			int n_error_code = WSAGetLastError();//接收錯誤代碼
			if(WSAENOTCONN == n_error_code)
			{
				show_socket_msg("The socket is not connected!");
			}
			else if(WSAESHUTDOWN == n_error_code)
			{
				show_socket_msg("The socket has been shut down!");
			}
			else	if(WSAETIMEDOUT == n_error_code)
			{
				show_socket_msg("The connection has been dropped!");
			}
			else if(WSAECONNRESET == n_error_code)
			{
				show_socket_msg("The virtual circuit was reset by remote side!");
			}
			else
			{
			}
}

void SocketBase::show_socket_msg(const char *msg)
{
	MessageBoxA(NULL, msg, "SERVER ERROR", MB_OK);
}

int SocketBase::handle_socket_error(const char * msg)
{
	show_socket_msg(msg);
	WSACleanup();
	return SERVER_API_ERROR;
}

int SocketBase::exit_client(int n_exit)
{
	closesocket(s_main);
	WSACleanup();
	return n_exit;
}

#endif //SOCKET_BASE_H_H