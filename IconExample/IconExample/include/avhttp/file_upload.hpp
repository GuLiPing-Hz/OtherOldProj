//
// upload.hpp
// ~~~~~~~~~~
//
// Copyright (c) 2003, Arvid Norberg All rights reserved.
// Copyright (c) 2013 Jack (jack dot wgm at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// path LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef AVHTTP_UPLOAD_HPP
#define AVHTTP_UPLOAD_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <boost/noncopyable.hpp>
#include "avhttp/http_stream.hpp"

BOOST_STATIC_ASSERT_MSG(BOOST_VERSION >= 105400, "You must use boost-1.54 or later!!!");

namespace avhttp {

// WebForm�ļ��ϴ����.
// ����RFC1867(http://www.servlets.com/rfcs/rfc1867.txt)ʵ��.
// @begin example
// 	boost::asio::io_service io;
// 	avhttp::file_upload upload(io);
// 	avhttp::file_upload::form_agrs fields;
// 	fields["username"] = "Cai";
// 	boost::system::error_code ec;
// 	upload.open("http://example.upload/upload", "cppStudy.tar.bz2",
// 		"file", fields, ec);
// 	if (ec)
// 	{
// 		// �������.
// 	}
// 	// ��ʼ�ϴ��ļ�����.
// 	avhttp::default_storge file;
// 	file.open("\\root\\cppStudy.tar.bz2", ec);
// 	if (ec)
// 	{
// 		// �������.
// 	}
//
// 	boost::array<char, 1024> buffer;
// 	while (!file.eof())
// 	{
// 		int readed = file.read(buffer.data(), 1024);
// 		boost::asio::write(upload, boost::asio::buffer(buffer, readed), ec);
// 		if (ec)
// 		{
// 			// �������.
// 		}
// 	}
// 	upload.write_tail(ec);
// 	if (ec)
// 	{
// 		// �������.
// 	}
//  ...
// @end example
class file_upload : public boost::noncopyable
{
public:
	typedef std::map<std::string, std::string> form_args;

	///Constructor.
	// @param io�û�ָ����io_service����.
	// @param fake_continueָ������fake-continue��Ϣ.
	// fake continue��Ϣ������http��������֧��100��ʱ��,
	// ��async_open/open��ʱ, ����һ��fake continue.
	AVHTTP_DECL explicit file_upload(boost::asio::io_service& io, bool fake_continue = false);

	/// Destructor.
	AVHTTP_DECL virtual ~file_upload();

	///�첽���ļ��ϴ�.
	template <typename Handler>
	void async_open(const std::string& url, const std::string& filename,
		const std::string& file_of_form, const form_args& args, BOOST_ASIO_MOVE_ARG(Handler) handler);

	///���ļ��ϴ�.
	// @param urlָ���ϴ��ļ���url.
	// @param filenameָ�����ϴ��ļ���.
	// @param file_of_form��web form�е��ϴ��ļ����ֶ�.
	// @param args�������ϴ��ļ��ֶ�.
	// @param ec������Ϣ.
	// @begin example
	//  avhttp::file_upload f(io_service);
	//  file_upload::form_agrs fields;
	//  fields["username"] = "Cai";
	//  boost::system::error_code ec;
	//  f.open("http://example.upload/upload.cgi", "cppStudy.tar.bz2",
	//    "file", fields, ec);
	// @end example
	AVHTTP_DECL void open(const std::string& url, const std::string& filename,
		const std::string& file_of_form, const form_args& args, boost::system::error_code& ec);

	///���ļ��ϴ�.
	// @param urlָ���ϴ��ļ���url.
	// @param filenameָ�����ϴ��ļ���.
	// @param file_of_form��web form�е��ϴ��ļ����ֶ�.
	// @param args�������ϴ��ļ��ֶ�.
	// ʧ�ܽ��׳�һ��boost::system::system_error�쳣.
	// @begin example
	//  avhttp::file_upload f(io_service);
	//  file_upload::form_agrs fields;
	//  fields["username"] = "Cai";
	//  boost::system::error_code ec;
	//  f.open("http://example.upload/upload.cgi", "cppStudy.tar.bz2",
	//    "file", fields, ec);
	// @end example
	AVHTTP_DECL void open(const std::string& url, const std::string& filename,
		const std::string& file_of_form, const form_args& agrs);

	///����һЩ�ϴ����ļ�����.
	// @param buffers��һ���������ڷ������ݻ���. ������ͱ�������ConstBufferSequence, �ο��ĵ�:
	// http://www.boost.org/doc/libs/1_54_0/doc/html/boost_asio/reference/ConstBufferSequence.html
	// @����ʵ�ַ��͵����ݴ�С.
	// @��ע: �ú�������������һֱ�ȴ����ݱ����ͻ�������ʱ�ŷ���.
	// write_some����֤��������������, �û���Ҫ���ݷ���ֵ��ȷ���Ѿ����͵����ݴ�С.
	// @begin example
	//  try
	//  {
	//    std::size bytes_transferred = f.write_some(boost::asio::buffer(data, size));
	//  }
	//  catch (boost::asio::system_error& e)
	//  {
	//    std::cerr << e.what() << std::endl;
	//  }
	//  ...
	// @end example
	// ����ʾ���е�boost::asio::buffer�÷����Բο�boost�е��ĵ�. �����Խ���һ��
	// boost.array��std.vector��Ϊ��������.
	template <typename ConstBufferSequence>
	std::size_t write_some(const ConstBufferSequence& buffers);

	///����һЩ�ϴ����ļ�����.
	// @param buffers��һ���������ڷ������ݻ���. ������ͱ�������ConstBufferSequence, �ο��ĵ�:
	// http://www.boost.org/doc/libs/1_54_0/doc/html/boost_asio/reference/ConstBufferSequence.html
	// @����ʵ�ַ��͵����ݴ�С.
	// @��ע: �ú�������������һֱ�ȴ����ݱ����ͻ�������ʱ�ŷ���.
	// write_some����֤��������������, �û���Ҫ���ݷ���ֵ��ȷ���Ѿ����͵����ݴ�С.
	// @begin example
	//  boost::system::error_code ec;
	//  std::size bytes_transferred = f.write_some(boost::asio::buffer(data, size), ec);
	//  ...
	// @end example
	// ����ʾ���е�boost::asio::buffer�÷����Բο�boost�е��ĵ�. �����Խ���һ��
	// boost.array��std.vector��Ϊ��������.
	template <typename ConstBufferSequence>
	std::size_t write_some(const ConstBufferSequence& buffers,
		boost::system::error_code& ec);

	///�첽�ϴ�һЩ�ļ�����.
	// @param buffersһ���������ڶ�ȡ���ݵĻ�����, ������ͱ�������ConstBufferSequence,
	//  ConstBufferSequence�Ķ�����boost.asio�ĵ���.
	// http://www.boost.org/doc/libs/1_54_0/doc/html/boost_asio/reference/ConstBufferSequence.html
	// @param handler�ڷ��Ͳ�����ɻ���ִ���ʱ, �����ص�, ��������������:
	// @begin code
	//  void handler(
	//    int bytes_transferred,				// ���ط��͵������ֽ���.
	//    const boost::system::error_code& ec	// ���ڷ��ز���״̬.
	//  );
	// @end code
	// @begin example
	//   void handler(int bytes_transferred, const boost::system::error_code& ec)
	//   {
	//		// �����첽�ص�.
	//   }
	//   file_upload f(io_service);
	//   ...
	//   f.async_write_some(boost::asio::buffer(data, size), handler);
	//   ...
	// @end example
	// ����ʾ���е�boost::asio::buffer�÷����Բο�boost�е��ĵ�. �����Խ���һ��
	// boost.array��std.vector��Ϊ��������.
	template <typename ConstBufferSequence, typename Handler>
	void async_write_some(const ConstBufferSequence& buffers, BOOST_ASIO_MOVE_ARG(Handler) handler);

	///���ͽ�β��.
	// @param ec������Ϣ.
	AVHTTP_DECL void write_tail(boost::system::error_code& ec);

	///���ͽ�β��.
	// ʧ�ܽ��׳�һ��boost::system::system_error�쳣.
	AVHTTP_DECL void write_tail();

	///�첽���ͽ�β��.
	// @param handler�ڷ��Ͳ�����ɻ���ִ���ʱ, �����ص�, ��������������:
	// @begin code
	//  void handler(
	//    const boost::system::error_code& ec // ���ڷ��ز���״̬.
	//  );
	// @end code
	// @begin example
	//  void tail_handler(const boost::system::error_code& ec)
	//  {
	//    if (!ec)
	//    {
	//      // ���ͳɹ�!
	//    }
	//  }
	//  ...
	//  avhttp::file_upload f(io_service);
	//  ...
	//  f.async_write_tail(handler);
	// @end example
	// @��ע: handlerҲ����ʹ��boost.bind����һ�����Ϲ涨�ĺ�����
	// Ϊasync_open�Ĳ���handler.
	template <typename Handler>
	void async_write_tail(BOOST_ASIO_MOVE_ARG(Handler) handler);

	///����http headerѡ��.
	AVHTTP_DECL void request_option(request_opts& opts);

	///����http_stream���������.
	AVHTTP_DECL http_stream& get_http_stream();

private:

	template <typename Handler>
	struct open_coro;

	template <typename Handler>
	struct tail_coro;

	///�������������ڴ���Э�̲����� Templet type deduction.
	template <typename Handler>
	open_coro<Handler> make_open_coro(const std::string& url, const std::string& filename,
		const std::string& file_of_form, const form_args& args, BOOST_ASIO_MOVE_ARG(Handler) handler);

	///�������������ڴ���Э�̲����� Templet type deduction.
	template <typename Handler>
	tail_coro<Handler> make_tail_coro(BOOST_ASIO_MOVE_ARG(Handler) handler);

private:

	// io_service����.
	boost::asio::io_service& m_io_service;

	// http_stream����.
	http_stream m_http_stream;

	// �߽��.
	std::string m_boundary;

	// ������.
	form_args m_form_args;

	// �Ƿ�����fake-continue��Ϣ.
	// fake continue��Ϣ������http��
	// ������֧��100��ʱ��, ��
	// async_open/open��ʱ, ����һ��
	// fake continue.
	bool m_fake_continue;
};

} // namespace avhttp

#if defined(AVHTTP_HEADER_ONLY)
#	include "avhttp/impl/file_upload.cpp"
#endif

#endif // AVHTTP_UPLOAD_HPP
