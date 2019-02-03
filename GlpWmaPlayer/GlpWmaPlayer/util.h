#pragma  once

#include <streams.h>

#ifndef SAFE_RELEASE
#define SAFE_RELEASE(x)  do{if((x)) \
	(x)->Release(); (x)=NULL; }while(0)
#endif//SAFE_RELEASE

#ifndef SAFE_DELETE
#define SAFE_DELETE(x) \
	if ((x) != NULL)      \
{                   \
	delete (x);        \
	(x) = NULL;        \
}
#endif//SAFE_DELETE

#ifndef SAFE_ARRAY_DELETE
#define SAFE_ARRAY_DELETE(x) \
	if ((x) != NULL)            \
{                         \
	delete[] (x);            \
	(x) = NULL;              \
}
#endif//SAFE_ARRAY_DELETE

#ifndef SAFE_FREE
#define SAFE_FREE(x) \
	if ((x) != NULL)            \
{                         \
	free((x));            \
	(x) = NULL;              \
}
#endif//SAFE_FREE

// ö��filter
HRESULT EnumFilters (/*in*/IFilterGraph *pGraph);//����graph�е�filter��������ʾfilter������

//ö��pin
HRESULT GetPin(/*in*/IBaseFilter *pFilter, /*in*/BOOL in_bInput,/*out*/ IPin **ppPin);

//�Ƿ�֧��ָ��ý������
HRESULT PinIsSupportMedia(/*in*/IPin *pPin,/*in*/const AM_MEDIA_TYPE & am_md,/*out*/bool* pbIsSupport);

//ע����Щfilter�ǲ���ͨ��with CoCreateInstance���������ġ�����AVI Compressor  Filter��WDM Video Capture filter
HRESULT AddFilterByCLSID(/*in*/IGraphBuilder *pGraph,	// Pointer to the Filter Graph Manager.
						 /*in*/ const GUID& clsid,			// CLSID of the filter to create.
						 /*in*/ LPCWSTR wszName,        // A name for the filter.
						 /*out*/ IBaseFilter **ppF);				// Receives a pointer to the filter.
//��ȡδʹ�õ�Pin
HRESULT GetUnconnectedPin(/*in*/IBaseFilter *pFilter,   // Pointer to the filter.
						  /*in*/BOOL in_bInput,//PIN_DIRECTION PinDir,   // Direction of the pin to find.PINDIR_OUTPUT/PINDIR_INPUT
						  /*out*/IPin **ppPin);           // Receives a pointer to the pin.
//�Ͽ������ӵ�Pin
HRESULT DisConnectedPin(/*in*/IGraphBuilder *pGraph,
						/*in*/IBaseFilter *pFilter);
//����output IPin��filter
HRESULT ConnectFilters(/*in*/IGraphBuilder *pGraph, // Filter Graph Manager.
					   /*in*/IPin *pOut,            // Output pin on the upstream filter.
					   /*in*/IBaseFilter *pDest,// Downstream filter.
					   /*in*/const AM_MEDIA_TYPE * inMediaType = NULL);    
//��������filter
HRESULT ConnectFilters( /*in*/IGraphBuilder *pGraph, // Filter Graph Manager.
					   /*in*/IBaseFilter *pSrc, //output IPin of filter
					   /*in*/IBaseFilter *pDest,//input IPin of filter
					   /*in*/const AM_MEDIA_TYPE * inMediaType = NULL);
//��������pDest���е�input Pin��������������ϣ�����S_FALSE
HRESULT TryConnectFilter(/*in*/IGraphBuilder *pGraph, // Filter Graph Manager.
						 /*in*/IPin *pOut,            // Output pin on the upstream filter.
						 /*in*/IBaseFilter *pDest,// Downstream filter.)
						 /*in*/const AM_MEDIA_TYPE * inMediaType=0);   
//����pSrc���е�output Pinȥ�������е�pDest��input Pin����ʧ�ܣ�����S_FALSE
HRESULT TryConnectFilter(/*in*/IGraphBuilder *pGraph, 
						 /*in*/IBaseFilter *pSrc, 
						 /*in*/IBaseFilter *pDest, 
						 /*in*/const AM_MEDIA_TYPE * inMediaType=0);
//��Filter Graph��ȡָ��GUID��filter
HRESULT FindFilterInterface(/*in*/IGraphBuilder *pGraph, // Pointer to the Filter Graph Manager.
							/*in*/REFGUID iid,           // IID of the interface to retrieve.
							/*out*/void **ppUnk);          // Receives the interface pointer.
//��Filter Graph��ȡָ��GUID��Pin
HRESULT FindPinInterface(/*in*/IBaseFilter *pFilter,  // Pointer to the filter to search.
						 /*in*/REFGUID iid,           // IID of the interface.
						 /*out*/void **ppUnk);       // Receives the interface pointer.
//��Filter Graph��ȡָ��GUID�� ʵ��������filter����pin
HRESULT FindInterfaceAnywhere(/*in*/ IGraphBuilder *pGraph, 
							  /*in*/REFGUID iid, 
							  /*out*/void **ppUnk);
//��ȡ������filter
HRESULT GetNextFilter(/*in*/IBaseFilter *pFilter, // ��ʼ��filter
					  /*in*/BOOL in_bInput,//PIN_DIRECTION Dir,    // ��������(upstream /input ���� downstream/output)
					  /*out*/IBaseFilter **ppNext); // Receives a pointer to the next filter.

typedef CGenericList<IBaseFilter> CFilterList;
//��Ӳ�ͬ��filter���б���
void AddFilterUnique(CFilterList &FilterList, IBaseFilter *pNew);
//��ȡ��ͬ�����filter
HRESULT GetPeerFilters(
					   /*in*/IBaseFilter *pFilter, // Pointer to the starting filter
					   /*in*/BOOL in_bInput,//PIN_DIRECTION Dir,    // Direction to search (upstream or downstream)
					   /*out*/CFilterList &FilterList);  // Collect the results in this list.
//Ĭ���ͷ�filter�����ָ��pGraph����һ����pGraph��remove��
void ReleaseFilter(/*in*/IBaseFilter* pFilter,/*in*/IGraphBuilder* pGraph=NULL);
//ɾ�����е�filter
HRESULT RemoveAllFilter(/*in*/IGraphBuilder *pGraph); // Filter Graph Manager.
