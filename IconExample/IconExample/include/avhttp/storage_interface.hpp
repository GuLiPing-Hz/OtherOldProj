//
// storage_interface.hpp
// ~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2013 Jack (jack dot wgm at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// path LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef AVHTTP_STORAGE_INTERFACE_HPP
#define AVHTTP_STORAGE_INTERFACE_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <boost/filesystem.hpp>
#include <boost/system/error_code.hpp>
#include <boost/cstdint.hpp>

namespace avhttp {

namespace fs = boost::filesystem;

// ���ݴ洢�ӿ�.
struct storage_interface
{
	storage_interface() {}
	virtual ~storage_interface() {}

	// �洢�����ʼ��.
	// @param file_pathָ�����ļ���·����Ϣ.
	// @param ec�ڳ���ʱ��������ϸ�Ĵ�����Ϣ.
	virtual void open(const fs::path& file_path, boost::system::error_code& ec) = 0;

	// �رմ洢���.
	virtual void close() = 0;

	// д�����ݵ��ļ�.
	// @param buf����Ҫд������ݻ���.
	// @param sizeָ����д������ݻ����С.
	// @����ֵΪʵ��д����ֽ���, ����-1��ʾд��ʧ��.
	// ��ע: ���ļ�ָ�뵱ǰλ��д��, д����ɽ��Զ��ƶ��ļ�ָ�뵽���λ��, ��֤��fwrite��Ϊһ��.
	virtual std::streamsize write(const char* buf, int size) = 0;

	// д������.
	// @param buf����Ҫд������ݻ���.
	// @param offset��д���ƫ��λ��.
	// @param sizeָ����д������ݻ����С.
	// @����ֵΪʵ��д����ֽ���, ����-1��ʾд��ʧ��.
	virtual std::streamsize write(const char* buf, boost::int64_t offset, int size) = 0;

	// ���ļ���ȡ����.
	// @param buf����Ҫ��ȡ�����ݻ���.
	// @param sizeָ���˶�ȡ�����ݻ����С.
	// @����ֵΪʵ�ʶ�ȡ���ֽ���, ����-1��ʾ��ȡʧ��.
	// ��ע: ���ļ�ָ�뵱ǰλ�ÿ�ʼ��ȡ, ��ȡ��ɽ��Զ��ƶ��ļ�ָ�뵽���λ��, ��֤��fread��Ϊһ��.
	virtual std::streamsize read(char* buf, int size) = 0;

	// ��ȡ����.
	// @param buf����Ҫ��ȡ�����ݻ���.
	// @param offset�Ƕ�ȡ��ƫ��λ��.
	// @param sizeָ���˶�ȡ�����ݻ����С.
	// @����ֵΪʵ�ʶ�ȡ���ֽ���, ����-1��ʾ��ȡʧ��.
	virtual std::streamsize read(char* buf, boost::int64_t offset, int size) = 0;

	// �ж��Ƿ��ļ�����.
	// ����ֵtrue��ʾ�ļ�����.
	virtual bool eof() = 0;
};

// �ض���storage_interface��������ָ��, ��multi_download�ڲ�ͨ������������ɴ���storage_interface.
typedef storage_interface* (*storage_constructor_type)();

}

#endif // AVHTTP_STORAGE_INTERFACE_HPP
