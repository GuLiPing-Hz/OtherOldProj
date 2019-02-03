#pragma once

#include "util.h"

class CFilterApp : public CWinApp
{
public:
	CFilterApp();

	BOOL InitInstance();
	BOOL ExitInstance();

	DECLARE_MESSAGE_MAP()
};

