
// Leakpress.h : PROJECT_NAME Ӧ�ó������ͷ�ļ�
//

#pragma once

#ifndef __AFXWIN_H__
	#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif

#include "resource.h"		// ������


// CLeakpressApp:
// �йش����ʵ�֣������ Leakpress.cpp
//

class CLeakpressApp : public CWinApp
{
public:
	CLeakpressApp();

// ��д
public:
	virtual BOOL InitInstance();

// ʵ��

	DECLARE_MESSAGE_MAP()
};

extern CLeakpressApp theApp;