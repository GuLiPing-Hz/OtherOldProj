#include "stdafx.h"
#include "File.h"
#include <Stream.h>
#include<sys/stat.h>

using namespace Burning;

CFile::CFile(void)
	: m_pf( NULL )
{

}

CFile::~CFile(void)
{
}

_eFileOpenReturn CFile::Open( LPCTSTR pszFileName, _eFileOpenFlag flag )
{
	if ( FOP_READ == flag ) 
	{
		m_pf = _tfopen( pszFileName, TEXT("rb") );
	}
	else if ( FOP_WRITE == flag ) 
	{
		m_pf = _tfopen( pszFileName, TEXT("wb") );
	}

	return ( ( m_pf != NULL ) ? FILE_OPEN : FILE_CANTOPEN);
}

long CFile::GetSize()
{
	if ( NULL == m_pf ) 
	{
		return 0;
	}

	struct stat statbuf;
	fstat( _fileno(m_pf), &statbuf );
	return ( statbuf.st_size );
}

long CFile::GetSizeEx()
{
	if ( NULL == m_pf ) 
	{
		return 0;
	}

	struct stat statbuf;
	fstat( fileno(m_pf), &statbuf );
	return ( statbuf.st_size );
}

void CFile::Skin(long pos)
{
	if ( NULL == m_pf ) 
	{
		return;
	}

	fseek( m_pf, pos, SEEK_BEG );
}

size_t CFile::Read( void * pBuf, size_t s )
{
	size_t r = fread( pBuf, 1, s, m_pf );
	return r;
}

