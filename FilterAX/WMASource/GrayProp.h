#pragma once
#include <streams.h>

//graphedit 属性类
class __declspec(uuid("D60C5C7D-956C-44a5-AA6E-FA6A547BA06B"))
CGrayProp : public CBasePropertyPage
{
private:
	CGrayProp(IUnknown *pUnk);
public:
	static CUnknown * WINAPI CreateInstance(LPUNKNOWN pUnk, HRESULT *pHr) ;
	//CBasePropertyPage
	//启动对话框的时候
	virtual HRESULT OnConnect(IUnknown *pUnknown);
	//关闭对话框的时候
	virtual HRESULT OnDisconnect();
	//类似OnInitDialog
	virtual HRESULT OnActivate();
	//
	//virtual HRESULT OnDeactivate() { return NOERROR; };
	//点击Applay
	virtual HRESULT OnApplyChanges();
	//对话框消息
	virtual INT_PTR OnReceiveMessage(HWND hwnd,UINT uMsg,WPARAM wParam,LPARAM lParam);
private:
	//This function informs the property frame that it should enable the Apply button.
	void SetDirty();
private:
	IFileSourceFilter* m_pFilter;
};
