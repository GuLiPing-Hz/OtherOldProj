/*
	注释时间:2014-4-25
	author: glp
	wave 数据保存类
*/

#ifndef WAVEBUFFER__H__
#define WAVEBUFFER__H__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <Windows.h>
#include <MMSystem.h>
#include <assert.h>

class CWaveBuffer  
{
public:
	CWaveBuffer(DWORD bufferSize=4*1024*1024);
	virtual ~CWaveBuffer();

	int GetSampleSize() const;
	DWORD GetNumSamples() const;
	void* GetBuffer() const;
	DWORD GetCurTotalBufferSize()const {return m_uBufferSize;}
	//最常用的设置函数：
	//根据输入参数中的pBuffer来设置成员变量m_pBuffer
	//如果：m_pBuffer原来为空，则：用pBuffer来设置m_pBuffer，之后两者等长
	//如果：m_pBuffer不为空，则：用pBuffer覆盖m_pBuffer中的等长部分；
	//------若m_pBuffer原来的长度比pBuffer长，则，m_pBuffer中的多出部分清0；
	//------若m_pBuffer原来长度比pBuffer短，则只将其值用pBuffer中对应值替代，长度仍然维持m_pBuffer的原有长度
	//注：CopyBuffer中调用了SetNumSamples(),而SetNumSamples中又调用了SetBuffer()
	void CopyBuffer(void* pBuffer, DWORD dwNumSamples, int nSize = sizeof(short));

	//是一个比下面的SetBuffer更友好的设置函数：
	//在堆中分配指定大小的新空间给临时指针，然后调用SetBuffer()赋给m_pBuffer
	void SetNumSamples(DWORD dwNumSamples, int nSize = sizeof(short));

	//相当于是CWaveBuffer的成员变量设置函数：
	//其中最主要的是：将pBuffer赋给类CWaveBuffer的成员变量m_pBuffer, 
	//void SetBuffer(void* pBuffer, DWORD dwNumSamples, int nSize);

	//在原缓冲区尾部添加新的缓冲区数据，生成新的长缓冲区
	void AddBuffer(void* pBuffer, DWORD dwNumSamples, int nSize );
	void ResetBuffer();

private:
	int			m_nSampleSize;
	void*		m_pBuffer;
	DWORD	m_dwNum;
	DWORD	m_uCurPos;
	DWORD	m_uBufferSize;
};

#endif //WAVEBUFFER__H__