//
// async_read_body.hpp
// ~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2013 Jack (jack dot wgm at gmail dot com)
// Copyright (C) 2012 - 2013  ΢�� <microcai@fedoraproject.org>
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// path LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef AVHTTP_MISC_HTTP_READBODY_HPP
#define AVHTTP_MISC_HTTP_READBODY_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

BOOST_STATIC_ASSERT_MSG(BOOST_VERSION >= 105400, "You must use boost-1.54 or later!!!");

#include "avhttp/http_stream.hpp"
#include "avhttp/completion_condition.hpp"

namespace avhttp {
namespace detail {

template <typename AsyncReadStream, typename MutableBufferSequence, typename Handler>
class read_body_op : boost::asio::coroutine
{
public:
	read_body_op(AsyncReadStream& stream, const avhttp::url& url,
		MutableBufferSequence& buffers, Handler handler)
		: m_stream(stream)
		, m_buffers(buffers)
		, m_handler(BOOST_ASIO_MOVE_CAST(Handler)(handler))
	{
		m_stream.async_open(url, *this);
	}

	void operator()(const boost::system::error_code& ec, std::size_t bytes_transferred = 0)
	{
		BOOST_ASIO_CORO_REENTER(this)
		{
			if(!ec)
			{
				BOOST_ASIO_CORO_YIELD boost::asio::async_read(
					m_stream, m_buffers, transfer_response_body(m_stream.content_length()), *this);
			}
			else
			{
				m_handler(ec, bytes_transferred);
				return;
			}

			if(ec == boost::asio::error::eof && m_stream.content_length() == -1)
			{
				m_handler(boost::system::error_code(), bytes_transferred);
			}
			else
			{
				m_handler(ec, bytes_transferred);
			}
		}
	}

// private:
	AsyncReadStream& m_stream;
	MutableBufferSequence& m_buffers;
	Handler m_handler;
};

template <typename AsyncReadStream, typename MutableBufferSequence, typename Handler>
read_body_op<AsyncReadStream, MutableBufferSequence, Handler>
	make_read_body_op(AsyncReadStream& stream,
	const avhttp::url& url, MutableBufferSequence& buffers, Handler handler)
{
	return read_body_op<AsyncReadStream, MutableBufferSequence, Handler>(
		stream, url, buffers, handler);
}

} // namespace detail


///����http_stream�첽����url.
// �����������http_stream�첽����ָ����url, ��ͨ��handler�ص�֪ͨ�û�, ���ݽ�
// �������û������ṩ��buffers��.
// @ע��:
//  1. �ú����ص�����Ϊֱ����ȡ������body��eof����������, ������Ϣͨ��error_code����.
//  2. ���������������, Ӧ�ñ��� stream �� buffers��������.
// @param stream һ��http_stream����.
// @param url ָ����url.
// @param buffers һ���������ڶ�ȡ���ݵĻ�����
// ������ͱ�������MutableBufferSequence, MutableBufferSequence�Ķ���.
// ����ɲο�boost.asio�ĵ�����Ӧ������.
// @param handler�ڶ�ȡ������ɻ���ִ���ʱ, �����ص�, ��������������:
// @begin code
//  void handler(
//    const boost::system::error_code& ec,	// ���ڷ��ز���״̬.
//    std::size_t bytes_transferred			// ���ض�ȡ�������ֽ���.
//  );
// @end code
// @begin example
//  void handler(const boost::system::error_code& ec, std::size_t bytes_transferred)
//  {
//      // �����첽�ص�.
//  }
//  ...
//  avhttp::http_stream h(io);
//  async_read_body(h, "http://www.boost.org/LICENSE_1_0.txt",
//      boost::asio::buffer(data, 1024), handler);
//  io.run();
//  ...
// @end example
template<typename AsyncReadStream, typename MutableBufferSequence, typename Handler>
AVHTTP_DECL void async_read_body(AsyncReadStream& stream,
	const avhttp::url& url, MutableBufferSequence& buffers, Handler handler)
{
	detail::make_read_body_op(stream, url, buffers, handler);
}

} // namespace avhttp

#endif // AVHTTP_MISC_HTTP_READBODY_HPP
