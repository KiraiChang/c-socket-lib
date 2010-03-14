#ifndef SERVER_CLIENT_H_
#define SERVER_CLIENT_H_

#include<list>

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
		  HEADERLEN	 = (sizeof(hdr)),//表頭長度
		  EXPRESSION = 'E',//算數運算式
		  BYEBYE = 'B',//訊息byebye
		  TIMEFOR_THREAD_CLIENT = 500,
		   INVALID_NUM = 2,
		   ZERO = 0,
		   INVALID_OPERATOR = 1,
		};

class SocketSClient
{
	SocketSClient()															{}
	DATABUF m_data;
	SOCKET ms_main;
	sockaddr_in m_addr;
	HANDLE mh_event;
	HANDLE mh_thread_recv;
	HANDLE mh_thread_send;
	CRITICAL_SECTION m_critical_section;
	BOOL mb_exit;
	BOOL mb_conning;
public:
	SocketSClient(SOCKET socket, sockaddr_in addr);
	virtual ~SocketSClient();
	BOOL start_running(void);
	void handle_data(const char *p_expr);
	BOOL is_exit(void)													{return mb_exit;}
	void dis_conning(void)												{mb_conning = FALSE;}
	BOOL is_conning(void)											{return mb_conning;}

	static DWORD __stdcall recv_data_thread(void *pParam);
	static DWORD __stdcall send_data_thread(void *pParam);
};

SocketSClient::SocketSClient(SOCKET socket, sockaddr_in addr)
{
	ms_main = socket;
	m_addr = addr;
	mh_event = CreateEvent(NULL, FALSE, FALSE, NULL);//手動設定信號狀態，初始會為無信號
	mh_thread_recv = NULL;
	mh_thread_send = NULL;
	memset(m_data.buf, 0, MAX_NUM_BUF);
	InitializeCriticalSection(&m_critical_section);//初始化臨界區
	mb_exit = FALSE;
	mb_conning = FALSE;
}

SocketSClient::~SocketSClient()
{
	closesocket(ms_main);
	ms_main = INVALID_SOCKET;
	DeleteCriticalSection(&m_critical_section);
	CloseHandle(mh_event);
}

BOOL SocketSClient::start_running(void)
{
	mb_conning = TRUE;
	unsigned long ul_thread_id;
	mh_thread_recv = CreateThread(NULL, 0, recv_data_thread, this, 0, &ul_thread_id);//建立接收資料執行緒
	if(NULL == mh_thread_recv)
	{
		return FALSE;
	}
	else
	{
		CloseHandle(mh_thread_recv);
	}

	mh_thread_send = CreateThread(NULL, 0, send_data_thread, this, 0, &ul_thread_id);//建立接收資料執行緒
	if(NULL == mh_thread_send)
	{
		return FALSE;
	}
	else
	{
		CloseHandle(mh_thread_send);
	}
	return TRUE;
}

void SocketSClient::handle_data(const char *p_expr)
{
	memset(m_data.buf, 0, MAX_NUM_BUF);
	if(BYEBYE == ((phdr)p_expr)->type)
	{
		EnterCriticalSection(&m_critical_section);
		phdr p_header_send = (phdr)m_data.buf;
		p_header_send->type = BYEBYE;
		p_header_send->len = HEADERLEN + strlen("OK");
		memcpy(m_data.buf + HEADERLEN, "OK", strlen("OK"));
		LeaveCriticalSection(&m_critical_section);
	}
	else//運算式
	{
		int n_fir_num;
		int n_sec_num;
		char c_oper;
		int n_result;
		sscanf_s(p_expr + HEADERLEN, "%d%c%d", &n_fir_num, &c_oper, &n_sec_num);
		switch(c_oper)
		{
		case '+':
			{
				n_result = n_fir_num + n_sec_num;
				break;
			}
		case '-':
			{
				n_result = n_fir_num - n_sec_num;
				break;
			}
		case '*':
			{
				n_result = n_fir_num * n_sec_num;
				break;
			}
		case '/':
			{
				if(ZERO == n_sec_num)
				{
					n_result = INVALID_NUM;
				}
				else
				{
					n_result = n_fir_num / n_sec_num;
				}
				break;
			}
		default:
			n_result = INVALID_OPERATOR;
			break;

		}
		char temp[MAX_NUM_BUF];
		char c_equ = '=';
		sprintf_s(temp, "%d%c%d%c%d", n_fir_num, c_oper, n_sec_num, c_equ, n_result);
		//打包資料
		EnterCriticalSection(&m_critical_section);
		phdr p_header_send = (phdr)m_data.buf;
		p_header_send->type = EXPRESSION;
		p_header_send->len = HEADERLEN + strlen(temp);
		memcpy(m_data.buf+HEADERLEN, temp, strlen(temp));
		LeaveCriticalSection(&m_critical_section);
	}
}


DWORD __stdcall SocketSClient::recv_data_thread(void *pParam)
{
	SocketSClient *p_client = (SocketSClient *)pParam;
	int ret_val;
	char temp[MAX_NUM_BUF];
	memset(temp, 0, MAX_NUM_BUF);
	for(;p_client->mb_conning;)
	{
		ret_val = recv(p_client->ms_main, temp, MAX_NUM_BUF, 0);
		//處理錯誤回傳值
		if(SOCKET_ERROR == ret_val)
		{
			int n_error_code = WSAGetLastError();
			if(WSAEWOULDBLOCK)
			{
				continue;
			}
			else if(WSAENETDOWN == n_error_code ||
				WSAETIMEDOUT == n_error_code||
				WSAECONNRESET == n_error_code)
			{
				break;
			}
		}
		//客戶端關閉連線
		if(ret_val == 0)
		{
			break;
		}

		//收到資料
		if(ret_val == HEADERLEN)
		{
			p_client->handle_data(temp);//處理資料
			SetEvent(p_client->mh_event);//通知傳送資料執行緒
			memset(temp, 0, MAX_NUM_BUF);//清空臨時變數
		}
		Sleep(TIMEFOR_THREAD_CLIENT);
	}
	p_client->mb_conning = FALSE;//與客戶端的連線中斷
	SetEvent(p_client->mh_event);//通知傳送資料執行緒退出
	return 0;
}

DWORD __stdcall SocketSClient::send_data_thread(void *pParam)
{
	SocketSClient *p_client = (SocketSClient *)pParam;
	for(;p_client->mb_conning;)
	{
		if(WAIT_OBJECT_0 == WaitForSingleObject(p_client->mh_event, INFINITE))
		{
			//當客戶端的連線中斷，接收資料執行緒先退出，然後該執行緒退出，並設置退出旗標
			if(!p_client->mb_conning)
			{
				p_client->mb_exit = TRUE;
				break;
			}
			EnterCriticalSection(&p_client->m_critical_section);
			phdr p_header = (phdr)p_client->m_data.buf;
			int n_send_len = p_header->len;
			int ret_val = send(p_client->ms_main, p_client->m_data.buf, n_send_len, 0);
			//處理錯誤回傳值
			if(SOCKET_ERROR == ret_val)
			{
				int n_error_code = WSAGetLastError();
				if(WSAEWOULDBLOCK)//傳送資料緩衝區不可用
				{
					continue;
				}
				else if(WSAENETDOWN == n_error_code ||
					WSAETIMEDOUT == n_error_code||
					WSAECONNRESET == n_error_code)//客戶端關閉連線
				{
					LeaveCriticalSection(&p_client->m_critical_section);
					p_client->mb_conning = FALSE;
					p_client->mb_exit = TRUE;
					break;
				}
				else
				{
					LeaveCriticalSection(&p_client->m_critical_section);
					p_client->mb_conning = FALSE;
					p_client->mb_exit = TRUE;
					break;
				}
			}
			//成功傳送資料
			LeaveCriticalSection(&p_client->m_critical_section);//離開臨界區
			ResetEvent(&p_client->mh_event);//設置事件為無信號狀態
		}
	}
	return 0;
}


typedef std::list<SocketSClient *> SOCKET_SERVER_CLIENT;
#endif //SERVER_CLIENT_H_