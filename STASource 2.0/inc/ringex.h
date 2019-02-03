#pragma once
/**
	@file ringex.h
	@brief ��״�ڴ��,�����дͬ������,Ŀǰ�ڴ���䷽ʽ������ COM �ڴ�������.
*/

namespace Burning
{
	class  CRingPos
	{
	public:
		CRingPos();
		~CRingPos();
		bool			IsOk();
		void			clear();
		void			RingPosWaitEvent(DWORD time = INFINITE);
		void			RingPosSetEvent();
	public:
		unsigned int	m_nPos;
		HANDLE			m_hEvent;
		bool			m_bOk;
		unsigned int	m_nRound;

	};

	class CRingEx
	{
	public:
		CRingEx();

		~CRingEx();
		
		//�رն�д��Ϣ
		void			Close();

		//��ʼ��
		bool			Initialize(const unsigned int size);

		/**
			@brief �������ݼ�¼.
			@param data ��Ҫ��¼������.
			@param size ��Ҫ��¼�����ݳ���(�ֽ�).
			@return ���ؼ�¼���ֽ���
		*/
		unsigned int	SetData(const byte* data,const unsigned int size,IWMReader* pReader);

		/**
			@brief ��ȡ�������е�����
			@param data ��Ҫ��¼��ȡ���ݵĻ�����.
			@param size ��¼��������С(�ֽ�).
			@return �ɹ���ȡ�����ݳ���(�ֽ�).
		*/
		unsigned int	Read(byte* data,const unsigned int& size);
		/**
			@brief ���ü�¼����.
		*/
		void			Reset();

		//�ȴ���亯��
		void			RingWaitFillEvent(unsigned int fillsize,DWORD time);
		/**
			@brief ���Ի�õĻ��������ݳ���.
			@return ���ػ������пɶ�ȡ���ݵĳ���.
		*/
		unsigned int	Length();

	private:
		byte*			m_buffer;	///< ������
		unsigned int	m_size;		///< ��������С
		Locker			m_locker;	///< �ٽ�����
		bool			m_bQuit;	///< ǿ���˳�����

		CRingPos		m_read;
		CRingPos		m_write;

		HANDLE			m_fillevent;///< ����¼�
		DWORD			m_fillsize;	///< ��Ҫ�ȴ����������ֵ
	};
}
