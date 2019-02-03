#include "../StdAfx.h"
#include <stdio.h>
#include <string.h>
#include "ParticleSystem.h"
#include <mmsystem.h>
#include <shellapi.h>

//////////////////////////////////////////////////////////////////////////1
#define LOWORDINT(n) ((int)((signed short)(LOWORD(n))))
#define HIWORDINT(n) ((int)((signed short)(HIWORD(n))))


const char *WINDOW_CLASS_NAME = "HGE__WNDCLASS";

LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);


int			nRef=0;
CParticleSystem*	pHGE=NULL;



// BOOL APIENTRY DllMain(HANDLE, DWORD, LPVOID)
// {
//     return TRUE;
// }


HGE* CALL hgeCreate(int ver)
{
	if(ver==HGE_VERSION)
		return (HGE*)CParticleSystem::_Interface_Get();
	else
		return 0;
}


CParticleSystem* CParticleSystem::_Interface_Get()
{
	if(!pHGE) pHGE=new CParticleSystem();

	nRef++;

	return pHGE;
}


void CALL CParticleSystem::Release()
{
	nRef--;

	if(!nRef)
	{
		if(pHGE->hwnd) pHGE->System_Shutdown();
		//Resource_RemoveAllPacks();
		delete pHGE;
		pHGE=0;
	}
}


bool CALL CParticleSystem::System_Initiate()
{
	OSVERSIONINFO	os_ver;
	SYSTEMTIME		tm;
	MEMORYSTATUS	mem_st;
	WNDCLASS		winclass;
	int				width, height;

	// Log system info

	System_Log("HGE Started..\n");

	System_Log("HGE version: %X.%X", HGE_VERSION>>8, HGE_VERSION & 0xFF);
	GetLocalTime(&tm);
	System_Log("Date: %02d.%02d.%d, %02d:%02d:%02d\n", tm.wDay, tm.wMonth, tm.wYear, tm.wHour, tm.wMinute, tm.wSecond);

	System_Log("Application: %s",szWinTitle);
	os_ver.dwOSVersionInfoSize=sizeof(os_ver);
	GetVersionEx(&os_ver);
	System_Log("OS: Windows %ld.%ld.%ld",os_ver.dwMajorVersion,os_ver.dwMinorVersion,os_ver.dwBuildNumber);

	GlobalMemoryStatus(&mem_st);
	System_Log("Memory: %ldK total, %ldK free\n",mem_st.dwTotalPhys/1024L,mem_st.dwAvailPhys/1024L);


	// Register window class
	
	winclass.style = CS_DBLCLKS| CS_OWNDC  | CS_HREDRAW | CS_VREDRAW;
	winclass.lpfnWndProc	= WindowProc;
	winclass.cbClsExtra		= 0;
	winclass.cbWndExtra		= 0;
	winclass.hInstance		= hInstance;
	winclass.hCursor		= LoadCursor(NULL, IDC_ARROW);
	winclass.hbrBackground	= (HBRUSH)GetStockObject(BLACK_BRUSH);
	winclass.lpszMenuName	= NULL; 
	winclass.lpszClassName	= WINDOW_CLASS_NAME;
	if(szIcon) winclass.hIcon = LoadIcon(hInstance, szIcon);
	else winclass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	
	if (!RegisterClass(&winclass)) {
		_PostError("Can't register window class");
		return false;
	}

	// Create window

	width=nScreenWidth + GetSystemMetrics(SM_CXFIXEDFRAME)*2;
	height=nScreenHeight + GetSystemMetrics(SM_CYFIXEDFRAME)*2 + GetSystemMetrics(SM_CYCAPTION);

	rectW.left=(GetSystemMetrics(SM_CXSCREEN)-width)/2;
	rectW.top=(GetSystemMetrics(SM_CYSCREEN)-height)/2;
	rectW.right=rectW.left+width;
	rectW.bottom=rectW.top+height;
	styleW=WS_POPUP|WS_CAPTION|WS_SYSMENU|WS_MINIMIZEBOX|WS_VISIBLE; //WS_OVERLAPPED | WS_SYSMENU | WS_MINIMIZEBOX;

	rectFS.left=0;
	rectFS.top=0;
	rectFS.right=nScreenWidth;
	rectFS.bottom=nScreenHeight;
	styleFS=WS_POPUP|WS_VISIBLE; //WS_POPUP

	if(hwndParent)
	{
		rectW.left=0;
		rectW.top=0;
		rectW.right=nScreenWidth;
		rectW.bottom=nScreenHeight;
		styleW=WS_CHILD|WS_VISIBLE; 
		bWindowed=true;
	}

	if(bWindowed)
		hwnd = CreateWindowEx(0, WINDOW_CLASS_NAME, szWinTitle, styleW,
				rectW.left, rectW.top, rectW.right-rectW.left, rectW.bottom-rectW.top,
				hwndParent, NULL, hInstance, NULL);
	else
		hwnd = CreateWindowEx(WS_EX_TOPMOST, WINDOW_CLASS_NAME, szWinTitle, styleFS,
				0, 0, 0, 0,
				NULL, NULL, hInstance, NULL);
	if (!hwnd)
	{
		_PostError("Can't create window");
		return false;
	}

	ShowWindow(hwnd, SW_SHOW);

	// Init subsystems

	timeBeginPeriod(1);
	//Random_Seed();
	_InitPowerStatus();
	//_InputInit();
	if(!_GfxInit()) { System_Shutdown(); return false; }
	if(!_SoundInit()) { System_Shutdown(); return false; }

	System_Log("Init done.\n");

	fTime=0.0f;
	t0=t0fps=timeGetTime();
	dt=cfps=0;
	nFPS=0;

	// Show splash

#ifdef DEMO

	bool			(*func)();
	bool			(*rfunc)();
	HWND			hwndTmp;

	if(pHGE->bDMO)
	{
//			Sleep(200);
// 		func=(bool(*)())pHGE->System_GetStateFunc(HGE_FRAMEFUNC);//保存
// 		rfunc=(bool(*)())pHGE->System_GetStateFunc(HGE_RENDERFUNC);
// 		hwndTmp=hwndParent; hwndParent=0;
// 		pHGE->System_SetStateFunc(HGE_FRAMEFUNC, DFrame);//显示logo 需要去掉
// 		pHGE->System_SetStateFunc(HGE_RENDERFUNC, 0);
// 		DInit();
// 		pHGE->System_Start();
// 		DDone();
// 		hwndParent=hwndTmp;
// 		pHGE->System_SetStateFunc(HGE_FRAMEFUNC, func);//还原
// 		pHGE->System_SetStateFunc(HGE_RENDERFUNC, rfunc);
	}

#endif

	// Done

	return true;
}

void CALL CParticleSystem::System_Shutdown()
{
	System_Log("\nFinishing..");

	timeEndPeriod(1);
	if(hSearch) { FindClose(hSearch); hSearch=0; }
	//_ClearQueue();
	_SoundDone();
	_GfxDone();
	_DonePowerStatus();

	if(hwnd && !hwndParent)//glp
	{
		//ShowWindow(hwnd, SW_HIDE);
		//SetWindowLong(hwnd, GWL_EXSTYLE, GetWindowLong(hwnd, GWL_EXSTYLE) | WS_EX_TOOLWINDOW);
		//ShowWindow(hwnd, SW_SHOW);
		DestroyWindow(hwnd);
		hwnd=0;
	}

	if(hInstance) UnregisterClass(WINDOW_CLASS_NAME, hInstance);

	System_Log("The End.");
}


bool CALL CParticleSystem::System_Start()
{
	MSG		msg;

	if(!hwnd)
	{
		_PostError("System_Start: System_Initiate wasn't called");
		return false;
	}

	if(!procFrameFunc) {
		_PostError("System_Start: No frame function defined");
		return false;
	}

	bActive=true;

	// MAIN LOOP

	for(;;)
	{
		
		// Process window messages if not in "child mode"
		// (if in "child mode" the parent application will do this for us)

		if(!hwndParent)
		{
			if (PeekMessage(&msg,NULL,0,0,PM_REMOVE))
			{ 
				if (msg.message == WM_QUIT)	break;
				// TranslateMessage(&msg);
				DispatchMessage(&msg);
				continue;
			}
		}

		// Check if mouse is over HGE window for Input_IsMouseOver

		//_UpdateMouse();

		// If HGE window is focused or we have the "don't suspend" state - process the main loop

		if(bActive || bDontSuspend)
		{
			// Ensure we have at least 1ms time step
			// to not confuse user's code with 0

			do { dt=timeGetTime() - t0; } while(dt < 1);

			// If we reached the time for the next frame
			// or we just run in unlimited FPS mode, then
			// do the stuff

			if(dt >= nFixedDelta)
			{
				// fDeltaTime = time step in seconds returned by Timer_GetDelta

				fDeltaTime=dt/1000.0f;

				// Cap too large time steps usually caused by lost focus to avoid jerks

				if(fDeltaTime > 0.2f)
				{
					fDeltaTime = nFixedDelta ? nFixedDelta/1000.0f : 0.01f;
				}

				// Update time counter returned Timer_GetTime

				fTime += fDeltaTime;

				// Store current time for the next frame
				// and count FPS

				t0=timeGetTime();
				if(t0-t0fps <= 1000) cfps++;
				else
				{
					nFPS=cfps; cfps=0; t0fps=t0;
					_UpdatePowerStatus();
				}

				// Do user's stuff
				if(procRenderFunc) procRenderFunc();//render放在前面
				if(procFrameFunc()) break;//后面处理更新
				//if(procRenderFunc) procRenderFunc();
				
				// If if "child mode" - return after processing single frame

				if(hwndParent) break;

				// Clean up input events that were generated by
				// WindowProc and weren't handled by user's code

				//_ClearQueue();

				// If we use VSYNC - we could afford a little
				// sleep to lower CPU usage

				// if(!bWindowed && nHGEFPS==HGEFPS_VSYNC) Sleep(1);
			}

			// If we have a fixed frame rate and the time
			// for the next frame isn't too close, sleep a bit

			else
			{
				if(nFixedDelta && dt+3 < nFixedDelta) Sleep(1);
			}
		}

		// If main loop is suspended - just sleep a bit
		// (though not too much to allow instant window
		// redraw if requested by OS)

		else Sleep(1);
	}

	//_ClearQueue();

	bActive=false;

	return true;
}

void CALL CParticleSystem::System_SetStateBool(hgeBoolState state, bool value)
{
	switch(state)
	{
		case HGE_WINDOWED:		if(VertArray || hwndParent) break;
								if(pD3DDevice && bWindowed != value)
								{
									if(d3dppW.BackBufferFormat==D3DFMT_UNKNOWN || d3dppFS.BackBufferFormat==D3DFMT_UNKNOWN) break;

									if(bWindowed) GetWindowRect(hwnd, &rectW);
									bWindowed=value;
									if(bWindowed) d3dpp=&d3dppW;
									else d3dpp=&d3dppFS;

									if(_format_id(d3dpp->BackBufferFormat) < 4) nScreenBPP=16;
									else nScreenBPP=32;

									_GfxRestore();
									_AdjustWindow();
								}
								else bWindowed=value;
								break;

		case HGE_ZBUFFER:		if(!pD3DDevice)	bZBuffer=value;
								break;

		case HGE_TEXTUREFILTER: bTextureFilter=value;
								if(pD3DDevice)
								{
									_render_batch();
									if(bTextureFilter)
									{
										//pD3DDevice->SetTextureStageState(0,D3DTSS_MAGFILTER,D3DTEXF_LINEAR);
										pD3DDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
										//pD3DDevice->SetTextureStageState(0,D3DTSS_MINFILTER,D3DTEXF_LINEAR);
										pD3DDevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
									}
									else
									{
										//pD3DDevice->SetTextureStageState(0,D3DTSS_MAGFILTER,D3DTEXF_POINT);
										pD3DDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_POINT);
										//pD3DDevice->SetTextureStageState(0,D3DTSS_MINFILTER,D3DTEXF_POINT);
										pD3DDevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_POINT);
									}
								}
								break;

		case HGE_USESOUND:		if(bUseSound!=value)
								{
									bUseSound=value;
									if(bUseSound && hwnd) _SoundInit();
									if(!bUseSound && hwnd) _SoundDone();
								}
								break;

		case HGE_HIDEMOUSE:		bHideMouse=value; break;

		case HGE_DONTSUSPEND:	bDontSuspend=value; break;

		#ifdef DEMO
		case HGE_SHOWSPLASH:	bDMO=value; break;
		#endif
	}
}

void CALL CParticleSystem::System_SetStateFunc(hgeFuncState state, hgeCallback value)
{
	switch(state)
	{
		case HGE_FRAMEFUNC:			procFrameFunc=value; break;
		case HGE_RENDERFUNC:			procRenderFunc=value; break;
		case HGE_FOCUSLOSTFUNC:	procFocusLostFunc=value; break;
		case HGE_FOCUSGAINFUNC:	procFocusGainFunc=value; break;
		case HGE_GFXRESTOREFUNC:	procGfxRestoreFunc=value; break;
		case HGE_EXITFUNC:					procExitFunc=value; break;
	}
}

void CALL CParticleSystem::System_SetStateHwnd(hgeHwndState state, HWND value)
{
	switch(state)
	{
		case HGE_HWNDPARENT:	if(!hwnd) hwndParent=value; break;
	}
}

void CALL CParticleSystem::System_SetStateInt(hgeIntState state, int value)
{
	switch(state)
	{
		case HGE_SCREENWIDTH:	if(!pD3DDevice) nScreenWidth=value; break;

		case HGE_SCREENHEIGHT:	if(!pD3DDevice) nScreenHeight=value; break;

		case HGE_SCREENBPP:		if(!pD3DDevice) nScreenBPP=value; break;

		case HGE_SAMPLERATE:	if(!hBass) nSampleRate=value;
								break;

		case HGE_FXVOLUME:		nFXVolume=value;
								_SetFXVolume(nFXVolume);
								break;

		case HGE_MUSVOLUME:		nMusVolume=value;
								_SetMusVolume(nMusVolume);
								break;

		case HGE_STREAMVOLUME:	nStreamVolume=value;
								_SetStreamVolume(nStreamVolume);
								break;

		case HGE_FPS:			if(VertArray) break;

								if(pD3DDevice)
								{
									if((nHGEFPS>=0 && value <0) || (nHGEFPS<0 && value>=0))
									{
										if(value==HGEFPS_VSYNC)//垂直同步
										{
											d3dppW.SwapEffect = D3DSWAPEFFECT_COPY;
											d3dppFS.PresentationInterval = D3DPRESENT_INTERVAL_ONE;
										}
										else
										{
											d3dppW.SwapEffect = D3DSWAPEFFECT_COPY;
											d3dppFS.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;
										}
										//if(procFocusLostFunc) procFocusLostFunc();
										_GfxRestore();
										//if(procFocusGainFunc) procFocusGainFunc();
									}
								}
								nHGEFPS=value;
								if(nHGEFPS>0) nFixedDelta=int(1000.0f/value);
								else nFixedDelta=0;
								break;
	}
}

void CALL CParticleSystem::System_SetStateString(hgeStringState state, const char *value)
{
	FILE *hf;
	
	switch(state)
	{
		case HGE_ICON:			szIcon=value;
								if(pHGE->hwnd) SetClassLong(pHGE->hwnd, GCL_HICON, (LONG)LoadIcon(pHGE->hInstance, szIcon));
								break;
		case HGE_TITLE:			strcpy(szWinTitle,value);
								if(pHGE->hwnd) SetWindowText(pHGE->hwnd, szWinTitle);
								break;
		case HGE_INIFILE:		if(value) strcpy(szIniFile,Resource_MakePath(value));
								else szIniFile[0]=0;
								break;
		case HGE_LOGFILE:		if(value)
								{
									strcpy(szLogFile,Resource_MakePath(value));
									hf=fopen(szLogFile, "w");
									if(!hf) szLogFile[0]=0;
									else fclose(hf);
								}
								else szLogFile[0]=0;
								break;
	}
}

bool CALL CParticleSystem::System_GetStateBool(hgeBoolState state)
{
	switch(state)
	{
		case HGE_WINDOWED:		return bWindowed;
		case HGE_ZBUFFER:		return bZBuffer;
		case HGE_TEXTUREFILTER:	return bTextureFilter;
		case HGE_USESOUND:		return bUseSound;
		case HGE_DONTSUSPEND:	return bDontSuspend;
		case HGE_HIDEMOUSE:		return bHideMouse;

		#ifdef DEMO
		case HGE_SHOWSPLASH:	return bDMO;
		#endif
	}

	return false;
}

hgeCallback CALL CParticleSystem::System_GetStateFunc(hgeFuncState state)
{
	switch(state)
	{
		case HGE_FRAMEFUNC:			return procFrameFunc;
		case HGE_RENDERFUNC:			return procRenderFunc;
		case HGE_FOCUSLOSTFUNC:	return procFocusLostFunc;
		case HGE_FOCUSGAINFUNC:	return procFocusGainFunc;
		case HGE_EXITFUNC:					return procExitFunc;
	}

	return NULL;
}

HWND CALL CParticleSystem::System_GetStateHwnd(hgeHwndState state)
{
	switch(state)
	{
		case HGE_HWND:			return hwnd;
		case HGE_HWNDPARENT:	return hwndParent;
	}

	return 0;
}

int CALL CParticleSystem::System_GetStateInt(hgeIntState state)
{
	switch(state)
	{
		case HGE_SCREENWIDTH:	return nScreenWidth;
		case HGE_SCREENHEIGHT:	return nScreenHeight;
		case HGE_SCREENBPP:		return nScreenBPP;
		case HGE_SAMPLERATE:	return nSampleRate;
		case HGE_FXVOLUME:		return nFXVolume;
		case HGE_MUSVOLUME:		return nMusVolume;
		case HGE_STREAMVOLUME:	return nStreamVolume;
		case HGE_FPS:			return nHGEFPS;
		case HGE_POWERSTATUS:	return nPowerStatus;
	}

	return 0;
}

const char* CALL CParticleSystem::System_GetStateString(hgeStringState state) {
	switch(state) {
		case HGE_ICON:			return szIcon;
		case HGE_TITLE:			return szWinTitle;
		case HGE_INIFILE:		if(szIniFile[0]) return szIniFile;
								else return 0;
		case HGE_LOGFILE:		if(szLogFile[0]) return szLogFile;
								else return 0;
	}

	return NULL;
}

char* CALL CParticleSystem::System_GetErrorMessage()
{
	return szError;
}

void CALL CParticleSystem::System_Log(const char *szFormat, ...)
{
	FILE *hf = NULL;
	va_list ap;
	
	if(!szLogFile[0]) return;

	hf = fopen(szLogFile, "a");
	if(!hf) return;

	va_start(ap, szFormat);
	vfprintf(hf, szFormat, ap);
	va_end(ap);

	fprintf(hf, "\n");

	fclose(hf);
}

bool CALL CParticleSystem::System_Launch(const char *url)
{
	if((DWORD)ShellExecute(pHGE->hwnd, NULL, url, NULL, NULL, SW_SHOWMAXIMIZED)>32) return true;
	else return false;
}

void CALL CParticleSystem::System_Snapshot(const char *filename)
{
	LPDIRECT3DSURFACE9 pSurf;
	char *shotname, tempname[_MAX_PATH];
	int i;

	if(!filename)
	{
		i=0;
		shotname=Resource_EnumFiles("shot???.bmp");
		while(shotname)
		{
			i++;
			shotname=Resource_EnumFiles();
		}
		sprintf(tempname, "shot%03d.bmp", i);
		filename=Resource_MakePath(tempname);
	}

	if(pD3DDevice)
	{
		pD3DDevice->GetBackBuffer(0,0,D3DBACKBUFFER_TYPE_MONO, &pSurf);
		D3DXSaveSurfaceToFile(filename, D3DXIFF_BMP, pSurf, NULL, NULL);
		pSurf->Release();
	}
}

////////////////
CParticleSystem::CParticleSystem(void)
{
	hInstance=GetModuleHandle(0);
	hwnd=0;
	bActive=false;
	szError[0]=0;

	pD3D=0;
	pD3DDevice=0;
	d3dpp=0;
	pTargets=0;
	pCurTarget=0;
	pScreenSurf=0;
	pScreenDepth=0;
	pVB=0;
	pIB=0;
	VertArray=0;
	textures=0;

// 	hBass=0;
// 	bSilent=false;
// 	streams=0;

	hSearch=0;
	res=0;

	nHGEFPS=HGEFPS_UNLIMITED;
	fTime=0.0f;
	fDeltaTime=0.0f;
	nFPS=0;

	procFrameFunc=0;
	procRenderFunc=0;
	procFocusLostFunc=0;
	procFocusGainFunc=0;
	procGfxRestoreFunc=0;
	procExitFunc=0;
	szIcon=0;
	strcpy(szWinTitle,"GDX ParticleSystem");
	nScreenWidth=800;
	nScreenHeight=600;
	nScreenBPP=32;
	bWindowed=false;
	bZBuffer=false;
	bTextureFilter=true;
	szLogFile[0]=0;
	szIniFile[0]=0;
	bUseSound=true;
	nSampleRate=44100;
	nFXVolume=100;
	nMusVolume=100;
	nStreamVolume=100;
	nFixedDelta=0;
	bHideMouse=true;
	bDontSuspend=false;
	hwndParent=0;

	nPowerStatus=HGEPWR_UNSUPPORTED;
	hKrnl32 = NULL;
	lpfnGetSystemPowerStatus = NULL;

#ifdef DEMO
	bDMO=true;
#endif


	GetModuleFileName(GetModuleHandle(NULL), szAppPath, sizeof(szAppPath));
	int i;
	for(i=strlen(szAppPath)-1; i>0; i--) if(szAppPath[i]=='\\') break;
	szAppPath[i+1]=0;
}

CParticleSystem::~CParticleSystem(void)
{
}

void CParticleSystem::_PostError(char *error)
{
	System_Log(error);
	strcpy(szError,error);
}

void CParticleSystem::_FocusChange(bool bAct)
{
	bActive=bAct;

	if(bActive)
	{
		if(procFocusGainFunc) procFocusGainFunc();
	}
	else
	{
		if(procFocusLostFunc) procFocusLostFunc();
	}
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	bool bActivating;

	switch(msg)
	{	
		case WM_CREATE: 
			return FALSE;
		
		case WM_PAINT:
			if(pHGE->pD3D && pHGE->procRenderFunc && pHGE->bWindowed) pHGE->procRenderFunc();
			break;

		case WM_DESTROY:
			PostQuitMessage(0);
			return FALSE;

/*
		case WM_ACTIVATEAPP:
			bActivating = (wparam == TRUE);
			if(pHGE->pD3D && pHGE->bActive != bActivating) pHGE->_FocusChange(bActivating);
			return FALSE;
*/
		case WM_ACTIVATE:
			// tricky: we should catch WA_ACTIVE and WA_CLICKACTIVE,
			// but only if HIWORD(wParam) (fMinimized) == FALSE (0)
			bActivating = (LOWORD(wparam) != WA_INACTIVE) && (HIWORD(wparam) == 0);
			if(pHGE->pD3D && pHGE->bActive != bActivating) pHGE->_FocusChange(bActivating);
			return FALSE;


		case WM_SETCURSOR:
			if(pHGE->bActive && LOWORD(lparam)==HTCLIENT && pHGE->bHideMouse) SetCursor(NULL);
			else SetCursor(LoadCursor(NULL, IDC_ARROW));
			return FALSE;

		case WM_SYSKEYDOWN:
			if(wparam == VK_F4)
			{
				if(pHGE->procExitFunc && !pHGE->procExitFunc()) return FALSE;
				return DefWindowProc(hwnd, msg, wparam, lparam);
			}
			else if(wparam == VK_RETURN)
			{
				pHGE->System_SetState(HGE_WINDOWED, !pHGE->System_GetState(HGE_WINDOWED));
				return FALSE;
			}
			else
			{
				pHGE->_BuildEvent(INPUT_KEYDOWN, wparam, HIWORD(lparam) & 0xFF, (lparam & 0x40000000) ? HGEINP_REPEAT:0, -1, -1);
				return FALSE;
			}

		case WM_KEYDOWN:
			pHGE->_BuildEvent(INPUT_KEYDOWN, wparam, HIWORD(lparam) & 0xFF, (lparam & 0x40000000) ? HGEINP_REPEAT:0, -1, -1);
			return FALSE;
		case WM_SYSKEYUP:
			pHGE->_BuildEvent(INPUT_KEYUP, wparam, HIWORD(lparam) & 0xFF, 0, -1, -1);
			return FALSE;
		case WM_KEYUP:
			pHGE->_BuildEvent(INPUT_KEYUP, wparam, HIWORD(lparam) & 0xFF, 0, -1, -1);
			return FALSE;

		case WM_LBUTTONDOWN:
			SetFocus(hwnd);
			pHGE->_BuildEvent(INPUT_MBUTTONDOWN, HGEK_LBUTTON, 0, 0, LOWORDINT(lparam), HIWORDINT(lparam));
			return FALSE;
		case WM_MBUTTONDOWN:
			SetFocus(hwnd);
			pHGE->_BuildEvent(INPUT_MBUTTONDOWN, HGEK_MBUTTON, 0, 0, LOWORDINT(lparam), HIWORDINT(lparam));
			return FALSE;
		case WM_RBUTTONDOWN:
			SetFocus(hwnd);
			pHGE->_BuildEvent(INPUT_MBUTTONDOWN, HGEK_RBUTTON, 0, 0, LOWORDINT(lparam), HIWORDINT(lparam));
			return FALSE;

		case WM_LBUTTONDBLCLK:
			pHGE->_BuildEvent(INPUT_MBUTTONDOWN, HGEK_LBUTTON, 0, HGEINP_REPEAT, LOWORDINT(lparam), HIWORDINT(lparam));
			return FALSE;
		case WM_MBUTTONDBLCLK:
			pHGE->_BuildEvent(INPUT_MBUTTONDOWN, HGEK_MBUTTON, 0, HGEINP_REPEAT, LOWORDINT(lparam), HIWORDINT(lparam));
			return FALSE;
		case WM_RBUTTONDBLCLK:
			pHGE->_BuildEvent(INPUT_MBUTTONDOWN, HGEK_RBUTTON, 0, HGEINP_REPEAT, LOWORDINT(lparam), HIWORDINT(lparam));
			return FALSE;

		case WM_LBUTTONUP:
			pHGE->_BuildEvent(INPUT_MBUTTONUP, HGEK_LBUTTON, 0, 0, LOWORDINT(lparam), HIWORDINT(lparam));
			return FALSE;
		case WM_MBUTTONUP:
			pHGE->_BuildEvent(INPUT_MBUTTONUP, HGEK_MBUTTON, 0, 0, LOWORDINT(lparam), HIWORDINT(lparam));
			return FALSE;
		case WM_RBUTTONUP:
			pHGE->_BuildEvent(INPUT_MBUTTONUP, HGEK_RBUTTON, 0, 0, LOWORDINT(lparam), HIWORDINT(lparam));
			return FALSE;

		case WM_MOUSEMOVE:
			pHGE->_BuildEvent(INPUT_MOUSEMOVE, 0, 0, 0, LOWORDINT(lparam), HIWORDINT(lparam));
			return FALSE;
		case 0x020A: // WM_MOUSEWHEEL, GET_WHEEL_DELTA_WPARAM(wparam);
			pHGE->_BuildEvent(INPUT_MOUSEWHEEL, short(HIWORD(wparam))/120, 0, 0, LOWORDINT(lparam), HIWORDINT(lparam));
			return FALSE;

		case WM_SIZE:
			if(pHGE->pD3D && wparam==SIZE_RESTORED) pHGE->_Resize(LOWORD(lparam), HIWORD(lparam));
			//return FALSE;
			break;

		case WM_SYSCOMMAND:
			if(wparam==SC_CLOSE)
			{
				if(pHGE->procExitFunc && !pHGE->procExitFunc()) return FALSE;
				pHGE->bActive=false;
				return DefWindowProc(hwnd, msg, wparam, lparam);
			}
			break;
	}

	return DefWindowProc(hwnd, msg, wparam, lparam);
}

//////////////////////////////////////////////////////////////////////////2
void CALL CParticleSystem::Gfx_Clear(DWORD color)
{
	if(pCurTarget)
	{
		if(pCurTarget->pDepth)
			pD3DDevice->Clear( 0L, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, color, 1.0f, 0L );
		else
			pD3DDevice->Clear( 0L, NULL, D3DCLEAR_TARGET, color, 1.0f, 0L );
	}
	else
	{
		if(bZBuffer)
			pD3DDevice->Clear( 0L, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, color, 1.0f, 0L );
		else
			pD3DDevice->Clear( 0L, NULL, D3DCLEAR_TARGET, color, 1.0f, 0L );
	}
}

void CALL CParticleSystem::Gfx_SetClipping(int x, int y, int w, int h)
{
	//D3DVIEWPORT8 vp;
	D3DVIEWPORT9 vp;
	int scr_width, scr_height;

	if(!pCurTarget) {
		scr_width=pHGE->System_GetStateInt(HGE_SCREENWIDTH);
		scr_height=pHGE->System_GetStateInt(HGE_SCREENHEIGHT);
	}
	else {
		scr_width=Texture_GetWidth((HTEXTURE)pCurTarget->pTex);
		scr_height=Texture_GetHeight((HTEXTURE)pCurTarget->pTex);
	}

	if(!w) {
		vp.X=0;
		vp.Y=0;
		vp.Width=scr_width;
		vp.Height=scr_height;
	}
	else
	{
		if(x<0) { w+=x; x=0; }
		if(y<0) { h+=y; y=0; }

		if(x+w > scr_width) w=scr_width-x;
		if(y+h > scr_height) h=scr_height-y;

		vp.X=x;
		vp.Y=y;
		vp.Width=w;
		vp.Height=h;
	}

	vp.MinZ=0.0f;
	vp.MaxZ=1.0f;

	_render_batch();
	pD3DDevice->SetViewport(&vp);

	D3DXMATRIX tmp;
	D3DXMatrixScaling(&matProj, 1.0f, -1.0f, 1.0f);
	D3DXMatrixTranslation(&tmp, -0.5f, +0.5f, 0.0f);
	D3DXMatrixMultiply(&matProj, &matProj, &tmp);
	D3DXMatrixOrthoOffCenterLH(&tmp, (float)vp.X, (float)(vp.X+vp.Width), -((float)(vp.Y+vp.Height)), -((float)vp.Y), vp.MinZ, vp.MaxZ);
	D3DXMatrixMultiply(&matProj, &matProj, &tmp);
	pD3DDevice->SetTransform(D3DTS_PROJECTION, &matProj);
}

void CALL CParticleSystem::Gfx_SetTransform(float x, float y, float dx, float dy, float rot, float hscale, float vscale)
{
	D3DXMATRIX tmp;

	if(vscale==0.0f) D3DXMatrixIdentity(&matView);
	else
	{
		D3DXMatrixTranslation(&matView, -x, -y, 0.0f);
		D3DXMatrixScaling(&tmp, hscale, vscale, 1.0f);
		D3DXMatrixMultiply(&matView, &matView, &tmp);
		D3DXMatrixRotationZ(&tmp, -rot);
		D3DXMatrixMultiply(&matView, &matView, &tmp);
		D3DXMatrixTranslation(&tmp, x+dx, y+dy, 0.0f);
		D3DXMatrixMultiply(&matView, &matView, &tmp);
	}

	_render_batch();
	pD3DDevice->SetTransform(D3DTS_VIEW, &matView);
}

bool CALL CParticleSystem::Gfx_BeginScene(HTARGET targ)
{
	LPDIRECT3DSURFACE9 pSurf=0, pDepth=0;
	D3DDISPLAYMODE Mode;
	CRenderTargetList *target=(CRenderTargetList *)targ;

	HRESULT hr = pD3DDevice->TestCooperativeLevel();
	if (hr == D3DERR_DEVICELOST) return false;
	else if (hr == D3DERR_DEVICENOTRESET)
	{
		if(bWindowed)
		{
			if(FAILED(pD3D->GetAdapterDisplayMode(D3DADAPTER_DEFAULT, &Mode)) || Mode.Format==D3DFMT_UNKNOWN) 
			{
				_PostError("Can't determine desktop video mode");
				return false;
			}

			d3dppW.BackBufferFormat = Mode.Format;
			if(_format_id(Mode.Format) < 4) nScreenBPP=16;
			else nScreenBPP=32;
		}

		if(!_GfxRestore()) return false; 
	}

	if(VertArray)
	{
		_PostError("Gfx_BeginScene: Scene is already being rendered");
		return false;
	}

	if(target != pCurTarget)
	{
		if(target)
		{
			target->pTex->GetSurfaceLevel(0, &pSurf);
			pDepth=target->pDepth;
		}
		else
		{
			pSurf=pScreenSurf;
			pDepth=pScreenDepth;
		}
		//if(FAILED(pD3DDevice->SetRenderTarget(pSurf, pDepth)))
		if (FAILED(pD3DDevice->SetRenderTarget(0,pSurf))
			||FAILED(pD3DDevice->SetDepthStencilSurface(pDepth)))
		{
			if(target) pSurf->Release();
			_PostError("Gfx_BeginScene: Can't set render target");
			return false;
		}
		if(target)
		{
			pSurf->Release();
			if(target->pDepth) pD3DDevice->SetRenderState( D3DRS_ZENABLE, D3DZB_TRUE ); 
			else pD3DDevice->SetRenderState( D3DRS_ZENABLE, D3DZB_FALSE ); 
			_SetProjectionMatrix(target->width, target->height);
		}
		else
		{
			if(bZBuffer) pD3DDevice->SetRenderState( D3DRS_ZENABLE, D3DZB_TRUE ); 
			else pD3DDevice->SetRenderState( D3DRS_ZENABLE, D3DZB_FALSE );
			_SetProjectionMatrix(nScreenWidth, nScreenHeight);
		}

		pD3DDevice->SetTransform(D3DTS_PROJECTION, &matProj);
		D3DXMatrixIdentity(&matView);
		pD3DDevice->SetTransform(D3DTS_VIEW, &matView);

		pCurTarget=target;
	}

	pD3DDevice->BeginScene();
	pVB->Lock( 0, 0, (void**)&VertArray, 0 );

	return true;
}

void CALL CParticleSystem::Gfx_EndScene()
{
	_render_batch(true);
	pD3DDevice->EndScene();
	if(!pCurTarget) pD3DDevice->Present( NULL, NULL, NULL, NULL );
}

void CALL CParticleSystem::Gfx_RenderLine(float x1, float y1, float x2, float y2, DWORD color, float z)
{
	if(VertArray)
	{
		if(CurPrimType!=HGEPRIM_LINES || nPrim>=VERTEX_BUFFER_SIZE/HGEPRIM_LINES || CurTexture || CurBlendMode!=BLEND_DEFAULT)
		{
			_render_batch();

			CurPrimType=HGEPRIM_LINES;
			if(CurBlendMode != BLEND_DEFAULT) _SetBlendMode(BLEND_DEFAULT);
			if(CurTexture) { pD3DDevice->SetTexture(0, 0); CurTexture=0; }
		}

		int i=nPrim*HGEPRIM_LINES;
		VertArray[i].point.x = x1; VertArray[i+1].point.x = x2;
		VertArray[i].point.y = y1; VertArray[i+1].point.y = y2;
		VertArray[i].point.z     = VertArray[i+1].point.z = z;
		VertArray[i].colour   = VertArray[i+1].colour = color;
		VertArray[i].u    = VertArray[i+1].u =
		VertArray[i].v    = VertArray[i+1].v = 0.0f;

		nPrim++;
	}
}

void CALL CParticleSystem::Gfx_RenderTriple(const hgeTriple *triple)
{
	if(VertArray)
	{
		if(CurPrimType!=HGEPRIM_TRIPLES || nPrim>=VERTEX_BUFFER_SIZE/HGEPRIM_TRIPLES || CurTexture!=triple->tex || CurBlendMode!=triple->blend)
		{
			_render_batch();

			CurPrimType=HGEPRIM_TRIPLES;
			if(CurBlendMode != triple->blend) _SetBlendMode(triple->blend);
			if(triple->tex != CurTexture) {
				pD3DDevice->SetTexture( 0, (LPDIRECT3DTEXTURE9)(triple->tex) );
				CurTexture = triple->tex;
			}
		}

		memcpy(&VertArray[nPrim*HGEPRIM_TRIPLES], triple->v, sizeof(CUSTOMVERTEX)*HGEPRIM_TRIPLES);
		nPrim++;
	}
}

void CALL CParticleSystem::Gfx_RenderQuad(const hgeQuad *quad)
{
	if(VertArray)
	{
		if(CurPrimType!=HGEPRIM_QUADS || nPrim>=VERTEX_BUFFER_SIZE/HGEPRIM_QUADS || CurTexture!=quad->tex || CurBlendMode!=quad->blend)
		{
			_render_batch();

			CurPrimType=HGEPRIM_QUADS;
			if(CurBlendMode != quad->blend) _SetBlendMode(quad->blend);
			if(quad->tex != CurTexture)
			{
				pD3DDevice->SetTexture( 0, (LPDIRECT3DTEXTURE9)(quad->tex) );
				CurTexture = quad->tex;
			}
		}

		memcpy(&VertArray[nPrim*HGEPRIM_QUADS], quad->v, sizeof(CUSTOMVERTEX)*HGEPRIM_QUADS);
		nPrim++;
	}
}

CUSTOMVERTEX* CALL CParticleSystem::Gfx_StartBatch(int prim_type, HTEXTURE tex, int blend, int *max_prim)
{
	if(VertArray)
	{
		_render_batch();

		CurPrimType=prim_type;
		if(CurBlendMode != blend) _SetBlendMode(blend);
		if(tex != CurTexture)
		{
			pD3DDevice->SetTexture( 0, (LPDIRECT3DTEXTURE9)tex );
			CurTexture = tex;
		}

		*max_prim=VERTEX_BUFFER_SIZE / prim_type;
		return VertArray;
	}
	else return 0;
}

void CALL CParticleSystem::Gfx_FinishBatch(int nprim)
{
	nPrim=nprim;
}

HTARGET CALL CParticleSystem::Target_Create(int width, int height, bool zbuffer)
{
	CRenderTargetList *pTarget;
	D3DSURFACE_DESC TDesc;

	pTarget = new CRenderTargetList;
	pTarget->pTex=0;
	pTarget->pDepth=0;

	if(FAILED(D3DXCreateTexture(pD3DDevice, width, height, 1, D3DUSAGE_RENDERTARGET,
		d3dpp->BackBufferFormat, D3DPOOL_DEFAULT, &pTarget->pTex)))
	{
		_PostError("Can't create render target texture");
		delete pTarget;
		return 0;
	}

	pTarget->pTex->GetLevelDesc(0, &TDesc);
	pTarget->width=TDesc.Width;
	pTarget->height=TDesc.Height;

	if(zbuffer)
	{
		if(FAILED(pD3DDevice->CreateDepthStencilSurface(pTarget->width, pTarget->height,
			D3DFMT_D16, D3DMULTISAMPLE_NONE,0, TRUE ,&pTarget->pDepth,NULL)))//0 glp
		{   
			pTarget->pTex->Release();
			_PostError("Can't create render target depth buffer");
			delete pTarget;
			return 0;
		}
	}

	pTarget->next=pTargets;
	pTargets=pTarget;

	return (HTARGET)pTarget;
}

void CALL CParticleSystem::Target_Free(HTARGET target)
{
	CRenderTargetList *pTarget=pTargets, *pPrevTarget=NULL;

	while(pTarget)
	{
		if((CRenderTargetList *)target == pTarget)
		{
			if(pPrevTarget)
				pPrevTarget->next = pTarget->next;
			else
				pTargets = pTarget->next;

			if(pTarget->pTex) pTarget->pTex->Release();
			if(pTarget->pDepth) pTarget->pDepth->Release();

			delete pTarget;
			return;
		}

		pPrevTarget = pTarget;
		pTarget = pTarget->next;
	}
}

HTEXTURE CALL CParticleSystem::Target_GetTexture(HTARGET target)
{
	CRenderTargetList *targ=(CRenderTargetList *)target;
	if(target) return (HTEXTURE)targ->pTex;
	else return 0;
}

HTEXTURE CALL CParticleSystem::Texture_Create(int width, int height)
{
	LPDIRECT3DTEXTURE9 pTex;

	if( FAILED( D3DXCreateTexture( pD3DDevice, width, height,
		1,					// Mip levels
		0,					// Usage
		D3DFMT_A8R8G8B8,	// Format
		D3DPOOL_MANAGED,	// Memory pool
		&pTex ) ) )
	{	
		_PostError("Can't create texture");
		return NULL;
	}

	return (HTEXTURE)pTex;
}

HTEXTURE CALL CParticleSystem::Texture_Load(const char *filename, DWORD size, bool bMipmap)
{
	void *data;
	DWORD _size;
	D3DFORMAT fmt1, fmt2;
	//LPDIRECT3DTEXTURE8 pTex;
	LPDIRECT3DTEXTURE9 pTex;
	D3DXIMAGE_INFO info;
	CTextureList *texItem;

	if(size) { data=(void *)filename; _size=size; }
	else
	{
		data=pHGE->Resource_Load(filename, &_size);
		if(!data) return NULL;
	}

	if(*(DWORD*)data == 0x20534444) // Compressed DDS format magic number
	{
		fmt1=D3DFMT_UNKNOWN;
		fmt2=D3DFMT_A8R8G8B8;
	}
	else
	{
		fmt1=D3DFMT_A8R8G8B8;
		fmt2=D3DFMT_UNKNOWN;
	}

	//	if( FAILED( D3DXCreateTextureFromFileInMemory( pD3DDevice, data, _size, &pTex ) ) ) pTex=NULL;
	if( FAILED( D3DXCreateTextureFromFileInMemoryEx( pD3DDevice, data, _size,
		D3DX_DEFAULT, D3DX_DEFAULT,
		bMipmap ? 0:1,		// Mip levels
		0,					// Usage
		fmt1,				// Format
		D3DPOOL_MANAGED,	// Memory pool
		D3DX_FILTER_NONE,	// Filter
		D3DX_DEFAULT,		// Mip filter
		0,					// Color key
		&info, NULL,
		&pTex ) ) )
	{
		if( FAILED( D3DXCreateTextureFromFileInMemoryEx( pD3DDevice, data, _size,
			D3DX_DEFAULT, D3DX_DEFAULT,
			bMipmap ? 0:1,		// Mip levels
			0,					// Usage
			fmt2,				// Format
			D3DPOOL_MANAGED,	// Memory pool
			D3DX_FILTER_NONE,	// Filter
			D3DX_DEFAULT,		// Mip filter
			0,					// Color key
			&info, NULL,
			&pTex ) ) )

		{	
			_PostError("Can't create texture");
			if(!size) Resource_Free(data);
			return NULL;
		}
	}

	if(!size) Resource_Free(data);

	texItem=new CTextureList;
	texItem->tex=(HTEXTURE)pTex;
	texItem->width=info.Width;
	texItem->height=info.Height;
	texItem->next=textures;
	textures=texItem;

	return (HTEXTURE)pTex;
}

void CALL CParticleSystem::Texture_Free(HTEXTURE tex)
{
	LPDIRECT3DTEXTURE9 pTex=reinterpret_cast<LPDIRECT3DTEXTURE9>(tex);
	CTextureList *texItem=textures, *texPrev=0;

	while(texItem)
	{
		if(texItem->tex==tex)//找到使用该纹理的item
		{
			if(texPrev) texPrev->next=texItem->next;//指向下一个
			else textures=texItem->next;//直接指向下一个
			delete texItem;
			break;
		}
		texPrev=texItem;
		texItem=texItem->next;
	}
	if(pTex != NULL) pTex->Release();
}

int CALL CParticleSystem::Texture_GetWidth(HTEXTURE tex, bool bOriginal)
{
	D3DSURFACE_DESC TDesc;
	LPDIRECT3DTEXTURE9 pTex=(LPDIRECT3DTEXTURE9)tex;
	CTextureList *texItem=textures;

	if(bOriginal)
	{
		while(texItem)
		{
			if(texItem->tex==tex) return texItem->width;
			texItem=texItem->next;
		}
		return 0;
	}
	else
	{
		if(FAILED(pTex->GetLevelDesc(0, &TDesc))) return 0;
		else return TDesc.Width;
	}
}


int CALL CParticleSystem::Texture_GetHeight(HTEXTURE tex, bool bOriginal)
{
	D3DSURFACE_DESC TDesc;
	LPDIRECT3DTEXTURE9 pTex=(LPDIRECT3DTEXTURE9)tex;
	CTextureList *texItem=textures;

	if(bOriginal)
	{
		while(texItem)
		{
			if(texItem->tex==tex) return texItem->height;
			texItem=texItem->next;
		}
		return 0;
	}
	else
	{
		if(FAILED(pTex->GetLevelDesc(0, &TDesc))) return 0;
		else return TDesc.Height;
	}
}


DWORD * CALL CParticleSystem::Texture_Lock(HTEXTURE tex, bool bReadOnly, int left, int top, int width, int height)
{
	LPDIRECT3DTEXTURE9 pTex=(LPDIRECT3DTEXTURE9)tex;
	D3DSURFACE_DESC TDesc;
	D3DLOCKED_RECT TRect;
	RECT region, *prec;
	int flags;

	pTex->GetLevelDesc(0, &TDesc);
	if(TDesc.Format!=D3DFMT_A8R8G8B8 && TDesc.Format!=D3DFMT_X8R8G8B8) return 0;

	if(width && height)
	{
		region.left=left;
		region.top=top;
		region.right=left+width;
		region.bottom=top+height;
		prec=&region;
	}
	else prec=0;

	if(bReadOnly) flags=D3DLOCK_READONLY;
	else flags=0;

	if(FAILED(pTex->LockRect(0, &TRect, prec, flags)))
	{
		_PostError("Can't lock texture");
		return 0;
	}

	return (DWORD *)TRect.pBits;
}


void CALL CParticleSystem::Texture_Unlock(HTEXTURE tex)
{
	LPDIRECT3DTEXTURE9 pTex=(LPDIRECT3DTEXTURE9)tex;
	pTex->UnlockRect(0);
}


//////// Implementation ////////

void CParticleSystem::_render_batch(bool bEndScene)
{
	if(VertArray)
	{
		pVB->Unlock();

		if(nPrim)
		{
			switch(CurPrimType)
			{
			case HGEPRIM_QUADS:
				pD3DDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, nPrim<<2, 0, nPrim<<1);
				break;

			case HGEPRIM_TRIPLES:
				pD3DDevice->DrawPrimitive(D3DPT_TRIANGLELIST, 0, nPrim);
				break;

			case HGEPRIM_LINES:
				pD3DDevice->DrawPrimitive(D3DPT_LINELIST, 0, nPrim);
				break;
			}

			nPrim=0;
		}

		if(bEndScene) VertArray = 0;
		else pVB->Lock( 0, 0, (void**)&VertArray, 0 );
	}
}

void CParticleSystem::_SetBlendMode(int blend)
{
	if((blend & BLEND_ALPHABLEND) != (CurBlendMode & BLEND_ALPHABLEND))
	{
		if(blend & BLEND_ALPHABLEND) pD3DDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
		else pD3DDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);
	}

	if((blend & BLEND_ZWRITE) != (CurBlendMode & BLEND_ZWRITE))
	{
		if(blend & BLEND_ZWRITE) pD3DDevice->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
		else pD3DDevice->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
	}			

	if((blend & BLEND_COLORADD) != (CurBlendMode & BLEND_COLORADD))
	{
		if(blend & BLEND_COLORADD) pD3DDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_ADD);
		else pD3DDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
	}

	CurBlendMode = blend;
}

void CParticleSystem::_SetProjectionMatrix(int width, int height)
{
	D3DXMATRIX tmp;
	D3DXMatrixScaling(&matProj, 1.0f, -1.0f, 1.0f);
	D3DXMatrixTranslation(&tmp, -0.5f, height+0.5f, 0.0f);
	D3DXMatrixMultiply(&matProj, &matProj, &tmp);
	D3DXMatrixOrthoOffCenterLH(&tmp, 0, (float)width, 0, (float)height, 0.0f, 1.0f);
	D3DXMatrixMultiply(&matProj, &matProj, &tmp);
}

bool CParticleSystem::_GfxInit()
{
	static const char *szFormats[]={"UNKNOWN", "R5G6B5", "X1R5G5B5", "A1R5G5B5", "X8R8G8B8", "A8R8G8B8"};
	D3DADAPTER_IDENTIFIER9 AdID;
	D3DDISPLAYMODE Mode;
	D3DFORMAT Format=D3DFMT_UNKNOWN;
	UINT nModes, i;

	// Init D3D

	pD3D=Direct3DCreate9(D3D_SDK_VERSION); // D3D_SDK_VERSION
	if(pD3D==NULL)
	{
		_PostError("Can't create D3D interface");
		return false;
	}

	// Get adapter info
	//D3DENUM_WHQL_LEVEL
	pD3D->GetAdapterIdentifier(D3DADAPTER_DEFAULT, 0 , &AdID);//D3DENUM_NO_WHQL_LEVEL
	System_Log("D3D Driver: %s",AdID.Driver);
	System_Log("Description: %s",AdID.Description);
	System_Log("Version: %d.%d.%d.%d",
		HIWORD(AdID.DriverVersion.HighPart),
		LOWORD(AdID.DriverVersion.HighPart),
		HIWORD(AdID.DriverVersion.LowPart),
		LOWORD(AdID.DriverVersion.LowPart));

	// Set up Windowed presentation parameters

	if(FAILED(pD3D->GetAdapterDisplayMode(D3DADAPTER_DEFAULT, &Mode)) || Mode.Format==D3DFMT_UNKNOWN) 
	{
		_PostError("Can't determine desktop video mode");
		if(bWindowed) return false;
	}
	//窗口参数
	ZeroMemory(&d3dppW, sizeof(d3dppW));

	d3dppW.BackBufferWidth  = nScreenWidth;
	d3dppW.BackBufferHeight = nScreenHeight;
	d3dppW.BackBufferFormat = Mode.Format;
	d3dppW.BackBufferCount  = 1;
	d3dppW.MultiSampleType  = D3DMULTISAMPLE_NONE;
	d3dppW.hDeviceWindow    = hwnd;
	d3dppW.Windowed         = TRUE;
	//垂直同步
	if(nHGEFPS==HGEFPS_VSYNC)
	{
		d3dppW.PresentationInterval=D3DPRESENT_INTERVAL_ONE;
	}
	//else					  
	d3dppW.SwapEffect = D3DSWAPEFFECT_COPY;

	if(bZBuffer)
	{
		d3dppW.EnableAutoDepthStencil = TRUE;
		d3dppW.AutoDepthStencilFormat = D3DFMT_D16;
	}

	// Set up Full Screen presentation parameters

	nModes=pD3D->GetAdapterModeCount(D3DADAPTER_DEFAULT,Mode.Format);

	for(i=0; i<nModes; i++)
	{
		pD3D->EnumAdapterModes(D3DADAPTER_DEFAULT,Mode.Format, i, &Mode);
		if(Mode.Width != (UINT)nScreenWidth || Mode.Height != (UINT)nScreenHeight) continue;
		if(nScreenBPP==16 && (_format_id(Mode.Format) > _format_id(D3DFMT_A1R5G5B5))) continue;
		if(_format_id(Mode.Format) > _format_id(Format)) Format=Mode.Format;
	}

	if(Format == D3DFMT_UNKNOWN)
	{
		_PostError("Can't find appropriate full screen video mode");
		if(!bWindowed) return false;
	}
	//全屏参数
	ZeroMemory(&d3dppFS, sizeof(d3dppFS));

	d3dppFS.BackBufferWidth  = nScreenWidth;
	d3dppFS.BackBufferHeight = nScreenHeight;
	d3dppFS.BackBufferFormat = Format;
	d3dppFS.BackBufferCount  = 1;
	d3dppFS.MultiSampleType  = D3DMULTISAMPLE_NONE;
	d3dppFS.hDeviceWindow    = hwnd;
	d3dppFS.Windowed         = FALSE;

	d3dppFS.SwapEffect       = D3DSWAPEFFECT_FLIP;
	d3dppFS.FullScreen_RefreshRateInHz = D3DPRESENT_RATE_DEFAULT;
	//垂直同步
	if(nHGEFPS==HGEFPS_VSYNC) d3dppFS.PresentationInterval = D3DPRESENT_INTERVAL_ONE;
	else					  d3dppFS.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;

	if(bZBuffer)
	{
		d3dppFS.EnableAutoDepthStencil = TRUE;
		d3dppFS.AutoDepthStencilFormat = D3DFMT_D16;
	}

	d3dpp = bWindowed ? &d3dppW : &d3dppFS;

	if(_format_id(d3dpp->BackBufferFormat) < 4) nScreenBPP=16;
	else nScreenBPP=32;

	// Create D3D Device

	if( FAILED( pD3D->CreateDevice( D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hwnd,
		D3DCREATE_MIXED_VERTEXPROCESSING|D3DCREATE_MULTITHREADED,//D3DCREATE_SOFTWARE_VERTEXPROCESSING,
		d3dpp, &pD3DDevice ) ) )
	{
		_PostError("Can't create D3D device");
		return false;
	}

	_AdjustWindow();

	System_Log("Mode: %d x %d x %s\n",nScreenWidth,nScreenHeight,szFormats[_format_id(Format)]);

	// Create vertex batch buffer

	VertArray=0;
	textures=0;

	// Init all stuff that can be lost

	_SetProjectionMatrix(nScreenWidth, nScreenHeight);
	D3DXMatrixIdentity(&matView);

	if(!_init_lost()) return false;

	Gfx_Clear(0);

	return true;
}

int CParticleSystem::_format_id(D3DFORMAT fmt)
{
	switch(fmt) {
		case D3DFMT_R5G6B5:		return 1;
		case D3DFMT_X1R5G5B5:	return 2;
		case D3DFMT_A1R5G5B5:	return 3;
		case D3DFMT_X8R8G8B8:	return 4;
		case D3DFMT_A8R8G8B8:	return 5;
		default:				return 0;
	}
}

void CParticleSystem::_AdjustWindow()
{
	RECT *rc;
	LONG style;

	if(bWindowed) {rc=&rectW; style=styleW; }
	else  {rc=&rectFS; style=styleFS; }
	SetWindowLong(hwnd, GWL_STYLE, style);

	style=GetWindowLong(hwnd, GWL_EXSTYLE);
	if(bWindowed)
	{
		SetWindowLong(hwnd, GWL_EXSTYLE, style & (~WS_EX_TOPMOST));
		SetWindowPos(hwnd, HWND_NOTOPMOST, rc->left, rc->top, rc->right-rc->left, rc->bottom-rc->top, SWP_FRAMECHANGED);
	}
	else
	{
		SetWindowLong(hwnd, GWL_EXSTYLE, style | WS_EX_TOPMOST);
		SetWindowPos(hwnd, HWND_TOPMOST, rc->left, rc->top, rc->right-rc->left, rc->bottom-rc->top, SWP_FRAMECHANGED);
	}
}

void CParticleSystem::_Resize(int width, int height)
{
	if(hwndParent)
	{
		//if(procFocusLostFunc) procFocusLostFunc();

		d3dppW.BackBufferWidth=width;
		d3dppW.BackBufferHeight=height;
		nScreenWidth=width;
		nScreenHeight=height;

		_SetProjectionMatrix(nScreenWidth, nScreenHeight);
		_GfxRestore();

		//if(procFocusGainFunc) procFocusGainFunc();
	}
}

void CParticleSystem::_GfxDone()
{
	CRenderTargetList *target=pTargets, *next_target;

	while(textures)	Texture_Free(textures->tex);

	if(pScreenSurf) { pScreenSurf->Release(); pScreenSurf=0; }
	if(pScreenDepth) { pScreenDepth->Release(); pScreenDepth=0; }

	while(target)
	{
		if(target->pTex) target->pTex->Release();
		if(target->pDepth) target->pDepth->Release();
		next_target=target->next;
		delete target;
		target=next_target;
	}
	pTargets=0;

	if(pIB)
	{
		pD3DDevice->SetIndices(NULL);
		pIB->Release();
		pIB=0;
	}
	if(pVB)
	{
		if(VertArray) {	pVB->Unlock(); VertArray=0;	}
		pD3DDevice->SetStreamSource( 0,NULL,0, sizeof(CUSTOMVERTEX) );
		pVB->Release();
		pVB=0;
	}
	if(pD3DDevice) { pD3DDevice->Release(); pD3DDevice=0; }
	if(pD3D) { pD3D->Release(); pD3D=0; }
}


bool CParticleSystem::_GfxRestore()
{
	CRenderTargetList *target=pTargets;

	//if(!pD3DDevice) return false;
	//if(pD3DDevice->TestCooperativeLevel() == D3DERR_DEVICELOST) return;

	if(pScreenSurf) pScreenSurf->Release();
	if(pScreenDepth) pScreenDepth->Release();

	while(target)
	{
		if(target->pTex) target->pTex->Release();
		if(target->pDepth) target->pDepth->Release();
		target=target->next;
	}

	if(pIB)
	{
		pD3DDevice->SetIndices(NULL);
		pIB->Release();
	}
	if(pVB)
	{
		pD3DDevice->SetStreamSource( 0, NULL,0, sizeof(CUSTOMVERTEX) );
		pVB->Release();
	}

	pD3DDevice->Reset(d3dpp);

	if(!_init_lost()) return false;

	if(procGfxRestoreFunc) return procGfxRestoreFunc();

	return true;
}

bool CParticleSystem::_init_lost()
{
	CRenderTargetList *target=pTargets;

	// Store render target

	pScreenSurf=0;
	pScreenDepth=0;

	pD3DDevice->GetRenderTarget(0,&pScreenSurf);
	pD3DDevice->GetDepthStencilSurface(&pScreenDepth);

	while(target)
	{
		if(target->pTex)
			D3DXCreateTexture(pD3DDevice, target->width, target->height, 1, D3DUSAGE_RENDERTARGET,
			d3dpp->BackBufferFormat, D3DPOOL_DEFAULT, &target->pTex);
		if(target->pDepth)
			pD3DDevice->CreateDepthStencilSurface(target->width, target->height,
			D3DFMT_D16, D3DMULTISAMPLE_NONE,0,TRUE, &target->pDepth,NULL);//0 glp
		target=target->next;
	}

	// Create Vertex buffer

	if( FAILED (pD3DDevice->CreateVertexBuffer(VERTEX_BUFFER_SIZE*sizeof(CUSTOMVERTEX),
		D3DUSAGE_WRITEONLY,
		D3DFVF_HGEVERTEX,
		D3DPOOL_MANAGED, &pVB,NULL )))//D3DPOOL_DEFAULT
	{
		_PostError("Can't create D3D vertex buffer");
		return false;
	}

	//pD3DDevice->SetVertexShader( D3DFVF_HGEVERTEX );
	pD3DDevice->SetFVF(D3DFVF_HGEVERTEX);
	pD3DDevice->SetStreamSource( 0, pVB,0, sizeof(CUSTOMVERTEX) );

	// Create and setup Index buffer

	if( FAILED( pD3DDevice->CreateIndexBuffer(VERTEX_BUFFER_SIZE*6/4*sizeof(WORD),//VERTEX_BUFFER_SIZE*3
		D3DUSAGE_WRITEONLY,
		D3DFMT_INDEX16,
		D3DPOOL_MANAGED, &pIB,NULL ) ) )//D3DPOOL_DEFAULT
	{
		_PostError("Can't create D3D index buffer");
		return false;
	}

	pD3DDevice->SetIndices(pIB);//glp 2012-12-07
	WORD *pIndices;//, n=0;
	if( FAILED( pIB->Lock( 0,VERTEX_BUFFER_SIZE*3 , (void**)&pIndices, 0 ) ) )//0
	{
		_PostError("Can't lock D3D index buffer");
		return false;
	}

	for(int i=0; i<VERTEX_BUFFER_SIZE; i+=4) {///4
		*pIndices++=i;
		*pIndices++=i+1;
		*pIndices++=i+2;
		*pIndices++=i+2;
		*pIndices++=i+3;
		*pIndices++=i;
		//n+=4;
	}

	pIB->Unlock();
	//pD3DDevice->SetIndices(pIB,0);

	// Set common render states

	//pD3DDevice->SetRenderState( D3DRS_LASTPIXEL, FALSE );
	pD3DDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_NONE );//如何剔除朝后的三角形
	pD3DDevice->SetRenderState( D3DRS_LIGHTING, FALSE );//是否有灯光，默认TRUE

	pD3DDevice->SetRenderState( D3DRS_ALPHABLENDENABLE,   TRUE );//alpha 融合
	pD3DDevice->SetRenderState( D3DRS_SRCBLEND,  D3DBLEND_SRCALPHA );//原图的 alpha乘积 源alpha
	pD3DDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA );//目的图的alpha乘积 1-源alpha

	pD3DDevice->SetRenderState( D3DRS_ALPHATESTENABLE, TRUE );//启用alpha测试
	pD3DDevice->SetRenderState( D3DRS_ALPHAREF,        0x01 );//设置所有像素的参考alpha值
	pD3DDevice->SetRenderState( D3DRS_ALPHAFUNC, D3DCMP_GREATEREQUAL );//alpha测试函数

	pD3DDevice->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_MODULATE );//纹理颜色混合方法
	pD3DDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );//纹理1的颜色作为上面混合方法的第一个参数，采用纹理颜色
	pD3DDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE );//纹理2的颜色作为上面混合方法的第二个参数，采用扩散颜色

	pD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_MODULATE );//纹理alpha混合方法
	pD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
	pD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE );

	//pD3DDevice->SetTextureStageState(0, D3DTSS_MIPFILTER, D3DTEXF_POINT);
	pD3DDevice->SetSamplerState(0,D3DSAMP_MIPFILTER,D3DTEXF_POINT);//速度最快，效果比较差

	if(bTextureFilter)
	{
		//pD3DDevice->SetTextureStageState(0,D3DTSS_MAGFILTER,D3DTEXF_LINEAR);
		pD3DDevice->SetSamplerState(0,D3DSAMP_MAGFILTER,D3DTEXF_LINEAR);//放大采用的方法,线性柔和
		//pD3DDevice->SetTextureStageState(0,D3DTSS_MINFILTER,D3DTEXF_LINEAR);
		pD3DDevice->SetSamplerState(0,D3DSAMP_MINFILTER,D3DTEXF_LINEAR);//缩小采用的方法
	}
	else
	{
		//pD3DDevice->SetTextureStageState(0,D3DTSS_MAGFILTER,D3DTEXF_POINT);
		pD3DDevice->SetSamplerState(0,D3DSAMP_MAGFILTER,D3DTEXF_POINT);
		//pD3DDevice->SetTextureStageState(0,D3DTSS_MINFILTER,D3DTEXF_POINT);
		pD3DDevice->SetSamplerState(0,D3DSAMP_MINFILTER,D3DTEXF_POINT);
	}

	nPrim=0;
	CurPrimType=HGEPRIM_QUADS;
	CurBlendMode = BLEND_DEFAULT;
	CurTexture = NULL;

	pD3DDevice->SetTransform(D3DTS_VIEW, &matView);
	pD3DDevice->SetTransform(D3DTS_PROJECTION, &matProj);

	return true;
}

////////////////////////////////////////////////////////////3
// void CALL CParticleSystem::Ini_SetInt(const char *section, const char *name, int value)
// {
// 	char buf[256];
// 
// 	if(szIniFile[0]) {
// 		sprintf(buf,"%d",value);
// 		WritePrivateProfileString(section, name, buf, szIniFile);
// 	}
// }
// 
// 
// int CALL CParticleSystem::Ini_GetInt(const char *section, const char *name, int def_val)
// {
// 	char buf[256];
// 
// 	if(szIniFile[0]) {
// 		if(GetPrivateProfileString(section, name, "", buf, sizeof(buf), szIniFile))
// 		{ return atoi(buf); }
// 		else { return def_val; }
// 	}
// 	return def_val;
// }
// 
// 
// void CALL CParticleSystem::Ini_SetFloat(const char *section, const char *name, float value)
// {
// 	char buf[256];
// 
// 	if(szIniFile[0]) {
// 		sprintf(buf,"%f",value);
// 		WritePrivateProfileString(section, name, buf, szIniFile);
// 	}
// }
// 
// 
// float CALL CParticleSystem::Ini_GetFloat(const char *section, const char *name, float def_val)
// {
// 	char buf[256];
// 
// 	if(szIniFile[0]) {
// 		if(GetPrivateProfileString(section, name, "", buf, sizeof(buf), szIniFile))
// 		{ return (float)atof(buf); }
// 		else { return def_val; }
// 	}
// 	return def_val;
// }
// 
// 
// void CALL CParticleSystem::Ini_SetString(const char *section, const char *name, const char *value)
// {
// 	if(szIniFile[0]) WritePrivateProfileString(section, name, value, szIniFile);
// }
// 
// 
// char* CALL CParticleSystem::Ini_GetString(const char *section, const char *name, const char *def_val)
// {
// 	if(szIniFile[0]) GetPrivateProfileString(section, name, def_val, szIniString, sizeof(szIniString), szIniFile);
// 	else strcpy(szIniString, def_val);
// 	return szIniString;
// }

//////////////////////////////////////////////////////////////////////////4


//////////////////////////////////////////////////////////////////////////5

//////////////////////////////////////////////////////////////////////////6
//rand_
//////////////////////////////////////////////////////////////////////////7


//////////////////////////////////////////////////////////////////////////8
float CALL CParticleSystem::Timer_GetTime()
{
	return fTime;
}

float CALL CParticleSystem::Timer_GetDelta()
{
	return fDeltaTime;
}


int CALL CParticleSystem::Timer_GetFPS()
{
	return nFPS;
}
//////////////////////////////////////////////////////////////////////////9

