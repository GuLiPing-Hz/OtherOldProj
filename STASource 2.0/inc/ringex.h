#pragma once
/**
	@file ringex.h
	@brief 环状内存池,加入读写同步控制,目前内存分配方式采用了 COM 内存管理机制.
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
		
		//关闭读写消息
		void			Close();

		//初始化
		bool			Initialize(const unsigned int size);

		/**
			@brief 用于数据记录.
			@param data 需要记录的数据.
			@param size 需要记录的数据长度(字节).
			@return 返回记录的字节数
		*/
		unsigned int	SetData(const byte* data,const unsigned int size,IWMReader* pReader);

		/**
			@brief 读取缓冲区中的数据
			@param data 需要记录读取数据的缓冲区.
			@param size 记录缓冲区大小(字节).
			@return 成功读取的数据长度(字节).
		*/
		unsigned int	Read(byte* data,const unsigned int& size);
		/**
			@brief 重置记录数据.
		*/
		void			Reset();

		//等待填充函数
		void			RingWaitFillEvent(unsigned int fillsize,DWORD time);
		/**
			@brief 可以获得的缓冲区数据长度.
			@return 返回缓冲区中可读取数据的长度.
		*/
		unsigned int	Length();

	private:
		byte*			m_buffer;	///< 缓冲区
		unsigned int	m_size;		///< 缓冲区大小
		Locker			m_locker;	///< 临界区锁
		bool			m_bQuit;	///< 强制退出控制

		CRingPos		m_read;
		CRingPos		m_write;

		HANDLE			m_fillevent;///< 填充事件
		DWORD			m_fillsize;	///< 需要等待的最少填充值
	};
}
