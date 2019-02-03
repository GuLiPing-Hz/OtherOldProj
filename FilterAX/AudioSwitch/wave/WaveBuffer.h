// WaveBuffer.h: interface for the CWaveBuffer class.
//

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
	//��õ����ú�����
	//������������е�pBuffer�����ó�Ա����m_pBuffer
	//�����m_pBufferԭ��Ϊ�գ�����pBuffer������m_pBuffer��֮�����ߵȳ�
	//�����m_pBuffer��Ϊ�գ�����pBuffer����m_pBuffer�еĵȳ����֣�
	//------��m_pBufferԭ���ĳ��ȱ�pBuffer������m_pBuffer�еĶ��������0��
	//------��m_pBufferԭ�����ȱ�pBuffer�̣���ֻ����ֵ��pBuffer�ж�Ӧֵ�����������Ȼά��m_pBuffer��ԭ�г���
	//ע��CopyBuffer�е�����SetNumSamples(),��SetNumSamples���ֵ�����SetBuffer()
	void CopyBuffer(void* pBuffer, DWORD dwNumSamples, int nSize = sizeof(short));

	//��һ���������SetBuffer���Ѻõ����ú�����
	//�ڶ��з���ָ����С���¿ռ����ʱָ�룬Ȼ�����SetBuffer()����m_pBuffer
	void SetNumSamples(DWORD dwNumSamples, int nSize = sizeof(short));

	//�൱����CWaveBuffer�ĳ�Ա�������ú�����
	//��������Ҫ���ǣ���pBuffer������CWaveBuffer�ĳ�Ա����m_pBuffer, 
	//void SetBuffer(void* pBuffer, DWORD dwNumSamples, int nSize);

	//��ԭ������β������µĻ��������ݣ������µĳ�������
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