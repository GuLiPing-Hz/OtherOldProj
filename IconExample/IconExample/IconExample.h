// IconExample.h : PROJECT_NAME Ӧ�ó������ͷ�ļ�
//

#pragma once

#ifndef __AFXWIN_H__
	#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif

#include "resource.h"		// ������


// CIconExampleApp:
// �йش����ʵ�֣������ IconExample.cpp
//

class CIconExampleApp : public CWinApp
{
public:
	CIconExampleApp();

// ��д
	public:
	virtual BOOL InitInstance();

// ʵ��

	DECLARE_MESSAGE_MAP()
};

extern CIconExampleApp theApp;