#include "GrayProp.h"
#include "Resource.h"
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
: CBasePropertyPage(NAME("TGGrayProp"), pUnk, IDD_PROPPAGE, IDS_MYSTRING)
,m_pAS(0)
,m_pCP(0)
,m_lVal(0)
,m_bControl(true)
,m_lNewVal(0)
,m_bCopy(TRUE)
{
}
//查询一个接口
HRESULT CGrayProp::OnConnect(IUnknown *pUnk)
{
	if (pUnk == NULL)
	{
		return E_POINTER;
	}
	ASSERT(m_pAS == NULL);
	ASSERT(m_pCP == NULL);
	HRESULT hr = pUnk->QueryInterface(IID_IOS_ChangePitch,(void**)&m_pCP);
	hr |= pUnk->QueryInterface(IID_IOS_AudioSwitch, reinterpret_cast<void**>(&m_pAS));
	if (FAILED(hr))
	{
		SAFE_RELEASE(m_pAS);
		SAFE_RELEASE(m_pCP);
	}
	return hr;
}

HRESULT CGrayProp::OnActivate(void)
{
	INITCOMMONCONTROLSEX icc;
	icc.dwSize = sizeof(INITCOMMONCONTROLSEX);
	icc.dwICC = ICC_BAR_CLASSES;
	if (InitCommonControlsEx(&icc) == FALSE)
	{
		return E_FAIL;
	}
	
	ASSERT(m_pAS != NULL);
	ASSERT(m_pCP != NULL);
	// 	HRESULT hr = m_pGray->GetSaturation(&m_lVal);
	// 	if (SUCCEEDED(hr))
	// 	{
	// 		SendDlgItemMessage(m_Dlg, IDC_SLIDER1, TBM_SETPOS, 1, m_lVal);
	// 	}
	// 	return hr;
	SendDlgItemMessage(m_Dlg,IDC_SPIN_PITCH,UDM_SETRANGE,0,MAKELONG ((short) 5, (short) -5));
	SendDlgItemMessage(m_Dlg,IDC_SPIN_PITCH,UDM_SETPOS,0, MAKELONG ((short) 0, 0));
	SetDlgItemText(m_Dlg,IDC_SCUR_PITCH,L"cur pitch : 0");

	m_pAS->GetSTrackCopy(&m_bCopy);
	if (m_bCopy)
	{
		SetDlgItemText(m_Dlg,IDC_BSWITCH_COPY,L"Copy");
	}
	else
	{
		SetDlgItemText(m_Dlg,IDC_BSWITCH_COPY,L"Nocopy");
	}

	return S_OK;
}

HRESULT CGrayProp::OnApplyChanges(void)
{
	m_lVal = m_lNewVal;
	return S_OK;
} 

INT_PTR CGrayProp::OnReceiveMessage(HWND hwnd,UINT uMsg,WPARAM wParam,LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_VSCROLL:
		{
			HWND hWndSpinPitch = GetDlgItem(m_Dlg,IDC_SPIN_PITCH);
			if ( ((HWND)lParam) == hWndSpinPitch)
			{
				int nPitch = (int)((short)(HIWORD((DWORD)wParam)));
				HRESULT hr = m_pCP->ChangeCurPitch( nPitch );
				if (SUCCEEDED(hr))
				{
					wchar_t wtxt[260] = { 0 };
					swprintf_s(wtxt,259,L"cur pitch : %d",nPitch);
					SetDlgItemText(m_Dlg,IDC_SCUR_PITCH,wtxt);
				}
			}
			break;
		}
	case WM_COMMAND:
		{
			switch(LOWORD(wParam))
			{
			case IDC_BSWITCH_COPY:
				{
					m_bCopy = !m_bCopy;
					HRESULT hr = m_pAS->SetSTrackCopy(m_bCopy);
					if (SUCCEEDED(hr))
					{
						if (m_bCopy)
						{
							SetDlgItemText(m_Dlg,IDC_BSWITCH_COPY,L"Copy");
						}
						else
						{
							SetDlgItemText(m_Dlg,IDC_BSWITCH_COPY,L"Nocopy");
						}
					}
					break;
				}
			case IDC_BSWITCH_TRACK:
				{
					// User clicked the 'switch audio' button.
					//m_pGray->GetSaturation(&m_lNewVal);
					static bool b = true;
					b = !b;

					HRESULT hr = m_pAS->SwitchATrack(b);
					if (SUCCEEDED(hr))
					{
						if (b)
						{
							SetDlgItemText(m_Dlg,IDC_SAUDIO_TRACK,L"Audio Line 1");
						}
						else
						{
							SetDlgItemText(m_Dlg,IDC_SAUDIO_TRACK,L"Audio Line 2");
						}
					}
					//SetDirty();
					return  1;
				}
			case IDC_BSWITCH_SOUND:
				{
					static int number = 0;
					number ++;
					if (number == 3)
					{
						number = 0;
					}

					HRESULT hr = m_pAS->SwitchSTrack((eSoundTrack)number);
					if (SUCCEEDED(hr))
					{
						switch(number)
						{
						case 0:
							{
								SetDlgItemText(m_Dlg,IDC_SSOUND_TRACK,L"stereo");
								break;
							}
						case 1:
							{
								SetDlgItemText(m_Dlg,IDC_SSOUND_TRACK,L"mono left");
								break;
							}
						case 2:
							{
								SetDlgItemText(m_Dlg,IDC_SSOUND_TRACK,L"mono right");
								break;
							}
						}
					}
					break;
				}
			}
			break;
		}
	}

	return CBasePropertyPage::OnReceiveMessage(hwnd,uMsg,wParam,lParam);
}

HRESULT CGrayProp::OnDisconnect(void)
{
	if (m_pAS)
	{
		// If the user clicked OK, m_lVal holds the new value.
		// Otherwise, if the user clicked Cancel, m_lVal is the old value.
		//m_pGray->SetSaturation(m_lVal);  
		m_pAS->Release();
		m_pAS = NULL;
	}
	SAFE_RELEASE(m_pCP);
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


