// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

// Modify the following defines if you have to target a platform prior to the ones specified below.
// Refer to MSDN for the latest info on corresponding values for different platforms.
#ifndef WINVER				// Allow use of features specific to Windows XP or later.
#define WINVER 0x0501		// Change this to the appropriate value to target other versions of Windows.
#endif

#ifndef _WIN32_WINNT		// Allow use of features specific to Windows XP or later.                   
#define _WIN32_WINNT 0x0501	// Change this to the appropriate value to target other versions of Windows.
#endif						

#ifndef _WIN32_WINDOWS		// Allow use of features specific to Windows 98 or later.
#define _WIN32_WINDOWS 0x0410 // Change this to the appropriate value to target Windows Me or later.
#endif

#ifndef _WIN32_IE			// Allow use of features specific to IE 6.0 or later.
#define _WIN32_IE 0x0600	// Change this to the appropriate value to target other versions of IE.
#endif

#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>
#include <assert.h>
#include <wchar.h>

#ifndef SAFE_RELEASE
#define SAFE_RELEASE(x)  do{if((x)) \
	(x)->Release(); (x)=NULL; }while(0)
#endif//SAFE_RELEASE

#ifndef SAFE_DELETE
#define SAFE_DELETE(x) \
if ((x) != NULL)      \
{                   \
	delete (x);        \
	(x) = NULL;        \
}
#endif//SAFE_DELETE

#ifndef SAFE_ARRAY_DELETE
#define SAFE_ARRAY_DELETE(x) \
if ((x) != NULL)            \
{                         \
	delete[] (x);            \
	(x) = NULL;              \
}
#endif//SAFE_ARRAY_DELETE

#ifndef SAFE_FREE
#define SAFE_FREE(x) \
if ((x) != NULL)            \
{                         \
	free((x));            \
	(x) = NULL;              \
}
#endif//SAFE_FREE

#ifndef SAFE_CLOSEHANDLE
#define SAFE_CLOSEHANDLE(x) \
if((x)!=NULL)\
{\
	CloseHandle((x));\
	(x) = NULL;\
}
#endif//SAFE_CLOSEHANDLE

#ifndef QI
#define QI(i)  (riid == __uuidof(i)) ? GetInterface((i*)this, ppv) :
#endif//QI
#ifndef QI2
#define QI2(i) (riid == IID_##i) ? GetInterface((i*)this, ppv) 
#endif//QI2

#ifndef BREAK_IF_FAILED
#define BREAK_IF_FAILED(hr)\
if (FAILED(hr))\
{\
	break;\
}
#endif

// TODO: reference additional headers your program requires here
