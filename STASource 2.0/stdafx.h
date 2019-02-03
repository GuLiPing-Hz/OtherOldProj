#pragma once

#pragma warning(disable:4995)
#pragma warning(disable:4312)
#pragma warning(disable:4311)

// #define DEVELOP_DLL
#define DEVELOP_EXPORTS

#include <streams.h>
#include <vector>
#include <map>
#include <wmsdk.h>
#include <string>
 
//#pragma comment(lib, "../../lib/STRMBASED.lib")		
/*
#include "common/win32.h"
#include "osassistant/stringfunc.h"
#include "osassistant/file.h"
#include "osassistant/filefunc.h"
#include "osassistant/wmfunc.h"
#include "memory/Ring.h"
using namespace Burning;
*/
#include "tchar.h"
typedef std::basic_string<TCHAR> tstring;

#define SafeCloseHandle(h)	do								\
{															\
	if ( NULL != h && INVALID_HANDLE_VALUE != h ) {			\
		CloseHandle( (h) );									\
	}														\
} while (0);

#define SafeRelease(p)	do									\
{															\
	if ( NULL != p ) {										\
	(p)->Release();											\
	}														\
} while (0);

#define SafeDelete(p)	do									\
{															\
	if ( NULL != p ) {										\
	delete (p);												\
	(p) = NULL;												\
	}														\
} while (0);

#define SafeDeleteArray(p)	do								\
{															\
	if ( NULL != p ) {										\
	delete [] (p);											\
	(p) = NULL;												\
	}														\
} while (0);


#define SafeComMemFree(p)	do								\
{															\
	if ( NULL != p ) {										\
	CoTaskMemFree(p);										\
	}														\
} while (0);

class Locker
{
	Locker( const Locker & l );
	Locker & operator=(const Locker& l);

	CRITICAL_SECTION m_cs;
public:
	Locker()	{ InitializeCriticalSection(&m_cs); }

	~Locker()	{ DeleteCriticalSection(&m_cs); }

	void lock()	{ EnterCriticalSection(&m_cs);		}

	void unlock()	{ LeaveCriticalSection(&m_cs);	}
};

class Lockerguard
{
	Lockerguard( const Lockerguard & l );
	Lockerguard & operator=(const Lockerguard& l);

	Locker * m_pLocker;
public:
	Lockerguard( Locker * pLocker ) : m_pLocker(pLocker)	{	m_pLocker->lock();		}
	~Lockerguard()					{	m_pLocker->unlock();	}
};

static bool IsNull( LPCTSTR p ) {	return(NULL==p);	}

#define BREAK_IF_ISNOTSOK(hr)	if ( S_OK != hr ) break;

#include <File.h>
#include <ringex.h>
using namespace Burning;

const int g_head = 104;
const int g_last = 16;

#define OUTPUTPLAYERDEBUG 0
#if defined(OUTPUTPLAYERDEBUG)
extern TCHAR g_buffer[MAX_PATH];
#endif

#define SAMPLEDATABLOCKSUM	4