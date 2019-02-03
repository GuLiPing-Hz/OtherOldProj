#include "util.h"

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

	POSITION pos = FilterList.GetHeadPosition();
	while (pos)
	{
		IBaseFilter *pF = FilterList.GetNext(pos);
		if (IsEqualObject(pF, pNew))
		{
			return;
		}
	}
	pNew->AddRef();  // The caller must release everything in the list.
	FilterList.AddTail(pNew);
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
			//pGraph->RemoveFilter(pFilter);
			//DisConnectedPin(pGraph,pFilter);
			ReleaseFilter(pFilter,pGraph);
			// Reset the enumerator.
			pEnum->Reset();
			//pFilter->Release();
		}//363
		pEnum->Release();
	}
	return hr;
}
