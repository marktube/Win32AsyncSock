// Win32SockAsyncServer.cpp : ����Ӧ�ó������ڵ㡣
//

#include "stdafx.h"
#include "Win32SockAsyncServer.h"

#define MAX_LOADSTRING 100
#define UM_SERVER (WM_USER+200) //�Զ���������Ϣ

#pragma warning(disable:4996)

// ȫ�ֱ���: 
HINSTANCE hInst;								// ��ǰʵ��
TCHAR szTitle[MAX_LOADSTRING];					// �������ı�
TCHAR szWindowClass[MAX_LOADSTRING];			// ����������
HWND hdlg;                                     //�����ڶԻ���
HWND hListview;                                //�б�ؼ�
SOCKET	m_ListenSock;                          //�����׽���
SOCKET	m_ClientSock;                          //�����׽���
int cnt=0;                                       //��Ϣ��Ŀͳ��

// �˴���ģ���а����ĺ�����ǰ������: 
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);

INT_PTR CALLBACK DlgProc(HWND hdlg, UINT msg, WPARAM wParam, LPARAM lParam);//�Ի�����Ϣ����ص�����

void OnSock(WPARAM, LPARAM);
void insertInfo(LPWSTR);

int APIENTRY _tWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPTSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

 	// TODO:  �ڴ˷��ô��롣
	MSG msg;
	HACCEL hAccelTable;

	// ��ʼ��ȫ���ַ���
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_WIN32SOCKASYNCSERVER, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// ִ��Ӧ�ó����ʼ��: 
	if (!InitInstance (hInstance, nCmdShow))
	{
		return FALSE;
	}

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_WIN32SOCKASYNCSERVER));

	// ����Ϣѭ��: 
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
//  ����:  MyRegisterClass()
//
//  Ŀ��:  ע�ᴰ���ࡣ
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
//   ����:  InitInstance(HINSTANCE, int)
//
//   Ŀ��:  ����ʵ�����������������
//
//   ע��: 
//
//        �ڴ˺����У�������ȫ�ֱ����б���ʵ�������
//        ��������ʾ�����򴰿ڡ�
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   HWND hWnd;

   hInst = hInstance; // ��ʵ������洢��ȫ�ֱ�����

   hWnd = CreateWindow(szWindowClass,//����������
	   szTitle,//���������
	   WS_OVERLAPPEDWINDOW,//���ڷ��
      CW_USEDEFAULT,//X����
	  0,//Y����
	  575,//���
	  430,//�߶�
	  NULL, NULL, hInstance, NULL);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   //��ʾ�Ի���
   
   hdlg = CreateDialog(hInst//Ӧ�ó���ʵ���������дWinMain�����е�hInstance ����
	   , MAKEINTRESOURCE(IDD_FORMVIEW)//�������ڵ���Դģ�档��MAKEINTRESOURCE(�����ԴID)����
	   , hWnd//��ǰ���ڵľ��
	   , (DLGPROC)DlgProc);//�������ĶԻ�����Ϣ������
   ShowWindow(hdlg, SW_SHOWNA);
   
   hListview = GetDlgItem(hdlg,IDC_MYLIST);//��ȡlistview���ھ��

   if (hListview == NULL){
	   MessageBox(hWnd, L"û�ҵ��ؼ�",L"GG", MB_OK);
	   return true;
   }

   //����listview����
   LVCOLUMN vcl;
   vcl.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
   // ��һ��  
   vcl.pszText = L"Info";//�б���  
   vcl.cx = 528;//�п�  
   vcl.iSubItem = 0;//�������� 
   ListView_InsertColumn(hListview, 0, &vcl);

   //�����Ŀ
   insertInfo(L"��������......");
   insertInfo(L"��ʼ��Winsock��......");

   //��ʼ��winsock
   WSADATA wsaData;
   WSAStartup(MAKEWORD(2, 2), &wsaData);

   //�����׽��ֲ���Ϊ������ģʽ
   m_ListenSock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
   //�����׽���Ϊ������ģʽ
   WSAAsyncSelect(m_ListenSock,//��Ҫָ���ķ�����ģʽ���׽���
	   hdlg, //���������¼�����Ϣ���ܴ���
	   UM_SERVER,//�����¼��������򴰿ڷ��͵ģ��Զ��壩��Ϣ
	   FD_ACCEPT);//ָ���׽��ֵ�֪ͨ��

   sockaddr_in addr;
   addr.sin_family = AF_INET;
   addr.sin_addr.S_un.S_addr = ADDR_ANY;
   addr.sin_port = htons(6666);//����6666�˿�

   //��IP��ַ��6666�˿�
   bind(m_ListenSock, (SOCKADDR*)&addr, sizeof(addr));
   listen(m_ListenSock, 2);// ��������Ϊ2������������У����ѵ������������������еȴ�����


   return TRUE;
}

//
//  ����:  WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  Ŀ��:    ���������ڵ���Ϣ��
//
//  WM_COMMAND	- ����Ӧ�ó���˵�
//  WM_PAINT	- ����������
//  WM_DESTROY	- �����˳���Ϣ������
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
		// �����˵�ѡ��: 
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
		// TODO:  �ڴ���������ͼ����...
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

// �����ڡ������Ϣ�������
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

//TODO���Ի�����Ϣ����
INT_PTR CALLBACK DlgProc(HWND hdlg, UINT msg, WPARAM wParam, LPARAM lParam){
	UNREFERENCED_PARAMETER(lParam);
	switch (msg)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;
	case UM_SERVER://�����׽�����Ϣ
		OnSock(wParam, lParam);
		//MessageBox(hdlg, _T("�ɹ�"), L"GG", MB_OK);
		break;
	default:
		//MessageBox(hdlg, _T("δ֪����Ϣ"), L"GG", MB_OK);
		break;
	}
	return (INT_PTR)FALSE;
}

void insertInfo(LPWSTR info){
	//�����Ŀ
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
		m_StrMsg.Format(_T("�����ַ��%s:%d"), inet_ntoa(ClientAddr.sin_addr), ntohs(ClientAddr.sin_port));
		insertInfo((LPWSTR)(LPCTSTR)m_StrMsg);
		
		break;
		//TODO:����FDD_READ,FDD_CLOSE,FDD_CONNECT
	default:
		break;
	}
}