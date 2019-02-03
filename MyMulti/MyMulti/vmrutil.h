//------------------------------------------------------------------------------
// File: VMRUtil.h
//
// Desc: DirectShow sample code - header file for C++ VMR9 sample applications
//       that do not use MFC.  This header contains several methods useful
//       for creating filter graphs with the Video Mixing Renderer 9.
//
//       Because graph building with the VMR9 requires a few extra steps
//       in order to guarantee that the VMR9 is used instead of another
//       video renderer, these helper methods are implemented in a header file
//       so that they can be easily integrated into a non-MFC application.
//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//------------------------------------------------------------------------------

#ifndef __INC_VMRUTIL_H__
#define __INC_VMRUTIL_H__

#pragma once

#ifndef JIF
#define JIF(x) if (FAILED(hr=(x))) {return hr;}
#endif

//----------------------------------------------------------------------------
//  VerifyVMR9
//
//  Verifies that VMR9 COM objects exist on the system and that the VMR9
//  can be instantiated.
//
//  Returns: FALSE if the VMR9 can't be created
//----------------------------------------------------------------------------
/*#include <windows>*/
#include <Windows.h>
#include <dshow.h>
#include <d3d9.h>
#include <vmr9.h>
#include <list>
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

typedef std::list<IBaseFilter*> CFilterList;
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

BOOL VerifyVMR9(void);

BOOL IsWindowsMediaFile(WCHAR *lpszFile);

HRESULT RenderFileToVMR9(IGraphBuilder *pGB, WCHAR *wFileName,
                         IBaseFilter *pRenderer, BOOL bRenderAudio=TRUE);


#endif

