#include "stdafx.h"
#include "stasource.h"

#if defined(OUTPUTPLAYERDEBUG)
TCHAR g_buffer[MAX_PATH];
#endif

#define REGNAME TEXT("STASource Filter 3.0")

///< ========================================================= ×¢²á =============================

const AMOVIESETUP_MEDIATYPE subPinTypes[] = 
{
	{
		&MEDIATYPE_Audio,            // Major type
		&MEDIASUBTYPE_NULL          // Minor type
	}
};

const AMOVIESETUP_PIN pPin[] = 
{
	{
		L"Output",          // String pin name
		FALSE,              // Is it rendered
		TRUE,               // Is it an output
		FALSE,              // Allowed none
		FALSE,              // Allowed many
		&CLSID_NULL,        // Connects to filter
		L"Input",           // Connects to pin
		1,                  // Number of types
		&subPinTypes[0]       // The pin details
	}
};

const AMOVIESETUP_FILTER subFilter = 
{
	&CLSID_STASOURCE,
	REGNAME,
	MERIT_DO_NOT_USE,
	1,
	pPin
};

CFactoryTemplate g_Templates[]=
{ 
	{
		REGNAME, 
		&CLSID_STASOURCE, 
		CSTASource::CreateInstance, 
		NULL,
		&subFilter
	}
};

int g_cTemplates = sizeof(g_Templates)/sizeof(g_Templates[0]); 

// ====================================================================== µ¼³öº¯Êý ================================================

extern "C" BOOL WINAPI DllEntryPoint(HINSTANCE, ULONG, LPVOID);
BOOL APIENTRY DllMain(HANDLE hModule,DWORD  dwReason,LPVOID lpReserved)
{
	return DllEntryPoint((HINSTANCE)(hModule), dwReason, lpReserved);
}


//HRESULT APIENTRY DllGetClassObject(void)
//{
//	return NOERROR;
//}

STDAPI DllRegisterServer()
{
	return AMovieDllRegisterServer2(TRUE);
}

STDAPI DllUnregisterServer()
{
	return AMovieDllRegisterServer2(FALSE);
}