#pragma once
// #include <afxwin.h>
#include "util.h"
#include "uguid.h"
#include "iaudioswitch.h"
// #include <atlbase.h>
// #include <atlcoll.h>

//graphedit ������
class __declspec(uuid("8B24C2F3-2D50-4a42-A75A-4C2121763A75"))
CGrayProp : public CBasePropertyPage
{
private:
	CGrayProp(IUnknown *pUnk);
public:
	static CUnknown * WINAPI CreateInstance(LPUNKNOWN pUnk, HRESULT *pHr) ;
	//CBasePropertyPage
	//�����Ի����ʱ��
	virtual HRESULT OnConnect(IUnknown *pUnknown);
	//�رնԻ����ʱ��
	virtual HRESULT OnDisconnect();
	//����OnInitDialog
	virtual HRESULT OnActivate();
	//
	//virtual HRESULT OnDeactivate() { return NOERROR; };
	//���Applay
	virtual HRESULT OnApplyChanges();
	//�Ի�����Ϣ
	virtual INT_PTR OnReceiveMessage(HWND hwnd,UINT uMsg,WPARAM wParam,LPARAM lParam);
private:
	//This function informs the property frame that it should enable the Apply button.
	void SetDirty();
private:
	IOS_AudioSwitch*	m_pAS;    // Pointer to the filter's custom interface.
	IOS_ChangePitch*	m_pCP;
	BOOL						m_bCopy;
	bool							m_bControl;
	long							m_lVal;       // Store the old value, so we can revert.
	long							m_lNewVal;   // New value.

};
