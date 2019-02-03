#pragma once
#include <stdio.h>

namespace Burning
{
	enum _eFileOpenFlag
	{
		FOP_READ	= 0,
		FOP_WRITE
	};

	enum _eFileOpenReturn
	{
		FILE_CANTOPEN = 0,
		FILE_OPEN
	};

	class CFile
	{
	public:
		CFile(void);

		~CFile(void);

		_eFileOpenReturn Open( LPCTSTR pszFileName, _eFileOpenFlag flag );

		long GetSize();

		long GetSizeEx();

		void Skin(long pos);

		size_t Read( void * pBuf, size_t s );

	private:
		FILE *	m_pf;
	};
};