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
// 枚举filter
HRESULT EnumFilters (/*in*/IFilterGraph *pGraph);//遍历graph中的filter，并且显示filter的名字

//枚举pin
HRESULT GetPin(/*in*/IBaseFilter *pFilter, /*in*/BOOL in_bInput,/*out*/ IPin **ppPin);

//是否支持指定媒体类型
HRESULT PinIsSupportMedia(/*in*/IPin *pPin,/*in*/const AM_MEDIA_TYPE & am_md,/*out*/bool* pbIsSupport);

//注：有些filter是不能通过with CoCreateInstance方法创建的。例如AVI Compressor  Filter和WDM Video Capture filter
HRESULT AddFilterByCLSID(/*in*/IGraphBuilder *pGraph,	// Pointer to the Filter Graph Manager.
												/*in*/ const GUID& clsid,			// CLSID of the filter to create.
												/*in*/ LPCWSTR wszName,        // A name for the filter.
												/*out*/ IBaseFilter **ppF);				// Receives a pointer to the filter.
//获取未使用的Pin
HRESULT GetUnconnectedPin(/*in*/IBaseFilter *pFilter,   // Pointer to the filter.
													/*in*/BOOL in_bInput,//PIN_DIRECTION PinDir,   // Direction of the pin to find.PINDIR_OUTPUT/PINDIR_INPUT
													/*out*/IPin **ppPin);           // Receives a pointer to the pin.
//断开已连接的Pin
HRESULT DisConnectedPin(/*in*/IGraphBuilder *pGraph,
												/*in*/IBaseFilter *pFilter);
//连接output IPin与filter
HRESULT ConnectFilters(/*in*/IGraphBuilder *pGraph, // Filter Graph Manager.
										   /*in*/IPin *pOut,            // Output pin on the upstream filter.
										   /*in*/IBaseFilter *pDest,// Downstream filter.
										   /*in*/const AM_MEDIA_TYPE * inMediaType = NULL);    
//连接两个filter
HRESULT ConnectFilters( /*in*/IGraphBuilder *pGraph, // Filter Graph Manager.
										   /*in*/IBaseFilter *pSrc, //output IPin of filter
										   /*in*/IBaseFilter *pDest,//input IPin of filter
										   /*in*/const AM_MEDIA_TYPE * inMediaType = NULL);
//尝试连接pDest所有的input Pin，如果都不能连上，返回S_FALSE
HRESULT TryConnectFilter(/*in*/IGraphBuilder *pGraph, // Filter Graph Manager.
											 /*in*/IPin *pOut,            // Output pin on the upstream filter.
											 /*in*/IBaseFilter *pDest,// Downstream filter.)
											 /*in*/const AM_MEDIA_TYPE * inMediaType=0);   
//尝试pSrc所有的output Pin去连接所有的pDest的input Pin，都失败，返回S_FALSE
HRESULT TryConnectFilter(/*in*/IGraphBuilder *pGraph, 
											 /*in*/IBaseFilter *pSrc, 
											 /*in*/IBaseFilter *pDest, 
											 /*in*/const AM_MEDIA_TYPE * inMediaType=0);
//从Filter Graph获取指定GUID的filter
HRESULT FindFilterInterface(/*in*/IGraphBuilder *pGraph, // Pointer to the Filter Graph Manager.
												/*in*/REFGUID iid,           // IID of the interface to retrieve.
												/*out*/void **ppUnk);          // Receives the interface pointer.
//从Filter Graph获取指定GUID的Pin
HRESULT FindPinInterface(/*in*/IBaseFilter *pFilter,  // Pointer to the filter to search.
											 /*in*/REFGUID iid,           // IID of the interface.
											 /*out*/void **ppUnk);       // Receives the interface pointer.
//从Filter Graph获取指定GUID的 实例可能是filter或者pin
HRESULT FindInterfaceAnywhere(/*in*/ IGraphBuilder *pGraph, 
														  /*in*/REFGUID iid, 
														  /*out*/void **ppUnk);
//获取相连的filter
HRESULT GetNextFilter(/*in*/IBaseFilter *pFilter, // 开始的filter
										/*in*/BOOL in_bInput,//PIN_DIRECTION Dir,    // 搜索方向(upstream /input 还是 downstream/output)
										/*out*/IBaseFilter **ppNext); // Receives a pointer to the next filter.

typedef std::list<IBaseFilter*> CFilterList;
//添加不同的filter到列表中
void AddFilterUnique(CFilterList &FilterList, IBaseFilter *pNew);
//获取相同方向的filter
HRESULT GetPeerFilters(
					   /*in*/IBaseFilter *pFilter, // Pointer to the starting filter
					   /*in*/BOOL in_bInput,//PIN_DIRECTION Dir,    // Direction to search (upstream or downstream)
					   /*out*/CFilterList &FilterList);  // Collect the results in this list.
//默认释放filter，如果指定pGraph，则一并从pGraph上remove掉
void ReleaseFilter(/*in*/IBaseFilter* pFilter,/*in*/IGraphBuilder* pGraph=NULL);
//删除所有的filter
HRESULT RemoveAllFilter(/*in*/IGraphBuilder *pGraph); // Filter Graph Manager.

BOOL VerifyVMR9(void);

BOOL IsWindowsMediaFile(WCHAR *lpszFile);

HRESULT RenderFileToVMR9(IGraphBuilder *pGB, WCHAR *wFileName,
                         IBaseFilter *pRenderer, BOOL bRenderAudio=TRUE);


#endif

