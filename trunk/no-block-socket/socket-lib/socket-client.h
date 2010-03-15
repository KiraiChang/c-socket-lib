#ifndef CLIENT_H_
#define CLIENT_H_


#pragma comment(lib, "WS2_32.lib")
#include<winsock2.h>
#include<iostream>
#include <winbase.h>

#include "socket-addr.h"

using namespace std;

enum{
		  MAX_NUM_BUF=48,//
		};


typedef struct _head
{
	char type;//型別
	unsigned short len;//資料包的長度
}hdr, *phdr;

typedef struct _data 
{
	char	buf[MAX_NUM_BUF];//資料
}DATABUF, *pDataBuf;

enum{
		  CLIENT_SETUP_FAIL = 1,
		  CLIENT_CREATETHREAD_FAIL = 2	,
		  CLIENT_CONNECT_FAIL = 3,
		  TIMEFOR_THREAD_EXIT = 1000,
		  TIMEFOR_THREAD_SLEEP = 500,
		  ADD = '+',
		  SUB = '-',
		  MUT = '*',
		  DIV = '/',
		  EQU = '=',
		  EXPRESSION = 'E',
		  BYEBYE = 'B',
		  HEADERLEN = (sizeof(hdr)),

		};


class SocketClient
{
	SOCKET ms_main;
	HANDLE mh_thread_send;
	HANDLE mh_thread_recv;
	HANDLE mh_event_show_data_result;
	HANDLE marr_thread[2];//子執行緒陣列
	DATABUF m_buf_send;
	DATABUF m_buf_recv;
	CRITICAL_SECTION mcs_send;
	CRITICAL_SECTION mcs_recv;
	BOOL mb_send_data;
	BOOL mb_conning;
public:
	SocketClient()														{inti_member();}
	~SocketClient()														{exit_client();}
	int work(SocketAddr addr);
	void inti_member(void);
	BOOL init_socket(void);
	BOOL init_client(void);
	BOOL connect_server(SocketAddr addr);
	BOOL create_send_and_recv_thread(void);
	BOOL pack_expression(const char *p_expr);
	void input_and_output(void);

	BOOL	pack_bye_bye(const char* p_expr);
	void exit_client(void);
	void show_connect_msg(BOOL b_conning);
	void	show_data_result_msg(void);
	void show_tip_msg(BOOL b_first_input);

	static DWORD __stdcall send_thread(void *p_param);
	static DWORD __stdcall recv_thread(void *p_param);

};

int SocketClient::work(SocketAddr addr)
{
	if (!init_client())
	{	
		exit_client();
		return CLIENT_SETUP_FAIL;
	}	
	
	if (connect_server(addr))
	{
		show_connect_msg(TRUE);	
	}
	else
	{
		show_connect_msg(FALSE);		
		exit_client();
		return CLIENT_SETUP_FAIL;		
	}
	
	if (!create_send_and_recv_thread())
	{
		exit_client();
		return CLIENT_CREATETHREAD_FAIL;
	}
	
	input_and_output();
	
	exit_client();

	return 0;
}

void SocketClient::inti_member(void)
{
	InitializeCriticalSection(&mcs_send);
	InitializeCriticalSection(&mcs_recv);

	ms_main = INVALID_SOCKET;
	mh_thread_send = NULL;
	mh_thread_recv = NULL;

	mb_send_data = FALSE;
	mb_conning = FALSE;

	memset(m_buf_send.buf, 0, MAX_NUM_BUF);
	memset(m_buf_recv.buf, 0, MAX_NUM_BUF);
	memset(marr_thread, 0, 2);

	mh_event_show_data_result = (HANDLE)CreateEvent(NULL, TRUE, FALSE, NULL);
}

BOOL SocketClient::init_socket()
{
	int ret_val;
	//初始化Windows Sockets DLL
	WSADATA wsaData;
	ret_val = WSAStartup(MAKEWORD(2, 2), &wsaData);
	ms_main = socket(AF_INET, SOCK_STREAM, 0);
	if(INVALID_SOCKET == ms_main)
		return FALSE;
	unsigned long ul = 1;
	ret_val = ioctlsocket(ms_main, FIONBIO, (unsigned long *)&ul);
	if(INVALID_SOCKET == ms_main)
		return FALSE;
	return TRUE;
}

BOOL SocketClient::init_client(void)
{
	if(!init_socket())
	{
		return FALSE;
	}
	return TRUE;
}

BOOL SocketClient::connect_server(SocketAddr addr)
{
	int ret_val;
	for(;;)
	{
		ret_val = connect(ms_main, (LPSOCKADDR)&addr.get_addr(), sizeof(sockaddr_in));

		if(SOCKET_ERROR == ret_val)
		{
			int n_error_code = WSAGetLastError();
			if(WSAEWOULDBLOCK == n_error_code)
			{
				continue;
			}
			else if(WSAEISCONN == n_error_code)
			{
				break;
			}
		}

		if(ret_val == 0)
			break;
	}
	mb_conning = TRUE;
	return TRUE;
}

BOOL SocketClient::create_send_and_recv_thread(void)
{
	unsigned long ul_threas_ID;
	mh_thread_recv = CreateThread(NULL, 0, recv_thread, this, 0, &ul_threas_ID);
	if(NULL == mh_thread_recv)
		return FALSE;

	mh_thread_send = CreateThread(NULL, 0, send_thread, this, 0, &ul_threas_ID);
	if(NULL == mh_thread_send)
		return FALSE;

	marr_thread[0] = mh_thread_recv;
	marr_thread[1] = mh_thread_send;	

	return TRUE;
}

void SocketClient::input_and_output(void)
{
	char c_input[MAX_NUM_BUF];//使用者輸入緩衝區
	BOOL b_first_input = TRUE;//第一次只能輸入算數運算式
	for(;mb_conning;)
	{
		memset(c_input, 0, MAX_NUM_BUF);
		show_tip_msg(b_first_input);
		cin >> c_input;
		char *p_temp = c_input;
		if(b_first_input)
		{
			if(!pack_expression(p_temp))
			{
				continue;
			}
			b_first_input = FALSE;
		}
		else if(!pack_bye_bye(p_temp))
		{
			if(!pack_expression(p_temp))
			{
				continue;
			}
		}
		//等待計算結果
		if(WAIT_OBJECT_0 == WaitForSingleObject(mh_event_show_data_result, INFINITE))
		{
			ResetEvent(mh_event_show_data_result);//設置為無信號
			if(!mb_conning)
			{
				break;
			}
			show_data_result_msg();//顯式資料結果
			if(0 == strcmp(m_buf_recv.buf, "OK"))//客戶端自動退出
			{
				mb_conning = FALSE;
				Sleep(TIMEFOR_THREAD_EXIT);//給資料傳送和接收執行緒退出時間
			}
		}
	}
	if(!mb_conning)//與伺服器連線已經中斷
	{
		show_connect_msg(FALSE);
	}
	DWORD ret_val = WaitForMultipleObjects(2, marr_thread, TRUE, INFINITE);
	if(WAIT_ABANDONED_0 ==ret_val)
	{
		int n_error_code = GetLastError();
	}
}

BOOL	SocketClient::pack_expression(const char *p_expr)
{
	
	char* p_temp = (char*)p_expr;
	while (!*p_temp)
		p_temp++;		
	
	char* pos1 = p_temp;
	char* pos2 = NULL;
	char* pos3 = NULL;	
	int len1 = 0;
	int len2 = 0;
	int len3 = 0;

	if ((*p_temp != '+') && 
		(*p_temp != '-') &&
		((*p_temp < '0') || (*p_temp > '9')))
	{
		return FALSE;
	}

	
	if ((*p_temp++ == '+')&&(*p_temp < '0' || *p_temp > '9'))
		return FALSE;
	--p_temp;
	
	
	if ((*p_temp++ == '-')&&(*p_temp < '0' || *p_temp > '9'))	
		return FALSE;
	--p_temp;
	
	char* p_num = p_temp;
	if (*p_temp == '+'||*p_temp == '-')
		p_temp++;
	
	while (*p_temp >= '0' && *p_temp <= '9')
		p_temp++;							
	
	len1 = p_temp - p_num;

	while(!*p_temp)							
		p_temp++;
	
	if ((ADD != *p_temp)&&			
		(SUB != *p_temp)&&
		(MUT != *p_temp)&&
		(DIV != *p_temp))
		return FALSE;
	
	pos2 = p_temp;
	len2 = 1;
	
	p_temp++;
	while(!*p_temp)
		p_temp++;
	
	pos3 = p_temp;
	if (*p_temp < '0' || *p_temp > '9')
		return FALSE;
	
	while (*p_temp >= '0' && *p_temp <= '9')
		p_temp++;
	
	if (EQU != *p_temp)
		return FALSE;
	
	len3 = p_temp - pos3;
	

	int n_exprlen = len1 + len2 + len3;

	EnterCriticalSection(&mcs_send);
	phdr p_header = (phdr)(m_buf_send.buf);
	p_header->type = EXPRESSION;
	p_header->len = n_exprlen + HEADERLEN;
	memcpy(m_buf_send.buf + HEADERLEN, pos1, len1);
	memcpy(m_buf_send.buf + HEADERLEN + len1, pos2, len2);
	memcpy(m_buf_send.buf + HEADERLEN + len1 + len2 , pos3,len3);
	LeaveCriticalSection(&mcs_send);
	p_header = NULL;

	mb_send_data = TRUE;
	return TRUE;
}

BOOL	SocketClient::pack_bye_bye(const char* p_expr)
{
	BOOL ret_val = FALSE;
	
	if(!strcmp("Byebye", p_expr)||!strcmp("byebye", p_expr))
	{
		EnterCriticalSection(&mcs_send);
		phdr p_header = (phdr)m_buf_send.buf;
		p_header->type = BYEBYE;
		p_header->len = HEADERLEN + strlen("Byebye");
		memcpy(m_buf_send.buf + HEADERLEN, p_expr, strlen(p_expr));
		LeaveCriticalSection(&mcs_send);
		
		p_header = NULL;
		mb_send_data = TRUE;
		ret_val = TRUE;		
	}
	
	return ret_val;
}

void SocketClient::exit_client(void)
{
	DeleteCriticalSection(&mcs_send);
	DeleteCriticalSection(&mcs_recv);
	CloseHandle(mh_thread_recv);
	CloseHandle(mh_thread_send);
	closesocket(ms_main);
	WSACleanup();
	return;
}

void	SocketClient::show_connect_msg(BOOL b_conning)
{
	if (b_conning)
	{
		cout << "******************************" << endl;
		cout << "*                            *" << endl;
		cout << "* Succeed to connect server! *" << endl;
		cout << "*                            *" << endl;
		cout << "******************************" << endl;
	}else{
		cout << "***************************" << endl;
		cout << "*                         *" << endl;
		cout << "* Fail to connect server! *" << endl;
		cout << "*                         *" << endl;
		cout << "***************************" << endl;
	}
	
    return;
}

void	SocketClient::show_data_result_msg(void)
{
	EnterCriticalSection(&mcs_recv);
	cout << "**********************************" << endl;
	cout << "*                                *" << endl;
	cout << "*  Result:                       *" << endl;
	cout <<  m_buf_recv.buf <<endl;
	cout << "*                                *" << endl;
	cout << "**********************************" << endl;	
	LeaveCriticalSection(&mcs_recv);
}

void	SocketClient::show_tip_msg(BOOL b_first_input)
{
	if (b_first_input)
	{
		cout << "**********************************" << endl;
		cout << "*                                *" << endl;
		cout << "* Please input expression.       *" << endl;
		cout << "* Usage:NumberOperatorNumber=    *" << endl;
		cout << "*                                *" << endl;
		cout << "**********************************" << endl;
	}else{
		cout << "**********************************" << endl;
		cout << "*                                *" << endl;
		cout << "* Please input: expression       *" << endl;
		cout << "* Usage:NumberOperatorNumber=    *" << endl;
		cout << "*                                *" << endl;
		cout << "* If you want to exit.           *" << endl;
		cout << "* Usage: Byebye or byebye        *" << endl;
		cout << "*                                *" << endl;
		cout << "**********************************" << endl;
	}	
}

DWORD __stdcall SocketClient::send_thread(void *p_param)
{
	SocketClient *p_client = (SocketClient *)p_param;
		while(p_client->mb_conning)
	{
		if(p_client->mb_send_data)
		{
			EnterCriticalSection(&p_client->mcs_send);
			for(;;)
			{
				int n_buf_len = ((phdr)(p_client->m_buf_send.buf))->len;
				int ret_val = send(p_client->ms_main, p_client->m_buf_send.buf, n_buf_len, 0);
				if(SOCKET_ERROR == ret_val)
				{
					int n_error_code = WSAGetLastError();
					if(WSAEWOULDBLOCK == n_error_code)
					{
						continue;
					}
					else
					{
						LeaveCriticalSection(&p_client->mcs_send);
						p_client->mb_conning = FALSE;
						break;
					}
				}
				p_client->mb_send_data = FALSE;
				break;
			}
			LeaveCriticalSection(&p_client->mcs_send);
		}
		Sleep(TIMEFOR_THREAD_SLEEP);//執行緒睡眠
	}
	return 0;
}

DWORD __stdcall SocketClient::recv_thread(void *p_param)
{
	SocketClient *p_client = (SocketClient *)p_param;

	int ret_val;
	char temp[MAX_NUM_BUF];
	memset(temp, 0, MAX_NUM_BUF);
	while(p_client->mb_conning)
	{
		ret_val = recv(p_client->ms_main, temp, MAX_NUM_BUF, 0);
		if(SOCKET_ERROR == ret_val)
		{
			int n_error_code = WSAGetLastError();
			if(WSAEWOULDBLOCK == n_error_code)
			{
				Sleep(TIMEFOR_THREAD_SLEEP);//執行緒睡眠
				continue;
			}
			else
			{
				p_client->mb_conning = FALSE;
				SetEvent(p_client->mh_event_show_data_result);
				return 0;
			}
		}
		if(ret_val == 0)
		{
			p_client->mb_conning = FALSE;
			SetEvent(p_client->mh_event_show_data_result);
			return 0;
		}

		if(ret_val > HEADERLEN && -1 != ret_val)
		{
			phdr p_header = (phdr)(temp);
			EnterCriticalSection(&p_client->mcs_recv);
			memset(p_client->m_buf_recv.buf, 0, MAX_NUM_BUF);
			memcpy(p_client->m_buf_recv.buf, temp + HEADERLEN, p_header->len - HEADERLEN);
			LeaveCriticalSection(&p_client->mcs_recv);

			SetEvent(p_client->mh_event_show_data_result);
			memset(temp, 0, MAX_NUM_BUF);
		}
		Sleep(TIMEFOR_THREAD_SLEEP);//執行緒睡眠
	}
	return 0;
}


#endif //CLIENT_H_