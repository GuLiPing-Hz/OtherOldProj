//
// impl/http_stream.ipp
// ~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2013 Jack (jack dot wgm at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef AVHTTP_HTTP_STREAM_IPP
#define AVHTTP_HTTP_STREAM_IPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "avhttp/logging.hpp"
#include "avhttp/http_stream.hpp"
#include "avhttp/detail/handler_type_requirements.hpp"
#include "avhttp/detail/escape_string.hpp"
namespace avhttp {

http_stream::http_stream(boost::asio::io_service& io)
	: m_io_service(io)
	, m_resolver(io)
	, m_sock(io)
	, m_nossl_socket(io)
	, m_check_certificate(true)
	, m_keep_alive(true)
	, m_status_code(-1)
	, m_redirects(0)
	, m_max_redirects(AVHTTP_MAX_REDIRECTS)
	, m_content_length(0)
	, m_body_size(0)
#ifdef AVHTTP_ENABLE_ZLIB
	, m_is_gzip(false)
#endif
	, m_is_chunked(false)
	, m_skip_crlf(true)
	, m_chunked_size(0)
{
#ifdef AVHTTP_ENABLE_ZLIB
	memset(&m_stream, 0, sizeof(z_stream));
#endif
	m_proxy.type = proxy_settings::none;
}

http_stream::~http_stream()
{
#ifdef AVHTTP_ENABLE_ZLIB
	if (m_stream.zalloc)
		inflateEnd(&m_stream);
#endif
}

void http_stream::open(const url& u)
{
	boost::system::error_code ec;
	open(u, ec);
	if (ec)
	{
		boost::throw_exception(boost::system::system_error(ec));
	}
}

void http_stream::open(const url& u, boost::system::error_code& ec)
{
	// ����url��ص���Ϣ.
	if (m_url.to_string() == "")
	{
		m_entry_url = u;
	}

	m_protocol = u.protocol();
	m_url = u;

	AVHTTP_LOG_DBG("Sync open url \'" << u.to_string() << "\'");

	// ���һЩѡ��.
	m_content_type = "";
	m_status_code = 0;
	m_content_length = -1;
	m_body_size = 0;
	m_content_type = "";
	m_request.consume(m_request.size());
	m_response.consume(m_response.size());
	m_skip_crlf = true;
	m_is_chunked = false;

	// �жϻ�������url����.
	if (m_protocol != "http"
#ifdef AVHTTP_ENABLE_OPENSSL
		&& m_protocol != "https"
#endif
		)
	{
		AVHTTP_LOG_ERR("Unsupported scheme \'" << m_protocol << "\'");
		ec = boost::asio::error::operation_not_supported;
		return;
	}

	// ����socket.
	if (m_protocol == "http")
	{
		m_sock.instantiate<nossl_socket>(m_io_service);
	}
#ifdef AVHTTP_ENABLE_OPENSSL
	else if (m_protocol == "https")
	{
		m_sock.instantiate<ssl_socket>(m_nossl_socket);

		// ����֤��·����֤��.
		ssl_socket* ssl_sock = m_sock.get<ssl_socket>();
		if (!m_ca_directory.empty())
		{
			ssl_sock->add_verify_path(m_ca_directory, ec);
			if (ec)
			{
				AVHTTP_LOG_ERR("Add verify path \'" << m_ca_directory <<
					"\', error message \'" << ec.message() << "\'");
				return;
			}
		}
		if (!m_ca_cert.empty())
		{
			ssl_sock->load_verify_file(m_ca_cert, ec);
			if (ec)
			{
				AVHTTP_LOG_ERR("Load verify file \'" << m_ca_cert <<
					"\', error message \'" << ec.message() << "\'");
				return;
			}
		}
		if (m_check_certificate)
		{
			ssl_sock->set_verify_callback(
				boost::asio::ssl::rfc2818_verification(m_url.host()), ec);
			if (ec)
			{
				AVHTTP_LOG_ERR("Set verify callback \'" << m_url.host() <<
					"\', error message \'" << ec.message() << "\'");
				return;
			}
		}
	}
#endif

	// ��ʼ��������.
	if (m_sock.instantiated() && !m_sock.is_open())
	{
		if (m_proxy.type == proxy_settings::none)
		{
			// ��ʼ�����˿ں�������.
			tcp::resolver resolver(m_io_service);
			std::ostringstream port_string;
			port_string.imbue(std::locale("C"));
			port_string << m_url.port();
			tcp::resolver::query query(m_url.host(), port_string.str());
			tcp::resolver::iterator endpoint_iterator = resolver.resolve(query, ec);
			tcp::resolver::iterator end;

			if (ec)	// ������������, ֱ�ӷ�����ش�����Ϣ.
			{
				AVHTTP_LOG_ERR("Resolve DNS error \'" << m_url.host() <<
					"\', error message \'" << ec.message() << "\'");
				return ;
			}

			// �������ӽ��������ķ�������ַ.
			ec = boost::asio::error::host_not_found;
			while (ec && endpoint_iterator != end)
			{
				m_sock.close(ec);
				m_sock.connect(*endpoint_iterator++, ec);
			}
			if (ec)
			{
				AVHTTP_LOG_ERR("Connect to \'" << m_url.host() <<
					"\', error message \'" << ec.message() << "\'");
				return;
			}
			else
			{
				AVHTTP_LOG_DBG("Connect to \'" << m_url.host() << "\'.");
			}
		}
		else if (m_proxy.type == proxy_settings::socks5 ||
			m_proxy.type == proxy_settings::socks4 ||
			m_proxy.type == proxy_settings::socks5_pw)	// socks����.
		{
			if (m_protocol == "http")
			{
				socks_proxy_connect(m_sock, ec);
				if (ec)
				{
					AVHTTP_LOG_ERR("Connect to socks proxy \'" << m_proxy.hostname << ":" << m_proxy.port <<
						"\', error message \'" << ec.message() << "\'");
					return;
				}
				else
				{
					AVHTTP_LOG_DBG("Connect to socks proxy \'" << m_proxy.hostname << ":" << m_proxy.port << "\'.");
				}
			}
#ifdef AVHTTP_ENABLE_OPENSSL
			else if (m_protocol == "https")
			{
				socks_proxy_connect(m_nossl_socket, ec);
				if (ec)
				{
					AVHTTP_LOG_ERR("Connect to socks proxy \'" << m_proxy.hostname << ":" << m_proxy.port <<
						"\', error message \'" << ec.message() << "\'");
					return;
				}
				else
				{
					AVHTTP_LOG_DBG("Connect to socks proxy \'" << m_proxy.hostname << ":" << m_proxy.port << "\'.");
				}
				// ��ʼ����.
				ssl_socket* ssl_sock = m_sock.get<ssl_socket>();
				ssl_sock->handshake(ec);
				if (ec)
				{
					AVHTTP_LOG_ERR("Handshake to \'" << m_url.host() <<
						"\', error message \'" << ec.message() << "\'");
					return;
				}
				else
				{
					AVHTTP_LOG_DBG("Handshake to \'" << m_url.host() << "\'.");
				}
			}
#endif
			// �ʹ�������������������.
		}
		else if (m_proxy.type == proxy_settings::http ||
			m_proxy.type == proxy_settings::http_pw)		// http����.
		{
#ifdef AVHTTP_ENABLE_OPENSSL
			if (m_protocol == "https")
			{
				// https������.
				https_proxy_connect(m_nossl_socket, ec);
				if (ec)
				{
					AVHTTP_LOG_ERR("Connect to http proxy \'" << m_proxy.hostname << ":" << m_proxy.port <<
						"\', error message \'" << ec.message() << "\'");
					return;
				}
				else
				{
					AVHTTP_LOG_DBG("Connect to http proxy \'" << m_proxy.hostname << ":" << m_proxy.port << "\'.");
				}
				// ��ʼ����.
				ssl_socket* ssl_sock = m_sock.get<ssl_socket>();
				ssl_sock->handshake(ec);
				if (ec)
				{
					AVHTTP_LOG_ERR("Handshake to \'" << m_url.host() <<
						"\', error message \'" << ec.message() << "\'");
					return;
				}
				else
				{
					AVHTTP_LOG_DBG("Handshake to \'" << m_url.host() << "\'.");
				}
			}
			else
#endif
			if (m_protocol == "http")
			{
				// ��ʼ�����˿ں�������.
				tcp::resolver resolver(m_io_service);
				std::ostringstream port_string;
				port_string.imbue(std::locale("C"));
				port_string << m_proxy.port;
				tcp::resolver::query query(m_proxy.hostname, port_string.str());
				tcp::resolver::iterator endpoint_iterator = resolver.resolve(query, ec);
				tcp::resolver::iterator end;

				if (ec)	// ������������, ֱ�ӷ�����ش�����Ϣ.
				{
					AVHTTP_LOG_ERR("Resolve DNS error \'" << m_proxy.hostname <<
						"\', error message \'" << ec.message() << "\'");
					return ;
				}

				// �������ӽ��������Ĵ����������ַ.
				ec = boost::asio::error::host_not_found;
				while (ec && endpoint_iterator != end)
				{
					m_sock.close(ec);
					m_sock.connect(*endpoint_iterator++, ec);
				}
				if (ec)
				{
					AVHTTP_LOG_ERR("Connect to http proxy \'" << m_proxy.hostname << ":" << m_proxy.port <<
						"\', error message \'" << ec.message() << "\'");
					return;
				}
				else
				{
					AVHTTP_LOG_DBG("Connect to proxy \'" << m_proxy.hostname << ":" << m_proxy.port << "\'.");
				}
			}
		}
		else
		{
			AVHTTP_LOG_ERR("Unsupported proxy \'" << m_proxy.type << "\'");
			// ��֧�ֵĲ�������.
			ec = boost::asio::error::operation_not_supported;
			return;
		}

		// ����Nagle��socket��.
		m_sock.set_option(tcp::no_delay(true), ec);
		if (ec)
		{
			AVHTTP_LOG_ERR("Set option to nodelay, error message \'" << ec.message() << "\'");
			return;
		}
	}
	else
	{
		// socket�Ѿ���.
		ec = boost::asio::error::already_open;
		AVHTTP_LOG_ERR("Open socket, error message\'" << ec.message() << "\'");
		return;
	}

	boost::system::error_code http_code;

	// ��������.
	request(m_request_opts_priv, http_code);

	// �ж��Ƿ���Ҫ��ת.
	if (http_code == errc::moved_permanently || http_code == errc::found)
	{
		m_sock.close(ec);
		if (++m_redirects <= m_max_redirects)
		{
			open(m_location, ec);
			return;
		}
	}

	// ����ض������.
	m_redirects = 0;

	// ����http״̬��������.
	if (http_code)
		ec = http_code;
	else
		ec = boost::system::error_code();	// �򿪳ɹ�.

	return;
}

template <typename Handler>
void http_stream::async_open(const url& u, BOOST_ASIO_MOVE_ARG(Handler) handler)
{
	AVHTTP_OPEN_HANDLER_CHECK(Handler, handler) type_check;

	boost::system::error_code ec;

	// ����url��ص���Ϣ.
	if (m_url.to_string() == "")
	{
		m_entry_url = u;
	}

	m_protocol = u.protocol();
	m_url = u;

	AVHTTP_LOG_DBG("Async open url \'" << u.to_string() << "\'");

	// ���һЩѡ��.
	m_content_type = "";
	m_status_code = 0;
	m_content_length = -1;
	m_body_size = 0;
	m_content_type = "";
	m_request.consume(m_request.size());
	m_response.consume(m_response.size());
	m_skip_crlf = true;
	m_is_chunked = false;

	// �жϻ�������url����.
	if (m_protocol != "http"
#ifdef AVHTTP_ENABLE_OPENSSL
		&& m_protocol != "https"
#endif
		)
	{
		AVHTTP_LOG_ERR("Unsupported scheme \'" << m_protocol << "\'");
		m_io_service.post(boost::asio::detail::bind_handler(
			handler, boost::asio::error::operation_not_supported));
		return;
	}

	// ����socket.
	if (m_protocol == "http")
	{
		m_sock.instantiate<nossl_socket>(m_io_service);
	}
#ifdef AVHTTP_ENABLE_OPENSSL
	else if (m_protocol == "https")
	{
		m_sock.instantiate<ssl_socket>(m_nossl_socket);

		// ����֤��·����֤��.
		ssl_socket* ssl_sock = m_sock.get<ssl_socket>();
		if (!m_ca_directory.empty())
		{
			ssl_sock->add_verify_path(m_ca_directory, ec);
			if (ec)
			{
				AVHTTP_LOG_ERR("Add verify path \'" << m_ca_directory <<
					"\', error message \'" << ec.message() << "\'");
				m_io_service.post(boost::asio::detail::bind_handler(
					handler, ec));
				return;
			}
		}
		if (!m_ca_cert.empty())
		{
			ssl_sock->load_verify_file(m_ca_cert, ec);
			if (ec)
			{
				AVHTTP_LOG_ERR("Load verify file \'" << m_ca_cert <<
					"\', error message \'" << ec.message() << "\'");
				m_io_service.post(boost::asio::detail::bind_handler(
					handler, ec));
				return;
			}
		}
		if (m_check_certificate)
		{
			ssl_sock->set_verify_callback(
				boost::asio::ssl::rfc2818_verification(m_url.host()), ec);
			if (ec)
			{
				AVHTTP_LOG_ERR("Set verify callback \'" << m_url.host() <<
					"\', error message \'" << ec.message() << "\'");
				m_io_service.post(boost::asio::detail::bind_handler(
					handler, ec));
				return;
			}
		}
	}
#endif

	// �ж�socket�Ƿ��.
	if (m_sock.instantiated() && m_sock.is_open())
	{
		ec = boost::asio::error::already_open;
		AVHTTP_LOG_ERR("Open socket, error message\'" <<	ec.message() << "\'");
		m_io_service.post(boost::asio::detail::bind_handler(handler, ec));
		return;
	}

	// �첽socks�����ܴ���.
	if (m_proxy.type == proxy_settings::socks4 || m_proxy.type == proxy_settings::socks5
		|| m_proxy.type == proxy_settings::socks5_pw)
	{
		if (m_protocol == "http")
		{
			async_socks_proxy_connect(m_sock, handler);
		}
#ifdef AVHTTP_ENABLE_OPENSSL
		else if (m_protocol == "https")
		{
			async_socks_proxy_connect(m_nossl_socket, handler);
		}
#endif
		return;
	}

	std::string host;
	std::ostringstream port_string;
	if (m_proxy.type == proxy_settings::http || m_proxy.type == proxy_settings::http_pw)
	{
#ifdef AVHTTP_ENABLE_OPENSSL
		if (m_protocol == "https")
		{
			// https����.
			async_https_proxy_connect(m_nossl_socket, handler);
			return;
		}
		else
#endif
		{
			host = m_proxy.hostname;
			port_string.imbue(std::locale("C"));
			port_string << m_proxy.port;
		}
	}
	else
	{
		host = m_url.host();
		port_string.imbue(std::locale("C"));
		port_string << m_url.port();
	}

	// �����첽��ѯHOST.
	tcp::resolver::query query(host, port_string.str());

	// ��ʼ�첽��ѯHOST��Ϣ.
	typedef boost::function<void (boost::system::error_code)> HandlerWrapper;
	HandlerWrapper h = handler;
	m_resolver.async_resolve(query,
		boost::bind(&http_stream::handle_resolve<HandlerWrapper>,
			this,
			boost::asio::placeholders::error,
			boost::asio::placeholders::iterator,
			h
		)
	);
}

template <typename MutableBufferSequence>
std::size_t http_stream::read_some(const MutableBufferSequence& buffers)
{
	boost::system::error_code ec;
	std::size_t bytes_transferred = read_some(buffers, ec);
	if (ec)
	{
		boost::throw_exception(boost::system::system_error(ec));
	}
	return bytes_transferred;
}

template <typename MutableBufferSequence>
std::size_t http_stream::read_some(const MutableBufferSequence& buffers,
	boost::system::error_code& ec)
{
	std::size_t bytes_transferred = 0;
	if (m_is_chunked)	// ��������˷ֿ鴫��ģʽ, ��������С, ����ȡС�ڿ��С������.
	{
		char crlf[2] = { '\r', '\n' };
		// chunked_size��СΪ0, ��ȡ��һ����ͷ��С.
		if (m_chunked_size == 0
#ifdef AVHTTP_ENABLE_ZLIB
			&& m_stream.avail_in == 0
#endif
			)
		{
			// �Ƿ�����CRLF, ����һ�ζ�ȡ��һ��������, �����ÿ��chunked����Ҫ��
			// ĩβ��CRLF����.
			if (!m_skip_crlf)
			{
				ec = boost::system::error_code();
				while (!ec && bytes_transferred != 2)
					bytes_transferred += read_some_impl(
						boost::asio::buffer(&crlf[bytes_transferred], 2 - bytes_transferred), ec);
				if (ec)
				{
					return 0;
				}

				if (m_is_chunked_end)
				{
					if (!m_keep_alive)
						ec = boost::asio::error::eof;
					return 0;
				}

				// ����CRLF? ��֪����ɶ���, ���Ե���bug.
				BOOST_ASSERT(crlf[0] == '\r' && crlf[1] == '\n');

				// ��release��, ��ȷ���ǲ��Ƿ������Ļظ�����, ��ʱ�����Ƿ������Ļظ�����!!!
				if(crlf[0] != '\r' || crlf[1] != '\n')
				{
					ec = errc::invalid_chunked_encoding;
					return bytes_transferred;
				}
			}
			std::string hex_chunked_size;
			// ��ȡ.
			while (!ec)
			{
				char c;
				bytes_transferred = read_some_impl(boost::asio::buffer(&c, 1), ec);
				if (bytes_transferred == 1)
				{
					hex_chunked_size.push_back(c);
					std::size_t s = hex_chunked_size.size();
					if (s >= 2)
					{
						if (hex_chunked_size[s - 2] == crlf[0] && hex_chunked_size[s - 1] == crlf[1])
							break;
					}
				}
			}
			if (ec)
			{
				return 0;
			}

			// �õ�chunked size.
			std::stringstream ss;
			ss << std::hex << hex_chunked_size;
			ss >> m_chunked_size;

			// chunked_size����������β��crlf, ����������β��crlfΪfalse״̬.
			m_skip_crlf = false;
		}

#ifdef AVHTTP_ENABLE_ZLIB
		if (m_chunked_size == 0 && m_is_gzip)
		{
			if (m_stream.avail_in == 0)
			{
				if (!m_keep_alive)
					ec = boost::asio::error::eof;
				return 0;
			}
		}
#endif
		if (m_chunked_size != 0
#ifdef AVHTTP_ENABLE_ZLIB
			|| m_stream.avail_in != 0
#endif
			)	// ��ʼ��ȡchunked�е�����, �����ѹ��, ���ѹ���û����ܻ���.
		{
			std::size_t max_length = 0;
			{
				typename MutableBufferSequence::const_iterator iter = buffers.begin();
				typename MutableBufferSequence::const_iterator end = buffers.end();
				// ����õ��û�buffer_size�ܴ�С.
				for (; iter != end; ++iter)
				{
					boost::asio::mutable_buffer buffer(*iter);
					max_length += boost::asio::buffer_size(buffer);
				}
				// �õ����ʵĻ����С.
				max_length = (std::min)(max_length, m_chunked_size);
			}

#ifdef AVHTTP_ENABLE_ZLIB
			if (!m_is_gzip)	// ���û������gzip, ��ֱ�Ӷ�ȡ���ݺ󷵻�.
#endif
			{
				bytes_transferred = read_some_impl(boost::asio::buffer(buffers, max_length), ec);
				m_chunked_size -= bytes_transferred;
				return bytes_transferred;
			}
#ifdef AVHTTP_ENABLE_ZLIB
			else					// �����ȡ���ݵ���ѹ������.
			{
				if (m_stream.avail_in == 0)
				{
					std::size_t buf_size = (std::min)(m_chunked_size, std::size_t(1024));
					bytes_transferred = read_some_impl(boost::asio::buffer(m_zlib_buffer, buf_size), ec);
					m_chunked_size -= bytes_transferred;
					m_zlib_buffer_size = bytes_transferred;
					m_stream.avail_in = (uInt)m_zlib_buffer_size;
					m_stream.next_in = (z_const Bytef *)&m_zlib_buffer[0];
				}

				bytes_transferred = 0;
				std::size_t buffer_size = 0;

				{
					typename MutableBufferSequence::const_iterator iter = buffers.begin();
					typename MutableBufferSequence::const_iterator end = buffers.end();
					// ����õ��û�buffer_size�ܴ�С.
					for (; iter != end; ++iter)
					{
						boost::asio::mutable_buffer buffer(*iter);
						m_stream.next_in = (z_const Bytef *)(&m_zlib_buffer[0] + m_zlib_buffer_size - m_stream.avail_in);
						m_stream.avail_out = boost::asio::buffer_size(buffer);
						if (m_stream.avail_out == 0)
						{
							break; // ����û��ṩ�Ļ����СΪ0, ��ֱ�ӷ���.
						}
						buffer_size += m_stream.avail_out;
						m_stream.next_out = boost::asio::buffer_cast<Bytef*>(buffer);
						int ret = inflate(&m_stream, Z_SYNC_FLUSH);
						if (ret < 0)
						{
							ec = boost::asio::error::operation_not_supported;
							return 0;
						}

						bytes_transferred += (boost::asio::buffer_size(buffer) - m_stream.avail_out);
						if (bytes_transferred != boost::asio::buffer_size(buffer))
						{
							break;
						}
					}
				}

				// û�н�ѹ������, ˵����ѹ����̫С, ������ȡ����, �Ա�֤�����ݷ���.
				if (buffer_size != 0 && bytes_transferred == 0)
				{
					return read_some(buffers, ec);
				}

				return bytes_transferred;
			}
#endif
		}

		if (m_chunked_size == 0)
		{
			m_is_chunked_end = true;
#ifdef AVHTTP_ENABLE_ZLIB	// �������chunk��β, ���ͷ�m_stream, ��Ϊ��һ��chunked����Ҫ���³�ʼ��.
			if (m_stream.zalloc)
			{
				inflateEnd(&m_stream);
				m_stream.zalloc = NULL;
			}
#endif
			if (!m_keep_alive)
			{
				ec = boost::asio::error::eof;
			}

			return 0;
		}
	}

	// ���û������chunked.
#ifdef AVHTTP_ENABLE_ZLIB
	if (m_is_gzip && !m_is_chunked)
	{
		if (m_stream.avail_in == 0)	// ��һ���ѹ���.
		{
			if (m_keep_alive)
			{
				if (m_content_length != -1 && m_body_size == m_content_length)
				{
					return 0;
				}
			}

			bytes_transferred = read_some_impl(boost::asio::buffer(m_zlib_buffer, 1024), ec);
			m_body_size += bytes_transferred;
			m_zlib_buffer_size = bytes_transferred;
			m_stream.avail_in = (uInt)m_zlib_buffer_size;
			m_stream.next_in = (z_const Bytef *)&m_zlib_buffer[0];
		}

		bytes_transferred = 0;
		std::size_t buffer_size = 0;

		{
			typename MutableBufferSequence::const_iterator iter = buffers.begin();
			typename MutableBufferSequence::const_iterator end = buffers.end();
			// ����õ��û�buffer_size�ܴ�С.
			for (; iter != end; ++iter)
			{
				boost::asio::mutable_buffer buffer(*iter);
				m_stream.next_in = (z_const Bytef *)(&m_zlib_buffer[0] + m_zlib_buffer_size - m_stream.avail_in);
				m_stream.avail_out = boost::asio::buffer_size(buffer);
				if (m_stream.avail_out == 0)
				{
					break; // ����û��ṩ�Ļ����СΪ0, ��ֱ�ӷ���.
				}
				buffer_size += m_stream.avail_out;
				m_stream.next_out = boost::asio::buffer_cast<Bytef*>(buffer);
				int ret = inflate(&m_stream, Z_SYNC_FLUSH);
				if (ret < 0)
				{
					ec = boost::asio::error::operation_not_supported;
					return 0;
				}

				bytes_transferred += (boost::asio::buffer_size(buffer) - m_stream.avail_out);
				if (bytes_transferred != boost::asio::buffer_size(buffer))
				{
					break;
				}
			}
		}

		// û�н�ѹ������, ˵����ѹ����̫С, ������ȡ����, �Ա�֤�����ݷ���.
		if (buffer_size != 0 && bytes_transferred == 0)
		{
			return read_some(buffers, ec);
		}

		return bytes_transferred;
	}
#endif

	// ���������keep_alive, ������Ƿ��ȡbody���, ������, ��ֱ�ӷ���0������ȥ��ȡ.
	if (m_keep_alive)
	{
		if (m_content_length != -1 && m_body_size == m_content_length)
		{
			return 0;
		}
	}

	// ������ȡ����.
	bytes_transferred = read_some_impl(buffers, ec);
	m_body_size += bytes_transferred;

	// �������ݴ�С.
	return bytes_transferred;
}

template <typename MutableBufferSequence, typename Handler>
void http_stream::async_read_some(const MutableBufferSequence& buffers, BOOST_ASIO_MOVE_ARG(Handler) handler)
{
	AVHTTP_READ_HANDLER_CHECK(Handler, handler) type_check;

	boost::system::error_code ec;

	if (m_is_chunked)	// ��������˷ֿ鴫��ģʽ, ��������С, ����ȡС�ڿ��С������.
	{
		// chunked_size��СΪ0, ��ȡ��һ����ͷ��С, ���������gzip, ������ѹ���������ݲ�
		// ��ȡ��һ��chunkͷ.
		if (m_chunked_size == 0
#ifdef AVHTTP_ENABLE_ZLIB
			&& m_stream.avail_in == 0
#endif
			)
		{
			int bytes_transferred = 0;
			int response_size = m_response.size();

			// �Ƿ�����CRLF, ����һ�ζ�ȡ��һ��������, �����ÿ��chunked����Ҫ��
			// ĩβ��CRLF����.
			if (!m_skip_crlf)
			{
				boost::shared_array<char> crlf(new char[2]);
				memset((void*)&crlf[0], 0, 2);

				if (response_size > 0)	// ��m_response����������.
				{
					bytes_transferred = m_response.sgetn(
						crlf.get(), (std::min)(response_size, 2));
					if (bytes_transferred == 1)
					{
						// �����첽��ȡ��һ��LF�ֽ�.
						typedef boost::function<void (boost::system::error_code, std::size_t)> HandlerWrapper;
						HandlerWrapper h(handler);
						m_sock.async_read_some(boost::asio::buffer(&crlf[1], 1),
							boost::bind(&http_stream::handle_skip_crlf<MutableBufferSequence, HandlerWrapper>,
								this, buffers, h, crlf,
								boost::asio::placeholders::error,
								boost::asio::placeholders::bytes_transferred
							)
						);
						return;
					}
					else
					{
						if (m_is_chunked_end)
						{
							if (!m_keep_alive)
								ec = boost::asio::error::eof;
							m_io_service.post(
								boost::asio::detail::bind_handler(handler, ec, 0));
							return;
						}
						// ��ȡ��CRLF, so, ����ֻ����2!!! Ȼ��ʼ����chunked size.
						BOOST_ASSERT(bytes_transferred == 2);
						// ����CRLF? ��֪����ɶ���, ���Ե���bug.
						BOOST_ASSERT(crlf.get()[0] == '\r' && crlf.get()[1] == '\n');
						// ��release��, ��ȷ���ǲ��Ƿ������Ļظ�����, �����Ƿ������Ļظ�����!!!
						if(crlf.get()[0] != '\r' || crlf.get()[1] != '\n')
						{
							ec = errc::invalid_chunked_encoding;
							m_io_service.post(
								boost::asio::detail::bind_handler(handler, ec, 0));
							return;
						}
					}
				}
				else
				{
					// �첽��ȡCRLF.
					typedef boost::function<void (boost::system::error_code, std::size_t)> HandlerWrapper;
					HandlerWrapper h(handler);
					m_sock.async_read_some(boost::asio::buffer(&crlf.get()[0], 2),
						boost::bind(&http_stream::handle_skip_crlf<MutableBufferSequence, HandlerWrapper>,
							this, buffers, h, crlf,
							boost::asio::placeholders::error,
							boost::asio::placeholders::bytes_transferred
						)
					);
					return;
				}
			}

			// ����CRLF, ��ʼ��ȡchunked size.
			typedef boost::function<void (boost::system::error_code, std::size_t)> HandlerWrapper;
			HandlerWrapper h(handler);
			boost::asio::async_read_until(m_sock, m_response, "\r\n",
				boost::bind(&http_stream::handle_chunked_size<MutableBufferSequence, HandlerWrapper>,
					this, buffers, h,
					boost::asio::placeholders::error,
					boost::asio::placeholders::bytes_transferred
				)
			);
			return;
		}
		else
		{
			std::size_t max_length = 0;

			// ����Ϊ0��ֱ�Ӷ�ȡm_response�е�����, �����ٴ�socket��ȡ����, ����
			// ��ȡ���ݵ�β����ʱ��, ������ʱ��ȴ������.
			if (m_response.size() != 0)
				max_length = 0;
			else
			{
				typename MutableBufferSequence::const_iterator iter = buffers.begin();
				typename MutableBufferSequence::const_iterator end = buffers.end();
				// ����õ��û�buffer_size�ܴ�С.
				for (; iter != end; ++iter)
				{
					boost::asio::mutable_buffer buffer(*iter);
					max_length += boost::asio::buffer_size(buffer);
				}
				// �õ����ʵĻ����С.
				max_length = (std::min)(max_length, m_chunked_size);
			}

			// ��ȡ���ݵ�m_response, �����ѹ��, ��Ҫ��handle_async_read�н�ѹ.
			boost::asio::streambuf::mutable_buffers_type bufs = m_response.prepare(max_length);
			typedef boost::function<void (boost::system::error_code, std::size_t)> HandlerWrapper;
			HandlerWrapper h(handler);
			m_sock.async_read_some(boost::asio::buffer(bufs),
				boost::bind(&http_stream::handle_async_read<MutableBufferSequence, HandlerWrapper>,
					this, buffers, h,
					boost::asio::placeholders::error,
					boost::asio::placeholders::bytes_transferred
				)
			);
			return;
		}
	}

	if (m_response.size() > 0)
	{
		std::size_t bytes_transferred = read_some(buffers, ec);
		m_io_service.post(
			boost::asio::detail::bind_handler(handler, ec, bytes_transferred));
		return;
	}

	{
#ifdef AVHTTP_ENABLE_ZLIB
		// ���û������gzip, ���ж���keep-aliveģʽ��, �û���ȡ�Ƿ�
		// ������, �����ȡ������, ��ص�����Ϊ0, ����������.
		if (!m_is_gzip)
#endif // AVHTTP_ENABLE_ZLIB
		{
			if (m_keep_alive)
			{
				if (m_content_length != -1 && m_body_size == m_content_length)
				{
					m_io_service.post(
						boost::asio::detail::bind_handler(handler, ec, 0));
					return;
				}
			}
		}

		std::size_t max_length = 0;

		// ����Ϊ0��ֱ�Ӷ�ȡm_response�е�����, �����ٴ�socket��ȡ����, ����
		// ��ȡ���ݵ�β����ʱ��, ������ʱ��ȴ������.
		if (m_response.size() != 0)
			max_length = 0;
		else
		{
			typename MutableBufferSequence::const_iterator iter = buffers.begin();
			typename MutableBufferSequence::const_iterator end = buffers.end();
			// ����õ��û�buffer_size�ܴ�С.
			for (; iter != end; ++iter)
			{
				boost::asio::mutable_buffer buffer(*iter);
				max_length += boost::asio::buffer_size(buffer);
			}
			// �õ����ʵĻ����С.
			max_length = (std::min)(
				(boost::int64_t)max_length, m_content_length == -1 ? 1024 : m_content_length);
		}

		// ��ȡ���ݵ�m_response, �����ѹ��, ��Ҫ��handle_read�н�ѹ.
		boost::asio::streambuf::mutable_buffers_type bufs = m_response.prepare(max_length);
		typedef boost::function<void (boost::system::error_code, std::size_t)> HandlerWrapper;
		HandlerWrapper h(handler);
		m_sock.async_read_some(boost::asio::buffer(bufs),
			boost::bind(&http_stream::handle_read<MutableBufferSequence, HandlerWrapper>,
				this, buffers, h,
				boost::asio::placeholders::error,
				boost::asio::placeholders::bytes_transferred
			)
		);
	}
}

template <typename ConstBufferSequence>
std::size_t http_stream::write_some(const ConstBufferSequence& buffers)
{
	boost::system::error_code ec;
	std::size_t bytes_transferred = write_some(buffers, ec);
	if (ec)
	{
		boost::throw_exception(boost::system::system_error(ec));
	}
	return bytes_transferred;
}

template <typename ConstBufferSequence>
std::size_t http_stream::write_some(const ConstBufferSequence& buffers,
	boost::system::error_code& ec)
{
	std::size_t bytes_transferred = m_sock.write_some(buffers, ec);
	if (ec == boost::asio::error::shut_down)
		ec = boost::asio::error::eof;
	return bytes_transferred;
}

template <typename ConstBufferSequence, typename Handler>
void http_stream::async_write_some(const ConstBufferSequence& buffers, BOOST_ASIO_MOVE_ARG(Handler) handler)
{
	AVHTTP_WRITE_HANDLER_CHECK(Handler, handler) type_check;

	m_sock.async_write_some(buffers, handler);
}

void http_stream::request(request_opts& opt)
{
	boost::system::error_code ec;
	request(opt, ec);
	if (ec)
	{
		boost::throw_exception(boost::system::system_error(ec));
	}
}

void http_stream::request(request_opts& opt, boost::system::error_code& ec)
{
	// �ж�socket�Ƿ��.
	if (!m_sock.is_open())
	{
		ec = boost::asio::error::network_reset;
		AVHTTP_LOG_ERR("Socket is open, error message\'" << ec.message() << "\'");
		return;
	}

	// ���浽һ���µ�opts�в���.
	request_opts opts = opt;
	// ���.
	m_request_opts.clear();
	m_is_chunked_end = false;
	m_skip_crlf = true;
	m_is_chunked = false;
	m_keep_alive = true;
	m_body_size = 0;

	// �õ�urlѡ��.
	std::string new_url;
	if (opts.find(http_options::url, new_url))
		opts.remove(http_options::url);		// ɾ���������ѡ��.

	if (!new_url.empty())
	{
		BOOST_ASSERT(url::from_string(new_url).host() == m_url.host());	// ������ͬһ����.
		m_url = new_url;
		m_request_opts.insert(http_options::url, new_url);
	}

	// �õ�request_method.
	std::string request_method = "GET";
	if (opts.find(http_options::request_method, request_method))
		opts.remove(http_options::request_method);	// ɾ���������ѡ��.
	m_request_opts.insert(http_options::request_method, request_method);

	// �õ�http�汾��Ϣ.
	std::string http_version = "HTTP/1.1";
	if (opts.find(http_options::http_version, http_version))
		opts.remove(http_options::http_version);	// ɾ���������ѡ��.
	m_request_opts.insert(http_options::http_version, http_version);

	// �õ�Host��Ϣ.
	std::string host = m_url.to_string(url::host_component | url::port_component);
	if (opts.find(http_options::host, host))
		opts.remove(http_options::host);	// ɾ���������ѡ��.
	m_request_opts.insert(http_options::host, host);

	// �õ�Accept��Ϣ.
	std::string accept = "text/html, application/xhtml+xml, */*";
	if (opts.find(http_options::accept, accept))
		opts.remove(http_options::accept);	// ɾ���������ѡ��.
	m_request_opts.insert(http_options::accept, accept);

	// ���user_agent.
	std::string user_agent = AVHTTP_VERSION_MIME;
	if (opts.find(http_options::user_agent, user_agent))
		opts.remove(http_options::user_agent);	// ɾ���������ѡ��.
	m_request_opts.insert(http_options::user_agent, user_agent);

	// ���cookies.
	std::string cookie = m_cookies.get_cookie_line(m_protocol == "https");

	// �������֤����.
	std::string auth;
	if ((m_proxy.type == proxy_settings::http_pw || m_proxy.type == proxy_settings::http)
		&& m_protocol != "https")
	{
		if (m_proxy.type == proxy_settings::http_pw)
		{
			auth = m_proxy.username + ":" + m_proxy.password;
			auth = "Basic " + detail::encode_base64(auth);
			m_request_opts.insert("Proxy-Authorization", auth);
		}
		else if (!m_url.user_info().empty())
		{
			auth = "Basic " + detail::encode_base64(m_url.user_info());
			m_request_opts.insert("Proxy-Authorization", auth);
		}
	}

	// Ĭ�����close.
	std::string connection = "close";
	if ((m_proxy.type == proxy_settings::http_pw || m_proxy.type == proxy_settings::http)
		&& m_protocol != "https")
	{
		if (opts.find(http_options::proxy_connection, connection))
			opts.remove(http_options::proxy_connection);		// ɾ���������ѡ��.
		m_request_opts.insert(http_options::proxy_connection, connection);
	}
	else
	{
		if (opts.find(http_options::connection, connection))
			opts.remove(http_options::connection);		// ɾ���������ѡ��.
		m_request_opts.insert(http_options::connection, connection);
	}

	// �Ƿ����bodyѡ��.
	std::string body;
	if (opts.find(http_options::request_body, body))
		opts.remove(http_options::request_body);	// ɾ���������ѡ��.
	m_request_opts.insert(http_options::request_body, body);

	// ѭ����������ѡ��.
	std::string other_option_string;
	request_opts::option_item_list& list = opts.option_all();
	for (request_opts::option_item_list::iterator val = list.begin(); val != list.end(); val++)
	{
		if (val->first == http_options::path ||
			val->first == http_options::url ||
			val->first == http_options::request_method ||
			val->first == http_options::http_version ||
			val->first == http_options::request_body ||
			val->first == http_options::status_code)
			continue;
		other_option_string += (val->first + ": " + val->second + "\r\n");
		m_request_opts.insert(val->first, val->second);
	}

	// ���ϸ�ѡ�Http�����ַ�����.
	std::string request_string;
	m_request.consume(m_request.size());
	std::ostream request_stream(&m_request);
	request_stream << request_method << " ";
	if ((m_proxy.type == proxy_settings::http_pw || m_proxy.type == proxy_settings::http)
		&& m_protocol != "https")
	{
		request_stream << m_url.to_string();
		m_request_opts.insert(http_options::path, m_url.to_string());
	}
	else
	{
		request_stream << m_url.to_string(url::path_component |
			url::query_component | url::fragment_component);
		m_request_opts.insert(http_options::path, m_url.to_string(
			url::path_component | url::query_component | url::fragment_component));
	}
	request_stream << " " << http_version << "\r\n";
	request_stream << "Host: " << host << "\r\n";
	request_stream << "Accept: " << accept << "\r\n";
	request_stream << "User-Agent: " << user_agent << "\r\n";
	if (!cookie.empty())
	{
		request_stream << "Cookie: " << cookie << "\r\n";
	}
	if ((m_proxy.type == proxy_settings::http_pw || m_proxy.type == proxy_settings::http)
		&& m_protocol != "https")
	{
		request_stream << "Proxy-Connection: " << connection << "\r\n";
	}
	else
	{
		request_stream << "Connection: " << connection << "\r\n";
	}
	if (!auth.empty())
	{
		request_stream << "Proxy-Authorization: " << auth << "\r\n";
	}
	request_stream << other_option_string << "\r\n";
	if (!body.empty())
	{
		request_stream << body;
	}

#if defined(DEBUG) || defined(_DEBUG)
	{
		int request_size = m_request.size();
		boost::asio::streambuf::const_buffers_type::const_iterator begin(m_request.data().begin());
		const char* ptr = boost::asio::buffer_cast<const char*>(*begin);
		AVHTTP_LOG_DBG("Request Header:\n" << std::string(ptr, request_size));
	}
#endif

	// ��������.
	boost::asio::write(m_sock, m_request, ec);
	if (ec)
	{
		AVHTTP_LOG_ERR("Send request, error message: \'" << ec.message() <<"\'");
		return;
	}

	// ��Ҫ����fake continue��Ϣ.
	if (m_request_opts_priv.fake_continue())
	{
		ec = errc::fake_continue;
		AVHTTP_LOG_WARN("Return \'" << ec.message() <<"\', "
			"need call receive_header for continue receive the http header.");
		return;
	}

	// ��ȡhttpͷ.
	receive_header(ec);
}

template <typename Handler>
void http_stream::async_request(const request_opts& opt, BOOST_ASIO_MOVE_ARG(Handler) handler)
{
	AVHTTP_REQUEST_HANDLER_CHECK(Handler, handler) type_check;

	boost::system::error_code ec;

	// �ж�socket�Ƿ��.
	if (!m_sock.is_open())
	{
		ec = boost::asio::error::network_reset;
		AVHTTP_LOG_ERR("Socket is open, error message\'" << ec.message() << "\'");
		handler(ec);
		return;
	}

	// ���浽һ���µ�opts�в���.
	request_opts opts = opt;
	// ���.
	m_request_opts.clear();
	m_skip_crlf = true;
	m_is_chunked = false;
	m_is_chunked_end = false;
	m_keep_alive = true;
	m_body_size = 0;

	// �õ�urlѡ��.
	std::string new_url;
	if (opts.find(http_options::url, new_url))
		opts.remove(http_options::url);		// ɾ���������ѡ��.

	if (!new_url.empty())
	{
		BOOST_ASSERT(url::from_string(new_url).host() == m_url.host());	// ������ͬһ����.
		m_url = new_url;
		m_request_opts.insert(http_options::url, new_url);
	}

	// �õ�request_method.
	std::string request_method = "GET";
	if (opts.find(http_options::request_method, request_method))
		opts.remove(http_options::request_method);	// ɾ���������ѡ��.
	m_request_opts.insert(http_options::request_method, request_method);

	// �õ�http�İ汾��Ϣ.
	std::string http_version = "HTTP/1.1";
	if (opts.find(http_options::http_version, http_version))
		opts.remove(http_options::http_version);	// ɾ���������ѡ��.
	m_request_opts.insert(http_options::http_version, http_version);

	// �õ�Host��Ϣ.
	std::string host = m_url.to_string(url::host_component | url::port_component);
	if (opts.find(http_options::host, host))
		opts.remove(http_options::host);	// ɾ���������ѡ��.
	m_request_opts.insert(http_options::host, host);

	// �õ�Accept��Ϣ.
	std::string accept = "text/html, application/xhtml+xml, */*";
	if (opts.find(http_options::accept, accept))
		opts.remove(http_options::accept);	// ɾ���������ѡ��.
	m_request_opts.insert(http_options::accept, accept);

	// ���user_agent.
	std::string user_agent = AVHTTP_VERSION_MIME;
	if (opts.find(http_options::user_agent, user_agent))
		opts.remove(http_options::user_agent);	// ɾ���������ѡ��.
	m_request_opts.insert(http_options::user_agent, user_agent);

	// ���cookies.
	std::string cookie = m_cookies.get_cookie_line(m_protocol == "https");

	// �������֤����.
	std::string auth;
	if ((m_proxy.type == proxy_settings::http_pw || m_proxy.type == proxy_settings::http)
		&& m_protocol != "https")
	{
		if (m_proxy.type == proxy_settings::http_pw)
		{
			auth = m_proxy.username + ":" + m_proxy.password;
			auth = "Basic " + detail::encode_base64(auth);
			m_request_opts.insert("Proxy-Authorization", auth);
		}
		else if (!m_url.user_info().empty())
		{
			auth = "Basic " + detail::encode_base64(m_url.user_info());
			m_request_opts.insert("Proxy-Authorization", auth);
		}
	}

	// Ĭ�����close.
	std::string connection = "close";
	if ((m_proxy.type == proxy_settings::http_pw || m_proxy.type == proxy_settings::http)
		&& m_protocol != "https")
	{
		if (opts.find(http_options::proxy_connection, connection))
			opts.remove(http_options::proxy_connection);		// ɾ���������ѡ��.
		m_request_opts.insert(http_options::proxy_connection, connection);
		if (connection == "close")
			m_keep_alive = false;
	}
	else
	{
		if (opts.find(http_options::connection, connection))
			opts.remove(http_options::connection);		// ɾ���������ѡ��.
		m_request_opts.insert(http_options::connection, connection);
	}

	// �Ƿ����bodyѡ��.
	std::string body;
	if (opts.find(http_options::request_body, body))
		opts.remove(http_options::request_body);	// ɾ���������ѡ��.
	m_request_opts.insert(http_options::request_body, body);

	// ѭ����������ѡ��.
	std::string other_option_string;
	request_opts::option_item_list& list = opts.option_all();
	for (request_opts::option_item_list::iterator val = list.begin(); val != list.end(); val++)
	{
		if (val->first == http_options::path ||
			val->first == http_options::url ||
			val->first == http_options::request_method ||
			val->first == http_options::http_version ||
			val->first == http_options::request_body ||
			val->first == http_options::status_code)
			continue;
		other_option_string += (val->first + ": " + val->second + "\r\n");
		m_request_opts.insert(val->first, val->second);
	}

	// ���ϸ�ѡ�Http�����ַ�����.
	std::string request_string;
	m_request.consume(m_request.size());
	std::ostream request_stream(&m_request);
	request_stream << request_method << " ";
	if ((m_proxy.type == proxy_settings::http_pw || m_proxy.type == proxy_settings::http)
		&& m_protocol != "https")
	{
		request_stream << m_url.to_string();
		m_request_opts.insert(http_options::path, m_url.to_string());
	}
	else
	{
		request_stream << m_url.to_string(url::path_component |
			url::query_component | avhttp::url::fragment_component);
		m_request_opts.insert(http_options::path, m_url.to_string(
			url::path_component | url::query_component | avhttp::url::fragment_component));
	}
	request_stream << " " << http_version << "\r\n";
	request_stream << "Host: " << host << "\r\n";
	request_stream << "Accept: " << accept << "\r\n";
	if (!auth.empty())
	{
		request_stream << "Proxy-Authorization: " << auth << "\r\n";
	}
	request_stream << "User-Agent: " << user_agent << "\r\n";
	if (!cookie.empty())
	{
		request_stream << "Cookie: " << cookie << "\r\n";
	}
	if ((m_proxy.type == proxy_settings::http_pw || m_proxy.type == proxy_settings::http)
		&& m_protocol != "https")
	{
		request_stream << "Proxy-Connection: " << connection << "\r\n";
	}
	else
	{
		request_stream << "Connection: " << connection << "\r\n";
	}
	request_stream << other_option_string << "\r\n";
	if (!body.empty())
	{
		request_stream << body;
	}

#if defined(DEBUG) || defined(_DEBUG)
	{
		int request_size = m_request.size();
		boost::asio::streambuf::const_buffers_type::const_iterator begin(m_request.data().begin());
		const char* ptr = boost::asio::buffer_cast<const char*>(*begin);
		AVHTTP_LOG_DBG("Request Header:\n" << std::string(ptr, request_size));
	}
#endif

	// �첽��������.
	typedef boost::function<void (boost::system::error_code)> HandlerWrapper;
	boost::asio::async_write(m_sock, m_request, boost::asio::transfer_exactly(m_request.size()),
		boost::bind(&http_stream::handle_request<HandlerWrapper>,
			this, HandlerWrapper(handler),
			boost::asio::placeholders::error
		)
	);
}

void http_stream::receive_header()
{
	boost::system::error_code ec;
	receive_header(ec);
	if (ec)
	{
		boost::throw_exception(boost::system::system_error(ec));
	}
}

void http_stream::receive_header(boost::system::error_code& ec)
{
	// ѭ����ȡ.
	for (;;)
	{
		boost::asio::read_until(m_sock, m_response, "\r\n", ec);
		if (ec)
		{
			AVHTTP_LOG_ERR("Read status line, error message: \'" << ec.message() <<"\'");
			return;
		}

		// ���Ƶ��µ�streambuf�д�������http״̬, �������http״̬��, ��ô������m_response�е�����,
		// ����Ҫ��Ϊ�˼��ݷǱ�׼http������ֱ����ͻ��˷����ļ�����Ҫ, ������Ȼ��Ҫ��malformed_status_line
		// ֪ͨ�û�, malformed_status_line������ζ�����ӹر�, ����m_response�е�������δ���, ���û��Լ�
		// �����Ƿ��ȡ, ��ʱ, �û�����ʹ��read_some/async_read_some����ȡ��������ϵ���������.
		boost::asio::streambuf tempbuf;
		int response_size = m_response.size();
		boost::asio::streambuf::const_buffers_type::const_iterator begin(m_response.data().begin());
		const char* ptr = boost::asio::buffer_cast<const char*>(*begin);
		std::ostream tempbuf_stream(&tempbuf);
		tempbuf_stream.write(ptr, response_size);

		// ���http״̬��, version_major��version_minor��httpЭ��İ汾��.
		int version_major = 0;
		int version_minor = 0;
		m_status_code = 0;
		if (!detail::parse_http_status_line(
			std::istreambuf_iterator<char>(&tempbuf),
			std::istreambuf_iterator<char>(),
			version_major, version_minor, m_status_code))
		{
			ec = errc::malformed_status_line;
			AVHTTP_LOG_ERR("Malformed status line");
			return;
		}

		// "continue"��Ϣ�д���2��CRLF, ������Ҫ���������1��CRLF.
		if (m_status_code == errc::continue_request)
		{
			// �����ʱtempbuf�����ݴ�С����2, ����������1��\r\n.
			if (tempbuf.size() >= 2)
			{
				std::iostream ios(&tempbuf);
				char crlf[2];
				ios.read(&crlf[0], 2);
				BOOST_ASSERT(crlf[0] == '\r' && crlf[1] == '\n');
				if (crlf[0] != '\r' || crlf[1] != '\n')
				{
					ec = errc::malformed_status_line;
					AVHTTP_LOG_ERR("Malformed status line");
					return;
				}
			}
			else
			{
				BOOST_ASSERT(0);	// Fuck! ����׼��http������������
				ec = errc::malformed_status_line;
				AVHTTP_LOG_ERR("Malformed status line");
				return;
			}

			m_response.consume(2);
		}

		// �����״̬����ռ�õ��ֽ���.
		m_response.consume(response_size - tempbuf.size());

		// ���http״̬���벻��ok��partial_content, ����status_code����һ��http_code, ����
		// ��Ҫ�ж�http_code�ǲ���302����ת, �����, �򽫽�����ת�߼�; �����http�����˴���
		// , ��ֱ�ӷ������״̬�����.
		if (m_status_code != errc::ok &&
			m_status_code != errc::partial_content)
		{
			ec = make_error_code(static_cast<errc::errc_t>(m_status_code));
		}

		// �����POST�Ļ�, �Ͳ�����, ��Ϊ������Ҫ����״̬, �Ա���������ļ�����.
		if (m_status_code != errc::continue_request ||
			m_request_opts_priv.find(http_options::request_method) == "POST")
			break;
	} // end for.

	AVHTTP_LOG_DBG("Status code: " << m_status_code);

	// ���ԭ�еķ���ѡ��, �����״̬��.
	m_response_opts.clear();
	m_response_opts.insert("_status_code", boost::str(boost::format("%d") % m_status_code));

	// �������errc::continue_request, ֱ�ӻص�, �����������������\r\n\r\n����.
	if (m_status_code == errc::continue_request)
	{
		ec = make_error_code(static_cast<errc::errc_t>(m_status_code));
		return;
	}

	// ѭ����������http header, �����浽header_string.
	std::string header_string;
	boost::system::error_code read_err;
	do
	{
		std::string temp;
		std::size_t bytes_transferred = boost::asio::read_until(m_sock, m_response, "\r\n", read_err);
		if (read_err)
		{
			// ˵�������˳���, ��û�еõ�Http header, ���ش�����ļ�ͷ��Ϣ��������eof.
			if (read_err == boost::asio::error::eof)
				ec = errc::malformed_response_headers;
			else
				ec = read_err;
			AVHTTP_LOG_ERR("Header error, error message: \'" << ec.message() << "\'");
			return;
		}

		// ���·���ռ�.
		temp.resize(bytes_transferred);
		m_response.sgetn(&temp[0], bytes_transferred);
		header_string += temp;

		// http header��β.
		if (temp == "\r\n")
			break;
	} while (true);

	AVHTTP_LOG_DBG("Http header:\n" << header_string);

	// ����Http Header.
	if (!detail::parse_http_headers(header_string.begin(), header_string.end(),
		m_content_type, m_content_length, m_location, m_response_opts.option_all()))
	{
		ec = errc::malformed_response_headers;
		AVHTTP_LOG_ERR("Parse header error, error message: \'" << ec.message() << "\'");
		return;
	}

	// �ж��Ƿ���Ҫ����location.
	if (!m_location.empty())
	{
		std::size_t found = m_location.find("://");
		if (found == std::string::npos)
		{
			// ��ѯlocation���Ƿ���Э����ر�ʶ, ���û��http��httpsǰ�, �����.
			std::string prefix = m_url.to_string(
				url::protocol_component|url::host_component|url::port_component);
			m_location = prefix + "/" + m_location;
		}
	}

	// �����Ƿ�������gzѹ��.
	std::string opt_str = m_response_opts.find(http_options::content_encoding);
#ifdef AVHTTP_ENABLE_ZLIB
	if (opt_str == "gzip" || opt_str == "x-gzip")
	{
		m_is_gzip = true;
		if (!m_stream.zalloc)	// ��ʼ��ZLIB��, ÿ�ν�ѹÿ��chunked��ʱ��, ����Ҫ���³�ʼ��.
		{
			if (inflateInit2(&m_stream, 32+15 ) != Z_OK)
			{
				ec = boost::asio::error::operation_not_supported;
				AVHTTP_LOG_ERR("Init zlib invalid, error message: \'" << ec.message() << "\'");
				return;
			}
		}
	}
#endif
	// �Ƿ�������chunked.
	opt_str = m_response_opts.find(http_options::transfer_encoding);
	if (opt_str == "chunked")
		m_is_chunked = true;
	// �Ƿ���������ɺ�ر�socket.
	opt_str = m_request_opts.find(http_options::connection);
	if (opt_str == "close")
		m_keep_alive = false;
	opt_str = m_response_opts.find(http_options::connection);
	if (opt_str == "close")
		m_keep_alive = false;

	// ����cookie��cookie����.
	response_opts::option_item_list& option_list = m_response_opts.option_all();
	response_opts::option_item_list::const_iterator i = option_list.begin();
	for (; i != option_list.end(); i++)
	{
		if (boost::to_lower_copy(i->first) == "set-cookie")
		{
			m_cookies(i->second);	// ����cookie�ַ���, �����浽m_cookies.
		}
	}
}

template <typename Handler>
void http_stream::async_receive_header(BOOST_ASIO_MOVE_ARG(Handler) handler)
{
	AVHTTP_RECEIVE_HEADER_CHECK(Handler, handler) type_check;

	// �첽��ȡHttp status.
	boost::asio::async_read_until(m_sock, m_response, "\r\n",
		boost::bind(&http_stream::handle_status<Handler>,
			this, handler,
			boost::asio::placeholders::error
		)
	);
}

void http_stream::clear()
{
	m_request.consume(m_request.size());
	m_response.consume(m_response.size());
}

void http_stream::close()
{
	boost::system::error_code ec;
	close(ec);
	if (ec)
	{
		boost::throw_exception(boost::system::system_error(ec));
	}
}

void http_stream::close(boost::system::error_code& ec)
{
	ec = boost::system::error_code();

	if (is_open())
	{
		// �ر�socket.
		m_sock.close(ec);

		// ����ڲ��ĸ��ֻ�����Ϣ.
		m_request.consume(m_request.size());
		m_response.consume(m_response.size());
		m_content_type.clear();
		m_location.clear();
		m_protocol.clear();
	}
}

bool http_stream::is_open() const
{
	return m_sock.is_open();
}

boost::asio::io_service& http_stream::get_io_service()
{
	return m_io_service;
}

void http_stream::max_redirects(int n)
{
	m_max_redirects = n;
}

void http_stream::proxy(const proxy_settings& s)
{
	m_proxy = s;
}

void http_stream::request_options(const request_opts& options)
{
	m_request_opts_priv = options;
}

request_opts http_stream::request_options(void) const
{
	if (m_request_opts.size() == 0)
		return m_request_opts_priv;
	return m_request_opts;
}

response_opts http_stream::response_options(void) const
{
	return m_response_opts;
}

const cookies& http_stream::http_cookies() const
{
	return m_cookies;
}

void http_stream::http_cookies(const cookies& cookie)
{
	m_cookies = cookie;
}

const std::string& http_stream::location() const
{
	return m_location;
}

const std::string http_stream::final_url() const
{
	return m_url.to_string();
}

const std::string http_stream::entry_url() const
{
	return m_entry_url.to_string();
}

boost::int64_t http_stream::content_length()
{
	return m_content_length;
}

void http_stream::check_certificate(bool is_check)
{
#ifdef AVHTTP_ENABLE_OPENSSL
	m_check_certificate = is_check;
#endif
}

void http_stream::add_verify_path(const std::string& path)
{
	m_ca_directory = path;
	return;
}

void http_stream::load_verify_file(const std::string& filename)
{
	m_ca_cert = filename;
	return;
}


// ����Ϊ�ڲ����ʵ��, �ǽӿ�.

template <typename MutableBufferSequence>
std::size_t http_stream::read_some_impl(const MutableBufferSequence& buffers,
	boost::system::error_code& ec)
{
	// �������������m_response��, �ȶ�ȡm_response�е�����.
	if (m_response.size() > 0)
	{
		std::size_t bytes_transferred = 0;
		typename MutableBufferSequence::const_iterator iter = buffers.begin();
		typename MutableBufferSequence::const_iterator end = buffers.end();
		for (; iter != end && m_response.size() > 0; ++iter)
		{
			boost::asio::mutable_buffer buffer(*iter);
			std::size_t length = boost::asio::buffer_size(buffer);
			if (length > 0)
			{
				bytes_transferred += m_response.sgetn(
					boost::asio::buffer_cast<char*>(buffer), length);
			}
		}
		ec = boost::system::error_code();
		return bytes_transferred;
	}

	// �ٴ�socket�ж�ȡ����.
	std::size_t bytes_transferred = m_sock.read_some(buffers, ec);
	if (ec == boost::asio::error::shut_down)
		ec = boost::asio::error::eof;
	return bytes_transferred;
}

template <typename Handler>
void http_stream::handle_resolve(const boost::system::error_code& err,
	tcp::resolver::iterator endpoint_iterator, Handler handler)
{
	if (!err)
	{
		// �����첽����.
		// !!!��ע: ����m_sock������ssl, ��ô���ӵ��������ʵ�ֱ���װ��ssl_stream
		// ��, ����, �����Ҫʹ��boost::asio::async_connect�Ļ�, ��Ҫ��http_stream
		// ��ʵ�����ֲ���, ���򽫻�õ�һ������.
		m_sock.async_connect(tcp::endpoint(*endpoint_iterator),
			boost::bind(&http_stream::handle_connect<Handler>,
				this, handler, endpoint_iterator,
				boost::asio::placeholders::error
			)
		);
	}
	else
	{
		AVHTTP_LOG_ERR("Resolve DNS error, \'" << m_url.host() <<
			"\', error message \'" << err.message() << "\'");
		// ����ص�.
		handler(err);
	}
}

template <typename Handler>
void http_stream::handle_connect(Handler handler,
	tcp::resolver::iterator endpoint_iterator, const boost::system::error_code& err)
{
	if (!err)
	{
		AVHTTP_LOG_DBG("Connect to \'" << m_url.host() << "\'.");
		// �����첽����.
		async_request(m_request_opts_priv, handler);
	}
	else
	{
		// ����Ƿ��Ѿ�������endpoint�б��е�����endpoint.
		if (++endpoint_iterator == tcp::resolver::iterator())
		{
			AVHTTP_LOG_ERR("Connect to \'" << m_url.host() <<
				"\', error message \'" << err.message() << "\'");
			handler(err);
		}
		else
		{
			// ���������첽����.
			// !!!��ע: ����m_sock������ssl, ��ô���ӵ��������ʵ�ֱ���װ��ssl_stream
			// ��, ����, �����Ҫʹ��boost::asio::async_connect�Ļ�, ��Ҫ��http_stream
			// ��ʵ�����ֲ���, ���򽫻�õ�һ������.
			m_sock.async_connect(tcp::endpoint(*endpoint_iterator),
				boost::bind(&http_stream::handle_connect<Handler>,
					this, handler, endpoint_iterator,
					boost::asio::placeholders::error
				)
			);
		}
	}
}

template <typename Handler>
void http_stream::handle_request(Handler handler, const boost::system::error_code& err)
{
	// ��������.
	if (err)
	{
		AVHTTP_LOG_ERR("Send request, error message: \'" << err.message() <<"\'");
		handler(err);
		return;
	}

	// ��Ҫ����fake continue��Ϣ.
	if (m_request_opts_priv.fake_continue())
	{
		boost::system::error_code ec = errc::fake_continue;
		AVHTTP_LOG_WARN("Return \'" << ec.message() <<"\', "
			"need call receive_header for continue receive the http header.");
		handler(ec);
		return;
	}

	// �첽��ȡHttp status.
	boost::asio::async_read_until(m_sock, m_response, "\r\n",
		boost::bind(&http_stream::handle_status<Handler>,
			this, handler,
			boost::asio::placeholders::error
		)
	);
}

template <typename Handler>
void http_stream::handle_status(Handler handler, const boost::system::error_code& err)
{
	// ��������.
	if (err)
	{
		AVHTTP_LOG_ERR("Read status line, error message: \'" << err.message() <<"\'");
		handler(err);
		return;
	}

	// ���Ƶ��µ�streambuf�д�������http״̬, �������http״̬��, ��ô������m_response�е�����,
	// ����Ҫ��Ϊ�˼��ݷǱ�׼http������ֱ����ͻ��˷����ļ�����Ҫ, ������Ȼ��Ҫ��malformed_status_line
	// ֪ͨ�û�, malformed_status_line������ζ�����ӹر�, ����m_response�е�������δ���, ���û��Լ�
	// �����Ƿ��ȡ, ��ʱ, �û�����ʹ��read_some/async_read_some����ȡ��������ϵ���������.
	boost::asio::streambuf tempbuf;
	int response_size = m_response.size();
	boost::asio::streambuf::const_buffers_type::const_iterator begin(m_response.data().begin());
	const char* ptr = boost::asio::buffer_cast<const char*>(*begin);
	std::ostream tempbuf_stream(&tempbuf);
	tempbuf_stream.write(ptr, response_size);

	// ���http״̬��, version_major��version_minor��httpЭ��İ汾��.
	int version_major = 0;
	int version_minor = 0;
	m_status_code = 0;
	if (!detail::parse_http_status_line(
		std::istreambuf_iterator<char>(&tempbuf),
		std::istreambuf_iterator<char>(),
		version_major, version_minor, m_status_code))
	{
		AVHTTP_LOG_ERR("Malformed status line");
		handler(errc::malformed_status_line);
		return;
	}

	// "continue"��Ϣ�д���2��CRLF, ������Ҫ���������1��CRLF.
	if (m_status_code == errc::continue_request)
	{
		// �����ʱtempbuf�����ݴ�С����2, ����������1��CRLF.
		if (tempbuf.size() >= 2)
		{
			std::iostream ios(&tempbuf);
			char crlf[2];
			ios.read(&crlf[0], 2);
			BOOST_ASSERT(crlf[0] == '\r' && crlf[1] == '\n');
			if (crlf[0] != '\r' || crlf[1] != '\n')
			{
				AVHTTP_LOG_ERR("Malformed status line");
				handler(errc::malformed_status_line);
				return;
			}
		}
		else
		{
			BOOST_ASSERT(0);	// Fuck! ����׼��http������������
			AVHTTP_LOG_ERR("Malformed status line");
			handler(errc::malformed_status_line);
			return;
		}

		m_response.consume(2);
	}

	// �����״̬����ռ�õ��ֽ���.
	m_response.consume(response_size - tempbuf.size());

	// "continue"��ʾ������Ҫ�����ȴ�����״̬, �����POST����ֱ�ӷ�����POST�м�������
	// ���ݵĻ���.
	if (m_status_code == errc::continue_request &&
		m_request_opts_priv.find(http_options::request_method) != "POST")
	{
		boost::asio::async_read_until(m_sock, m_response, "\r\n",
			boost::bind(&http_stream::handle_status<Handler>,
				this, handler,
				boost::asio::placeholders::error
			)
		);
	}
	else
	{
		// ���ԭ�еķ���ѡ��, �����״̬��.
		m_response_opts.clear();
		m_response_opts.insert("_status_code", boost::str(boost::format("%d") % m_status_code));

		// ���Ϊerrc::continue_request, ֱ�ӻص�, �����������ȡ"\r\n\r\n"��һֱ�ȴ�.
		if (m_status_code == errc::continue_request)
		{
			handler(make_error_code(static_cast<errc::errc_t>(m_status_code)));
			return;
		}

		// �첽��ȡ����Http header����.
		boost::asio::async_read_until(m_sock, m_response, "\r\n",
			boost::bind(&http_stream::handle_header<Handler>,
				this, handler, std::string(""),
				boost::asio::placeholders::bytes_transferred,
				boost::asio::placeholders::error
			)
		);
	}
}

template <typename Handler>
void http_stream::handle_header(Handler handler,
	std::string header_string, int bytes_transferred, const boost::system::error_code& err)
{
	if (err)
	{
		AVHTTP_LOG_ERR("Header error, error message: \'" << err.message() << "\'");
		handler(err);
		return;
	}

	// �õ�http header��ĳһ��.
	std::string temp;
	temp.resize(bytes_transferred);
	m_response.sgetn(&temp[0], bytes_transferred);
	header_string += temp;

	// �������Http headerβ��, �������ȡheader.
	if (temp != "\r\n")
	{
		boost::asio::async_read_until(m_sock, m_response, "\r\n",
			boost::bind(&http_stream::handle_header<Handler>,
				this, handler, header_string,
				boost::asio::placeholders::bytes_transferred,
				boost::asio::placeholders::error
			)
		);
		return;
	}

	AVHTTP_LOG_DBG("Status code: " << m_status_code);
	AVHTTP_LOG_DBG("Http header:\n" << header_string);

	boost::system::error_code ec;

	// ����Http Header.
	if (!detail::parse_http_headers(header_string.begin(), header_string.end(),
		m_content_type, m_content_length, m_location, m_response_opts.option_all()))
	{
		ec = errc::malformed_response_headers;
		AVHTTP_LOG_ERR("Parse header error, error message: \'" << ec.message() << "\'");
		handler(ec);
		return;
	}

	// �ж��Ƿ���Ҫ����location.
	if (!m_location.empty())
	{
		std::size_t found = m_location.find("://");
		if (found == std::string::npos)
		{
			// ��ѯlocation���Ƿ���Э����ر�ʶ, ���û��http��httpsǰ�, �����.
			std::string prefix = m_url.to_string(
				url::protocol_component|url::host_component|url::port_component);
			m_location = prefix + "/" + m_location;
		}
	}

	// �ж��Ƿ���Ҫ��ת.
	if (m_status_code == errc::moved_permanently || m_status_code == errc::found)
	{
		m_sock.close(ec);
		if (++m_redirects <= m_max_redirects)
		{
			url new_url = url::from_string(m_location, ec);
			if (ec == boost::system::errc::invalid_argument)
			{
				// ���û�������ת��ַ����.
				ec = errc::invalid_redirect;
				AVHTTP_LOG_ERR("Location url invalid, error message: \'" << ec.message() << "\'");
				handler(ec);
				return;
			}
			async_open(new_url, handler);
			return;
		}
	}

	// ����ض������.
	m_redirects = 0;

	if (m_status_code != errc::ok && m_status_code != errc::partial_content)
		ec = make_error_code(static_cast<errc::errc_t>(m_status_code));

	// �����Ƿ�������gzѹ��.
	std::string opt_str = m_response_opts.find(http_options::content_encoding);
#ifdef AVHTTP_ENABLE_ZLIB
	if (opt_str == "gzip" || opt_str == "x-gzip")
	{
		m_is_gzip = true;
		if (!m_stream.zalloc)
		{
			if (inflateInit2(&m_stream, 32+15 ) != Z_OK)	// ��ʼ��ZLIB��, ÿ�ν�ѹÿ��chunked��ʱ��, ����Ҫ���³�ʼ��.
			{
				ec = boost::asio::error::operation_not_supported;
				AVHTTP_LOG_ERR("Init zlib invalid, error message: \'" << ec.message() << "\'");
				handler(ec);
				return;
			}
		}
	}
#endif
	// �Ƿ�������chunked.
	opt_str = m_response_opts.find(http_options::transfer_encoding);
	if (opt_str == "chunked")
		m_is_chunked = true;
	// �Ƿ���������ɺ�ر�socket.
	opt_str = m_request_opts.find(http_options::connection);
	if (opt_str == "close")
		m_keep_alive = false;
	opt_str = m_response_opts.find(http_options::connection);
	if (opt_str == "close")
		m_keep_alive = false;

	// ����cookie��cookie����.
	response_opts::option_item_list& option_list = m_response_opts.option_all();
	response_opts::option_item_list::const_iterator i = option_list.begin();
	for (; i != option_list.end(); i++)
	{
		if (boost::to_lower_copy(i->first) == "set-cookie")
		{
			m_cookies(i->second);	// ����cookie�ַ���, �����浽m_cookies.
		}
	}

	// �ص�֪ͨ.
	handler(ec);
}

template <typename MutableBufferSequence, typename Handler>
void http_stream::handle_read(const MutableBufferSequence& buffers, Handler handler,
	const boost::system::error_code& ec, std::size_t bytes_transferred)
{
	boost::system::error_code err;
	if (!ec || m_response.size() > 0
#ifdef AVHTTP_ENABLE_ZLIB
		|| (m_is_gzip && m_stream.avail_in > 0)	// ������ѹ����δ��ѹ�껺�����е�����.
#endif
		)
	{
		// �ύ����.
		m_response.commit(bytes_transferred);

#ifdef AVHTTP_ENABLE_ZLIB
		if (!m_is_gzip)	// ���û������gzip, ��ֱ�Ӷ�ȡ���ݺ󷵻�.
#endif
		{
			if (bytes_transferred <= 0 && m_response.size() == 0)
			{
				handler(ec, bytes_transferred);
				return;
			}
			else
			{
				bytes_transferred = read_some_impl(buffers, err);	// ������ʵ���Ǵ�m_response�ж�ȡ����.
				BOOST_ASSERT(!err);
				m_body_size += bytes_transferred;					// ͳ�ƶ�ȡbody���ֽ���.
				handler(ec, bytes_transferred);
				return;
			}
		}
#ifdef AVHTTP_ENABLE_ZLIB
		else					// �����ȡ���ݵ���ѹ������.
		{
			if (m_stream.avail_in == 0)
			{
				if (bytes_transferred <= 0 && m_response.size() == 0)
				{
					handler(ec, bytes_transferred);
					return;
				}
				else
				{
					bytes_transferred = read_some_impl(boost::asio::buffer(m_zlib_buffer, 1024), err);

					// ͳ�ƶ�ȡbody���ֽ���, �������ѹ��������, Ŀǰ��������ȫȷ��
					// δ��ѹ�����ݳ��Ⱦ���content_length, ��Ϊ����������, ѹ������
					// ������chunked��ʽ����.
					m_body_size += bytes_transferred;

					m_zlib_buffer_size = bytes_transferred;
					m_stream.avail_in = (uInt)m_zlib_buffer_size;
					m_stream.next_in = (z_const Bytef *)&m_zlib_buffer[0];
				}
			}

			bytes_transferred = 0;
			std::size_t buffer_size = 0;

			{
				typename MutableBufferSequence::const_iterator iter = buffers.begin();
				typename MutableBufferSequence::const_iterator end = buffers.end();
				// ����õ��û�buffer_size�ܴ�С.
				for (; iter != end; ++iter)
				{
					boost::asio::mutable_buffer buffer(*iter);
					m_stream.next_in = (z_const Bytef *)(&m_zlib_buffer[0] + m_zlib_buffer_size - m_stream.avail_in);
					m_stream.avail_out = boost::asio::buffer_size(buffer);
					if (m_stream.avail_out == 0)
					{
						break; // ����û��ṩ�Ļ����СΪ0, ��ֱ�ӷ���.
					}
					buffer_size += m_stream.avail_out;
					m_stream.next_out = boost::asio::buffer_cast<Bytef*>(buffer);
					int ret = inflate(&m_stream, Z_SYNC_FLUSH);
					if (ret < 0)
					{
						err = boost::asio::error::operation_not_supported;
						// ��ѹ��������, ֪ͨ�û�����������.
						handler(err, 0);
						return;
					}

					bytes_transferred += (boost::asio::buffer_size(buffer) - m_stream.avail_out);
					if (bytes_transferred != boost::asio::buffer_size(buffer))
					{
						break;
					}
				}
			}

			// û�н�ѹ������, ˵��ѹ�����ݹ���, ������ȡ, �Ա�֤�ܹ���ѹ.
			if (buffer_size != 0 && bytes_transferred == 0)
			{
				async_read_some(buffers, handler);
				return;
			}

			if (m_stream.avail_in == 0)
			{
				err = ec;	// FIXME!!!
			}

			handler(err, bytes_transferred);
			return;
		}
#endif
	}
	else
	{
		handler(ec, bytes_transferred);
	}
}

template <typename MutableBufferSequence, typename Handler>
void http_stream::handle_skip_crlf(const MutableBufferSequence& buffers,
	Handler handler, boost::shared_array<char> crlf,
	const boost::system::error_code& ec, std::size_t bytes_transferred)
{
	if (!ec)
	{
		if (m_is_chunked_end)
		{
			boost::system::error_code err;
			if (!m_keep_alive)
				err = boost::asio::error::eof;
			handler(err, bytes_transferred);
			return;
		}

		// ����CRLF? ��֪����ɶ���, ���Ե���bug.
		BOOST_ASSERT(crlf[0] == '\r' && crlf[1] == '\n');

		// ��release��, ��ȷ���ǲ��Ƿ������Ļظ�����, ��ʱ�����Ƿ������Ļظ�����!!!
		if(crlf[0] != '\r' || crlf[1] != '\n')
		{
			boost::system::error_code err = errc::invalid_chunked_encoding;
			handler(err, bytes_transferred);
			return;
		}

		// ����CRLF, ��ʼ��ȡchunked size.
		typedef boost::function<void (boost::system::error_code, std::size_t)> HandlerWrapper;
		HandlerWrapper h(handler);
		boost::asio::async_read_until(m_sock, m_response, "\r\n",
			boost::bind(&http_stream::handle_chunked_size<MutableBufferSequence, HandlerWrapper>,
				this, buffers, h,
				boost::asio::placeholders::error,
				boost::asio::placeholders::bytes_transferred
			)
		);
		return;
	}
	else
	{
		handler(ec, bytes_transferred);
	}
}

template <typename MutableBufferSequence, typename Handler>
void http_stream::handle_async_read(const MutableBufferSequence& buffers,
	Handler handler, const boost::system::error_code& ec, std::size_t bytes_transferred)
{
	boost::system::error_code err;

	if (!ec || m_response.size() > 0)
	{
		// �ύ����.
		m_response.commit(bytes_transferred);

#ifdef AVHTTP_ENABLE_ZLIB
		if (!m_is_gzip)	// ���û������gzip, ��ֱ�Ӷ�ȡ���ݺ󷵻�.
#endif
		{
			bytes_transferred = read_some_impl(boost::asio::buffer(buffers, m_chunked_size), err);
			m_chunked_size -= bytes_transferred;
			handler(err, bytes_transferred);
			return;
		}
#ifdef AVHTTP_ENABLE_ZLIB
		else					// �����ȡ���ݵ���ѹ������.
		{
			if (m_stream.avail_in == 0)
			{
				std::size_t buf_size = (std::min)(m_chunked_size, std::size_t(1024));
				bytes_transferred = read_some_impl(boost::asio::buffer(m_zlib_buffer, buf_size), err);
				m_chunked_size -= bytes_transferred;
				m_zlib_buffer_size = bytes_transferred;
				m_stream.avail_in = (uInt)m_zlib_buffer_size;
				m_stream.next_in = (z_const Bytef *)&m_zlib_buffer[0];
			}

			bytes_transferred = 0;
			std::size_t buffer_size = 0;

			{
				typename MutableBufferSequence::const_iterator iter = buffers.begin();
				typename MutableBufferSequence::const_iterator end = buffers.end();
				// ����õ��û�buffer_size�ܴ�С.
				for (; iter != end; ++iter)
				{
					boost::asio::mutable_buffer buffer(*iter);
					m_stream.next_in = (z_const Bytef *)(&m_zlib_buffer[0] + m_zlib_buffer_size - m_stream.avail_in);
					m_stream.avail_out = boost::asio::buffer_size(buffer);
					if (m_stream.avail_out == 0)
					{
						break; // ����û��ṩ�Ļ����СΪ0, ��ֱ�ӷ���.
					}
					buffer_size += m_stream.avail_out;
					m_stream.next_out = boost::asio::buffer_cast<Bytef*>(buffer);
					int ret = inflate(&m_stream, Z_SYNC_FLUSH);
					if (ret < 0)
					{
						err = boost::asio::error::operation_not_supported;
						// ��ѹ��������, ֪ͨ�û�����������.
						handler(err, 0);
						return;
					}

					bytes_transferred += (boost::asio::buffer_size(buffer) - m_stream.avail_out);
					if (bytes_transferred != boost::asio::buffer_size(buffer))
					{
						break;
					}
				}
			}

			// ����û��������ռ䲻Ϊ��, ��û��ѹ������, ����������첽��ȡ����, �Ա�֤����ȷ�������ݸ��û�.
			if (buffer_size != 0 && bytes_transferred == 0)
			{
				async_read_some(buffers, handler);
				return;
			}

			if (m_chunked_size == 0 && m_stream.avail_in == 0)
			{
				err = ec;	// FIXME!!!
			}

			handler(err, bytes_transferred);
			return;
		}
#endif
	}
	else
	{
		handler(ec, bytes_transferred);
	}
}

template <typename MutableBufferSequence, typename Handler>
void http_stream::handle_chunked_size(const MutableBufferSequence& buffers,
	Handler handler, const boost::system::error_code& ec, std::size_t bytes_transferred)
{
	if (!ec)
	{
		// ����m_response�е�chunked size.
		std::string hex_chunked_size;
		boost::system::error_code err;
		while (!err && m_response.size() > 0)
		{
			char c;
			bytes_transferred = read_some_impl(boost::asio::buffer(&c, 1), err);
			if (bytes_transferred == 1)
			{
				hex_chunked_size.push_back(c);
				std::size_t s = hex_chunked_size.size();
				if (s >= 2)
				{
					if (hex_chunked_size[s - 2] == '\r' && hex_chunked_size[s - 1] == '\n')
						break;
				}
			}
		}
		BOOST_ASSERT(!err);
		// �õ�chunked size.
		std::stringstream ss;
		ss << std::hex << hex_chunked_size;
		ss >> m_chunked_size;

#ifdef AVHTTP_ENABLE_ZLIB
		if (m_chunked_size == 0 && m_is_gzip)
		{
			if (m_stream.avail_in == 0)
			{
				boost::system::error_code err;
				if (!m_keep_alive)
					err = boost::asio::error::eof;
				handler(err, 0);
				return;
			}
		}
#endif
		// chunked_size����������β��crlf, ����������β��crlfΪfalse״̬.
		m_skip_crlf = false;

		// ��ȡ����.
		if (m_chunked_size != 0)	// ��ʼ��ȡchunked�е�����, �����ѹ��, ���ѹ���û����ܻ���.
		{
			std::size_t max_length = 0;

			if (m_response.size() != 0)
				max_length = 0;
			else
			{
				typename MutableBufferSequence::const_iterator iter = buffers.begin();
				typename MutableBufferSequence::const_iterator end = buffers.end();
				// ����õ��û�buffer_size�ܴ�С.
				for (; iter != end; ++iter)
				{
					boost::asio::mutable_buffer buffer(*iter);
					max_length += boost::asio::buffer_size(buffer);
				}
				// �õ����ʵĻ����С.
				max_length = (std::min)(max_length, m_chunked_size);
			}

			// ��ȡ���ݵ�m_response, �����ѹ��, ��Ҫ��handle_async_read�н�ѹ.
			boost::asio::streambuf::mutable_buffers_type bufs = m_response.prepare(max_length);
			typedef boost::function<void (boost::system::error_code, std::size_t)> HandlerWrapper;
			HandlerWrapper h(handler);
			m_sock.async_read_some(boost::asio::buffer(bufs),
				boost::bind(&http_stream::handle_async_read<MutableBufferSequence, HandlerWrapper>,
					this, buffers, h,
					boost::asio::placeholders::error,
					boost::asio::placeholders::bytes_transferred
				)
			);
			return;
		}

		if (m_chunked_size == 0)
		{
			boost::system::error_code err;
			m_is_chunked_end = true;
#ifdef AVHTTP_ENABLE_ZLIB
					if (m_stream.zalloc)
					{
						inflateEnd(&m_stream);
						m_stream.zalloc = NULL;
					}
#endif
			if (!m_keep_alive)
				err = boost::asio::error::eof;
			handler(err, 0);
			return;
		}
	}

	// ��������, ֪ͨ�ϲ����.
	handler(ec, 0);
}

template <typename Stream>
void http_stream::socks_proxy_connect(Stream& sock, boost::system::error_code& ec)
{
	using namespace avhttp::detail;

	const proxy_settings& s = m_proxy;

	// ��ʼ��������Ķ˿ں�������.
	tcp::resolver resolver(m_io_service);
	std::ostringstream port_string;
	port_string.imbue(std::locale("C"));
	port_string << s.port;
	tcp::resolver::query query(s.hostname.c_str(), port_string.str());
	tcp::resolver::iterator endpoint_iterator = resolver.resolve(query, ec);
	tcp::resolver::iterator end;

	if (ec)	// ������������, ֱ�ӷ�����ش�����Ϣ.
	{
		AVHTTP_LOG_ERR("Resolve DNS error \'" << s.hostname <<
			"\', error message \'" << ec.message() << "\'");
		return ;
	}

	// �������ӽ��������ķ�������ַ.
	ec = boost::asio::error::host_not_found;
	while (ec && endpoint_iterator != end)
	{
		sock.close(ec);
		sock.connect(*endpoint_iterator++, ec);
	}
	if (ec)
	{
		return;
	}

	if (s.type == proxy_settings::socks5 || s.type == proxy_settings::socks5_pw)
	{
		// ���Ͱ汾��Ϣ.
		{
			m_request.consume(m_request.size());

			std::size_t bytes_to_write = s.username.empty() ? 3 : 4;
			boost::asio::mutable_buffer b = m_request.prepare(bytes_to_write);
			char* p = boost::asio::buffer_cast<char*>(b);
			write_uint8(5, p); // SOCKS VERSION 5.
			if (s.username.empty())
			{
				write_uint8(1, p); // 1 authentication method (no auth)
				write_uint8(0, p); // no authentication
			}
			else
			{
				write_uint8(2, p); // 2 authentication methods
				write_uint8(0, p); // no authentication
				write_uint8(2, p); // username/password
			}
			m_request.commit(bytes_to_write);
			boost::asio::write(sock, m_request, boost::asio::transfer_exactly(bytes_to_write), ec);
			if (ec)
				return;
		}

		// ��ȡ�汾��Ϣ.
		m_response.consume(m_response.size());
		boost::asio::read(sock, m_response, boost::asio::transfer_exactly(2), ec);
		if (ec)
			return;

		int version, method;
		{
			boost::asio::const_buffer b = m_response.data();
			const char* p = boost::asio::buffer_cast<const char*>(b);
			version = read_uint8(p);
			method = read_uint8(p);
			if (version != 5)	// �汾������5, ��֧��socks5.
			{
				ec = errc::socks_unsupported_version;
				return;
			}
		}
		if (method == 2)
		{
			if (s.username.empty())
			{
				ec = errc::socks_username_required;
				return;
			}

			// start sub-negotiation.
			m_request.consume(m_request.size());
			std::size_t bytes_to_write = s.username.size() + s.password.size() + 3;
			boost::asio::mutable_buffer b = m_request.prepare(bytes_to_write);
			char* p = boost::asio::buffer_cast<char*>(b);
			write_uint8(1, p);
			write_uint8(s.username.size(), p);
			write_string(s.username, p);
			write_uint8(s.password.size(), p);
			write_string(s.password, p);
			m_request.commit(bytes_to_write);

			// �����û�������Ϣ.
			boost::asio::write(sock, m_request, boost::asio::transfer_exactly(bytes_to_write), ec);
			if (ec)
				return;

			// ��ȡ״̬.
			m_response.consume(m_response.size());
			boost::asio::read(sock, m_response, boost::asio::transfer_exactly(2), ec);
			if (ec)
				return;
		}
		else if (method == 0)
		{
			socks_proxy_handshake(sock, ec);
			return;
		}

		{
			// ��ȡ�汾״̬.
			boost::asio::const_buffer b = m_response.data();
			const char* p = boost::asio::buffer_cast<const char*>(b);

			int version = read_uint8(p);
			int status = read_uint8(p);

			// ��֧�ֵ���֤�汾.
			if (version != 1)
			{
				ec = errc::socks_unsupported_authentication_version;
				return;
			}

			// ��֤����.
			if (status != 0)
			{
				ec = errc::socks_authentication_error;
				return;
			}

			socks_proxy_handshake(sock, ec);
		}
	}
	else if (s.type == proxy_settings::socks4)
	{
		socks_proxy_handshake(sock, ec);
	}
}

template <typename Stream>
void http_stream::socks_proxy_handshake(Stream& sock, boost::system::error_code& ec)
{
	using namespace avhttp::detail;

	const url& u = m_url;
	const proxy_settings& s = m_proxy;

	m_request.consume(m_request.size());
	std::string host = u.host();
	std::size_t bytes_to_write = 7 + host.size();
	if (s.type == proxy_settings::socks4)
		bytes_to_write = 9 + s.username.size();
	boost::asio::mutable_buffer mb = m_request.prepare(bytes_to_write);
	char* wp = boost::asio::buffer_cast<char*>(mb);

	if (s.type == proxy_settings::socks5 || s.type == proxy_settings::socks5_pw)
	{
		// ����socks5��������.
		write_uint8(5, wp); // SOCKS VERSION 5.
		write_uint8(1, wp); // CONNECT command.
		write_uint8(0, wp); // reserved.
		write_uint8(3, wp); // address type.
		BOOST_ASSERT(host.size() <= 255);
		write_uint8(host.size(), wp);				// domainname size.
		std::copy(host.begin(), host.end(),wp);		// domainname.
		wp += host.size();
		write_uint16(u.port(), wp);					// port.
	}
	else if (s.type == proxy_settings::socks4)
	{
		write_uint8(4, wp); // SOCKS VERSION 4.
		write_uint8(1, wp); // CONNECT command.
		// socks4Э��ֻ����ip��ַ, ��֧������.
		tcp::resolver resolver(m_io_service);
		std::ostringstream port_string;
		port_string.imbue(std::locale("C"));
		port_string << u.port();
		tcp::resolver::query query(host.c_str(), port_string.str());
		// �����������е�ip��ַ.
		tcp::resolver::iterator endpoint_iterator = resolver.resolve(query, ec);
		if (ec)	// ������������, ֱ�ӷ�����ش�����Ϣ.
		{
			AVHTTP_LOG_ERR("Resolve DNS error \'" << host <<
				"\', error message \'" << ec.message() << "\'");
			return;
		}
		unsigned long ip = endpoint_iterator->endpoint().address().to_v4().to_ulong();
		write_uint16(u.port(), wp);	// port.
		write_uint32(ip, wp);		// ip address.
		// username.
		if (!s.username.empty())
		{
			std::copy(s.username.begin(), s.username.end(), wp);
			wp += s.username.size();
		}
		// NULL terminator.
		write_uint8(0, wp);
	}
	else
	{
		ec = errc::socks_unsupported_version;
		return;
	}

	// ����.
	m_request.commit(bytes_to_write);
	boost::asio::write(sock, m_request, boost::asio::transfer_exactly(bytes_to_write), ec);
	if (ec)
		return;

	// ����socks����������.
	std::size_t bytes_to_read = 0;
	if (s.type == proxy_settings::socks5 || s.type == proxy_settings::socks5_pw)
		bytes_to_read = 10;
	else if (s.type == proxy_settings::socks4)
		bytes_to_read = 8;

	BOOST_ASSERT(bytes_to_read == 0);

	m_response.consume(m_response.size());
	boost::asio::read(sock, m_response,
		boost::asio::transfer_exactly(bytes_to_read), ec);

	// ��������������.
	boost::asio::const_buffer cb = m_response.data();
	const char* rp = boost::asio::buffer_cast<const char*>(cb);
	int version = read_uint8(rp);
	int response = read_uint8(rp);

	if (version == 5)
	{
		if (s.type != proxy_settings::socks5 && s.type != proxy_settings::socks5_pw)
		{
			// �����socksЭ�鲻��sock5.
			ec = errc::socks_unsupported_version;
			return;
		}

		if (response != 0)
		{
			ec = errc::socks_general_failure;
			// �õ�����ϸ�Ĵ�����Ϣ.
			switch (response)
			{
			case 2: ec = boost::asio::error::no_permission; break;
			case 3: ec = boost::asio::error::network_unreachable; break;
			case 4: ec = boost::asio::error::host_unreachable; break;
			case 5: ec = boost::asio::error::connection_refused; break;
			case 6: ec = boost::asio::error::timed_out; break;
			case 7: ec = errc::socks_command_not_supported; break;
			case 8: ec = boost::asio::error::address_family_not_supported; break;
			}
			return;
		}

		rp++;	// skip reserved.
		int atyp = read_uint8(rp);	// atyp.

		if (atyp == 1)		// address / port ��ʽ����.
		{
			m_response.consume(m_response.size());
			ec = boost::system::error_code();	// û�з�������, ����.
			return;
		}
		else if (atyp == 3)	// domainname ����.
		{
			int len = read_uint8(rp);	// ��ȡdomainname����.
			bytes_to_read = len - 3;
			// ������ȡ.
			m_response.commit(boost::asio::read(sock,
				m_response.prepare(bytes_to_read), boost::asio::transfer_exactly(bytes_to_read), ec));
			// if (ec)
			//	return;
			//
			// �õ�domainname.
			// std::string domain;
			// domain.resize(len);
			// std::copy(rp, rp + len, domain.begin());
			m_response.consume(m_response.size());
			ec = boost::system::error_code();
			return;
		}
		// else if (atyp == 4)	// ipv6 ����, ����ʵ��!
		// {
		//	ec = boost::asio::error::address_family_not_supported;
		//	return;
		// }
		else
		{
			ec = boost::asio::error::address_family_not_supported;
			return;
		}
	}
	else if (version == 4)
	{
		// 90: request granted.
		// 91: request rejected or failed.
		// 92: request rejected becasue SOCKS server cannot connect to identd on the client.
		// 93: request rejected because the client program and identd report different user-ids.
		if (response == 90)	// access granted.
		{
			m_response.consume(m_response.size());
			ec = boost::system::error_code();
			return;
		}
		else
		{
			ec = errc::socks_general_failure;
			switch (response)
			{
			case 91: ec = errc::socks_authentication_error; break;
			case 92: ec = errc::socks_no_identd; break;
			case 93: ec = errc::socks_identd_error; break;
			}
			return;
		}
	}
	else
	{
		ec = errc::socks_general_failure;
		return;
	}
}

// socks��������첽����.
template <typename Stream, typename Handler>
void http_stream::async_socks_proxy_connect(Stream& sock, Handler handler)
{
	// �����첽��ѯproxy������Ϣ.
	std::ostringstream port_string;
	port_string.imbue(std::locale("C"));
	port_string << m_proxy.port;
	tcp::resolver::query query(m_proxy.hostname, port_string.str());

	m_proxy_status = socks_proxy_resolve;

	// ��ʼ�첽��������Ķ˿ں�������.
	typedef boost::function<void (boost::system::error_code)> HandlerWrapper;
	m_resolver.async_resolve(query,
		boost::bind(&http_stream::async_socks_proxy_resolve<Stream, HandlerWrapper>,
			this,
			boost::asio::placeholders::error,
			boost::asio::placeholders::iterator,
			boost::ref(sock), HandlerWrapper(handler)
		)
	);
}

// �첽�����ѯ�ص�.
template <typename Stream, typename Handler>
void http_stream::async_socks_proxy_resolve(const boost::system::error_code& err,
	tcp::resolver::iterator endpoint_iterator, Stream& sock, Handler handler)
{
	if (err)
	{
		AVHTTP_LOG_ERR("Resolve socks server error, \'" << m_proxy.hostname << ":" << m_proxy.port <<
			"\', error message \'" << err.message() << "\'");
		handler(err);
		return;
	}

	if (m_proxy_status == socks_proxy_resolve)
	{
		m_proxy_status = socks_connect_proxy;
		// ��ʼ�첽���Ӵ���.
		boost::asio::async_connect(sock.lowest_layer(), endpoint_iterator,
			boost::bind(&http_stream::handle_connect_socks<Stream, Handler>,
				this, boost::ref(sock), handler,
				endpoint_iterator, boost::asio::placeholders::error
			)
		);

		return;
	}

	if (m_proxy_status == socks4_resolve_host)
	{
		// ����IP��PORT��Ϣ.
		m_remote_endp = *endpoint_iterator;
		m_remote_endp.port(m_url.port());

		// ����״̬.
		handle_socks_process(sock, handler, 0, err);
	}
}

template <typename Stream, typename Handler>
void http_stream::handle_connect_socks(Stream& sock, Handler handler,
	tcp::resolver::iterator endpoint_iterator, const boost::system::error_code& err)
{
	using namespace avhttp::detail;

	if (err)
	{
		tcp::resolver::iterator end;
		if (endpoint_iterator == end)
		{
			AVHTTP_LOG_ERR("Connect to socks proxy, \'" << m_proxy.hostname << ":" << m_proxy.port <<
				"\', error message \'" << err.message() << "\'");
			handler(err);
			return;
		}

		// ��������������һ��IP.
		endpoint_iterator++;
		boost::asio::async_connect(sock.lowest_layer(), endpoint_iterator,
			boost::bind(&http_stream::handle_connect_socks<Stream, Handler>,
				this, boost::ref(sock), handler,
				endpoint_iterator, boost::asio::placeholders::error
			)
		);

		return;
	}

	// ���ӳɹ�, ����Э��汾��.
	if (m_proxy.type == proxy_settings::socks5 || m_proxy.type == proxy_settings::socks5_pw)
	{
		// ���Ͱ汾��Ϣ.
		m_proxy_status = socks_send_version;

		m_request.consume(m_request.size());

		std::size_t bytes_to_write = m_proxy.username.empty() ? 3 : 4;
		boost::asio::mutable_buffer b = m_request.prepare(bytes_to_write);
		char* p = boost::asio::buffer_cast<char*>(b);
		write_uint8(5, p);		// SOCKS VERSION 5.
		if (m_proxy.username.empty())
		{
			write_uint8(1, p); // 1 authentication method (no auth)
			write_uint8(0, p); // no authentication
		}
		else
		{
			write_uint8(2, p); // 2 authentication methods
			write_uint8(0, p); // no authentication
			write_uint8(2, p); // username/password
		}

		m_request.commit(bytes_to_write);

		typedef boost::function<void (boost::system::error_code)> HandlerWrapper;
		boost::asio::async_write(sock, m_request, boost::asio::transfer_exactly(bytes_to_write),
			boost::bind(&http_stream::handle_socks_process<Stream, HandlerWrapper>,
				this, boost::ref(sock), HandlerWrapper(handler),
				boost::asio::placeholders::bytes_transferred,
				boost::asio::placeholders::error
			)
		);

		return;
	}

	if (m_proxy.type == proxy_settings::socks4)
	{
		m_proxy_status = socks4_resolve_host;

		// �����첽��ѯԶ��������HOST.
		std::ostringstream port_string;
		port_string.imbue(std::locale("C"));
		port_string << m_url.port();
		tcp::resolver::query query(m_url.host(), port_string.str());

		// ��ʼ�첽��������Ķ˿ں�������.
		typedef boost::function<void (boost::system::error_code)> HandlerWrapper;
		m_resolver.async_resolve(query,
			boost::bind(&http_stream::async_socks_proxy_resolve<Stream, HandlerWrapper>,
				this,
				boost::asio::placeholders::error, boost::asio::placeholders::iterator,
				boost::ref(sock), HandlerWrapper(handler)
			)
		);
	}
}

template <typename Stream, typename Handler>
void http_stream::handle_socks_process(Stream& sock, Handler handler,
	int bytes_transferred, const boost::system::error_code& err)
{
	using namespace avhttp::detail;

	if (err)
	{
		AVHTTP_LOG_ERR("Socks proxy process error, \'" << m_proxy.hostname << ":" << m_proxy.port <<
			"\', error message \'" << err.message() << "\'");
		handler(err);
		return;
	}

	switch (m_proxy_status)
	{
	case socks_send_version:	// ��ɰ汾�ŷ���.
		{
			// ����socks����������.
			std::size_t bytes_to_read;
			if (m_proxy.type == proxy_settings::socks5 || m_proxy.type == proxy_settings::socks5_pw)
				bytes_to_read = 10;
			else if (m_proxy.type == proxy_settings::socks4)
				bytes_to_read = 8;

			if (m_proxy.type == proxy_settings::socks4)
			{
				// �޸�״̬.
				m_proxy_status = socks4_response;

				m_response.consume(m_response.size());
				boost::asio::async_read(sock, m_response, boost::asio::transfer_exactly(bytes_to_read),
					boost::bind(&http_stream::handle_socks_process<Stream, Handler>,
						this, boost::ref(sock), handler,
						boost::asio::placeholders::bytes_transferred,
						boost::asio::placeholders::error
					)
				);

				return;
			}

			if (m_proxy.type == proxy_settings::socks5 || m_proxy.type == proxy_settings::socks5_pw)
			{
				m_proxy_status = socks5_response_version;

				// ��ȡ�汾��Ϣ.
				m_response.consume(m_response.size());
				boost::asio::async_read(sock, m_response, boost::asio::transfer_exactly(2),
					boost::bind(&http_stream::handle_socks_process<Stream, Handler>,
						this, boost::ref(sock), handler,
						boost::asio::placeholders::bytes_transferred,
						boost::asio::placeholders::error
					)
				);

				return;
			}
		}
		break;
	case socks4_resolve_host:	// socks4Э��, IP/PORT�Ѿ��õ�, ��ʼ���Ͱ汾��Ϣ.
		{
			m_proxy_status = socks_send_version;

			m_request.consume(m_request.size());
			std::size_t bytes_to_write = 9 + m_proxy.username.size();
			boost::asio::mutable_buffer mb = m_request.prepare(bytes_to_write);
			char* wp = boost::asio::buffer_cast<char*>(mb);

			write_uint8(4, wp); // SOCKS VERSION 4.
			write_uint8(1, wp); // CONNECT command.

			// socks4Э��ֻ����ip��ַ, ��֧������.
			unsigned long ip = m_remote_endp.address().to_v4().to_ulong();
			write_uint16(m_remote_endp.port(), wp);	// port.
			write_uint32(ip, wp);					// ip address.

			// username.
			if (!m_proxy.username.empty())
			{
				std::copy(m_proxy.username.begin(), m_proxy.username.end(), wp);
				wp += m_proxy.username.size();
			}
			// NULL terminator.
			write_uint8(0, wp);

			m_request.commit(bytes_to_write);

			boost::asio::async_write(sock, m_request, boost::asio::transfer_exactly(bytes_to_write),
				boost::bind(&http_stream::handle_socks_process<Stream, Handler>,
					this, boost::ref(sock), handler,
					boost::asio::placeholders::bytes_transferred, boost::asio::placeholders::error
				)
			);

			return;
		}
		break;
	case socks5_send_userinfo:
		{
			m_proxy_status = socks5_auth_status;
			// ��ȡ��֤״̬.
			m_response.consume(m_response.size());
			boost::asio::async_read(sock, m_response, boost::asio::transfer_exactly(2),
				boost::bind(&http_stream::handle_socks_process<Stream, Handler>,
					this, boost::ref(sock), handler,
					boost::asio::placeholders::bytes_transferred,
					boost::asio::placeholders::error
				)
			);
			return;
		}
		break;
	case socks5_connect_request:
		{
			m_proxy_status = socks5_connect_response;

			// ����״̬��Ϣ.
			m_request.consume(m_request.size());
			std::string host = m_url.host();
			std::size_t bytes_to_write = 7 + host.size();
			boost::asio::mutable_buffer mb = m_request.prepare(bytes_to_write);
			char* wp = boost::asio::buffer_cast<char*>(mb);
			// ����socks5��������.
			write_uint8(5, wp); // SOCKS VERSION 5.
			write_uint8(1, wp); // CONNECT command.
			write_uint8(0, wp); // reserved.
			write_uint8(3, wp); // address type.
			BOOST_ASSERT(host.size() <= 255);
			write_uint8(host.size(), wp);				// domainname size.
			std::copy(host.begin(), host.end(), wp);	// domainname.
			wp += host.size();
			write_uint16(m_url.port(), wp);				// port.
			m_request.commit(bytes_to_write);
			boost::asio::async_write(sock, m_request, boost::asio::transfer_exactly(bytes_to_write),
				boost::bind(&http_stream::handle_socks_process<Stream, Handler>,
					this, boost::ref(sock), handler,
					boost::asio::placeholders::bytes_transferred, boost::asio::placeholders::error
				)
			);

			return;
		}
		break;
	case socks5_connect_response:
		{
			m_proxy_status = socks5_result;
			std::size_t bytes_to_read = 10;
			m_response.consume(m_response.size());
			boost::asio::async_read(sock, m_response, boost::asio::transfer_exactly(bytes_to_read),
				boost::bind(&http_stream::handle_socks_process<Stream, Handler>,
					this, boost::ref(sock), handler,
					boost::asio::placeholders::bytes_transferred,
					boost::asio::placeholders::error
				)
			);
		}
		break;
	case socks4_response:	// socks4��������������.
		{
			// ��������������.
			boost::asio::const_buffer cb = m_response.data();
			const char* rp = boost::asio::buffer_cast<const char*>(cb);
			/*int version = */read_uint8(rp);
			int response = read_uint8(rp);

			// 90: request granted.
			// 91: request rejected or failed.
			// 92: request rejected becasue SOCKS server cannot connect to identd on the client.
			// 93: request rejected because the client program and identd report different user-ids.
			if (response == 90)	// access granted.
			{
				m_response.consume(m_response.size());	// û�з�������, ��ʼ�첽��������.

#ifdef AVHTTP_ENABLE_OPENSSL
				if (m_protocol == "https")
				{
					AVHTTP_LOG_DBG("Connect to socks proxy \'" << m_proxy.hostname << ":" << m_proxy.port << "\'.");

					// ��ʼ����.
					m_proxy_status = ssl_handshake;
					ssl_socket* ssl_sock = m_sock.get<ssl_socket>();
					ssl_sock->async_handshake(boost::bind(&http_stream::handle_socks_process<Stream, Handler>, this,
						boost::ref(sock), handler,
						0,
						boost::asio::placeholders::error));
					return;
				}
				else
#endif
				async_request(m_request_opts_priv, handler);
				return;
			}
			else
			{
				boost::system::error_code ec = errc::socks_general_failure;
				switch (response)
				{
				case 91: ec = errc::socks_authentication_error; break;
				case 92: ec = errc::socks_no_identd; break;
				case 93: ec = errc::socks_identd_error; break;
				}

				AVHTTP_LOG_ERR("Socks4 proxy process error, \'" << m_proxy.hostname << ":" << m_proxy.port <<
					"\', error message \'" << ec.message() << "\'");
				handler(ec);
				return;
			}
		}
		break;
#ifdef AVHTTP_ENABLE_OPENSSL
	case ssl_handshake:
		{
			AVHTTP_LOG_DBG("Handshake to \'" << m_url.host() <<
				"\', error message \'" << err.message() << "\'");

			async_request(m_request_opts_priv, handler);
		}
		break;
#endif
	case socks5_response_version:
		{
			boost::asio::const_buffer cb = m_response.data();
			const char* rp = boost::asio::buffer_cast<const char*>(cb);
			int version = read_uint8(rp);
			int method = read_uint8(rp);
			if (version != 5)	// �汾������5, ��֧��socks5.
			{
				boost::system::error_code ec = errc::socks_unsupported_version;
				AVHTTP_LOG_ERR("Socks5 response version, \'" << m_proxy.hostname << ":" << m_proxy.port <<
					"\', error message \'" << ec.message() << "\'");
				handler(ec);
				return;
			}

			const proxy_settings& s = m_proxy;

			if (method == 2)
			{
				if (s.username.empty())
				{
					boost::system::error_code ec = errc::socks_username_required;
					AVHTTP_LOG_ERR("Socks5 response version, \'" << m_proxy.hostname << ":" << m_proxy.port <<
						"\', error message \'" << ec.message() << "\'");
					handler(ec);
					return;
				}

				// start sub-negotiation.
				m_request.consume(m_request.size());
				std::size_t bytes_to_write = m_proxy.username.size() + m_proxy.password.size() + 3;
				boost::asio::mutable_buffer mb = m_request.prepare(bytes_to_write);
				char* wp = boost::asio::buffer_cast<char*>(mb);
				write_uint8(1, wp);
				write_uint8(s.username.size(), wp);
				write_string(s.username, wp);
				write_uint8(s.password.size(), wp);
				write_string(s.password, wp);
				m_request.commit(bytes_to_write);

				// �޸�״̬.
				m_proxy_status = socks5_send_userinfo;

				// �����û�������Ϣ.
				boost::asio::async_write(sock, m_request, boost::asio::transfer_exactly(bytes_to_write),
					boost::bind(&http_stream::handle_socks_process<Stream, Handler>,
						this, boost::ref(sock), handler,
						boost::asio::placeholders::bytes_transferred, boost::asio::placeholders::error
					)
				);

				return;
			}

			if (method == 0)
			{
				m_proxy_status = socks5_connect_request;
				AVHTTP_LOG_DBG("Socks5 response version, \'" << m_proxy.hostname << ":" << m_proxy.port <<
					"\', error message \'" << err.message() << "\'");
				handle_socks_process(sock, handler, 0, err);
				return;
			}
		}
		break;
	case socks5_auth_status:
		{
			boost::asio::const_buffer cb = m_response.data();
			const char* rp = boost::asio::buffer_cast<const char*>(cb);

			int version = read_uint8(rp);
			int status = read_uint8(rp);

			if (version != 1)	// ��֧�ֵİ汾.
			{
				boost::system::error_code ec = errc::socks_unsupported_authentication_version;
				AVHTTP_LOG_ERR("Socks5 auth status, \'" << m_proxy.hostname << ":" << m_proxy.port <<
					"\', error message \'" << ec.message() << "\'");
				handler(ec);
				return;
			}

			if (status != 0)	// ��֤����.
			{
				boost::system::error_code ec = errc::socks_authentication_error;
				AVHTTP_LOG_ERR("Socks5 auth status, \'" << m_proxy.hostname << ":" << m_proxy.port <<
					"\', error message \'" << ec.message() << "\'");
				handler(ec);
				return;
			}

			// ����������������.
			m_proxy_status = socks5_connect_request;
			handle_socks_process(sock, handler, 0, err);
		}
		break;
	case socks5_result:
		{
			// ��������������.
			boost::asio::const_buffer cb = m_response.data();
			const char* rp = boost::asio::buffer_cast<const char*>(cb);
			int version = read_uint8(rp);
			int response = read_uint8(rp);

			if (version != 5)
			{
				boost::system::error_code ec = errc::socks_general_failure;
				AVHTTP_LOG_ERR("Socks5 result, \'" << m_proxy.hostname << ":" << m_proxy.port <<
					"\', error message \'" << ec.message() << "\'");
				handler(ec);
				return;
			}

			if (response != 0)
			{
				boost::system::error_code ec = errc::socks_general_failure;
				// �õ�����ϸ�Ĵ�����Ϣ.
				switch (response)
				{
				case 2: ec = boost::asio::error::no_permission; break;
				case 3: ec = boost::asio::error::network_unreachable; break;
				case 4: ec = boost::asio::error::host_unreachable; break;
				case 5: ec = boost::asio::error::connection_refused; break;
				case 6: ec = boost::asio::error::timed_out; break;
				case 7: ec = errc::socks_command_not_supported; break;
				case 8: ec = boost::asio::error::address_family_not_supported; break;
				}
				AVHTTP_LOG_ERR("Socks5 result, \'" << m_proxy.hostname << ":" << m_proxy.port <<
					"\', error message \'" << ec.message() << "\'");
				handler(ec);
				return;
			}

			rp++;	// skip reserved.
			int atyp = read_uint8(rp);	// atyp.

			if (atyp == 1)		// address / port ��ʽ����.
			{
				m_response.consume(m_response.size());

#ifdef AVHTTP_ENABLE_OPENSSL
				if (m_protocol == "https")
				{
					// ��ʼ����.
					m_proxy_status = ssl_handshake;
					ssl_socket* ssl_sock = m_sock.get<ssl_socket>();
					ssl_sock->async_handshake(boost::bind(&http_stream::handle_socks_process<Stream, Handler>, this,
						boost::ref(sock), handler,
						0,
						boost::asio::placeholders::error));
					return;
				}
				else
#endif
				{
					AVHTTP_LOG_DBG("Connect to socks5 proxy \'" << m_proxy.hostname << ":" << m_proxy.port << "\'.");
					// û�з�������, ��ʼ�첽��������.
					async_request(m_request_opts_priv, handler);
					return;
				}
			}
			else if (atyp == 3)				// domainname ����.
			{
				int len = read_uint8(rp);	// ��ȡdomainname����.
				std::size_t bytes_to_read = len - 3;

				m_proxy_status = socks5_read_domainname;

				m_response.consume(m_response.size());
				boost::asio::async_read(sock, m_response, boost::asio::transfer_exactly(bytes_to_read),
					boost::bind(&http_stream::handle_socks_process<Stream, Handler>,
						this, boost::ref(sock), handler,
						boost::asio::placeholders::bytes_transferred,
						boost::asio::placeholders::error
					)
				);

				return;
			}
			// else if (atyp == 4)	// ipv6 ����, ����ʵ��!
			// {
			//	ec = boost::asio::error::address_family_not_supported;
			//	return;
			// }
			else
			{
				boost::system::error_code ec = boost::asio::error::address_family_not_supported;
				AVHTTP_LOG_ERR("Socks5 result, \'" << m_proxy.hostname << ":" << m_proxy.port <<
					"\', error message \'" << ec.message() << "\'");
				handler(ec);
				return;
			}
		}
		break;
	case socks5_read_domainname:
		{
			m_response.consume(m_response.size());

#ifdef AVHTTP_ENABLE_OPENSSL
			if (m_protocol == "https")
			{
				AVHTTP_LOG_DBG("Connect to socks5 proxy \'" << m_proxy.hostname << ":" << m_proxy.port << "\'.");
				// ��ʼ����.
				m_proxy_status = ssl_handshake;
				ssl_socket* ssl_sock = m_sock.get<ssl_socket>();
				ssl_sock->async_handshake(boost::bind(&http_stream::handle_socks_process<Stream, Handler>, this,
					boost::ref(sock), handler,
					0,
					boost::asio::placeholders::error));
				return;
			}
			else
#endif
			{
				AVHTTP_LOG_DBG("Connect to socks5 proxy \'" << m_proxy.hostname << ":" << m_proxy.port << "\'.");
				// û�з�������, ��ʼ�첽��������.
				async_request(m_request_opts_priv, handler);
			}
			return;
		}
		break;
	}
}


#ifdef AVHTTP_ENABLE_OPENSSL

// ʵ��CONNECTָ��, ��������Ŀ��Ϊhttps����ʱʹ��.
template <typename Stream, typename Handler>
void http_stream::async_https_proxy_connect(Stream& sock, Handler handler)
{
	// �����첽��ѯproxy������Ϣ.
	std::ostringstream port_string;
	port_string.imbue(std::locale("C"));
	port_string << m_proxy.port;
	tcp::resolver::query query(m_proxy.hostname, port_string.str());

	// ��ʼ�첽��������Ķ˿ں�������.
	typedef boost::function<void (boost::system::error_code)> HandlerWrapper;
	m_resolver.async_resolve(query,
		boost::bind(&http_stream::async_https_proxy_resolve<Stream, HandlerWrapper>,
			this, boost::asio::placeholders::error,
			boost::asio::placeholders::iterator,
			boost::ref(sock),
			HandlerWrapper(handler)
		)
	);
}

template <typename Stream, typename Handler>
void http_stream::async_https_proxy_resolve(const boost::system::error_code& err,
	tcp::resolver::iterator endpoint_iterator, Stream& sock, Handler handler)
{
	if (err)
	{
		AVHTTP_LOG_ERR("Connect to http proxy \'" << m_proxy.hostname << ":" << m_proxy.port <<
			"\', error message \'" << err.message() << "\'");
		handler(err);
		return;
	}
	// ��ʼ�첽���Ӵ���.
	boost::asio::async_connect(sock.lowest_layer(), endpoint_iterator,
		boost::bind(&http_stream::handle_connect_https_proxy<Stream, Handler>,
			this, boost::ref(sock), handler,
			endpoint_iterator, boost::asio::placeholders::error
		)
	);
	return;
}

template <typename Stream, typename Handler>
void http_stream::handle_connect_https_proxy(Stream& sock, Handler handler,
	tcp::resolver::iterator endpoint_iterator, const boost::system::error_code& err)
{
	if (err)
	{
		tcp::resolver::iterator end;
		if (endpoint_iterator == end)
		{
			AVHTTP_LOG_ERR("Connect to http proxy \'" << m_proxy.hostname << ":" << m_proxy.port <<
				"\', error message \'" << err.message() << "\'");
			handler(err);
			return;
		}

		// ��������������һ��IP.
		endpoint_iterator++;
		boost::asio::async_connect(sock.lowest_layer(), endpoint_iterator,
			boost::bind(&http_stream::handle_connect_https_proxy<Stream, Handler>,
				this, boost::ref(sock), handler,
				endpoint_iterator, boost::asio::placeholders::error
			)
		);

		return;
	}

	// ����CONNECT����.
	request_opts opts = m_request_opts;

	// �����������.
	std::string request_method = "CONNECT";

	// ������http/1.1�汾.
	std::string http_version = "HTTP/1.1";

	// ���user_agent.
	std::string user_agent = AVHTTP_VERSION_MIME;
	if (opts.find(http_options::user_agent, user_agent))
		opts.remove(http_options::user_agent);	// ɾ���������ѡ��.

	// �õ�Accept��Ϣ.
	std::string accept = "text/html, application/xhtml+xml, */*";
	if (opts.find(http_options::accept, accept))
		opts.remove(http_options::accept);		// ɾ���������ѡ��.

	// �õ�Host��Ϣ.
	std::string host = m_url.to_string(url::host_component | url::port_component);
	if (opts.find(http_options::host, host))
		opts.remove(http_options::host);		// ɾ���������ѡ��.

	// �������֤����.
	std::string auth;
	if (m_proxy.type == proxy_settings::http_pw)
	{
		std::string user_info = m_proxy.username + ":" + m_proxy.password;
		user_info = "Basic " + detail::encode_base64(user_info);
		auth = "Proxy-Authorization:" + user_info;
	}
	else if (!m_url.user_info().empty())
	{
		std::string user_info = "Basic " + detail::encode_base64(m_url.user_info());
		auth = "Proxy-Authorization:" + user_info;
	}

	// ���ϸ�ѡ�Http�����ַ�����.
	std::string request_string;
	m_request.consume(m_request.size());
	std::ostream request_stream(&m_request);
	request_stream << request_method << " ";
	request_stream << m_url.host() << ":" << m_url.port();
	request_stream << " " << http_version << "\r\n";
	request_stream << "Host: " << host << "\r\n";
	if (!auth.empty())
	{
		request_stream << auth << "\r\n";
	}
	request_stream << "Accept: " << accept << "\r\n";
	request_stream << "User-Agent: " << user_agent << "\r\n\r\n";

#if defined(DEBUG) || defined(_DEBUG)
	{
		int request_size = m_request.size();
		boost::asio::streambuf::const_buffers_type::const_iterator begin(m_request.data().begin());
		const char* ptr = boost::asio::buffer_cast<const char*>(*begin);
		AVHTTP_LOG_DBG("Http proxy request Header:\n" << std::string(ptr, request_size));
	}
#endif

	// �첽��������.
	typedef boost::function<void (boost::system::error_code)> HandlerWrapper;
	boost::asio::async_write(sock, m_request, boost::asio::transfer_exactly(m_request.size()),
		boost::bind(&http_stream::handle_https_proxy_request<Stream, HandlerWrapper>,
			this,
			boost::ref(sock), HandlerWrapper(handler),
			boost::asio::placeholders::error
		)
	);
}

template <typename Stream, typename Handler>
void http_stream::handle_https_proxy_request(Stream& sock, Handler handler,
	const boost::system::error_code& err)
{
	// ��������.
	if (err)
	{
		AVHTTP_LOG_ERR("Connect to http proxy \'" << m_proxy.hostname << ":" << m_proxy.port <<
			"\', error message \'" << err.message() << "\'");
		handler(err);
		return;
	}

	// �첽��ȡHttp status.
	boost::asio::async_read_until(sock, m_response, "\r\n",
		boost::bind(&http_stream::handle_https_proxy_status<Stream, Handler>,
			this,
			boost::ref(sock), handler,
			boost::asio::placeholders::error
		)
	);
}

template <typename Stream, typename Handler>
void http_stream::handle_https_proxy_status(Stream& sock, Handler handler,
	const boost::system::error_code& err)
{
	// ��������.
	if (err)
	{
		AVHTTP_LOG_ERR("Connect to http proxy, \'" << m_proxy.hostname << ":" << m_proxy.port <<
			"\', error message \'" << err.message() << "\'");
		handler(err);
		return;
	}

	boost::system::error_code ec;
	// ����״̬��.
	// ���http״̬��, version_major��version_minor��httpЭ��İ汾��.
	int version_major = 0;
	int version_minor = 0;
	m_status_code = 0;
	if (!detail::parse_http_status_line(
		std::istreambuf_iterator<char>(&m_response),
		std::istreambuf_iterator<char>(),
		version_major, version_minor, m_status_code))
	{
		ec = errc::malformed_status_line;
		AVHTTP_LOG_ERR("Connect to http proxy, \'" << m_proxy.hostname << ":" << m_proxy.port <<
			"\', error message \'" << ec.message() << "\'");
		handler(ec);
		return;
	}

	// "continue"��ʾ������Ҫ�����ȴ�����״̬.
	if (m_status_code == errc::continue_request)
	{
		boost::asio::async_read_until(sock, m_response, "\r\n",
			boost::bind(&http_stream::handle_https_proxy_status<Stream, Handler>,
				this,
				boost::ref(sock), handler,
				boost::asio::placeholders::error
			)
		);
	}
	else
	{
		// ���ԭ�еķ���ѡ��, �����״̬��.
		m_response_opts.clear();
		m_response_opts.insert("_status_code", boost::str(boost::format("%d") % m_status_code));

		// �첽��ȡ����Http header����.
		boost::asio::async_read_until(sock, m_response, "\r\n\r\n",
			boost::bind(&http_stream::handle_https_proxy_header<Stream, Handler>,
				this,
				boost::ref(sock), handler,
				boost::asio::placeholders::bytes_transferred,
				boost::asio::placeholders::error
			)
		);
	}
}

template <typename Stream, typename Handler>
void http_stream::handle_https_proxy_header(Stream& sock, Handler handler,
	int bytes_transferred, const boost::system::error_code& err)
{
	if (err)
	{
		AVHTTP_LOG_ERR("Connect to http proxy, \'" << m_proxy.hostname << ":" << m_proxy.port <<
			"\', error message \'" << err.message() << "\'");
		handler(err);
		return;
	}

	boost::system::error_code ec;
	std::string header_string;
	header_string.resize(bytes_transferred);
	m_response.sgetn(&header_string[0], bytes_transferred);

	// ����Http Header.
	if (!detail::parse_http_headers(header_string.begin(), header_string.end(),
		m_content_type, m_content_length, m_location, m_response_opts.option_all()))
	{
		ec = errc::malformed_response_headers;
		AVHTTP_LOG_ERR("Connect to http proxy, \'" << m_proxy.hostname << ":" << m_proxy.port <<
			"\', error message \'" << ec.message() << "\'");
		handler(ec);
		return;
	}

	if (m_status_code != errc::ok)
	{
		ec = make_error_code(static_cast<errc::errc_t>(m_status_code));
		AVHTTP_LOG_ERR("Connect to http proxy, \'" << m_proxy.hostname << ":" << m_proxy.port <<
			"\', error message \'" << ec.message() << "\'");
		// �ص�֪ͨ.
		handler(ec);
		return;
	}

	AVHTTP_LOG_DBG("Connect to http proxy \'" << m_proxy.hostname << ":" << m_proxy.port << "\'.");

	// ��ʼ�첽����.
	ssl_socket* ssl_sock = m_sock.get<ssl_socket>();
	ssl_sock->async_handshake(
		boost::bind(&http_stream::handle_https_proxy_handshake<Stream, Handler>,
			this,
			boost::ref(sock),
			handler,
			boost::asio::placeholders::error
		)
	);
	return;
}

template <typename Stream, typename Handler>
void http_stream::handle_https_proxy_handshake(Stream& sock, Handler handler,
	const boost::system::error_code& err)
{
	if (err)
	{
		AVHTTP_LOG_ERR("Handshake to \'" << m_url.host() <<
			"\', error message \'" << err.message() << "\'");
		// �ص�֪ͨ.
		handler(err);
		return;
	}

	AVHTTP_LOG_DBG("Handshake to \'" << m_url.host() << "\'.");

	// ��ս��ջ�����.
	m_response.consume(m_response.size());

	// �����첽����.
	async_request(m_request_opts_priv, handler);
}

// ʵ��CONNECTָ��, ��������Ŀ��Ϊhttps����ʱʹ��.
template <typename Stream>
void http_stream::https_proxy_connect(Stream& sock, boost::system::error_code& ec)
{
	// ��ʼ�����˿ں�������.
	tcp::resolver resolver(m_io_service);
	std::ostringstream port_string;
	port_string.imbue(std::locale("C"));
	port_string << m_proxy.port;
	tcp::resolver::query query(m_proxy.hostname, port_string.str());
	tcp::resolver::iterator endpoint_iterator = resolver.resolve(query, ec);
	tcp::resolver::iterator end;

	if (ec)	// ������������, ֱ�ӷ�����ش�����Ϣ.
	{
		AVHTTP_LOG_ERR("Resolve DNS error \'" << m_proxy.hostname <<
			"\', error message \'" << ec.message() << "\'");
		return ;
	}

	// �������ӽ��������Ĵ����������ַ.
	ec = boost::asio::error::host_not_found;
	while (ec && endpoint_iterator != end)
	{
		sock.close(ec);
		sock.connect(*endpoint_iterator++, ec);
	}
	if (ec)
	{
		return;
	}

	// ����CONNECT����.
	request_opts opts = m_request_opts;

	// �����������.
	std::string request_method = "CONNECT";

	// ������http/1.1�汾.
	std::string http_version = "HTTP/1.1";

	// ���user_agent.
	std::string user_agent = AVHTTP_VERSION_MIME;
	if (opts.find(http_options::user_agent, user_agent))
		opts.remove(http_options::user_agent);	// ɾ���������ѡ��.

	// �õ�Accept��Ϣ.
	std::string accept = "text/html, application/xhtml+xml, */*";
	if (opts.find(http_options::accept, accept))
		opts.remove(http_options::accept);		// ɾ���������ѡ��.

	// �õ�Host��Ϣ.
	std::string host = m_url.to_string(url::host_component | url::port_component);
	if (opts.find(http_options::host, host))
		opts.remove(http_options::host);		// ɾ���������ѡ��.

	// ���ϸ�ѡ�Http�����ַ�����.
	std::string request_string;
	m_request.consume(m_request.size());
	std::ostream request_stream(&m_request);
	request_stream << request_method << " ";
	request_stream << m_url.host() << ":" << m_url.port();
	request_stream << " " << http_version << "\r\n";
	request_stream << "Host: " << host << "\r\n";
	request_stream << "Accept: " << accept << "\r\n";
	request_stream << "User-Agent: " << user_agent << "\r\n\r\n";

	// ��������.
	boost::asio::write(sock, m_request, ec);
	if (ec)
	{
		return;
	}

	// ѭ����ȡ.
	for (;;)
	{
		boost::asio::read_until(sock, m_response, "\r\n", ec);
		if (ec)
		{
			return;
		}

		// ���http״̬��, version_major��version_minor��httpЭ��İ汾��.
		int version_major = 0;
		int version_minor = 0;
		m_status_code = 0;
		if (!detail::parse_http_status_line(
			std::istreambuf_iterator<char>(&m_response),
			std::istreambuf_iterator<char>(),
			version_major, version_minor, m_status_code))
		{
			ec = errc::malformed_status_line;
			return;
		}

		// ���http״̬���벻��ok���ʾ����.
		if (m_status_code != errc::ok)
		{
			ec = make_error_code(static_cast<errc::errc_t>(m_status_code));
		}

		// "continue"��ʾ������Ҫ�����ȴ�����״̬.
		if (m_status_code != errc::continue_request)
			break;
	} // end for.

	// ���ԭ�еķ���ѡ��, �����״̬��.
	m_response_opts.clear();
	m_response_opts.insert("_status_code", boost::str(boost::format("%d") % m_status_code));

	// ���յ�����Http Header.
	boost::system::error_code read_err;
	std::size_t bytes_transferred = boost::asio::read_until(sock, m_response, "\r\n\r\n", read_err);
	if (read_err)
	{
		// ˵�������˽�����û�еõ�Http header, ���ش�����ļ�ͷ��Ϣ��������eof.
		if (read_err == boost::asio::error::eof)
			ec = errc::malformed_response_headers;
		else
			ec = read_err;
		return;
	}

	std::string header_string;
	header_string.resize(bytes_transferred);
	m_response.sgetn(&header_string[0], bytes_transferred);

	// ����Http Header.
	if (!detail::parse_http_headers(header_string.begin(), header_string.end(),
		m_content_type, m_content_length, m_location, m_response_opts.option_all()))
	{
		ec = errc::malformed_response_headers;
		return;
	}

	m_response.consume(m_response.size());

	return;
}

#endif

std::streambuf::int_type http_stream::underflow()
{
	if (gptr() < egptr())	// ������δ����.
	{
		return traits_type::to_int_type(*gptr());
	}
	if (gptr() == egptr())	// ���˶�ȡ����β.
	{
		// ����ϴ��Ѿ��Ǵ���״̬, ������󲢷���.
		if (m_last_error)
		{
			if (m_last_error != boost::asio::error::eof)
			{
				boost::throw_exception(boost::system::system_error(m_last_error));
			}
			else
			{
				return traits_type::eof();
			}
		}

		// ��http������ͬ����ȡ����.
		boost::system::error_code ec;
		std::size_t bytes_transferred = read_some(
			boost::asio::buffer(boost::asio::buffer(m_get_buffer) + putback_max), ec);
		if (bytes_transferred == 0)
		{
			if (ec == boost::asio::error::eof)
			{
				return traits_type::eof();
			}
			if (!m_last_error)
			{
				return traits_type::eof();
			}

			// ��ȡ��0�ֽ�, �׳������쳣.
			boost::throw_exception(boost::system::system_error(m_last_error));
		}
		if (ec)
		{
			// ��Ϊĩβ������, ���浱ǰ����״̬, �������ش���.
			m_last_error = ec;
		}

		// ���ø�����ָ��.
		setg(m_get_buffer.begin(), m_get_buffer.begin() + putback_max,
			m_get_buffer.begin() + putback_max + bytes_transferred);

		return traits_type::to_int_type(*gptr());
	}
	else
	{
		return traits_type::eof();
	}
}

}

#endif // AVHTTP_HTTP_STREAM_IPP
