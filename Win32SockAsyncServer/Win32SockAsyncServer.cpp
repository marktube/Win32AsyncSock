// Win32SockAsyncServer.cpp : 定义应用程序的入口点。
//

#include "stdafx.h"
#include "Win32SockAsyncServer.h"

#define MAX_LOADSTRING 100
#define UM_SERVER (WM_USER+200) //自定义网络消息

#pragma warning(disable:4996)

// 全局变量: 
HINSTANCE hInst;								// 当前实例
TCHAR szTitle[MAX_LOADSTRING];					// 标题栏文本
TCHAR szWindowClass[MAX_LOADSTRING];			// 主窗口类名
HWND hdlg;                                     //主窗口对话框
HWND hListview;                                //列表控件
SOCKET	m_ListenSock;                          //监听套接字
SOCKET	m_ClientSock;                          //连接套接字
int cnt=0;                                       //消息条目统计

// 此代码模块中包含的函数的前向声明: 
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);

INT_PTR CALLBACK DlgProc(HWND hdlg, UINT msg, WPARAM wParam, LPARAM lParam);//对话框消息处理回调函数

void OnSock(WPARAM, LPARAM);
void insertInfo(LPWSTR);

int APIENTRY _tWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPTSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

 	// TODO:  在此放置代码。
	MSG msg;
	HACCEL hAccelTable;

	// 初始化全局字符串
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_WIN32SOCKASYNCSERVER, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// 执行应用程序初始化: 
	if (!InitInstance (hInstance, nCmdShow))
	{
		return FALSE;
	}

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_WIN32SOCKASYNCSERVER));

	// 主消息循环: 
	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return (int) msg.wParam;
}



//
//  函数:  MyRegisterClass()
//
//  目的:  注册窗口类。
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_WIN32SOCKASYNCSERVER));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= MAKEINTRESOURCE(IDC_WIN32SOCKASYNCSERVER);
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassEx(&wcex);
}

//
//   函数:  InitInstance(HINSTANCE, int)
//
//   目的:  保存实例句柄并创建主窗口
//
//   注释: 
//
//        在此函数中，我们在全局变量中保存实例句柄并
//        创建和显示主程序窗口。
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   HWND hWnd;

   hInst = hInstance; // 将实例句柄存储在全局变量中

   hWnd = CreateWindow(szWindowClass,//窗口类名称
	   szTitle,//窗口类标题
	   WS_OVERLAPPEDWINDOW,//窗口风格
      CW_USEDEFAULT,//X坐标
	  0,//Y坐标
	  575,//宽度
	  430,//高度
	  NULL, NULL, hInstance, NULL);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   //显示对话框
   
   hdlg = CreateDialog(hInst//应用程序实例句柄，填写WinMain函数中的hInstance 参数
	   , MAKEINTRESOURCE(IDD_FORMVIEW)//创建窗口的资源模版。用MAKEINTRESOURCE(你的资源ID)生成
	   , hWnd//当前窗口的句柄
	   , (DLGPROC)DlgProc);//所创建的对话框消息处理函数
   ShowWindow(hdlg, SW_SHOWNA);
   
   hListview = GetDlgItem(hdlg,IDC_MYLIST);//获取listview窗口句柄

   if (hListview == NULL){
	   MessageBox(hWnd, L"没找到控件",L"GG", MB_OK);
	   return true;
   }

   //设置listview的列
   LVCOLUMN vcl;
   vcl.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
   // 第一列  
   vcl.pszText = L"Info";//列标题  
   vcl.cx = 528;//列宽  
   vcl.iSubItem = 0;//子项索引 
   ListView_InsertColumn(hListview, 0, &vcl);

   //添加条目
   insertInfo(L"正在启动......");
   insertInfo(L"初始化Winsock中......");

   //初始化winsock
   WSADATA wsaData;
   WSAStartup(MAKEWORD(2, 2), &wsaData);

   //创建套接字并设为非阻塞模式
   m_ListenSock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
   //设置套接字为非阻塞模式
   WSAAsyncSelect(m_ListenSock,//需要指定的非阻塞模式的套接字
	   hdlg, //发生网络事件的消息接受窗口
	   UM_SERVER,//网络事件发生后向窗口发送的（自定义）消息
	   FD_ACCEPT);//指定套接字的通知码

   sockaddr_in addr;
   addr.sin_family = AF_INET;
   addr.sin_addr.S_un.S_addr = ADDR_ANY;
   addr.sin_port = htons(6666);//监听6666端口

   //绑定IP地址及6666端口
   bind(m_ListenSock, (SOCKADDR*)&addr, sizeof(addr));
   listen(m_ListenSock, 2);// 建立长度为2的连接请求队列，并把到来的连接请求加入队列等待处理。


   return TRUE;
}

//
//  函数:  WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  目的:    处理主窗口的消息。
//
//  WM_COMMAND	- 处理应用程序菜单
//  WM_PAINT	- 绘制主窗口
//  WM_DESTROY	- 发送退出消息并返回
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;

	switch (message)
	{
	case WM_COMMAND:
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		// 分析菜单选择: 
		switch (wmId)
		{
		case IDM_ABOUT:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		// TODO:  在此添加任意绘图代码...
		EndPaint(hWnd, &ps);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

// “关于”框的消息处理程序。
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}

//TODO：对话框消息处理
INT_PTR CALLBACK DlgProc(HWND hdlg, UINT msg, WPARAM wParam, LPARAM lParam){
	UNREFERENCED_PARAMETER(lParam);
	switch (msg)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;
	case UM_SERVER://处理套接字消息
		OnSock(wParam, lParam);
		//MessageBox(hdlg, _T("成功"), L"GG", MB_OK);
		break;
	default:
		//MessageBox(hdlg, _T("未知的消息"), L"GG", MB_OK);
		break;
	}
	return (INT_PTR)FALSE;
}

void insertInfo(LPWSTR info){
	//添加条目
	LVITEM vitem;
	vitem.mask = LVIF_TEXT;
	vitem.pszText = info;
	vitem.iItem = cnt++;
	vitem.iSubItem = 0;
	ListView_InsertItem(hListview, &vitem);
	ListView_SetItem(hListview, &vitem);
}

void OnSock(WPARAM wParam, LPARAM lParam){
	CString m_StrMsg;
	int nSize;
	if (WSAGETSELECTERROR(lParam)){
		MessageBox(hdlg, L"WSAGETSELECTERROR", L"GG", MB_OK);
		return;
	}
	switch (WSAGETSELECTEVENT(lParam))
	{
	case FD_ACCEPT:
		sockaddr_in ClientAddr;
		nSize = sizeof(ClientAddr);
		m_ClientSock = accept(m_ListenSock, (SOCKADDR*)&ClientAddr, &nSize);
		WSAAsyncSelect(m_ClientSock, hdlg, UM_SERVER, FD_READ | FD_CLOSE);
		m_StrMsg.Format(_T("请求地址是%s:%d"), inet_ntoa(ClientAddr.sin_addr), ntohs(ClientAddr.sin_port));
		insertInfo((LPWSTR)(LPCTSTR)m_StrMsg);
		
		break;
		//TODO:后续FDD_READ,FDD_CLOSE,FDD_CONNECT
	default:
		break;
	}
}