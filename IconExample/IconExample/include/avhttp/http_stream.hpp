//
// http_stream.hpp
// ~~~~~~~~~~~~~~~
//
// Copyright (c) 2013 Jack (jack dot wgm at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef AVHTTP_HTTP_STREAM_HPP
#define AVHTTP_HTTP_STREAM_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <vector>
#include <cstring>		// for std::strcmp/std::strlen
#include <streambuf>	// support streambuf.

#include <boost/array.hpp>
#include <boost/shared_array.hpp>

#include "avhttp/url.hpp"
#include "avhttp/settings.hpp"
#include "avhttp/detail/io.hpp"
#include "avhttp/detail/parsers.hpp"
#include "avhttp/detail/error_codec.hpp"
#include "avhttp/cookie.hpp"
#ifdef AVHTTP_ENABLE_OPENSSL
#include "avhttp/detail/ssl_stream.hpp"
#endif
#ifdef AVHTTP_ENABLE_ZLIB
extern "C"
{
#include "zlib.h"
#ifndef z_const
# define z_const
#endif
}
#endif

#include "avhttp/detail/socket_type.hpp"
#include "avhttp/detail/utf8.hpp"


namespace avhttp {

// һ��http����ʵ��, ����ͬ�����첽����һ��ָ����url�ϵ�����.
// Ŀǰ֧��http/httpsЭ��.
// @��ע: ����http_stream�Ķ�����̰߳�ȫ!
// ������ͬ����ʽ����һ��url�е�����ʹ��ʾ��.
// @begin example
//  try
//  {
//  	boost::asio::io_service io_service;
//  	avhttp::http_stream h(io_service);
//  	avhttp::request_opts opt;
//
//  	// ��������ѡ��.
//  	opt.insert("Connection", "close");
//  	h.request_options(opt);
//  	h.open("http://www.boost.org/LICENSE_1_0.txt");
//
//  	boost::system::error_code ec;
//  	while (!ec)
//  	{
//  		char data[1024];
//  		std::size_t bytes_transferred = h.read_some(boost::asio::buffer(data, 1024), ec);
//			// ���Ҫ��ȡָ����С������, ����ʹ��boost::asio::read, ����:
//			// std::size_t bytes_transferred = boost::asio::read(h, boost::asio::buffer(buf), ec);
//  		std::cout.write(data, bytes_transferred);
//  	}
//  }
//  catch (std::exception& e)
//  {
//  	std::cerr << "Exception: " << e.what() << std::endl;
//  }
// @end example
//
// �������첽��ʽ����һ��url�е�����ʹ��ʾ��.
// @begin example
//  class downloader
//  {
//  public:
//  	downloader(boost::asio::io_service& io)
//  		: m_io_service(io)
//  		, m_stream(io)
//  	{
//  		// ��������ѡ��.
//  		avhttp::request_opts opt;
//  		opt.insert("Connection", "close");
//  		m_stream.request_options(opt);
//			// �����첽����.
//  		m_stream.async_open("http://www.boost.org/LICENSE_1_0.txt",
//  			boost::bind(&downloader::handle_open, this, boost::asio::placeholders::error));
//  	}
//  	~downloader()
//  	{}
//  
//  public:
//  	void handle_open(const boost::system::error_code& ec)
//  	{
//  		if (!ec)
//  		{
//  			m_stream.async_read_some(boost::asio::buffer(m_buffer),
//  				boost::bind(&downloader::handle_read, this,
//  				boost::asio::placeholders::bytes_transferred,
//  				boost::asio::placeholders::error));
//				// ������Ҳ֧��ʹ��boost::asio::async_read����ȡһ������С������, �÷���boost.asio, ����:
//				boost::asio::async_read(m_stream, boost::asio::buffer(m_buffer),
// 					boost::bind(&downloader::handle_read, this,
// 					boost::asio::placeholders::bytes_transferred,
// 					boost::asio::placeholders::error));
//  		}
//  	}
//  
//  	void handle_read(int bytes_transferred, const boost::system::error_code& ec)
//  	{
//  		if (!ec)
//  		{
//  			std::cout.write(m_buffer.data(), bytes_transferred);
//  			m_stream.async_read_some(boost::asio::buffer(m_buffer),
//  				boost::bind(&downloader::handle_read, this,
//  				boost::asio::placeholders::bytes_transferred,
//  				boost::asio::placeholders::error));
//  		}
//  	}
//  
//  private:
//  	boost::asio::io_service& m_io_service;
//  	avhttp::http_stream m_stream;
//  	boost::array<char, 1024> m_buffer;
//  };
//
//  int main(int argc, char* argv[])
//  {
//		boost::asio::io_service io;
//		downloader d(io);
//		io.run();
//		return 0;
//  }
// @end example
//
// ������ͨ����׼����ʽ����http��ʾ��.
// @begin example
// 	int main(int argc, char* argv[])
// 	{
// 		boost::asio::io_service io;
// 		avhttp::http_stream h(io);
//
// 		h.open("http://www.boost.org/LICENSE_1_0.txt");
// 		std::istream in(&h);
// 		std::string s;
// 		while (in >> s)
// 			std::cout << s;
//
// 		return 0;
// 	}
// @end example
//
// ��һ��ͨ�����ַ�������ʽֱ�ӷ���.
// @begin example
// 	int main(int argc, char* argv[])
// 	{
// 		boost::asio::io_service io;
// 		avhttp::http_stream h(io);
//
// 		h.open("http://www.boost.org/LICENSE_1_0.txt");
// 		std::cout <<& h;
//
// 		return 0;
// 	}
// @end example


using boost::asio::ip::tcp;

class http_stream
	: public std::streambuf
	, public boost::noncopyable
{
public:

	/// Constructor.
	AVHTTP_DECL explicit http_stream(boost::asio::io_service& io);

	/// Destructor.
	AVHTTP_DECL virtual ~http_stream();

	///��һ��ָ����url.
	// ʧ�ܽ��׳�һ��boost::system::system_error�쳣.
	// @param u ��Ҫ�򿪵�URL.
	// @begin example
	//   avhttp::http_stream h(io_service);
	//   try
	//   {
	//     h.open("http://www.boost.org");
	//   }
	//   catch (boost::system::system_error& e)
	//   {
	//     std::cerr << e.what() << std::endl;
	//   }
	// @end example
	AVHTTP_DECL void open(const url& u);

	///��һ��ָ����url.
	// @param u ��Ҫ�򿪵�URL.
	// ͨ��ec���û��ִ��״̬.
	// @begin example
	//   avhttp::http_stream h(io_service);
	//   boost::system::error_code ec;
	//   h.open("http://www.boost.org", ec);
	//   if (ec)
	//   {
	//     std::cerr << e.what() << std::endl;
	//   }
	// @end example
	AVHTTP_DECL void open(const url& u, boost::system::error_code& ec);

	///�첽��һ��ָ����URL.
	// @param u ��Ҫ�򿪵�URL.
	// @param handler ���������ڴ����ʱ. ������������������:
	// @begin code
	//  void handler(
	//    const boost::system::error_code& ec // ���ڷ��ز���״̬.
	//  );
	// @end code
	// @begin example
	//  void open_handler(const boost::system::error_code& ec)
	//  {
	//    if (!ec)
	//    {
	//      // �򿪳ɹ�!
	//    }
	//  }
	//  ...
	//  avhttp::http_stream h(io_service);
	//  h.async_open("http://www.boost.org", open_handler);
	// @end example
	// @��ע: handlerҲ����ʹ��boost.bind����һ�����Ϲ涨�ĺ�����
	// Ϊasync_open�Ĳ���handler.
	template <typename Handler>
	void async_open(const url& u, BOOST_ASIO_MOVE_ARG(Handler) handler);

	///�����http_stream�ж�ȡһЩ����.
	// @param buffersһ��������ȡ���ݵĻ�����, ������ͱ�������MutableBufferSequence,
	// MutableBufferSequence�Ķ�����boost.asio�ĵ���.
	// @�������ض�ȡ�������ݴ�С.
	// @ʧ�ܽ��׳�boost::asio::system_error�쳣.
	// @��ע: �ú�������������һֱ�ȴ������ݻ�������ʱ�ŷ���.
	// read_some���ܶ�ȡָ����С������.
	// @begin example
	//  try
	//  {
	//    std::size bytes_transferred = h.read_some(boost::asio::buffer(data, size));
	//  } catch (boost::asio::system_error& e)
	//  {
	//    std::cerr << e.what() << std::endl;
	//  }
	//  ...
	// @end example
	template <typename MutableBufferSequence>
	std::size_t read_some(const MutableBufferSequence& buffers);

	///�����http_stream��ȡһЩ����.
	// @param buffersһ���������ڶ�ȡ���ݵĻ�����, ������ͱ�������
	// MutableBufferSequence, MutableBufferSequence�Ķ�����boost.asio
	// �ĵ���.
	// @param ec�ڷ�������ʱ, �����ش�����Ϣ.
	// @�������ض�ȡ�������ݴ�С.
	// @��ע: �ú�������������һֱ�ȴ������ݻ�������ʱ�ŷ���.
	// read_some���ܶ�ȡָ����С������.
	// @begin example
	//  boost::system::error_code ec;
	//  std::size bytes_transferred = h.read_some(boost::asio::buffer(data, size), ec);
	//  ...
	// @end example
	// ����ʾ���е�boost::asio::buffer�÷����Բο�boost�е��ĵ�. �����Խ���һ��
	// boost.array��std.vector��Ϊ��������.
	template <typename MutableBufferSequence>
	std::size_t read_some(const MutableBufferSequence& buffers,
		boost::system::error_code& ec);

	///�����http_stream�첽��ȡһЩ����.
	// @param buffersһ���������ڶ�ȡ���ݵĻ�����, ������ͱ�������MutableBufferSequence,
	//  MutableBufferSequence�Ķ�����boost.asio�ĵ���.
	// http://www.boost.org/doc/libs/1_54_0/doc/html/boost_asio/reference/MutableBufferSequence.html
	// @param handler�ڶ�ȡ������ɻ���ִ���ʱ, �����ص�, ��������������:
	// @begin code
	//  void handler(
	//    const boost::system::error_code& ec,	// ���ڷ��ز���״̬.
	//    std::size_t bytes_transferred			// ���ض�ȡ�������ֽ���.
	//  );
	// @end code
	// @begin example
	//   void handler(const boost::system::error_code& ec, std::size_t bytes_transferred)
	//   {
	//		// �����첽�ص�.
	//   }
	//   http_stream h(io_service);
	//   ...
	//   boost::array<char, 1024> buffer;
	//   boost::asio::async_read(h, boost::asio::buffer(buffer), handler);
	//   ...
	// @end example
	// ����ʾ���е�boost::asio::buffer�÷����Բο�boost�е��ĵ�. �����Խ���һ��
	// boost.array��std.vector��Ϊ��������.
	template <typename MutableBufferSequence, typename Handler>
	void async_read_some(const MutableBufferSequence& buffers, BOOST_ASIO_MOVE_ARG(Handler) handler);

	///�����http_stream�з���һЩ����.
	// @param buffers��һ���������ڷ������ݻ���. ������ͱ�������ConstBufferSequence, �ο��ĵ�:
	// http://www.boost.org/doc/libs/1_54_0/doc/html/boost_asio/reference/ConstBufferSequence.html
	// @����ʵ�ַ��͵����ݴ�С.
	// @��ע: �ú�������������һֱ�ȴ����ݱ����ͻ�������ʱ�ŷ���.
	// write_some����֤��������������, �û���Ҫ���ݷ���ֵ��ȷ���Ѿ����͵����ݴ�С.
	// @begin example
	//  try
	//  {
	//    std::size bytes_transferred = h.write_some(boost::asio::buffer(data, size));
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

	///�����http_stream�з���һЩ����.
	// @param buffers��һ���������ڷ������ݻ���. ������ͱ�������ConstBufferSequence, �ο��ĵ�:
	// http://www.boost.org/doc/libs/1_54_0/doc/html/boost_asio/reference/ConstBufferSequence.html
	// @����ʵ�ַ��͵����ݴ�С.
	// @��ע: �ú�������������һֱ�ȴ����ݱ����ͻ�������ʱ�ŷ���.
	// write_some����֤��������������, �û���Ҫ���ݷ���ֵ��ȷ���Ѿ����͵����ݴ�С.
	// @begin example
	//  boost::system::error_code ec;
	//  std::size bytes_transferred = h.write_some(boost::asio::buffer(data, size), ec);
	//  ...
	// @end example
	// ����ʾ���е�boost::asio::buffer�÷����Բο�boost�е��ĵ�. �����Խ���һ��
	// boost.array��std.vector��Ϊ��������.
	template <typename ConstBufferSequence>
	std::size_t write_some(const ConstBufferSequence& buffers,
		boost::system::error_code& ec);

	///�����http_stream�첽����һЩ����.
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
	//   http_stream h(io_service);
	//   ...
	//   h.async_write_some(boost::asio::buffer(data, size), handler);
	//   ...
	// @end example
	// ����ʾ���е�boost::asio::buffer�÷����Բο�boost�е��ĵ�. �����Խ���һ��
	// boost.array��std.vector��Ϊ��������.
	template <typename ConstBufferSequence, typename Handler>
	void async_write_some(const ConstBufferSequence& buffers, BOOST_ASIO_MOVE_ARG(Handler) handler);

	///��http����������һ������.
	// @��http����������һ������, ���ʧ���׳��쳣.
	// @param opt������������������ѡ����Ϣ.
	// @begin example
	//  avhttp::http_stream h(io_service);
	//  ...
	//  request_opts opt;
	//  opt.insert("cookie", "name=admin;passwd=#@aN@2*242;");
	//  ...
	//  h.request(opt);
	// @end example
	AVHTTP_DECL void request(request_opts& opt);

	///��http����������һ������.
	// @param opt������������������ѡ����Ϣ.
	// @param ec�ڷ�������ʱ, �����ش�����Ϣ.
	// @begin example
	//  avhttp::http_stream h(io_service);
	//  ...
	//  request_opts opt;
	//  boost::system::error_code ec;
	//  opt.insert("cookie", "name=admin;passwd=#@aN@2*242;");
	//  ...
	//  h.request(opt, ec);
	//  ...
	// @end example
	AVHTTP_DECL void request(request_opts& opt, boost::system::error_code& ec);

	///��http����������һ���첽����.
	// @param optָ����http����ѡ��.
	// @param handler ���������ڴ����ʱ. ������������������:
	// @begin code
	//  void handler(
	//    const boost::system::error_code& ec	// ���ڷ��ز���״̬.
	//  );
	// @end code
	// @begin example
	//  void request_handler(const boost::system::error_code& ec)
	//  {
	//    if (!ec)
	//    {
	//      // ����ɹ�!
	//    }
	//  }
	//  ...
	//  avhttp::http_stream h(io_service);
	//  ...
	//  request_opts opt;
	//  opt.insert("cookie", "name=admin;passwd=#@aN@2*242;");
	//  h.async_request(opt, boost::bind(&request_handler, boost::asio::placeholders::error));
	// @end example
	template <typename Handler>
	void async_request(const request_opts& opt, BOOST_ASIO_MOVE_ARG(Handler) handler);

	///����һ��httpͷ��Ϣ, ʧ�ܽ��׳�һ��boost::system::system_error�쳣.
	// @��ע: �ú�������ʼ����һ��httpͷ(ֱ������\r\n\r\n)������, ���������
	// ��response_options��.
	AVHTTP_DECL void receive_header();

	///����һ��httpͷ��Ϣ.
	// @param ec�ڷ�������ʱ, �����ش�����Ϣ.
	// @��ע: �ú�������ʼ����һ��httpͷ(ֱ������\r\n\r\n)������, ���������
	// ��response_options��.
	AVHTTP_DECL void receive_header(boost::system::error_code& ec);

	///�첽����һ��httpͷ��Ϣ.
	// @param handler ���������ڴ����ʱ. ��������������ǩ��:
	// @begin code
	//  void handler(
	//    const boost::system::error_code& ec	// ���ڷ��ز���״̬.
	//  );
	// @end code
	// @begin example
	//  void receive_header_handler(const boost::system::error_code& ec)
	//  {
	//    if (!ec)
	//    {
	//      // ����ɹ�!
	//    }
	//  }
	//  ...
	//  avhttp::http_stream h(io_service);
	//  ...
	//  h.async_recvive_header(boost::bind(&receive_header_handler, boost::asio::placeholders::error));
	// @end example
	template <typename Handler>
	void async_receive_header(BOOST_ASIO_MOVE_ARG(Handler) handler);

	///�����д����������.
	// @��ע: ���̰߳�ȫ! ��Ӧ�����ڽ��ж�д����ʱ���иò���!
	AVHTTP_DECL void clear();

	///�ر�http_stream.
	// @ʧ���׳�asio::system_error�쳣.
	// @��ע: ֹͣ�������ڽ��еĶ�д����, ���ڽ��е��첽���ý��ص�
	// boost::asio::error::operation_aborted����.
	AVHTTP_DECL void close();

	///�ر�http_stream.
	// @param ec����ʧ����Ϣ.
	// @��ע: ֹͣ�������ڽ��еĶ�д����, ���ڽ��е��첽���ý��ص�
	// boost::asio::error::operation_aborted����.
	AVHTTP_DECL void close(boost::system::error_code& ec);

	///�ж��Ƿ��.
	// @�����Ƿ��.
	AVHTTP_DECL bool is_open() const;

	///���ص�ǰhttp_stream��ʹ�õ�io_service������.
	AVHTTP_DECL boost::asio::io_service& get_io_service();

	///��������ض������.
	// @param n ָ������ض������, Ϊ0��ʾ�����ض���.
	AVHTTP_DECL void max_redirects(int n);

	///���ô���, ͨ�����ô������http������.
	// @param s ָ���˴������.
	// @begin example
	//  avhttp::http_stream h(io_service);
	//  proxy_settings s;
	//  s.type = socks5;
	//  s.hostname = "example.proxy.com";
	//  s.port = 8080;
	//  h.proxy(s);
	//  ...
	// @end example
	AVHTTP_DECL void proxy(const proxy_settings& s);

	///��������ʱ��httpѡ��.
	// @param options Ϊhttp��ѡ��. Ŀǰ�����¼����ض�ѡ��:
	//  _request_method, ȡֵ "GET/POST/HEAD", Ĭ��Ϊ"GET".
	//  Host, ȡֵΪhttp������, Ĭ��Ϊhttp������.
	//  Accept, ȡֵ����, Ĭ��Ϊ"*/*".
	// @begin example
	//  avhttp::http_stream h(io_service);
	//  request_opts options;
	//  options.insert("_request_method", "POST"); // Ĭ��ΪGET��ʽ.
	//  h.request_options(options);
	//  ...
	// @end example
	AVHTTP_DECL void request_options(const request_opts& options);

	///��������ʱ��httpѡ��.
	// @begin example
	//  avhttp::http_stream h(io_service);
	//  request_opts options;
	//  options = h.request_options();
	//  ...
	// @end example
	AVHTTP_DECL request_opts request_options(void) const;

	///http�������ظ�ѡ��.
	// @���ط������ظ�������ѡ����Ϣ, key/value��ʽ.
	AVHTTP_DECL response_opts response_options(void) const;

	///�õ�����cookies.
	// @����http���������ص�cookies.
	AVHTTP_DECL const cookies& http_cookies() const;

	///��������cookies.
	// @param cookieָ�����õ�cookies.
	// @��ע: һ�����ڷ�������.
	AVHTTP_DECL void http_cookies(const cookies& cookie);

	///����location.
	// @����location��Ϣ, ���û���򷵻ؿմ�.
	AVHTTP_DECL const std::string& location() const;

	///�������������url��Ϣ.
	AVHTTP_DECL const std::string final_url() const;

	///�û���������url.
	AVHTTP_DECL const std::string entry_url() const;

	///����content_length.
	// @content_length��Ϣ, ���û����Ϊ-1.
	AVHTTP_DECL boost::int64_t content_length();

	///�����Ƿ���֤������֤��.
	// @param is_check ���Ϊtrue��ʾ��֤������֤��, ���Ϊfalse��ʾ����֤������֤��.
	// Ĭ��Ϊ��֤������֤��.
	AVHTTP_DECL void check_certificate(bool is_check);

	///���֤��·��.
	// @param path֤��·��.
	AVHTTP_DECL void add_verify_path(const std::string& path);

	///����֤���ļ�.
	// @param filenameָ����֤���ļ���.
	AVHTTP_DECL void load_verify_file(const std::string& filename);


protected:

	// �ڲ����ʵ��, ���ⲿ�ӿ�.

	template <typename MutableBufferSequence>
	std::size_t read_some_impl(const MutableBufferSequence& buffers,
		boost::system::error_code& ec);

	// �첽����ģ���Ա�����ʵ��.

	template <typename Handler>
	void handle_resolve(const boost::system::error_code& err,
		tcp::resolver::iterator endpoint_iterator, Handler handler);

	template <typename Handler>
	void handle_connect(Handler handler,
		tcp::resolver::iterator endpoint_iterator, const boost::system::error_code& err);

	template <typename Handler>
	void handle_request(Handler handler, const boost::system::error_code& err);

	template <typename Handler>
	void handle_status(Handler handler, const boost::system::error_code& err);

	template <typename Handler>
	void handle_header(Handler handler, std::string header_string,
		int bytes_transferred, const boost::system::error_code& err);

	template <typename MutableBufferSequence, typename Handler>
	void handle_read(const MutableBufferSequence& buffers,
		Handler handler, const boost::system::error_code& ec, std::size_t bytes_transferred);

	template <typename MutableBufferSequence, typename Handler>
	void handle_skip_crlf(const MutableBufferSequence& buffers,
		Handler handler, boost::shared_array<char> crlf,
		const boost::system::error_code& ec, std::size_t bytes_transferred);

	template <typename MutableBufferSequence, typename Handler>
	void handle_async_read(const MutableBufferSequence& buffers,
		Handler handler, const boost::system::error_code& ec, std::size_t bytes_transferred);

	template <typename MutableBufferSequence, typename Handler>
	void handle_chunked_size(const MutableBufferSequence& buffers,
		Handler handler, const boost::system::error_code& ec, std::size_t bytes_transferred);

	// ���ӵ�socks����, ����һ������ɺ�socks����Ϣ��������, ������Ϣ��ec��.
	template <typename Stream>
	void socks_proxy_connect(Stream& sock, boost::system::error_code& ec);

	template <typename Stream>
	void socks_proxy_handshake(Stream& sock, boost::system::error_code& ec);

	// socks��������첽����.
	template <typename Stream, typename Handler>
	void async_socks_proxy_connect(Stream& sock, Handler handler);

	// �첽�����ѯ�ص�.
	template <typename Stream, typename Handler>
	void async_socks_proxy_resolve(const boost::system::error_code& err,
		tcp::resolver::iterator endpoint_iterator, Stream& sock, Handler handler);

	template <typename Stream, typename Handler>
	void handle_connect_socks(Stream& sock, Handler handler,
		tcp::resolver::iterator endpoint_iterator, const boost::system::error_code& err);

	template <typename Stream, typename Handler>
	void handle_socks_process(Stream& sock, Handler handler,
		int bytes_transferred, const boost::system::error_code& err);

#ifdef AVHTTP_ENABLE_OPENSSL
	// ʵ��CONNECTָ��, ��������Ŀ��Ϊhttps����ʱʹ��.
	template <typename Stream, typename Handler>
	void async_https_proxy_connect(Stream& sock, Handler handler);

	template <typename Stream, typename Handler>
	void async_https_proxy_resolve(const boost::system::error_code& err,
		tcp::resolver::iterator endpoint_iterator, Stream& sock, Handler handler);

	template <typename Stream, typename Handler>
	void handle_connect_https_proxy(Stream& sock, Handler handler,
		tcp::resolver::iterator endpoint_iterator, const boost::system::error_code& err);

	template <typename Stream, typename Handler>
	void handle_https_proxy_request(Stream& sock, Handler handler,
		const boost::system::error_code& err);

	template <typename Stream, typename Handler>
	void handle_https_proxy_status(Stream& sock, Handler handler,
		const boost::system::error_code& err);

	template <typename Stream, typename Handler>
	void handle_https_proxy_header(Stream& sock, Handler handler,
		int bytes_transferred, const boost::system::error_code& err);

	template <typename Stream, typename Handler>
	void handle_https_proxy_handshake(Stream& sock, Handler handler,
		const boost::system::error_code& err);

	// ʵ��CONNECTָ��, ��������Ŀ��Ϊhttps����ʱʹ��.
	template <typename Stream>
	void https_proxy_connect(Stream& sock, boost::system::error_code& ec);
#endif

	// for support streambuf.
	AVHTTP_DECL std::streambuf::int_type underflow();

protected:

	// ����socket_type����, socket_type��variant_stream���ض���, ��������
	// ����Ϊssl_socket��nossl_socket, ����, �ڷ���socket��ʱ��, �Ͳ���Ҫ
	// �����д��ͬ�Ĵ���.
#ifdef AVHTTP_ENABLE_OPENSSL
	typedef avhttp::detail::ssl_stream<tcp::socket&> ssl_socket;
#endif
	typedef tcp::socket nossl_socket;
	typedef avhttp::detail::variant_stream<
		nossl_socket
#ifdef AVHTTP_ENABLE_OPENSSL
		, ssl_socket
#endif
	> socket_type;

	// socks��������״̬.
	enum socks_status
	{
		socks_proxy_resolve,	// ��ѯproxy������IP.
		socks_connect_proxy,	// ����proxy����.
		socks_send_version,		// ����socks�汾��.
		socks4_resolve_host,	// ����socks4��ѯ���ӵ�����IP�˿���Ϣ.
		socks4_response,		// socks4��������������.
		socks5_response_version,// socks5���ذ汾��Ϣ.
		socks5_send_userinfo,	// �����û�������Ϣ.
		socks5_connect_request,	// ������������.
		socks5_connect_response,// ������������������.
		socks5_auth_status,		// ��֤״̬.
		socks5_result,			// ���ս��.
		socks5_read_domainname,	// ��ȡ������Ϣ.
#ifdef AVHTTP_ENABLE_OPENSSL
		ssl_handshake,			// ssl�����첽����.
#endif
	};

	// for support streambuf.
	enum { putback_max = 8 };
	enum { buffer_size = 16 };

private:

	// io_service����.
	boost::asio::io_service& m_io_service;

	// ����HOST.
	tcp::resolver m_resolver;

	// socket.
	socket_type m_sock;

	// ��ssl socket, ֻ����https��proxyʵ��.
	nossl_socket m_nossl_socket;

	// �Ƿ���֤�����֤��.
	bool m_check_certificate;

	// ֤��·��.
	std::string m_ca_directory;

	// CA֤���ļ�.
	std::string m_ca_cert;

	// ��http�����������ͷ��Ϣ.
	request_opts m_request_opts;

	// ��http�����������ͷ��Ϣ.
	request_opts m_request_opts_priv;

	// http���������ص�httpͷ��Ϣ.
	response_opts m_response_opts;

	// ��������.
	proxy_settings m_proxy;

	// �첽�д���״̬.
	int m_proxy_status;

	// ����socks4������.
	tcp::endpoint m_remote_endp;

	// Э������(http/https).
	std::string m_protocol;

	// ���浱ǰ�����url.
	url m_url;

	// �����û���������url.
	url m_entry_url;

	// ���connectionѡ��, ͬʱ��m_response_optsӰ��.
	bool m_keep_alive;

	// http����״̬��.
	int m_status_code;

	// �ض����������.
	std::size_t m_redirects;

	// �ض����������.
	std::size_t m_max_redirects;

	// ��������.
	std::string m_content_type;

	// �������ݳ���.
	boost::int64_t m_content_length;

	// body��С, ����Ҫ������������keep_alive�������, ��ǰ��
	// �����յ���content���ȼ������, �Ա�����յ���һ��requst
	// �����ص�http header.
	std::size_t m_body_size;

	// �ض���ĵ�ַ.
	std::string m_location;

	// ���󻺳�.
	boost::asio::streambuf m_request;

	// �ظ�����.
	boost::asio::streambuf m_response;

#ifdef AVHTTP_ENABLE_ZLIB
	// zlib֧��.
	z_stream m_stream;

	// ��ѹ����.
	char m_zlib_buffer[1024];

	// ������ֽ���.
	std::size_t m_zlib_buffer_size;

	// �Ƿ�ʹ��gz.
	bool m_is_gzip;
#endif

	// �Ƿ�ʹ��chunked����.
	bool m_is_chunked;

	// ����crlf.
	bool m_skip_crlf;

	// ����chunked footer.
	bool m_is_chunked_end;

	// chunked��С.
	std::size_t m_chunked_size;

	// ����stream��ʽ�Ķ�ȡ����.
	boost::array<char, buffer_size> m_get_buffer;

	// ���ڼ�¼��������Ϣ.
	boost::system::error_code m_last_error;

	// �������cookies.
	cookies m_cookies;
};

}

#if defined(AVHTTP_HEADER_ONLY)
#	include "avhttp/impl/http_stream.cpp"
#endif

#endif // AVHTTP_HTTP_STREAM_HPP
