// GlpWmaPlayer.h : PROJECT_NAME Ӧ�ó������ͷ�ļ�
//

#pragma once

#ifndef __AFXWIN_H__
	#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif

#include "resource.h"		// ������


// CGlpWmaPlayerApp:
// �йش����ʵ�֣������ GlpWmaPlayer.cpp
//

class CGlpWmaPlayerApp : public CWinApp
{
public:
	CGlpWmaPlayerApp();

// ��д
	public:
	virtual BOOL InitInstance();

// ʵ��

	DECLARE_MESSAGE_MAP()
};

extern CGlpWmaPlayerApp theApp;