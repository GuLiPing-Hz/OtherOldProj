// AudioDev.h : PROJECT_NAME Ӧ�ó������ͷ�ļ�
//

#pragma once

#ifndef __AFXWIN_H__
	#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif

#include "resource.h"		// ������


// CAudioDevApp:
// �йش����ʵ�֣������ AudioDev.cpp
//

class CAudioDevApp : public CWinApp
{
public:
	CAudioDevApp();

// ��д
	public:
	virtual BOOL InitInstance();

// ʵ��

	DECLARE_MESSAGE_MAP()
};

extern CAudioDevApp theApp;