#pragma once
#include <streams.h>

//graphedit ������
class __declspec(uuid("D60C5C7D-956C-44a5-AA6E-FA6A547BA06B"))
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
	IFileSourceFilter* m_pFilter;
};
