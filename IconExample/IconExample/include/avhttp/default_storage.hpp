//
// default_storage.hpp
// ~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2013 Jack (jack dot wgm at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// path LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef AVHTTP_DEFAULT_STORAGE_HPP
#define AVHTTP_DEFAULT_STORAGE_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "avhttp/file.hpp"
#include "avhttp/storage_interface.hpp"

namespace avhttp {

class default_storge : public storage_interface
{
public:
	default_storge()
	{}

	virtual ~default_storge()
	{}

	// �洢�����ʼ��.
	// @param file_pathָ�����ļ���·����Ϣ.
	// @param ec�ڳ���ʱ��������ϸ�Ĵ�����Ϣ.
	virtual void open(const fs::path& file_path, boost::system::error_code& ec)
	{
		m_file.open(file_path, file::read_write, ec);
	}

	// �رմ洢���.
	virtual void close()
	{
		m_file.close();
	}

	// д�����ݵ��ļ�.
	// @param buf����Ҫд������ݻ���.
	// @param sizeָ����д������ݻ����С.
	// @����ֵΪʵ��д����ֽ���, ����-1��ʾд��ʧ��.
	// ��ע: ���ļ�ָ�뵱ǰλ��д��, д����ɽ��Զ��ƶ��ļ�ָ�뵽���λ��, ��֤��fwrite��Ϊһ��.
	virtual std::streamsize write(const char* buf, int size)
	{
		return m_file.write(buf, size);
	}

	// д������.
	// @param buf����Ҫд������ݻ���.
	// @param offset��д���ƫ��λ��.
	// @param sizeָ����д������ݻ����С.
	// @����ֵΪʵ��д����ֽ���, ����-1��ʾд��ʧ��.
	virtual std::streamsize write(const char* buf, boost::int64_t offset, int size)
	{
		return m_file.write(offset, buf, size);
	}

	// ���ļ���ȡ����.
	// @param buf����Ҫ��ȡ�����ݻ���.
	// @param sizeָ���˶�ȡ�����ݻ����С.
	// @����ֵΪʵ�ʶ�ȡ���ֽ���, ����-1��ʾ��ȡʧ��.
	// ��ע: ���ļ�ָ�뵱ǰλ�ÿ�ʼ��ȡ, ��ȡ��ɽ��Զ��ƶ��ļ�ָ�뵽���λ��, ��֤��fread��Ϊһ��.
	virtual std::streamsize read(char* buf, int size)
	{
		return m_file.read(buf, size);
	}

	// ��ȡ����.
	// @param buf����Ҫ��ȡ�����ݻ���.
	// @param offset�Ƕ�ȡ��ƫ��λ��.
	// @param sizeָ���˶�ȡ�����ݻ����С.
	// @����ֵΪʵ�ʶ�ȡ���ֽ���, ����-1��ʾ��ȡʧ��.
	virtual std::streamsize read(char* buf, boost::int64_t offset, int size)
	{
		return m_file.read(offset, buf, size);
	}

	// �ж��Ƿ��ļ�����.
	// ����ֵtrue��ʾ�ļ�����.
	virtual bool eof()
	{
		char tmp;
		boost::system::error_code ec;
		boost::int64_t offset = m_file.offset(ec);
		BOOST_ASSERT(!ec);
		int ret = m_file.read(&tmp, 1);
		m_file.offset(offset, ec);
		BOOST_ASSERT(!ec);
		return ret == 0 ? true : false;
	}

protected:
	file m_file;
};

// Ĭ�ϴ洢����.
static storage_interface* default_storage_constructor()
{
	return new default_storge();
}

} // namespace avhttp

#endif // AVHTTP_DEFAULT_STORAGE_HPP
