
#include "StdAfx.h"
#include "vmrutil.h"
#include <streams.h>
//------------------------------------------------------------------------------
// File: VMRUtil.cpp
//
//------------------------------------------------------------------------------


//----------------------------------------------------------------------------
//  VerifyVMR9
//
//  Verifies that VMR9 COM objects exist on the system and that the VMR9
//  can be instantiated.
//
//  Returns: FALSE if the VMR9 can't be created
//----------------------------------------------------------------------------
//filter pin相关实现
HRESULT EnumFilters (IFilterGraph *pGraph) 
{
	IEnumFilters *pEnum = NULL;
	IBaseFilter *pFilter;
	ULONG cFetched;
	HRESULT hr = pGraph->EnumFilters(&pEnum);
	if (FAILED(hr)) return hr;
	while(pEnum->Next(1, &pFilter, &cFetched) == S_OK)
	{
		FILTER_INFO FilterInfo;
		hr = pFilter->QueryFilterInfo(&FilterInfo);
		if (FAILED(hr))
		{
			MessageBox(NULL, TEXT("Could not get the filter info"),
				TEXT("Error"), MB_OK | MB_ICONERROR);
			continue;  // Maybe the next one will work.
		}

#ifdef UNICODE
		MessageBox(NULL, FilterInfo.achName, TEXT("Filter Name"), MB_OK);
#else
		char szName[MAX_FILTER_NAME];
		int cch = WideCharToMultiByte(CP_ACP, 0, FilterInfo.achName,
			MAX_FILTER_NAME, szName, MAX_FILTER_NAME, 0, 0);
		if (cch > 0)
			MessageBox(NULL, szName, TEXT("Filter Name"), MB_OK);
#endif
		// The FILTER_INFO structure holds a pointer to the Filter Graph
		// Manager, with a reference coun that must be released.
		if (FilterInfo.pGraph != NULL)
		{
			FilterInfo.pGraph->Release();
		}
		pFilter->Release();
	}
	pEnum->Release();
	return S_OK;
}


HRESULT GetPin(/*in*/IBaseFilter *pFilter, /*in*/BOOL in_bInput,/*out*/ IPin **ppPin)
{
	PIN_DIRECTION direction = in_bInput ? PINDIR_INPUT : PINDIR_OUTPUT;
	IEnumPins  *pEnum = NULL;
	IPin       *pPin = NULL;
	HRESULT    hr;

	if (ppPin == NULL)
	{
		return E_POINTER;
	}

	hr = pFilter->EnumPins(&pEnum);//获取枚举pin的对象
	if (FAILED(hr))
	{
		return hr;
	}
	while(pEnum->Next(1, &pPin, 0) == S_OK)//获取下一个pin
	{
		PIN_DIRECTION PinDirThis;
		hr = pPin->QueryDirection(&PinDirThis);//查询类型
		if (FAILED(hr))
		{
			pPin->Release();
			pEnum->Release();
			return hr;
		}
		if (direction == PinDirThis) //如果类型符合
		{
			// Found a match. Return the IPin pointer to the caller.
			*ppPin = pPin;
			pEnum->Release();
			return S_OK;
		}
		// Release the pin for the next time through the loop.
		pPin->Release();
	}
	// No more pins. We did not find a match.
	pEnum->Release();
	return E_FAIL;  
}

HRESULT PinIsSupportMedia(IPin *pPin,const AM_MEDIA_TYPE & am_md,bool *pbIsSupport)
{
	IEnumMediaTypes* pEnumMT = NULL;
	AM_MEDIA_TYPE* temp_am_md = NULL;
	HRESULT hr;
	if (pPin = NULL)
	{
		return E_POINTER;
	}
	
	hr = pPin->EnumMediaTypes(&pEnumMT);
	if (FAILED(hr))
	{
		return hr;
	}
	*pbIsSupport = false;
	while(pEnumMT->Next(1,&temp_am_md,NULL) == S_OK)
	{
		if((IsEqualGUID(temp_am_md->majortype,am_md.majortype) == TRUE) &&
			(IsEqualGUID(temp_am_md->subtype,am_md.subtype) == TRUE) &&
			(IsEqualGUID(temp_am_md->formattype,am_md.formattype) == TRUE) &&
			(temp_am_md->cbFormat == am_md.cbFormat) &&
			((temp_am_md->cbFormat == 0) ||(memcmp(temp_am_md->pbFormat, am_md.pbFormat, temp_am_md->cbFormat) == 0)))
		{
			*pbIsSupport = true;
			DeleteMediaType(temp_am_md);
			pEnumMT->Release();
			return S_OK;
		}
		DeleteMediaType(temp_am_md);
	}
	
	pEnumMT->Release();
	return S_OK;

}

HRESULT AddFilterByCLSID(
						 IGraphBuilder *pGraph,	// Pointer to the Filter Graph Manager.
						 const GUID& clsid,			// CLSID of the filter to create.
						 LPCWSTR wszName,        // A name for the filter.
						 IBaseFilter **ppF)				// Receives a pointer to the filter.
{
	if (!pGraph || ! ppF) 
	{
		return E_POINTER;
	}
		
	*ppF = 0;
	IBaseFilter *pF = 0;
	HRESULT hr = CoCreateInstance(clsid, 0, CLSCTX_INPROC_SERVER,IID_IBaseFilter, reinterpret_cast<void**>(&pF));
	if (SUCCEEDED(hr))
	{
		hr = pGraph->AddFilter(pF, wszName);
		if (SUCCEEDED(hr))
			*ppF = pF;
		else
			pF->Release();
	}
	return hr;
}

HRESULT DisConnectedPin(/*in*/IGraphBuilder *pGraph,/*in*/IBaseFilter *pFilter)
{
	IEnumPins *pEnum = 0;
	IPin *pPin = 0;

	// Get a pin enumerator
	HRESULT hr = pFilter->EnumPins(&pEnum);
	if (FAILED(hr))
	{
		return hr;
	}

	// Look for the first unconnected pin
	while (pEnum->Next(1, &pPin, NULL) == S_OK)
	{
		//pEnum->Reset();

		IPin *pTmp = 0;

		hr = pPin->ConnectedTo(&pTmp);
		if (SUCCEEDED(hr))  // Already connected, not the pin we want.
		{
			hr = pGraph->Disconnect(pPin);
			pTmp->Release();
		}		
		pPin->Release();
	}

	// Release the enumerator
	pEnum->Release();

	// Did not find a matching pin
	return E_FAIL;
}

HRESULT GetUnconnectedPin(
						  IBaseFilter *pFilter,   // Pointer to the filter.
						  BOOL in_bInput,//PIN_DIRECTION PinDir,   // Direction of the pin to find.
						  IPin **ppPin)           // Receives a pointer to the pin.
{
	PIN_DIRECTION direction = in_bInput ? PINDIR_INPUT:PINDIR_OUTPUT;
	IEnumPins *pEnum = 0;
	IPin *pPin = 0;

	if (!ppPin)
		return E_POINTER;
	*ppPin = 0;

	// Get a pin enumerator
	HRESULT hr = pFilter->EnumPins(&pEnum);
	if (FAILED(hr))
	{
		return hr;
	}
		
	// Look for the first unconnected pin
	while (pEnum->Next(1, &pPin, NULL) == S_OK)
	{
		//pEnum->Reset();
		PIN_DIRECTION ThisPinDir;

		pPin->QueryDirection(&ThisPinDir);
		if (ThisPinDir == direction)
		{
			IPin *pTmp = 0;

			hr = pPin->ConnectedTo(&pTmp);
			if (SUCCEEDED(hr))  // Already connected, not the pin we want.
			{
				pTmp->Release();
			}
			else  // Unconnected, this is the pin we want.
			{
				pEnum->Release();
				*ppPin = pPin;
				return S_OK;
			}
		}
		pPin->Release();
	}

	// Release the enumerator
	pEnum->Release();

	// Did not find a matching pin
	return E_FAIL;
}

HRESULT ConnectFilters(
					   IGraphBuilder *pGraph, // Filter Graph Manager.
					   IPin *pOut,            // Output pin on the upstream filter.
					   IBaseFilter *pDest,
					   const AM_MEDIA_TYPE * inMediaType)    // Downstream filter.
{
	if ((pGraph == NULL) || (pOut == NULL) || (pDest == NULL))
	{
		return E_POINTER;
	}
#ifdef debug
	PIN_DIRECTION PinDir;
	pOut->QueryDirection(&PinDir);
	_ASSERTE(PinDir == PINDIR_OUTPUT);
#endif

	//找一个空闲的输入pin
	IPin *pIn = 0;
	HRESULT hr = GetUnconnectedPin(pDest, TRUE, &pIn);
	if (FAILED(hr))
	{
		return hr;
	}
	// Try to connect them.
	hr = pGraph->ConnectDirect(pOut, pIn,inMediaType);
	pIn->Release();
	return hr;
}

HRESULT ConnectFilters(
					   IGraphBuilder *pGraph, 
					   IBaseFilter *pSrc, 
					   IBaseFilter *pDest,
					   const AM_MEDIA_TYPE * inMediaType)
{
	if ((pGraph == NULL) || (pSrc == NULL) || (pDest == NULL))
	{
		return E_POINTER;
	}

	// 首先在第一个filter上查询一个输出的pin接口
	IPin *pOut = 0;
	HRESULT hr = GetUnconnectedPin(pSrc, FALSE, &pOut);
	if (FAILED(hr)) 
	{
		return hr;
	}
	//然后将它和第二个filter的输入接口衔接。
	hr = ConnectFilters(pGraph, pOut, pDest,inMediaType);
	pOut->Release();
	return hr;
}

HRESULT TryConnectFilter(IGraphBuilder *pGraph, // Filter Graph Manager.
											IPin *pOut,            // Output pin on the upstream filter.
											IBaseFilter *pDest,
											const AM_MEDIA_TYPE * inMediaType)    // Downstream filter.)
{
	if ((pGraph == NULL) || (pOut == NULL) || (pDest == NULL))
	{
		return E_POINTER;
	}
#ifdef debug
	PIN_DIRECTION PinDir;
	pOut->QueryDirection(&PinDir);
	_ASSERTE(PinDir == PINDIR_OUTPUT);
#endif

	//寻找所有空闲的pin，尝试连接
	//HRESULT hr = GetUnconnectedPin(pDest, PINDIR_INPUT, &pIn);
	IEnumPins *pEnum = 0;
	IPin *pPin = 0;

	// Get a pin enumerator
	HRESULT hr = pDest->EnumPins(&pEnum);
	if (FAILED(hr))
	{
		return hr;
	}

	// Look for the first unconnected pin
	while (pEnum->Next(1, &pPin, NULL) == S_OK)
	{
		//pEnum->Reset();
		PIN_DIRECTION ThisPinDir;

		pPin->QueryDirection(&ThisPinDir);
		if (ThisPinDir == PINDIR_INPUT)
		{
			IPin *pTmp = 0;

			hr = pPin->ConnectedTo(&pTmp);
			if (SUCCEEDED(hr))  // Already connected, not the pin we want.
			{
				pTmp->Release();
			}
			else  // Unconnected, this is the pin we want.
			{
				// Try to connect them.
				hr = pGraph->ConnectDirect(pOut, pPin,inMediaType);
				if (SUCCEEDED(hr))
				{
					pEnum->Release();
					pPin->Release();
					return S_OK;
				}
			}
		}
		pPin->Release();
	}

	// Release the enumerator
	pEnum->Release();

	// Did not find a matching pin
	return hr;
}

HRESULT TryConnectFilter(IGraphBuilder *pGraph, 
						 IBaseFilter *pSrc, 
						 IBaseFilter *pDest, 
						 const AM_MEDIA_TYPE * inMediaType)
{
	if ((pGraph == NULL) || (pSrc == NULL) || (pDest == NULL))
	{
		return E_POINTER;
	}

	//寻找所有空闲的pin，尝试连接
	//HRESULT hr = GetUnconnectedPin(pDest, PINDIR_INPUT, &pIn);
	IEnumPins *pEnum = 0;
	IPin *pPin = 0;

	// Get a pin enumerator
	HRESULT hr = pSrc->EnumPins(&pEnum);
	if (FAILED(hr))
	{
		return hr;
	}
	//pEnum->Reset();
	// Look for the first unconnected pin
	while (pEnum->Next(1, &pPin, NULL) == S_OK)
	{
		//pEnum->Reset();如果枚举有改变，则需要先Reset
		PIN_DIRECTION ThisPinDir;

		pPin->QueryDirection(&ThisPinDir);
		if (ThisPinDir == PINDIR_OUTPUT)
		{
			IPin *pTmp = 0;

			hr = pPin->ConnectedTo(&pTmp);
			if (SUCCEEDED(hr))  // Already connected, not the pin we want.
			{
				pTmp->Release();
			}
			else  // Unconnected, this is the pin we want.
			{
				// Try to connect them.
				hr = TryConnectFilter(pGraph,pPin,pDest,inMediaType);
				if (SUCCEEDED(hr))
				{
					pEnum->Release();
					pPin->Release();
					return S_OK;
				}
			}
		}
		pPin->Release();
	}

	// Release the enumerator
	pEnum->Release();

	// Did not find a matching pin
	return hr;
}

HRESULT FindFilterInterface(
							IGraphBuilder *pGraph, // Pointer to the Filter Graph Manager.
							REFGUID iid,           // IID of the interface to retrieve.
							void **ppUnk)          // Receives the interface pointer.
{
	if (!pGraph || !ppUnk) return E_POINTER;

	HRESULT hr = E_FAIL;
	IEnumFilters *pEnum = NULL;
	IBaseFilter *pF = NULL;
	if (FAILED(pGraph->EnumFilters(&pEnum)))
	{
		return E_FAIL;
	}
	// Query every filter for the interface.
	while (S_OK == pEnum->Next(1, &pF, 0))
	{
		hr = pF->QueryInterface(iid, ppUnk);//查询
		pF->Release();
		if (SUCCEEDED(hr))
		{
			break;
		}
	}
	pEnum->Release();
	return hr;
}

HRESULT FindPinInterface(
						 IBaseFilter *pFilter,  // Pointer to the filter to search.
						 REFGUID iid,           // IID of the interface.
						 void **ppUnk)          // Receives the interface pointer.
{
	if (!pFilter || !ppUnk) return E_POINTER;

	HRESULT hr = E_FAIL;
	IEnumPins *pEnum = 0;
	if (FAILED(pFilter->EnumPins(&pEnum)))
	{
		return E_FAIL;
	}
	// Query every pin for the interface.
	IPin *pPin = 0;
	while (S_OK == pEnum->Next(1, &pPin, 0))
	{
		hr = pPin->QueryInterface(iid, ppUnk);//查询
		pPin->Release();
		if (SUCCEEDED(hr))
		{
			break;
		}
	}
	pEnum->Release();
	return hr;
}

HRESULT FindInterfaceAnywhere(
							  IGraphBuilder *pGraph, 
							  REFGUID iid, 
							  void **ppUnk)
{
	if (!pGraph || !ppUnk) return E_POINTER;
	HRESULT hr = E_FAIL;
	IEnumFilters *pEnum = 0;
	if (FAILED(pGraph->EnumFilters(&pEnum)))
	{
		return E_FAIL;
	}
	// Loop through every filter in the graph.
	IBaseFilter *pF = 0;
	while (S_OK == pEnum->Next(1, &pF, 0))
	{
		hr = pF->QueryInterface(iid, ppUnk);
		if (FAILED(hr))
		{
			// The filter does not expose the interface, but maybe
			// one of its pins does. //调用的是上面的搜索pin的函数
			hr = FindPinInterface(pF, iid, ppUnk);
		}
		pF->Release();
		if (SUCCEEDED(hr))
		{
			break;
		}
	}
	pEnum->Release();
	return hr;
}

HRESULT GetNextFilter(
					  IBaseFilter *pFilter, // 开始的filter
					  BOOL in_bInput,//PIN_DIRECTION Dir,    // 搜索的方向 (upstream /input 还是 downstream/output)
					  IBaseFilter **ppNext) // Receives a pointer to the next filter.
{
	PIN_DIRECTION direction = in_bInput ? PINDIR_INPUT:PINDIR_OUTPUT;
	if (!pFilter || !ppNext) return E_POINTER;

	IEnumPins *pEnum = 0;
	IPin *pPin = 0;
	HRESULT hr = pFilter->EnumPins(&pEnum);
	if (FAILED(hr)) return hr;
	while (S_OK == pEnum->Next(1, &pPin, 0))
	{
		// See if this pin matches the specified direction.
		PIN_DIRECTION ThisPinDir;
		hr = pPin->QueryDirection(&ThisPinDir);
		if (FAILED(hr))
		{
			// Something strange happened.
			hr = E_UNEXPECTED;
			pPin->Release();
			break;
		}
		if (ThisPinDir == direction)
		{
			// Check if the pin is connected to another pin.
			IPin *pPinNext = 0;
			hr = pPin->ConnectedTo(&pPinNext);
			if (SUCCEEDED(hr))
			{
				// Get the filter that owns that pin.
				PIN_INFO PinInfo;
				hr = pPinNext->QueryPinInfo(&PinInfo);
				pPinNext->Release();
				pPin->Release();
				pEnum->Release();
				if (FAILED(hr) || (PinInfo.pFilter == NULL))
				{
					// Something strange happened.
					return E_UNEXPECTED;
				}
				// This is the filter we're looking for.
				*ppNext = PinInfo.pFilter; // Client must release.
				return S_OK;
			}
		}
		pPin->Release();
	}
	pEnum->Release();
	// Did not find a matching filter.
	return E_FAIL;
}

HRESULT GetPeerFilters(
					   IBaseFilter *pFilter, // Pointer to the starting filter
					   BOOL in_bInput,//PIN_DIRECTION Dir,    // Direction to search (upstream or downstream)
					   CFilterList &FilterList)  // Collect the results in this list.
{
	PIN_DIRECTION direction = in_bInput?PINDIR_INPUT:PINDIR_OUTPUT;
	if (!pFilter) return E_POINTER;

	IEnumPins *pEnum = 0;
	IPin *pPin = 0;
	HRESULT hr = pFilter->EnumPins(&pEnum);
	if (FAILED(hr)) return hr;
	while (S_OK == pEnum->Next(1, &pPin, 0))
	{
		// See if this pin matches the specified direction.
		PIN_DIRECTION ThisPinDir;
		hr = pPin->QueryDirection(&ThisPinDir);
		if (FAILED(hr))
		{
			// Something strange happened.
			hr = E_UNEXPECTED;
			pPin->Release();
			break;
		}
		if (ThisPinDir == direction)
		{
			// Check if the pin is connected to another pin.
			IPin *pPinNext = 0;
			hr = pPin->ConnectedTo(&pPinNext);
			if (SUCCEEDED(hr))
			{
				// Get the filter that owns that pin.
				PIN_INFO PinInfo;
				hr = pPinNext->QueryPinInfo(&PinInfo);
				pPinNext->Release();
				if (FAILED(hr) || (PinInfo.pFilter == NULL))
				{
					// Something strange happened.
					pPin->Release();
					pEnum->Release();
					return E_UNEXPECTED;
				}
				// 将符合的filter添加到list中
				AddFilterUnique(FilterList, PinInfo.pFilter);
				PinInfo.pFilter->Release();
			}
		}
		pPin->Release();
	}
	pEnum->Release();
	return S_OK;
}
void AddFilterUnique(CFilterList &FilterList, IBaseFilter *pNew)
{
	if (pNew == NULL) return;

	CFilterList::iterator it = FilterList.begin();
	for(it;it!=FilterList.end();it++)
	{
		IBaseFilter* pF = *it;
		if (IsEqualObject(pF, pNew))
		{
			return;
		}
	}
	pNew->AddRef();  // The caller must release everything in the list.
	FilterList.push_back(pNew);
}

void ReleaseFilter(IBaseFilter* pFilter,IGraphBuilder* pGraph)
{
	if (pFilter)
	{
		if (pGraph)
		{
			pGraph->RemoveFilter(pFilter);
		}
		pFilter->Release();
	}
}

HRESULT RemoveAllFilter(IGraphBuilder *pGraph) // Filter Graph Manager.
{
	// Enumerate the filters in the graph.
	if (!pGraph)
	{
		return S_OK;
	}
	IEnumFilters *pEnum = NULL;
	HRESULT hr = pGraph->EnumFilters(&pEnum);
	if (SUCCEEDED(hr))
	{
		IBaseFilter *pFilter = NULL;//390
		while (S_OK == pEnum->Next(1, &pFilter, NULL))
		{
			// Remove the filter.
			pGraph->RemoveFilter(pFilter);
			//DisConnectedPin(pGraph,pFilter);
			//ReleaseFilter(pFilter,pGraph);
			// Reset the enumerator.
			pEnum->Reset();
			pFilter->Release();
		}//363
		pEnum->Release();
	}
	return hr;
}


BOOL VerifyVMR9(void)
{
	HRESULT hr;

	// Verify that the VMR exists on this system
	IBaseFilter* pBF = NULL;
	hr = CoCreateInstance(CLSID_VideoMixingRenderer9, NULL,CLSCTX_INPROC,IID_IBaseFilter,(LPVOID *)&pBF);
	if(SUCCEEDED(hr))
	{
		pBF->Release();
		return TRUE;
	}
	else
	{
		MessageBox(NULL,
			TEXT("This application requires the Video Mixing Renderer, which is present\r\n")
			TEXT("only on DirectX 9 systems with hardware video acceleration enabled.\r\n\r\n")

			TEXT("The Video Mixing Renderer (VMR9) is not enabled when viewing a \r\n")
			TEXT("remote Windows XP machine through a Remote Desktop session.\r\n")
			TEXT("You can run VMR-enabled applications only on your local machine.\r\n\r\n")

			TEXT("To verify that hardware acceleration is enabled on a Windows XP\r\n")
			TEXT("system, follow these steps:\r\n")
			TEXT("-----------------------------------------------------------------------\r\n")
			TEXT(" - Open 'Display Properties' in the Control Panel\r\n")
			TEXT(" - Click the 'Settings' tab\r\n")
			TEXT(" - Click on the 'Advanced' button at the bottom of the page\r\n")
			TEXT(" - Click on the 'Troubleshooting' tab in the window that appears\r\n")
			TEXT(" - Verify that the 'Hardware Acceleration' slider is at the rightmost position\r\n")

			TEXT("\r\nThis sample will now exit."),

			TEXT("Video Mixing Renderer (VMR9) capabilities are required"), MB_OK);

		return FALSE;
	}
}


BOOL IsWindowsMediaFile(WCHAR *lpszFile)
{
	USES_CONVERSION;
	TCHAR szFilename[MAX_PATH];

	// Copy the file name to a local string and convert to lowercase
	(void)StringCchCopy(szFilename,NUMELMS(szFilename), W2T(lpszFile) );
	szFilename[MAX_PATH-1] = 0;
	_tcslwr(szFilename);

	if (_tcsstr(szFilename, TEXT(".asf")) ||
		_tcsstr(szFilename, TEXT(".wma")) ||
		_tcsstr(szFilename, TEXT(".wmv")))
		return TRUE;
	else
		return FALSE;
}

HRESULT RenderFileToVMR9(IGraphBuilder *pGB, WCHAR *wFileName,
						 IBaseFilter *pRenderer, BOOL bRenderAudio)
{
	HRESULT hr=S_OK;
	CComPtr <IPin> pOutputPin;
	CComPtr <IBaseFilter> pSource;
	CComPtr <IBaseFilter> pAudioRenderer;
	CComPtr <IFilterGraph2> pFG;

	// Add a file source filter for this media file
	if (!IsWindowsMediaFile(wFileName))
	{
		// Add the source filter to the graph
		if (FAILED(hr = pGB->AddSourceFilter(wFileName, L"SOURCE", &pSource)))
		{
			USES_CONVERSION;
			TCHAR szMsg[MAX_PATH + 128];

			hr = StringCchPrintf(szMsg, NUMELMS(szMsg), TEXT("Failed to add the source filter to the graph!  hr=0x%x\r\n\r\n")
				TEXT("Filename: %s\0"), hr, W2T(wFileName));
			MessageBox(NULL, szMsg, TEXT("Failed to render file to VMR9"), MB_OK | MB_ICONERROR);

			return hr;
		}

		// Get the interface for the first unconnected output pin
		JIF(GetUnconnectedPin(pSource, FALSE, &pOutputPin));
	}
	else
	{
		MessageBox(NULL, TEXT("Windows Media files (ASF,WMA,WMV) are not supported by this application.\r\n\r\n")
			TEXT("For a full example of Windows Media support using the\r\n")
			TEXT("DirectShow WM ASF Reader filter and implementing a key provider\r\n")
			TEXT("for Windows Media content, refer to the following SDK samples:\r\n\r\n")
			TEXT("\t- ASFCopy\t- AudioBox\r\n\t- Jukebox  \t- PlayWndASF\r\n\r\n")
			TEXT("Each of the listed samples provides the necessary extra code\r\n")
			TEXT("and links with the required Windows Media libraries.\0"),
			TEXT("Windows Media files are not supported"), MB_OK);
		return E_FAIL;
	}

	// Render audio if requested (defaults to TRUE)
	if (bRenderAudio)
	{
		// Because we will be rendering with the RENDERTOEXISTINGRENDERERS flag,
		// we need to create an audio renderer and add it to the graph.
		// Create an instance of the DirectSound renderer (for each media file).
		//
		// Note that if the system has no sound card (or if the card is disabled),
		// then creating the DirectShow renderer will fail.  In that case,
		// handle the failure quietly.
		if (SUCCEEDED(CoCreateInstance(CLSID_DSoundRender, NULL, CLSCTX_INPROC_SERVER,
			IID_IBaseFilter, (void **)&pAudioRenderer)))
		{
			// The audio renderer was successfully created, so add it to the graph
			JIF(pGB->AddFilter(pAudioRenderer, L"Audio Renderer"));
		}
	}

	// Get an IFilterGraph2 interface to assist in building the
	// multifile graph with the non-default VMR9 renderer
	JIF(pGB->QueryInterface(IID_IFilterGraph2, (void **)&pFG));

	// Render the output pin, using the VMR9 as the specified renderer.  This is
	// necessary in case the GraphBuilder needs to insert a Color Space convertor,
	// or if multiple filters insist on using multiple allocators.
	// The audio renderer will also be used, if the media file contains audio.
	JIF(pFG->RenderEx(pOutputPin, AM_RENDEREX_RENDERTOEXISTINGRENDERERS, NULL));

	// If this media file does not contain an audio stream, then the
	// audio renderer that we created will be unconnected.  If left in the
	// graph, it could interfere with rate changes and timing.
	// Therefore, if the audio renderer is unconnected, remove it from the graph.
	if (pAudioRenderer != NULL)
	{
		IPin *pUnconnectedPin=0;

		// Is the audio renderer's input pin connected?
		HRESULT hrPin = GetUnconnectedPin(pAudioRenderer, TRUE, &pUnconnectedPin);

		// If there is an unconnected pin, then remove the unused filter
		if (SUCCEEDED(hrPin) && (pUnconnectedPin != NULL))
		{
			// Release the returned IPin interface
			pUnconnectedPin->Release();

			// Remove the audio renderer from the graph
			hrPin = pGB->RemoveFilter(pAudioRenderer);
		}
	}

	return hr;
}



