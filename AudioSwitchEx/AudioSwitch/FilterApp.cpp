
#include "FilterApp.h"

extern "C" BOOL WINAPI DllEntryPoint(HINSTANCE, ULONG, LPVOID);

CFilterApp::CFilterApp()
{
}

BOOL CFilterApp::InitInstance()
{
	if (!__super::InitInstance()) {
		return FALSE;
	}

	SetRegistryKey(_T("Gabest"));

	DllEntryPoint(AfxGetInstanceHandle(), DLL_PROCESS_ATTACH, 0);

	return TRUE;
}

BOOL CFilterApp::ExitInstance()
{
	DllEntryPoint(AfxGetInstanceHandle(), DLL_PROCESS_DETACH, 0);

	return __super::ExitInstance();
}

BEGIN_MESSAGE_MAP(CFilterApp, CWinApp)
END_MESSAGE_MAP()
