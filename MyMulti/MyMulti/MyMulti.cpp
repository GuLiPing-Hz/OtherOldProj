// MyMulti.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "MyMulti.h"
#include "OpenGLWindow.h"

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;								// current instance
TCHAR szTitle[MAX_LOADSTRING];					// The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];			// the main window class name
#define TIME_DEBUG_ID 4703
COpenGLWindow* g_gl = NULL;
HANDLE					 g_hThread=NULL;
bool							 g_running=true;
// Forward declarations of functions included in this code module:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);
void onSelect();

int APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

 	// TODO: Place code here.
	MSG msg;
	HACCEL hAccelTable;

	//HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED/*COINIT_APARTMENTTHREADED*/);//初始化
	HRESULT hr = CoInitialize(NULL);
	if(FAILED(hr))
	{
		return FALSE;
	}
	// Initialize global strings
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_MYMULTI, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// Perform application initialization:
	if (!InitInstance (hInstance, nCmdShow))
	{
		CoUninitialize();
		return FALSE;
	}

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_MYMULTI));
	onSelect();
	SetTimer(g_gl->getWnd(),TIME_DEBUG_ID,5000,NULL);
	// Main message loop:
	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	CoUninitialize();
	return (int) msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
//  COMMENTS:
//
//    This function and its usage are only necessary if you want this code
//    to be compatible with Win32 systems prior to the 'RegisterClassEx'
//    function that was added to Windows 95. It is important to call this function
//    so that the application will get 'well formed' small icons associated
//    with it.
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
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_MYMULTI));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= MAKEINTRESOURCE(IDC_MYMULTI);
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassEx(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   HWND hWnd;

   hInst = hInstance; // Store instance handle in our global variable
   g_gl = COpenGLWindow::getWindowSingleton();
   hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, g_gl->getWinSize().width, g_gl->getWinSize().height, NULL, NULL, hInstance, NULL);

   if (!hWnd)
   {
      return FALSE;
   }

   g_gl->setWnd(hWnd);
   g_gl->m_pDM->initialize(g_gl);

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND	- process the application menu
//  WM_PAINT	- Paint the main window
//  WM_DESTROY	- post a quit message and return
//
//

DWORD WINAPI ThreadGLP(LPVOID pParam)
{
	timeBeginPeriod(1);
	while(g_running)
	{
		g_gl->m_pDM->setRenderTarget();
		g_gl->m_pDM->clearBuffer();
		g_gl->m_pDM->beginDrawing();
		//g_gl->m_pDM->renderVideo();
		g_gl->m_pDM->renderGraphic();
		g_gl->m_pDM->endDrawing();
		g_gl->m_pDM->present();
		Sleep(16);
	}
	timeEndPeriod(1);
	return 0;
}

std::string GetMoviePath()
{
	OPENFILENAME ofn;
	ZeroMemory(&ofn, sizeof(ofn));
	TCHAR  szBuffer[MAX_PATH];
	szBuffer[0] = NULL;

	static const TCHAR szFilter[]  
	= TEXT("Video Files (.ASF, .AVI, .MPG, .MPEG, .VOB, .QT, .WMV)\0*.ASF;*.AVI;*.MPG;*.MPEG;*.VOB;*.QT;*.WMV\0") \
		TEXT("All Files (*.*)\0*.*\0\0");
	ofn.lStructSize         = sizeof(OPENFILENAME);
	ofn.hwndOwner           = g_gl->getWnd();
	ofn.hInstance           = NULL;
	ofn.lpstrFilter         = szFilter;
	ofn.nFilterIndex        = 1;
	ofn.lpstrCustomFilter   = NULL;
	ofn.nMaxCustFilter      = 0;
	ofn.lpstrFile           = szBuffer;
	ofn.nMaxFile            = MAX_PATH;
	ofn.lpstrFileTitle      = NULL;
	ofn.nMaxFileTitle       = 0;
	ofn.lpstrInitialDir     = NULL;
	ofn.lpstrTitle          = TEXT("Select a video file to play...");
	ofn.Flags               = OFN_HIDEREADONLY;
	ofn.nFileOffset         = 0;
	ofn.nFileExtension      = 0;
	ofn.lpstrDefExt         = TEXT("AVI");
	ofn.lCustData           = 0L;
	ofn.lpfnHook            = NULL;
	ofn.lpTemplateName  = NULL; 

	if (GetOpenFileName (&ofn))  // user specified a file
	{
		return szBuffer;
	}

	return "";
}
//#define  IMAC
void onDebug()
{
	std::string file;
	static bool b = true;
	b = !b;
	if (b)//D:\\glp\\work\\SVN\\ktv play\\src\\DXDPdemo\\debug\\mv;F:\\glp\\video\\mv
#ifdef IMAC
		file = "D:\\glp\\work\\SVN\\ktv play\\src\\DXDPdemo\\debug\\mv\\003129=●=好想好想=●=古巨基.mpg";
	else
		file = "D:\\glp\\work\\SVN\\ktv play\\src\\DXDPdemo\\debug\\mv\\16322;010747=●=你不是我=●=徐良.mpg";
#else
		file = "F:\\glp\\video\\mv\\003129=●=好想好想=●=古巨基.mpg";
	else
		file = "F:\\glp\\video\\mv\\16322;010747=●=你不是我=●=徐良.mpg";
#endif
	FILE* fp = fopen("glp.txt","w");
	static int nS = 0;
	nS++;

	if(fp)
	{
		fprintf_s(fp,"切歌次数%d",nS);
		fclose(fp);
	}
	std::string newname;
	if(g_gl->changeFile(file.c_str(),false,newname,NULL))
		g_gl->run(newname.c_str(),true);
}

void onChange()
{
	std::string path = GetMoviePath();

	std::string newname;
	if(g_gl->changeFile(path.c_str(),false,newname,NULL))
		g_gl->run(newname.c_str(),true);
}

void onSelect()
{
	if (!g_hThread)
	{
		DWORD threadId;
		g_hThread = CreateThread(NULL,0,ThreadGLP,0,0,&threadId);
	}
}

void onAdd()
{
	if(g_gl->m_pDM->m_mapStrId.size() >= 4)
	{
		OutputDebugStringA("it's full\n");
		return;
	}
	std::string path = GetMoviePath();
	static int index = 0;
	index++;
	char buf[260] = {0};
	sprintf_s(buf,"%d",index);
	if(!g_gl->addFile(path.c_str(),buf,false))
	{
		return;
	}

	if (!g_hThread)
	{
		DWORD threadId;
		g_hThread = CreateThread(NULL,0,ThreadGLP,0,0,&threadId);
	}
	g_gl->runAll();
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;

	if(!g_gl)
	{
		g_gl = COpenGLWindow::getWindowSingleton();
	}

	switch (message)
	{
	case WM_TIMER:
		{
			if (wParam == TIME_DEBUG_ID)
			{
				onDebug();
			}
			break;
		}
	case WM_GRAPHNOTIFY:
		{
			//OutputDebugStringA("graph end\n");
			break;
		}
	case WM_COMMAND:
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		// Parse the menu selections:
		switch (wmId)
		{
		case ID_FILE_SELECT:
			{
				onSelect();
				break;
			}
		case ID_FILE_CHANGE:
			{
				if (g_gl->m_pDM->m_mapStrId.empty())
					onAdd();
				else
					onChange();
				break;
			}
		case ID_FILE_ADD:
			{
				onAdd();
				break;
			}
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
		// TODO: Add any drawing code here...
		EndPaint(hWnd, &ps);
		break;
	case WM_DESTROY:
		g_running = false;
		WaitForSingleObject(g_hThread,INFINITE);
		if(g_hThread)
			CloseHandle(g_hThread);
		COpenGLWindow::releaseWindowSingleton();
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

// Message handler for about box.
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
