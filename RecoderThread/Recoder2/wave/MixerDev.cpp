#include "StdAfx.h"
#include "MixerDev.h"

CMixerDev::CMixerDev(void)
:m_hMixer(NULL)
,m_uMxId(-1)
{
	m_nMixerDevs = mixerGetNumDevs();
}

CMixerDev::~CMixerDev(void)
{
	if(m_hMixer)
	{
		mixerClose(m_hMixer);
		m_hMixer = NULL;
	}
}

void CMixerDev::EnumMixerDevIdUnit( MAPSTRDEVID& mapMixerInfo,VECTMIXERCAPS& vectMixerCaps,const DWORD dwDestType )
{
	VECTMIXERLINE vect_mxl;
	EnumMixerDev(vectMixerCaps);
	for (UINT i=0;i<vectMixerCaps.size();i++)
	{
		if (EnumMixerDevUnit(vect_mxl,i,dwDestType))
		{
			std::string stdDevName = vectMixerCaps[i].szPname;
			mapMixerInfo.insert(std::pair<std::string,UINT>(stdDevName,i));
		}
	}
}

void CMixerDev::EnumMixerDev(/*out*/ VECTMIXERCAPS& vectMixerCaps)
{
	vectMixerCaps.clear();
	MIXERCAPS mixerCaps;
	UINT nDevs = mixerGetNumDevs();
	for(UINT i=0;i<nDevs;i++)
	{
		if (MMSYSERR_NOERROR==mixerGetDevCaps(i,&mixerCaps,sizeof(MIXERCAPS)) )
		{
			vectMixerCaps.push_back(mixerCaps);
		}	
	}
}
bool CMixerDev::EnumMixerDevUnit(/*out*/VECTMIXERLINE& vectMixerLine,
							/*in*/UINT uMxId,
							 /*in*/DWORD dwDestType)
{
	vectMixerLine.clear();
	MIXERLINE	mxl;
	MIXERCAPS mxcaps;

	if ( MMSYSERR_NOERROR !=  mixerGetDevCaps(uMxId,&mxcaps,sizeof(MIXERCAPS)) )
		return false;

	UINT u=0;
	do
	{
		mxl.cbStruct = sizeof(MIXERLINE);
		mxl.dwDestination = u;
		u++;
		if (MMSYSERR_NOERROR != mixerGetLineInfo((HMIXEROBJ)(DWORD_PTR)uMxId, &mxl, MIXER_OBJECTF_HMIXER|MIXER_GETLINEINFOF_DESTINATION))
		{
			continue ;
		}
	} while ((u < mxcaps.cDestinations) && (mxl.dwComponentType != dwDestType));

	if (mxl.dwComponentType != dwDestType)
	{
		//w riteLog("[DXLOG CMixerDev::EnumMixerDevUnit]:没有录音单元!请选择其他设备");
		return false;
	}

	UINT cConnections = (UINT)mxl.cConnections;
	u--;
	UINT v=0;
	do
	{
		mxl.cbStruct = sizeof(mxl);
		mxl.dwDestination = u;
		mxl.dwSource = v;
		v++;

		if (MMSYSERR_NOERROR == mixerGetLineInfo((HMIXEROBJ)(DWORD_PTR)uMxId, &mxl, MIXER_OBJECTF_HMIXER|MIXER_GETLINEINFOF_SOURCE))
		{
			vectMixerLine.push_back(mxl);
		} 
	} while ((v < cConnections));
	
	return true;
}

BOOL CMixerDev::Open(HWND hWnd,UINT defaultMxID)
{
	BOOL bstatus = TRUE;
	if (defaultMxID >= m_nMixerDevs)
	{
		MessageBoxA(NULL,"ERROR","device argument error",MB_OK);
		//LOG_ERN0("device argument error");
		return bstatus;
	}
	m_uMxId = defaultMxID;

	if (m_hMixer)
	{
		mixerClose(m_hMixer);
		m_hMixer = NULL;
	}

	if (MMSYSERR_NOERROR != mixerOpen(&m_hMixer, m_uMxId,(DWORD_PTR)hWnd, 0, CALLBACK_WINDOW))//使用dwCallback参数
	{
		bstatus = FALSE;
	}
	return bstatus;
}

UINT CMixerDev::getMixerID()
{
	if (MMSYSERR_NOERROR == 
		mixerGetID((HMIXEROBJ)m_hMixer, &m_uMxId, MIXER_OBJECTF_HMIXER))
		//(HMIXEROBJ)m_hMixer实际值是一个最小为0的整数，设备的标识符
	{
		/*return m_uMxId;*/
	}
	return m_uMxId;
}

BOOL CMixerDev::setRecordMixerLinebyType(MIXERLINE& mxl,MIXERLINE& mxlSub1,LPMIXERLINE pmxlSub2,DWORD dwControlType,bool bClean)//MIXERLINE& mxl
{
	//实现了"强制"单选指定线路为录音源输入
	MIXERCONTROL mxc;
	MIXERLINECONTROLS mxlc;
	mxlc.cbStruct = sizeof(MIXERLINECONTROLS);
	mxlc.dwLineID = mxl.dwLineID;
	mxlc.dwControlType = /*MIXERCONTROL_CONTROLTYPE_VOLUME*/dwControlType;//;////MIXERCONTROL_CONTROLTYPE_MUTE;
	mxlc.cControls = 1;
	mxlc.cbmxctrl = sizeof(MIXERCONTROL);
	mxlc.pamxctrl = &mxc; //为了得到相应的mxc.dwControlID，
	//锁定mxlc.dwLineID和mxlc.dwControlType指定的线路控制
	//目前将对录音目标单元的录音音源线路做单选（多路复用）
	HRESULT hr;
	if ( hr =mixerGetLineControls((HMIXEROBJ)m_hMixer,&mxlc,MIXER_OBJECTF_HMIXER |MIXER_GETLINECONTROLSF_ONEBYTYPE)
			!= MMSYSERR_NOERROR) 
	{
		return FALSE; 
	}
			
	//根据mxc.dwControlID检索线路，
	MIXERCONTROLDETAILS_BOOLEAN mxcdMute[8];
	MIXERCONTROLDETAILS mxcd;
	mxcd.cbStruct = sizeof(MIXERCONTROLDETAILS);
	mxcd.dwControlID = mxc.dwControlID;//在上面的&mxc得到
	mxcd.cChannels = 1;
	mxcd.cMultipleItems =mxc.cMultipleItems;//多路因子数目
	mxcd.cbDetails = sizeof(*mxcdMute);
	mxcd.paDetails =&mxcdMute;//存放检索结果
	if (mixerGetControlDetails((HMIXEROBJ)m_hMixer,&mxcd,MIXER_OBJECTF_HMIXER |MIXER_GETCONTROLDETAILSF_VALUE)
			!= MMSYSERR_NOERROR) 
	{
		return FALSE;
	}

	//做相应单选改变
	if (bClean)
	{
		for(int j=0;j<(int)mxc.cMultipleItems;j++)//单元个数
		{
			mxcdMute[j].fValue=false;	
		}
	}
		
	//顺序与录音控制面板上的顺序相反。
	mxcdMute[(int)mxc.cMultipleItems-1-mxlSub1.dwSource].fValue=true; //选中指定项
	if (pmxlSub2)
	{
		mxcdMute[(int)mxc.cMultipleItems-1-pmxlSub2->dwSource].fValue=true;
	}

	//重新设置dwControlID锁定的线路控制
	if (mixerSetControlDetails((HMIXEROBJ)m_hMixer,&mxcd,MIXER_OBJECTF_HMIXER |MIXER_GETCONTROLDETAILSF_VALUE)
			!= MMSYSERR_NOERROR)
	{
		return FALSE; 
	}

	return TRUE;
}

BOOL CMixerDev::getRecordLineInfoSource(LPMIXERLINE pmxl,LPMIXERLINE pmxlSub,DWORD nSrcType)
{
// 	DWORD dwSrcType = 0;
// 	if (nSrcType ==RECORDFROMLINE)
// 	{
// 		dwSrcType = MIXERLINE_COMPONENTTYPE_SRC_LINE;
// 	}
// 	else if (nSrcType == RECORDFROMMICPHONE)
// 	{
// 		dwSrcType = MIXERLINE_COMPONENTTYPE_SRC_MICROPHONE;
// 	}
// 	else if (nSrcType == RECORDFROMSTEREO)
// 	{
// 		dwSrcType = MIXERLINE_COMPONENTTYPE_SRC_WAVEOUT;
// 	}
// 	else
// 	{
// 		AfxMessageBox("bad argument!");
// 		return FALSE;
// 	}
	return GetLineInfo(pmxl,pmxlSub,MIXERLINE_COMPONENTTYPE_DST_WAVEIN,nSrcType);//录音
	//MIXERLINE_COMPONENTTYPE_DST_SPEAKERS 播放(扩音器)
}



BOOL CMixerDev::GetLineInfo(LPMIXERLINE pmxl,LPMIXERLINE pmxlSub, DWORD dwDstType, DWORD dwSrcType)
{
	MIXERCAPS mxcaps;
	if ( MMSYSERR_NOERROR !=  mixerGetDevCaps(m_uMxId,&mxcaps,sizeof(mxcaps)) )
		return FALSE;

	UINT u=0;
	do
	{
		pmxl->cbStruct = sizeof(*pmxl);
		pmxl->dwDestination = u;
		u++;
		if (MMSYSERR_NOERROR != mixerGetLineInfo((HMIXEROBJ)(DWORD_PTR)m_uMxId, pmxl, MIXER_OBJECTF_HMIXER|MIXER_GETLINEINFOF_DESTINATION))
		{
			return FALSE;
		}
	} while ((u < mxcaps.cDestinations) && (pmxl->dwComponentType != dwDstType));

	if (pmxl->dwComponentType != dwDstType)
		return FALSE;

	if (dwDstType == dwSrcType) 
		return TRUE;

	UINT cConnections = (UINT)pmxl->cConnections;

	UINT v=0;
	u--;
	do
	{
		pmxlSub->cbStruct = sizeof(*pmxlSub);
		pmxlSub->dwDestination = u;
		pmxlSub->dwSource = v;
		v++;

		if (MMSYSERR_NOERROR != mixerGetLineInfo((HMIXEROBJ)(DWORD_PTR)m_uMxId, pmxlSub, MIXER_OBJECTF_HMIXER|MIXER_GETLINEINFOF_SOURCE))
		{
			return FALSE;
		} 
	} while ((v < cConnections) && (pmxlSub->dwComponentType != dwSrcType));

	if(pmxlSub->dwComponentType !=dwSrcType)
	{
		if (dwSrcType == MIXERLINE_COMPONENTTYPE_SRC_WAVEOUT)//如果没有立体声，则选择MONO进行录制
		{
			dwSrcType = MIXERLINE_COMPONENTTYPE_SRC_ANALOG;
			v= 0;
			do
			{
				pmxlSub->cbStruct = sizeof(*pmxlSub);
				pmxlSub->dwDestination = u;
				pmxlSub->dwSource = v;
				v++;

				if (MMSYSERR_NOERROR != mixerGetLineInfo((HMIXEROBJ)(DWORD_PTR)m_uMxId, pmxlSub, MIXER_OBJECTF_HMIXER|MIXER_GETLINEINFOF_SOURCE))
				{
					return FALSE;
				} 
			} while ((v < cConnections) && (pmxlSub->dwComponentType != dwSrcType));

		}
		if (pmxlSub->dwComponentType !=dwSrcType)
		{
			return FALSE;
		}
	}

	return TRUE;
}

BOOL CMixerDev::GetLineControl(LPMIXERCONTROL pmxc, LPMIXERLINE pmxl, DWORD dwType,DWORD fdwControls)
{
	LPMIXERCONTROL pamxctrl;
	DWORD cbmxctrls = sizeof(*pamxctrl) * (UINT)pmxl->cControls;
	pamxctrl = (LPMIXERCONTROL)LocalAlloc(LPTR, cbmxctrls);

	MIXERLINECONTROLS mxlc;
	mxlc.cbStruct = sizeof(mxlc);
	mxlc.dwLineID = pmxl->dwLineID;
	mxlc.dwControlType = dwType;
	mxlc.cControls = pmxl->cControls;
	mxlc.cbmxctrl = sizeof(*pamxctrl);
	mxlc.pamxctrl = pamxctrl;
	//mixerGetControlDetails
	if (MMSYSERR_NOERROR != mixerGetLineControls((HMIXEROBJ)(DWORD_PTR)m_uMxId, &mxlc,fdwControls ))//MIXER_GETLINECONTROLSF_ONEBYTYPE
	{
		return FALSE;
	}
	memcpy(pmxc, pamxctrl, sizeof(*pamxctrl));
	LocalFree(pamxctrl);
	return TRUE;
} 

