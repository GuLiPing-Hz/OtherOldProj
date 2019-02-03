#include "stdafx.h"
#include "GrayProp.h"
#include "Resource.h"
#include <streams.h>
#include <CommCtrl.h>

CUnknown * WINAPI CGrayProp::CreateInstance(LPUNKNOWN pUnk, HRESULT *pHr) 
{
	CGrayProp *pNewObject = new CGrayProp(pUnk);
	if (pNewObject == NULL) 
	{
		*pHr = E_OUTOFMEMORY;
	}
	return pNewObject;
} 

CGrayProp::CGrayProp(IUnknown *pUnk) 
//属性页类名字,pUnk,对话框ID,标题string ID
: CBasePropertyPage(NAME("TGWMSourceGrayProp"), pUnk, IDD_PROPPAGE, IDS_HEADSTRING)
, m_pFilter(NULL)
{
}
//查询一个接口
HRESULT CGrayProp::OnConnect(IUnknown *pUnk)
{
	if (pUnk == NULL)
	{
		return E_POINTER;
	}
	assert(m_pFilter == NULL);
	return pUnk->QueryInterface(IID_IFileSourceFilter,(void**)&m_pFilter);
}

HRESULT CGrayProp::OnActivate()
{
	INITCOMMONCONTROLSEX icc;
	icc.dwSize = sizeof(INITCOMMONCONTROLSEX);
	icc.dwICC = ICC_BAR_CLASSES;
	if (InitCommonControlsEx(&icc) == FALSE)
	{
		return E_FAIL;
	}
	assert(m_pFilter);
	WCHAR* cur_file;
	CMediaType cur_mt;
	m_pFilter->GetCurFile(&cur_file,(AM_MEDIA_TYPE*)&cur_mt);
	SetDlgItemText(m_Dlg,IDC_SCURRENT_FILE,cur_file);
	CoTaskMemFree(cur_file);
	return S_OK;
}

HRESULT CGrayProp::OnApplyChanges(void)
{
	return S_OK;
} 

INT_PTR CGrayProp::OnReceiveMessage(HWND hwnd,UINT uMsg,WPARAM wParam,LPARAM lParam)
{
	return CBasePropertyPage::OnReceiveMessage(hwnd,uMsg,wParam,lParam);
}

HRESULT CGrayProp::OnDisconnect(void)
{
	if (m_pFilter)
	{
		m_pFilter->Release();
		m_pFilter = NULL;
	}
	return S_OK;
}


void CGrayProp::SetDirty()
{
	m_bDirty = TRUE;
	if (m_pPageSite)
	{
		m_pPageSite->OnStatusChange(PROPPAGESTATUS_DIRTY);
	}
}


