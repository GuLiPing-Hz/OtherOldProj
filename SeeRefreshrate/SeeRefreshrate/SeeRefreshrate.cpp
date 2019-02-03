// SeeRefreshrate.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <dxgi.h>
#include <d3d9.h>
#include <d3dx9.h>

#ifndef SAFE_RELEASE
#define SAFE_RELEASE(x)  do{if((x)) \
	(x)->Release(); (x)=NULL; }while(0)
#endif

#ifndef SAFE_DELETE
#define SAFE_DELETE(x) \
	if ((x) != NULL)      \
{                   \
	delete (x);        \
	(x) = NULL;        \
}
#endif


bool GetRefreshRate( unsigned int screenWidth, unsigned int screenHeight, unsigned int &  Numerator, unsigned int & Denominator )
{
	HRESULT hr;
	IDXGIFactory* factory;
	IDXGIAdapter* adapter;
	IDXGIOutput* adapterOutput;
	unsigned int numModes, i;
	DXGI_MODE_DESC* displayModeList;
	//DXGI_SWAP_CHAIN_DESC swapChainDesc;

	hr = CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)&factory);
	if(FAILED(hr))
	{
		return false;
	}

	// Use the factory to create an adapter for the primary graphics interface (video card).
	hr = factory->EnumAdapters(0, &adapter);
	if(FAILED(hr))
	{
		return false;
	}

	// Enumerate the primary adapter output (monitor).
	if (adapter->EnumOutputs(1, &adapterOutput) == DXGI_ERROR_NOT_FOUND)//first find the second monitor
	{
		hr = adapter->EnumOutputs(0, &adapterOutput);//find the primary monitor
		if(FAILED(hr))
		{
			return false;
		}
	}

	// Get the number of modes that fit the DXGI_FORMAT_R8G8B8A8_UNORM display format for the adapter output (monitor).
	hr = adapterOutput->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &numModes, NULL);
	if(FAILED(hr))
	{
		return false;
	}

	// Create a list to hold all the possible display modes for this monitor/video card combination.
	displayModeList = new DXGI_MODE_DESC[numModes];
	if(!displayModeList)
	{
		return false;
	}

	// Now fill the display mode list structures.
	hr = adapterOutput->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &numModes, displayModeList);
	if(FAILED(hr))
	{
		return false;
	}

	// Now go through all the display modes and find the one that matches the screen width and height.
	// When a match is found store the numerator and denominator of the refresh rate for that monitor.
	for(i=0; i<numModes; i++)
	{
		if ( screenWidth == displayModeList[i].Width && screenHeight == displayModeList[i].Height )
		{
			Numerator = displayModeList[i].RefreshRate.Numerator;
			Denominator = displayModeList[i].RefreshRate.Denominator;
			return true;
		}
	}

	SAFE_DELETE(displayModeList);
	SAFE_RELEASE(adapterOutput);
	SAFE_RELEASE(adapter);
	SAFE_RELEASE(factory);

	return false;
}

bool GetRefreshRate2(UINT& RefreshRate)
{
	IDirect3D9* pD3d = Direct3DCreate9(D3D_SDK_VERSION);
	if (!pD3d)
	{
		return false;
	}
	D3DDISPLAYMODE dm;
	pD3d->GetAdapterDisplayMode(0,&dm);
	RefreshRate = dm.RefreshRate;
	SAFE_RELEASE(pD3d);
	return true;
}

int _tmain(int argc, _TCHAR* argv[])
{
	int width = GetSystemMetrics(SM_CXSCREEN);
	int height = GetSystemMetrics(SM_CYSCREEN);
	unsigned int numerator;
	unsigned int denominator;
	GetRefreshRate(width,height,numerator,denominator);
	printf("first way:帧率计算：单位时间（%d），单位帧（%d），帧率（%06f）\n",denominator,numerator,numerator*1.0f/denominator);
	UINT refresh;
	GetRefreshRate2(refresh);
	printf("second way:帧率(%d)\n",refresh);
	getchar();
	return 0;
}

