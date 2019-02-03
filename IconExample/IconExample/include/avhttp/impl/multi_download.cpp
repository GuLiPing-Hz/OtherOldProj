//
// impl/multi_download.ipp
// ~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2013 Jack (jack dot wgm at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// path LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef AVHTTP_MULTI_DOWNLOAD_IPP
#define AVHTTP_MULTI_DOWNLOAD_IPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "avhttp/http_stream.hpp"
#include "avhttp/default_storage.hpp"
namespace avhttp {

struct multi_download::http_stream_object
{
	http_stream_object()
		: request_range(0, 0)
		, bytes_transferred(0)
		, bytes_downloaded(0)
		, request_count(0)
		, done(false)
		, direct_reconnect(false)
	{}

	// http_stream����.
	http_stream_ptr stream;

	// ���ݻ���, ����ʱ�Ļ���.
	boost::array<char, default_buffer_size> buffer;

	// ��������ݷ�Χ, ÿ����multi_download����һ�����ط�Χ, stream�������Χȥ����.
	range request_range;

	// ���������Ѿ����ص�����, �����request_range, ��һ��request_range������ɺ�,
	// bytes_transferred�Զ���Ϊ0.
	boost::int64_t bytes_transferred;

	// ��ǰ�������ص�����ͳ��.
	boost::int64_t bytes_downloaded;

	// ��ǰ����������Ĵ���.
	int request_count;

	// ��������ʱ��.
	boost::posix_time::ptime last_request_time;

	// ���Ĵ�����Ϣ.
	boost::system::error_code ec;

	// �Ƿ�����������.
	bool done;

	// �������³�������.
	bool direct_reconnect;
};

struct multi_download::byte_rate
{
	byte_rate()
		: seconds(5)
		, index(0)
		, current_byte_rate(0)
	{
		last_byte_rate.resize(seconds);
		for (int i = 0; i < seconds; i++)
		{
			last_byte_rate[i] = 0;
		}
	}

	// ����ͳ�����ʵ�ʱ��.
	const int seconds;

	// ����byte_rate.
	std::vector<int> last_byte_rate;

	// last_byte_rate���±�.
	int index;

	// ��ǰbyte_rate.
	int current_byte_rate;
};

struct multi_download::auto_outstanding
{
	auto_outstanding(multi_download &o)
		: obj(o)
	{
		obj.change_outstranding(true);
	}

	~auto_outstanding()
	{
		obj.change_outstranding(false);
	}

	multi_download &obj;
};

multi_download::multi_download(boost::asio::io_service& io)
	: m_io_service(io)
	, m_accept_multi(false)
	, m_keep_alive(false)
	, m_file_size(-1)
	, m_timer(io)
	, m_number_of_connections(0)
	, m_byte_rate(new byte_rate())
	, m_time_total(0)
	, m_download_point(0)
	, m_drop_size(-1)
	, m_outstanding(0)
	, m_abort(true)
{}

multi_download::~multi_download()
{
	BOOST_ASSERT(stopped()); // ���뱣֤������ǰ�Ѿ�����ֹͣ���ص�״̬.
}

void multi_download::start(const std::string& u, boost::system::error_code& ec)
{
	settings s;
	start(u, s, ec);
}

void multi_download::start(const std::string& u)
{
	settings s;
	boost::system::error_code ec;
	start(u, s, ec);
	if (ec)
	{
		boost::throw_exception(boost::system::system_error(ec));
	}
}

void multi_download::start(const std::string& u, const settings& s)
{
	boost::system::error_code ec;
	start(u, s, ec);
	if (ec)
	{
		boost::throw_exception(boost::system::system_error(ec));
	}
}

void multi_download::start(const std::string& u, const settings& s, boost::system::error_code& ec)
{
	auto_outstanding ao(*this);
	// �����������.
	{
#ifndef AVHTTP_DISABLE_THREAD
		boost::mutex::scoped_lock lock(m_streams_mutex);
#endif
		m_streams.clear();
	}

	// Ĭ���ļ���СΪ-1.
	m_file_size = -1;

	// ��������.
	m_settings = s;

	// ��urlת����utf8����.
	std::string utf8 = detail::ansi_utf8(u);
	utf8 = detail::escape_path(utf8);
	m_final_url = utf8;
	m_file_name = "";

	// ����һ��http_stream����.
	http_object_ptr obj = boost::make_shared<http_stream_object>();

	request_opts req_opt = m_settings.opts;
	req_opt.insert(http_options::range, "bytes=0-");
	req_opt.insert(http_options::connection, "keep-alive");

	// ����http_stream��ͬ����, ��鷵��״̬���Ƿ�Ϊ206, �����206���ʾ��http��������֧�ֶ������.
	obj->stream = boost::make_shared<http_stream>(boost::ref(m_io_service));
	http_stream& h = *obj->stream;
	// ��Ӵ�������.
	h.proxy(m_settings.proxy);
	// �����������.
	h.request_options(req_opt);
	// �����ssl����, Ĭ��Ϊ���֤��.
	h.check_certificate(m_settings.check_certificate);
	// ��http_stream.
	h.open(m_final_url, ec);
	// ��ʧ�����˳�.
	if (ec)
	{
		return;
	}

	// ��������url��Ϣ.
	std::string location = h.location();
	if (!location.empty())
	{
		m_final_url = location;
	}

	// �ж��Ƿ�֧�ֶ������.
	std::string status_code;
	h.response_options().find(http_options::status_code, status_code);
	if (status_code != "206")
	{
		m_accept_multi = false;
	}
	else
	{
		m_accept_multi = true;
	}

	// ���ò���ģʽ����.
	if (m_settings.disable_multi_download)
	{
		m_accept_multi = false;
	}

	// �õ��ļ���С.
	std::string length;
	h.response_options().find(http_options::content_length, length);
	if (length.empty())
	{
		h.response_options().find(http_options::content_length, length);
		std::string::size_type f = length.find('/');
		if (f++ != std::string::npos)
		{
			length = length.substr(f);
		}
		else
		{
			length = "";
		}

		if (length.empty())
		{
			// �õ����ļ�����, ����Ϊ��֧�ֶ�����ģʽ.
			m_accept_multi = false;
		}
	}

	boost::int64_t file_size = -1;
	if (!length.empty())
	{
		try
		{
			file_size = boost::lexical_cast<boost::int64_t>(length);
		}
		catch (boost::bad_lexical_cast&)
		{
			// �ò�����ȷ���ļ�����, ����Ϊ��֧�ֶ�����ģʽ.
			m_accept_multi = false;
		}
	}

	// ���ļ���С���·���rangefield.
	if (file_size != -1 && file_size != m_file_size)
	{
		m_file_size = file_size;
		m_rangefield.reset(m_file_size);
		m_downlaoded_field.reset(m_file_size);
	}

	// �Ƿ���ָ�����ļ���, ���Content-Disposition: attachment; filename="file.zip"
	std::string filename;
	h.response_options().find("Content-Disposition", filename);
	if (!filename.empty())
	{
		std::string value;
		detail::content_disposition_filename(filename.begin(), filename.end(), value);
		if (!value.empty())
		{
			m_file_name = value;
		}
	}

	// �Ƿ�֧�ֳ�����ģʽ, ��֧�ֶ������, ������Ҳû������.
	if (m_accept_multi)
	{
		std::string keep_alive;
		h.response_options().find(http_options::connection, keep_alive);
		boost::to_lower(keep_alive);
		if (keep_alive == "keep-alive")
		{
			m_keep_alive = true;
		}
		else
		{
			m_keep_alive = false;
		}

		// ���δָ��meta�ļ���, ��ʹ������url����meta�ļ���.
		if (m_settings.meta_file.empty())
		{
			// û��ָ��meta�ļ���, �Զ�����meta�ļ���.
			m_settings.meta_file = meta_name(m_final_url.to_string());
		}

		// ��meta�ļ�, ����򿪳ɹ�, ���ʾ��������Ӧ��λͼ��.
		if (!open_meta(m_settings.meta_file))
		{
			// λͼ��ʧ��, ����ν, ���ع����лᴴ���µ�λͼ, ɾ��meta�ļ�.
			m_file_meta.close();
			boost::system::error_code ignore;
			fs::remove(m_settings.meta_file, ignore);
		}
	}

	// �ж��ļ��Ƿ��Ѿ��������, �����ֱ�ӷ���.
	if (m_downlaoded_field.is_full())
	{
		return;
	}

	// �����洢����.
	if (!s.storage)
	{
		m_storage.reset(default_storage_constructor());
	}
	else
	{
		m_storage.reset(s.storage());
	}
	BOOST_ASSERT(m_storage);

	// ���ļ�, �����ļ���.
	m_storage->open(boost::filesystem::path(file_name()), ec);
	if (ec)
	{
		return;
	}

	// �������ٴ�С.
	m_drop_size = s.download_rate_limit;

	// ����Ĭ������.
	if (m_settings.connections_limit == -1)
	{
		m_settings.connections_limit = default_connections_limit;
	}
	if (m_settings.piece_size == -1 && m_file_size != -1)
	{
		m_settings.piece_size = default_piece_size(m_file_size);
	}

	// ���ݵ�1�����ӷ��ص���Ϣ, ������������ѡ��.
	req_opt = m_settings.opts;
	if (m_keep_alive)
	{
		req_opt.insert(http_options::connection, "keep-alive");
	}
	else
	{
		req_opt.insert(http_options::connection, "close");
	}

	// �޸���ֹ״̬.
	m_abort = false;

	// ���Ӽ�����Ϊ1.
	m_number_of_connections = 1;

	// ��ӵ�һ�����ӵ���������.
	{
#ifndef AVHTTP_DISABLE_THREAD
		boost::mutex::scoped_lock lock(m_streams_mutex);
#endif
		m_streams.push_back(obj);
	}

	// Ϊ��1�����������buffer��С.
	int available_bytes = default_buffer_size;

	// ���õ�1���������ط�Χ.
	if (m_accept_multi)
	{
		range req_range;
		bool need_reopen = false;

		// ���ļ������л��һ�οռ�, ���ǵ�һ�η����obj���ص�����.
		if (allocate_range(req_range))
		{
			// ���䵽����ʼ�߽粻��0, ��Ҫ����open���obj.
			if (req_range.left != 0)
			{
				need_reopen = true;
			}

			// ������������.
			obj->request_range = req_range;

			// �����������䵽����ѡ����.
			req_opt.remove(http_options::range);
			req_opt.insert(http_options::range, boost::str(
				boost::format("bytes=%lld-%lld", std::locale("C")) % req_range.left % req_range.right));

			// �����������ʱ��, ���ڼ�鳬ʱ����.
			obj->last_request_time = boost::posix_time::microsec_clock::local_time();

			// ��Ӵ�������.
			h.proxy(m_settings.proxy);
			// ��������ѡ��.
			h.request_options(req_opt);
			// �����ssl����, Ĭ��Ϊ���֤��.
			h.check_certificate(m_settings.check_certificate);
			// �����ض���.
			h.max_redirects(0);

			if (need_reopen)
			{
				h.close(ec);	// �ر�ԭ��������, ��Ҫ�����µ�����.
				if (ec)
				{
					return;
				}

				change_outstranding(true);
				// ��ʼ�첽��.
				h.async_open(m_final_url,
					boost::bind(&multi_download::handle_open,
						this,
						0, obj,
						boost::asio::placeholders::error
					)
				);
			}
			else
			{
				// �������ݶ�ȡ����.
				change_outstranding(true);
				// ����ָ��obj, ��ȷ�����̰߳�ȫ.
				h.async_read_some(boost::asio::buffer(obj->buffer, available_bytes),
					boost::bind(&multi_download::handle_read,
						this,
						0, obj,
						boost::asio::placeholders::bytes_transferred,
						boost::asio::placeholders::error
					)
				);
			}
		}
		else
		{
			// ����ռ�ʧ��, ˵�������Ѿ�û�п��еĿռ��ṩ
			// �����stream����������ֱ����������.
			obj->done = true;
		}
	}
	else	// ��������֧�ֶ������ģʽ, �����ӵ�1����������.
	{
		// �������ݶ�ȡ����.
		change_outstranding(true);
		// ����ָ��obj, ��ȷ�����̰߳�ȫ.
		h.async_read_some(boost::asio::buffer(obj->buffer, available_bytes),
			boost::bind(&multi_download::handle_read,
				this,
				0, obj,
				boost::asio::placeholders::bytes_transferred,
				boost::asio::placeholders::error
			)
		);
	}

	// ���֧�ֶ������, �����ô�������http_stream.
	if (m_accept_multi)
	{
		for (int i = 1; i < m_settings.connections_limit; i++)
		{
			http_object_ptr p = boost::make_shared<http_stream_object>();
			http_stream_ptr ptr = boost::make_shared<http_stream>(boost::ref(m_io_service));
			range req_range;

			// ���ļ������еõ�һ�οռ�.
			if (!allocate_range(req_range))
			{
				// ����ռ�ʧ��, ˵�������Ѿ�û�п��еĿռ��ṩ�����stream����������ֱ����������.
				p->done = true;
				continue;
			}

			// ������������.
			p->request_range = req_range;

			// �����������䵽����ѡ����.
			req_opt.remove(http_options::range);
			req_opt.insert(http_options::range, boost::str(
				boost::format("bytes=%lld-%lld", std::locale("C")) % req_range.left % req_range.right));

			// ��������ѡ��.
			ptr->request_options(req_opt);
			// �����ssl����, Ĭ��Ϊ���֤��.
			ptr->check_certificate(m_settings.check_certificate);
			// �����ض���.
			ptr->max_redirects(0);
			// ��Ӵ�������.
			ptr->proxy(m_settings.proxy);

			// ��������ӵ�������.
			p->stream = ptr;

			{
#ifndef AVHTTP_DISABLE_THREAD
				boost::mutex::scoped_lock lock(m_streams_mutex);
#endif
				m_streams.push_back(p);
			}

			// �����������ʱ��, �����鳬ʱ����.
			p->last_request_time = boost::posix_time::microsec_clock::local_time();

			m_number_of_connections++;
			change_outstranding(true);

			// ��ʼ�첽��, ����ָ��http_object_ptr, ��ȷ�����̰߳�ȫ.
			p->stream->async_open(m_final_url,
				boost::bind(&multi_download::handle_open,
					this,
					i, p,
					boost::asio::placeholders::error
				)
			);
		}
	}

	change_outstranding(true);
	// ������ʱ��, ִ������.
	m_timer.expires_from_now(boost::posix_time::seconds(1));
	m_timer.async_wait(boost::bind(&multi_download::on_tick, this, boost::asio::placeholders::error));

	return;
}

template <typename Handler>
void multi_download::async_start(const std::string& u, Handler handler)
{
	settings s;
	async_start(u, s, handler);
}

template <typename Handler>
void multi_download::async_start(const std::string& u, const settings& s, Handler handler)
{
	// �����������.
	{
#ifndef AVHTTP_DISABLE_THREAD
		boost::mutex::scoped_lock lock(m_streams_mutex);
#endif
		m_streams.clear();
	}

	// ����ļ���С.
	m_file_size = -1;

	// �������.
	std::string utf8 = detail::ansi_utf8(u);
	utf8 = detail::escape_path(utf8);
	m_final_url = utf8;
	m_file_name = "";
	m_settings = s;

	// ����״̬.
	m_abort = false;

	// ����һ��http_stream����.
	http_object_ptr obj = boost::make_shared<http_stream_object>();

	request_opts req_opt = m_settings.opts;
	req_opt.insert(http_options::range, "bytes=0-");
	req_opt.insert(http_options::connection, "keep-alive");

	// ����http_stream��ͬ����, ��鷵��״̬���Ƿ�Ϊ206, �����206���ʾ��http��������֧�ֶ������.
	obj->stream = boost::make_shared<http_stream>(boost::ref(m_io_service));
	http_stream& h = *obj->stream;

	// ��������ѡ��.
	h.request_options(req_opt);
	// ��Ӵ�������.
	h.proxy(m_settings.proxy);
	// �����ssl����, Ĭ��Ϊ���֤��.
	h.check_certificate(m_settings.check_certificate);

	change_outstranding(true);
	typedef boost::function<void (boost::system::error_code)> HandlerWrapper;
	h.async_open(m_final_url,
		boost::bind(&multi_download::handle_start<HandlerWrapper>,
			this,
			HandlerWrapper(handler), obj,
			boost::asio::placeholders::error
		)
	);

	return;
}

void multi_download::stop()
{
	m_abort = true;

	boost::system::error_code ignore;
	m_timer.cancel(ignore);

#ifndef AVHTTP_DISABLE_THREAD
	boost::mutex::scoped_lock lock(m_streams_mutex);
#endif
	for (std::size_t i = 0; i < m_streams.size(); i++)
	{
		const http_object_ptr& ptr = m_streams[i];
		if (ptr && ptr->stream)
		{
			ptr->stream->close(ignore);
		}
	}
}

template <typename MutableBufferSequence>
std::size_t multi_download::fetch_data(const MutableBufferSequence& buffers,
	boost::int64_t offset)
{
	if (!m_storage) // û�д洢�豸, �޷��������.
	{
		return 0;
	}

	// �������ص�λ��.
	m_download_point = offset;

	// �õ��û������С, ��ȷ������ȡ�ֽ���.
	std::size_t buffer_length = 0;
	{
		typename MutableBufferSequence::const_iterator iter = buffers.begin();
		typename MutableBufferSequence::const_iterator end = buffers.end();
		// ����õ��û�buffers���ܴ�С.
		for (; iter != end; ++iter)
		{
			boost::asio::mutable_buffer buffer(*iter);
			buffer_length += boost::asio::buffer_size(buffer);
		}
	}

	// �õ�offset����ɶ�ȡ�����ݴ�С, ʹ���۰뷨����ÿɶ��ռ��С.
	while (buffer_length != 0)
	{
		if (m_downlaoded_field.check_range(offset, buffer_length))
		{
			break;
		}
		buffer_length /= 2;
	}

	// ��ȡ����.
	if (buffer_length != 0)
	{
		std::size_t available_length = buffer_length;
		boost::int64_t offset_for_read = offset;

		typename MutableBufferSequence::const_iterator iter = buffers.begin();
		typename MutableBufferSequence::const_iterator end = buffers.end();
		// ����õ��û�buffers���ܴ�С.
		for (; iter != end; ++iter)
		{
			boost::asio::mutable_buffer buffer(*iter);

			char* buffer_ptr = boost::asio::buffer_cast<char*>(buffer);
			std::size_t buffer_size = boost::asio::buffer_size(buffer);

			if ((boost::int64_t)available_length - (boost::int64_t)buffer_size < 0)
				buffer_size = available_length;

			std::size_t length = m_storage->read(buffer_ptr, offset_for_read, buffer_size);
			BOOST_ASSERT(length == buffer_size);
			offset_for_read += length;
			available_length -= length;

			if (available_length == 0)
			{
				break;
			}
		}
		// ����ʵ�ʶ�ȡ���ֽ���.
		buffer_length = offset_for_read - offset;
	}

	return buffer_length;
}

const settings& multi_download::set() const
{
	return m_settings;
}

bool multi_download::stopped() const
{
	if (m_abort)
	{
#ifndef AVHTTP_DISABLE_THREAD
		boost::mutex::scoped_lock lock(m_outstanding_mutex);
#endif
		if (m_outstanding == 0)
		{
			return true;
		}
	}
	return false;
}

bool multi_download::wait_for_complete()
{
	while (!stopped())
	{
		if (!m_abort)
		{
			boost::mutex::scoped_lock l(m_quit_mtx);
			m_quit_cond.wait(l);
		}
	}
	// ����Ƿ��������, ��ɷ���true, ���򷵻�false.
	boost::int64_t fs = m_file_size;
	if (fs != -1)
	{
		if (fs != bytes_download())
		{
			return false;	// δ�������.
		}
	}

	return true; // �������.
}

void multi_download::check_certificate(bool check)
{
	m_settings.check_certificate = check;
}

boost::int64_t multi_download::file_size() const
{
	return m_file_size;
}

std::string multi_download::meta_name(const std::string& url) const
{
	// ʹ��url��crc��Ϊ�ļ���, ����ֻҪurl��ȷ����, ��ô�Ͳ����Ҵ�meta�ļ�.
	boost::crc_32_type result;
	result.process_bytes(url.c_str(), url.size());
	std::stringstream ss;
	ss.imbue(std::locale("C"));
	ss << std::hex << result.checksum() << ".meta";
	return ss.str();
}

std::string multi_download::file_name() const
{
	// ����ļ���Ϊ��, ������Ĭ���ļ���.
	if (m_file_name.empty())
	{
		// ����Ĭ���ļ���. ���url�е��ļ���Ϊ��, ��ô��Ĭ��Ϊindex.html, ����
		// ʹ��url��ָ�����ļ���, ����settingsָ���˱����ļ�·�����ļ���.
		m_file_name = fs::path(detail::utf8_ansi(m_final_url.path())).leaf().string();
		if (m_file_name == "/" || m_file_name == "")
			m_file_name = fs::path(m_final_url.query()).leaf().string();
		if (m_file_name == "/" || m_file_name == "" || m_file_name == ".")
			m_file_name = "index.html";
		if (!m_settings.save_path.empty())
		{
			if (fs::is_directory(m_settings.save_path))
			{
				fs::path p = m_settings.save_path / m_file_name;
				m_file_name = p.string();
			}
			else
			{
				m_file_name = m_settings.save_path.string();
			}
		}
		return m_file_name;
	}
	return m_file_name;
}

boost::int64_t multi_download::bytes_download() const
{
	if (m_file_size != -1)
	{
		return m_downlaoded_field.range_size();
	}

	boost::int64_t bytes_transferred = 0;

	{
#ifndef AVHTTP_DISABLE_THREAD
		boost::mutex::scoped_lock l(m_streams_mutex);
#endif

		for (std::size_t i = 0; i < m_streams.size(); i++)
		{
			const http_object_ptr& ptr = m_streams[i];
			if (ptr)
			{
				bytes_transferred += ptr->bytes_downloaded;
			}
		}
	}

	return bytes_transferred;
}

int multi_download::download_rate() const
{
	return m_byte_rate->current_byte_rate;
}

void multi_download::download_rate_limit(int rate)
{
	m_settings.download_rate_limit = rate;
}

int multi_download::download_rate_limit() const
{
	return m_settings.download_rate_limit;
}


//////////////////////////////////////////////////////////////////////////
// ����Ϊ�ڲ�ʵ��.

void multi_download::handle_open(const int index,
	http_object_ptr object_ptr, const boost::system::error_code& ec)
{
	auto_outstanding ao(*this);
	change_outstranding(false);
	http_stream_object& object = *object_ptr;
	if (ec || m_abort)
	{
		// �������Ĵ�����Ϣ, ����һЩ������Ч��û���ʿɵ����Ӳ��ϵĳ���.
		object.ec = ec;

		// ������ģʽ, ��ʾ����ֹͣ, ��ֹ����.
		if (!m_accept_multi)
		{
			m_abort = true;
			boost::system::error_code ignore;
			m_timer.cancel(ignore);
		}

		return;
	}

	if (!m_accept_multi)
	{
		// ����֧�ֶϵ�����ʱ, ��ʱ���󵽵��ļ���С��start���󵽵��ļ���С��һ��, ����Ҫ��file_size.
		if (object.stream->content_length() != -1 &&
			object.stream->content_length() != m_file_size)
		{
			m_file_size = object.stream->content_length();
			m_rangefield.reset(m_file_size);
			m_downlaoded_field.reset(m_file_size);
		}
	}

	// �����������ʱ��, �����鳬ʱ����.
	object.last_request_time = boost::posix_time::microsec_clock::local_time();

	// �����������ֽ���.
	int available_bytes = default_buffer_size;
	if (m_drop_size != -1)
	{
		available_bytes = (std::min)(m_drop_size, default_buffer_size);
		m_drop_size -= available_bytes;
		if (available_bytes == 0)
		{
			// ���������ռ�ô���CPU, �ó�CPU��Դ.
			boost::this_thread::sleep(boost::posix_time::millisec(1));
		}
	}

	// �������ݶ�ȡ����.
	http_stream_ptr& stream_ptr = object.stream;

	change_outstranding(true);
	// ����ָ��http_object_ptr, ��ȷ�����̰߳�ȫ.
	stream_ptr->async_read_some(boost::asio::buffer(object.buffer, available_bytes),
		boost::bind(&multi_download::handle_read,
			this,
			index, object_ptr,
			boost::asio::placeholders::bytes_transferred,
			boost::asio::placeholders::error
		)
	);
}

void multi_download::handle_read(const int index, 
	http_object_ptr object_ptr, int bytes_transferred, const boost::system::error_code& ec)
{
	auto_outstanding ao(*this);
	change_outstranding(false);
	http_stream_object& object = *object_ptr;

	// ��������, ��Զ�̷������Ͽ�ʱ, ecΪeof, ��֤����ȫ��д��.
	if (m_storage && bytes_transferred != 0 && (!ec || ec == boost::asio::error::eof))
	{
		// ����offset.
		boost::int64_t offset = object.request_range.left + object.bytes_transferred;

		// ���������������λͼ.
		if (m_file_size != -1)
		{
			m_downlaoded_field.update(offset, offset + bytes_transferred);
		}

		// ʹ��m_storageд��.
		m_storage->write(object.buffer.c_array(), offset, bytes_transferred);
	}

	// ͳ�Ʊ����Ѿ����ص����ֽ���.
	object.bytes_transferred += bytes_transferred;

	// ͳ���������ֽ���.
	object.bytes_downloaded += bytes_transferred;

	// ��������������ֹ.
	if (ec || m_abort)
	{
		// ������ģʽ, ��ʾ����ֹͣ, ��ֹ����.
		if (!m_accept_multi)
		{
			m_abort = true;
			boost::system::error_code ignore;
			m_timer.cancel(ignore);
		}

		// ���û����ֹ����, ��ô��������, ���ﷵ�ؽ�����on_tick�м���
		// ��ʱ, һ����ʱ���������·������ӽ�������.
		return;
	}

	// ���ڼ�����������.
	m_byte_rate->last_byte_rate[m_byte_rate->index] += bytes_transferred;

	// �ж���������������Ѿ��������, ����������, ������µ�����, �����µ�����.
	if (m_accept_multi && object.bytes_transferred >= object.request_range.size())
	{
		// ��֧�ֳ�����, �򴴽��µ�����.
		// ����ǵ�1������, ����Χ��0-�ļ�β, Ҳ��Ҫ�Ͽ���������.
		if (!m_keep_alive || (object.request_range.left == 0 && index == 0))
		{
			// �½��µ�http_stream����.
			object.direct_reconnect = true;
			return;
		}

		http_stream& stream = *object.stream;

		// ��������ѡ��.
		request_opts req_opt = m_settings.opts;

		// �����Ƿ�Ϊ������.
		if (m_keep_alive)
		{
			req_opt.insert(http_options::connection, "keep-alive");
		}

		// ���������пռ�ʧ��, ���������socket, �����������������socket.
		if (!allocate_range(object.request_range))
		{
			object.direct_reconnect = true;
			return;
		}

		// ��ռ���.
		object.bytes_transferred = 0;

		// �����µ���������.
		req_opt.insert(http_options::range,
			boost::str(boost::format("bytes=%lld-%lld", std::locale("C")) %
			object.request_range.left % object.request_range.right));

		// ��Ӵ�������.
		stream.proxy(m_settings.proxy);
		// ���õ�����ѡ����.
		stream.request_options(req_opt);
		// �����ssl����, Ĭ��Ϊ���֤��.
		stream.check_certificate(m_settings.check_certificate);
		// �����ض���.
		stream.max_redirects(0);

		// �����������ʱ��, �����鳬ʱ����.
		object.last_request_time = boost::posix_time::microsec_clock::local_time();

		change_outstranding(true);
		// �����첽http��������, ����ָ��http_object_ptr, ��ȷ�����̰߳�ȫ.
		if (!m_keep_alive)
		{
			stream.async_open(m_final_url,
				boost::bind(&multi_download::handle_open,
					this,
					index, object_ptr,
					boost::asio::placeholders::error
				)
			);
		}
		else
		{
			stream.async_request(req_opt,
				boost::bind(&multi_download::handle_request,
					this,
					index, object_ptr,
					boost::asio::placeholders::error
				)
			);
		}
	}
	else
	{
		// ��������֧�ֶ������, ˵�������Ѿ��������.
		if (!m_accept_multi &&
			(m_file_size != -1 && object.bytes_downloaded == m_file_size))
		{
			m_abort = true;
			boost::system::error_code ignore;
			m_timer.cancel(ignore);
			return;
		}

		// �����������ʱ��, �����鳬ʱ����.
		object.last_request_time = boost::posix_time::microsec_clock::local_time();

		// �����������ֽ���.
		int available_bytes = default_buffer_size;
		if (m_drop_size != -1)
		{
			available_bytes = (std::min)(m_drop_size, default_buffer_size);
			m_drop_size -= available_bytes;
			if (available_bytes == 0)
			{
				// ���������ռ�ô���CPU, �ó�CPU��Դ.
				boost::this_thread::sleep(boost::posix_time::millisec(1));
			}
		}

		change_outstranding(true);
		// ������ȡ����, ����ָ��http_object_ptr, ��ȷ�����̰߳�ȫ.
		object.stream->async_read_some(boost::asio::buffer(object.buffer, available_bytes),
			boost::bind(&multi_download::handle_read,
				this,
				index, object_ptr,
				boost::asio::placeholders::bytes_transferred,
				boost::asio::placeholders::error
			)
		);
	}
}

void multi_download::handle_request(const int index,
	http_object_ptr object_ptr, const boost::system::error_code& ec)
{
	auto_outstanding ao(*this);
	change_outstranding(false);
	http_stream_object& object = *object_ptr;
	object.request_count++;
	if (ec || m_abort)
	{
		// �������Ĵ�����Ϣ, ����һЩ������Ч��û���ʿɵ����Ӳ��ϵĳ���.
		object.ec = ec;

		// ������ģʽ, ��ʾ����ֹͣ, ��ֹ����.
		if (!m_accept_multi)
		{
			m_abort = true;
			boost::system::error_code ignore;
			m_timer.cancel(ignore);
		}

		return;
	}

	// �����������ʱ��, �����鳬ʱ����.
	object.last_request_time = boost::posix_time::microsec_clock::local_time();

	// �����������ֽ���.
	int available_bytes = default_buffer_size;
	if (m_drop_size != -1)
	{
		available_bytes = (std::min)(m_drop_size, default_buffer_size);
		m_drop_size -= available_bytes;
		if (available_bytes == 0)
		{
			// ���������ռ�ô���CPU, �ó�CPU��Դ.
			boost::this_thread::sleep(boost::posix_time::millisec(1));
		}
	}

	change_outstranding(true);
	// �������ݶ�ȡ����, ����ָ��http_object_ptr, ��ȷ�����̰߳�ȫ.
	object_ptr->stream->async_read_some(boost::asio::buffer(object.buffer, available_bytes),
		boost::bind(&multi_download::handle_read,
			this,
			index, object_ptr,
			boost::asio::placeholders::bytes_transferred,
			boost::asio::placeholders::error
		)
	);
}

template <typename Handler>
void multi_download::handle_start(Handler handler, http_object_ptr object_ptr, const boost::system::error_code& ec)
{
	auto_outstanding ao(*this);
	change_outstranding(false);

	// ��ʧ�����˳�.
	if (ec)
	{
		handler(ec);
		return;
	}

	boost::system::error_code err;

	// ����ʹ������http_stream_object����.
	http_stream_object& object = *object_ptr;

	// ͬ������http_stream����.
	http_stream& h = *object.stream;

	// ��������url��Ϣ.
	std::string location = h.location();
	if (!location.empty())
	{
		m_final_url = location;
	}

	// �ж��Ƿ�֧�ֶ������.
	std::string status_code;
	h.response_options().find(http_options::status_code, status_code);
	if (status_code != "206")
	{
		m_accept_multi = false;
	}
	else
	{
		m_accept_multi = true;
	}

	// ���ò���ģʽ����.
	if (m_settings.disable_multi_download)
	{
		m_accept_multi = false;
	}

	// �õ��ļ���С.
	std::string length;
	h.response_options().find(http_options::content_length, length);
	if (length.empty())
	{
		h.response_options().find(http_options::content_range, length);
		std::string::size_type f = length.find('/');
		if (f++ != std::string::npos)
		{
			length = length.substr(f);
		}
		else
		{
			length = "";
		}

		if (length.empty())
		{
			// �õ����ļ�����, ����Ϊ��֧�ֶ�����ģʽ.
			m_accept_multi = false;
		}
	}

	boost::int64_t file_size = -1;
	if (!length.empty())
	{
		try
		{
			file_size = boost::lexical_cast<boost::int64_t>(length);
		}
		catch (boost::bad_lexical_cast&)
		{
			// �ò�����ȷ���ļ�����, ����Ϊ��֧�ֶ�����ģʽ.
			m_accept_multi = false;
		}
	}

	// ���ļ���С����rangefield.
	if (file_size != -1 && file_size != m_file_size)
	{
		m_file_size = file_size;
		m_rangefield.reset(m_file_size);
		m_downlaoded_field.reset(m_file_size);
	}

	// �Ƿ���ָ�����ļ���, ���Content-Disposition: attachment; filename="file.zip"
	std::string filename;
	h.response_options().find("Content-Disposition", filename);
	if (!filename.empty())
	{
		std::string value;
		detail::content_disposition_filename(filename.begin(), filename.end(), value);
		if (!value.empty())
		{
			m_file_name = value;
		}
	}

	// �Ƿ�֧�ֳ�����ģʽ, ��֧�ֶ������, ������Ҳû������.
	if (m_accept_multi)
	{
		std::string keep_alive;
		h.response_options().find(http_options::connection, keep_alive);
		boost::to_lower(keep_alive);
		if (keep_alive == "keep-alive")
		{
			m_keep_alive = true;
		}
		else
		{
			m_keep_alive = false;
		}

		// ���δָ��meta�ļ���, ��ʹ������url����meta�ļ���.
		if (m_settings.meta_file.empty())
		{
			// û��ָ��meta�ļ���, �Զ�����meta�ļ���.
			m_settings.meta_file = meta_name(m_final_url.to_string());
		}

		// ��meta�ļ�, ����򿪳ɹ�, ���ʾ��������Ӧ��λͼ��.
		if (!open_meta(m_settings.meta_file))
		{
			// λͼ��ʧ��, ����ν, ���ع����лᴴ���µ�λͼ, ɾ��meta�ļ�.
			m_file_meta.close();
			fs::remove(m_settings.meta_file, err);
		}
	}

	// �ж��ļ��Ƿ��Ѿ��������, �����ֱ�ӷ���.
	if (m_downlaoded_field.is_full())
	{
		handler(err);
		return;
	}

	// �����洢����.
	if (!m_settings.storage)
	{
		m_storage.reset(default_storage_constructor());
	}
	else
	{
		m_storage.reset(m_settings.storage());
	}
	BOOST_ASSERT(m_storage);

	// ���ļ�, �����ļ���.
	m_storage->open(boost::filesystem::path(file_name()), err);
	if (err)
	{
		handler(err);
		return;
	}

	// ����Ĭ������.
	if (m_settings.connections_limit == -1)
	{
		m_settings.connections_limit = default_connections_limit;
	}
	if (m_settings.piece_size == -1 && m_file_size != -1)
	{
		m_settings.piece_size = default_piece_size(m_file_size);
	}

	// ���ݵ�1�����ӷ��ص���Ϣ, ��������ѡ��.
	request_opts req_opt = m_settings.opts;
	if (m_keep_alive)
	{
		req_opt.insert(http_options::connection, "keep-alive");
	}
	else
	{
		req_opt.insert(http_options::connection, "close");
	}

	// �޸���ֹ״̬.
	m_abort = false;

	// ���Ӽ�����Ϊ1.
	m_number_of_connections = 1;

	// ��ӵ�һ�����ӵ���������.
	{
#ifndef AVHTTP_DISABLE_THREAD
		boost::mutex::scoped_lock lock(m_streams_mutex);
#endif
		m_streams.push_back(object_ptr);
	}

	// Ϊ��1�����������buffer��С.
	int available_bytes = default_buffer_size;

	// ���õ�1���������ط�Χ.
	if (m_accept_multi)
	{
		range req_range;
		bool need_reopen = false;

		// ���ļ������л��һ�οռ�, ���ǵ�һ�η����obj���ص�����.
		if (allocate_range(req_range))
		{
			// ���䵽����ʼ�߽粻��0, ��Ҫ����open���obj.
			if (req_range.left != 0)
			{
				need_reopen = true;
			}

			// ������������.
			object_ptr->request_range = req_range;

			// �����������䵽����ѡ����.
			req_opt.remove(http_options::range);
			req_opt.insert(http_options::range, boost::str(
				boost::format("bytes=%lld-%lld", std::locale("C")) % req_range.left % req_range.right));

			// �����������ʱ��, ���ڼ�鳬ʱ����.
			object_ptr->last_request_time = boost::posix_time::microsec_clock::local_time();

			// ��Ӵ�������.
			h.proxy(m_settings.proxy);
			// ��������ѡ��.
			h.request_options(req_opt);
			// �����ssl����, Ĭ��Ϊ���֤��.
			h.check_certificate(m_settings.check_certificate);
			// �����ض���.
			h.max_redirects(0);

			if (need_reopen)
			{
				h.close(err);	// �ر�ԭ��������, ��Ҫ�����µ�����.
				if (err)
				{
					handler(err);
					return;
				}

				change_outstranding(true);
				// ��ʼ�첽��.
				h.async_open(m_final_url,
					boost::bind(&multi_download::handle_open,
						this,
						0, object_ptr,
						boost::asio::placeholders::error
					)
				);
			}
			else
			{
				// �������ݶ�ȡ����.
				change_outstranding(true);
				// ����ָ��obj, ��ȷ�����̰߳�ȫ.
				h.async_read_some(boost::asio::buffer(object_ptr->buffer, available_bytes),
					boost::bind(&multi_download::handle_read,
						this,
						0, object_ptr,
						boost::asio::placeholders::bytes_transferred,
						boost::asio::placeholders::error
					)
				);
			}
		}
		else
		{
			// ����ռ�ʧ��, ˵�������Ѿ�û�п��еĿռ��ṩ
			// �����stream����������ֱ����������.
			object_ptr->done = true;
		}
	}
	else	// ��������֧�ֶ������ģʽ, �����ӵ�1����������.
	{
		// �������ݶ�ȡ����.
		change_outstranding(true);
		// ����ָ��obj, ��ȷ�����̰߳�ȫ.
		h.async_read_some(boost::asio::buffer(object_ptr->buffer, available_bytes),
			boost::bind(&multi_download::handle_read,
				this,
				0, object_ptr,
				boost::asio::placeholders::bytes_transferred,
				boost::asio::placeholders::error
			)
		);
	}

	// ���֧�ֶ������, �����ô�������http_stream.
	if (m_accept_multi)
	{
		for (int i = 1; i < m_settings.connections_limit; i++)
		{
			http_object_ptr p = boost::make_shared<http_stream_object>();
			http_stream_ptr ptr = boost::make_shared<http_stream>(boost::ref(m_io_service));
			range req_range;

			// ���ļ������еõ�һ�οռ�.
			if (!allocate_range(req_range))
			{
				// ����ռ�ʧ��, ˵�������Ѿ�û�п��еĿռ��ṩ�����stream����������ֱ����������.
				p->done = true;
				continue;
			}

			// ������������.
			p->request_range = req_range;

			// �����������䵽����ѡ����.
			req_opt.remove(http_options::range);
			req_opt.insert(http_options::range, boost::str(
				boost::format("bytes=%lld-%lld", std::locale("C")) % req_range.left % req_range.right));

			// ��������ѡ��.
			ptr->request_options(req_opt);
			// ��Ӵ�������.
			ptr->proxy(m_settings.proxy);
			// �����ssl����, Ĭ��Ϊ���֤��.
			ptr->check_certificate(m_settings.check_certificate);
			// �����ض���.
			ptr->max_redirects(0);

			// ��������ӵ�������.
			p->stream = ptr;

			{
#ifndef AVHTTP_DISABLE_THREAD
				boost::mutex::scoped_lock lock(m_streams_mutex);
#endif
				m_streams.push_back(p);
			}

			// �����������ʱ��, �����鳬ʱ����.
			p->last_request_time = boost::posix_time::microsec_clock::local_time();

			m_number_of_connections++;
			change_outstranding(true);

			// ��ʼ�첽��, ����ָ��http_object_ptr, ��ȷ�����̰߳�ȫ.
			p->stream->async_open(m_final_url,
				boost::bind(&multi_download::handle_open,
					this,
					i, p,
					boost::asio::placeholders::error
				)
			);
		}
	}

	change_outstranding(true);

	// ������ʱ��, ִ������.
	m_timer.expires_from_now(boost::posix_time::seconds(1));
	m_timer.async_wait(boost::bind(&multi_download::on_tick, this, boost::asio::placeholders::error));

	// �ص�֪ͨ�û�, �Ѿ��ɹ���������.
	handler(ec);

	return;
}

void multi_download::on_tick(const boost::system::error_code& e)
{
	auto_outstanding ao(*this);
	change_outstranding(false);
	m_time_total++;

	// ���������λͼ.
	if (m_accept_multi)
	{
		update_meta();
	}

	// ÿ��1�����һ��on_tick.
	if (!m_abort && !e)
	{
		change_outstranding(true);
		m_timer.expires_from_now(boost::posix_time::seconds(1));
		m_timer.async_wait(boost::bind(&multi_download::on_tick,
			this, boost::asio::placeholders::error));
	}
	else
	{
		// �Ѿ���ֹ.
		return;
	}

	// ���ڼ��㶯̬��������.
	{
		int bytes_count = 0;

		for (int i = 0; i < m_byte_rate->seconds; i++)
			bytes_count += m_byte_rate->last_byte_rate[i];

		m_byte_rate->current_byte_rate = (double)bytes_count / m_byte_rate->seconds;

		if (m_byte_rate->index + 1 >= m_byte_rate->seconds)
			m_byte_rate->last_byte_rate[m_byte_rate->index = 0] = 0;
		else
			m_byte_rate->last_byte_rate[++m_byte_rate->index] = 0;
	}

	// ��������.
	m_drop_size = m_settings.download_rate_limit;

#ifndef AVHTTP_DISABLE_THREAD
	// ����m_streams�������в���, ��֤m_streams������Ψһ��.
	boost::mutex::scoped_lock lock(m_streams_mutex);
#endif
	for (std::size_t i = 0; i < m_streams.size(); i++)
	{
		http_object_ptr& object_ptr = m_streams[i];
		boost::posix_time::time_duration duration =
			boost::posix_time::microsec_clock::local_time() - object_ptr->last_request_time;
		bool expire = duration > boost::posix_time::seconds(m_settings.time_out);
		if (!object_ptr->done && (expire || object_ptr->direct_reconnect))
		{
			// ��ʱ�����, �رղ����´�������.
			boost::system::error_code ec;
			object_ptr->stream->close(ec);

			// ��������֮һ�Ĵ���, �����ٳ������ӷ�����, ��Ϊ����Ҳ��û�������.
			if (object_ptr->ec == avhttp::errc::forbidden
				|| object_ptr->ec == avhttp::errc::not_found
				|| object_ptr->ec == avhttp::errc::method_not_allowed)
			{
				object_ptr->done = true;
				continue;
			}

			// ������ģʽ, ��ʾ����ֹͣ, ��ֹ����.
			if (!m_accept_multi)
			{
				m_abort = true;
				object_ptr->done = true;
				m_number_of_connections--;
				continue;
			}

			// ����������ʶ.
			object_ptr->direct_reconnect = false;

			// ���´���http_object��http_stream.
			object_ptr = boost::make_shared<http_stream_object>(*object_ptr);
			http_stream_object& object = *object_ptr;

			// ʹ���µ�http_stream����.
			object.stream = boost::make_shared<http_stream>(boost::ref(m_io_service));

			http_stream& stream = *object.stream;

			// ��������ѡ��.
			request_opts req_opt = m_settings.opts;

			// �����Ƿ�Ϊ������.
			if (m_keep_alive)
			{
				req_opt.insert(http_options::connection, "keep-alive");
			}

			// �������ϴ�δ��ɵ�λ�ÿ�ʼ����.
			if (m_accept_multi)
			{
				boost::int64_t begin = object.request_range.left + object.bytes_transferred;
				boost::int64_t end = object.request_range.right;

				if (end - begin <= 0)
				{
					// ���������пռ�ʧ��, ���������socket.
					if (!allocate_range(object.request_range))
					{
						object.done = true;	// �Ѿ�ûʲô����������.
						m_number_of_connections--;
						continue;
					}

					object.bytes_transferred = 0;
					begin = object.request_range.left;
					end = object.request_range.right;
				}

				req_opt.insert(http_options::range, boost::str(
					boost::format("bytes=%lld-%lld", std::locale("C")) % begin % end));
			}

			// ��Ӵ�������.
			stream.proxy(m_settings.proxy);
			// ���õ�����ѡ����.
			stream.request_options(req_opt);
			// �����ssl����, Ĭ��Ϊ���֤��.
			stream.check_certificate(m_settings.check_certificate);
			// �����ض���.
			stream.max_redirects(0);

			// �����������ʱ��, �����鳬ʱ����.
			object.last_request_time = boost::posix_time::microsec_clock::local_time();

			change_outstranding(true);
			// ���·����첽����, ����object_item_ptrָ��, ��ȷ���̰߳�ȫ.
			stream.async_open(m_final_url,
				boost::bind(&multi_download::handle_open,
					this,
					i, object_ptr,
					boost::asio::placeholders::error
				)
			);
		}
	}

	// ͳ�Ʋ���������ɵ�http_stream�ĸ���.
	int done = 0;
	for (std::size_t i = 0; i < m_streams.size(); i++)
	{
		http_object_ptr& object_item_ptr = m_streams[i];
		if (object_item_ptr->done)
		{
			done++;
		}
	}

	// ��m_streams���������Ӷ�doneʱ, ��ʾ�Ѿ��������.
	if (done == m_streams.size())
	{
		boost::system::error_code ignore;
		m_abort = true;
		m_timer.cancel(ignore);
		// ֪ͨwait_for_complete�˳�.
		boost::mutex::scoped_lock l(m_quit_mtx);
		m_quit_cond.notify_one();
		return;
	}
}

bool multi_download::allocate_range(range& r)
{
#ifndef AVHTTP_DISABLE_THREAD
	// �ڶ��߳�����io_serviceʱ, �������, ��������ʱ����ظ�������ͬ����.
	// ���߳�ִ��io_service(��������AVHTTP_DISABLE_THREAD)���迼�Ǽ�
	// ��, ��Ϊ���в��������첽���еĶ���.
	boost::mutex::scoped_lock lock(m_rangefield_mutex);
#endif

	range temp(-1, -1);

	do
	{
		// ��ָ��λ��m_download_point��ʼ�ļ������еõ�һ�οռ�.
		if (!m_rangefield.out_space(m_download_point, temp.left, temp.right))
		{
			return false;
		}

		// ���ڵ���.
		BOOST_ASSERT(temp != r);
		BOOST_ASSERT(temp.size() >= 0);

		// ���¼���Ϊ���max_request_bytes��С.
		boost::int64_t max_request_bytes = m_settings.request_piece_num * m_settings.piece_size;
		if (temp.size() > max_request_bytes)
		{
			temp.right = temp.left + max_request_bytes;
		}

		r = temp;

		// ��m_rangefield�з�������ռ�.
		if (!m_rangefield.update(temp))
		{
			continue;
		}
		else
		{
			break;
		}
	} while (!m_abort);

	// �ұ߽߱��1, ��Ϊhttp����������ǰ����ұ߽�ֵ, ����ʱ�Ὣright�±�λ�õ��ֽ�����.
	if (--r.right < r.left)
	{
		return false;
	}

	return true;
}

bool multi_download::open_meta(const fs::path& file_path)
{
	boost::system::error_code ec;

	// �õ��ļ���С.
	boost::uintmax_t size = fs::file_size(file_path, ec);
	if (ec)
	{
		size = 0;
	}

	// ���ļ�.
	m_file_meta.close();
	m_file_meta.open(file_path, file::read_write, ec);
	if (ec)
	{
		return false;
	}

	// ���������, �����meta����.
	if (size != 0)
	{
		std::vector<char> buffer;

		buffer.resize(size);
		const std::streamsize num = m_file_meta.read(&buffer[0], size);
		if (num != size)
		{
			return false;
		}

		entry e = bdecode(buffer.begin(), buffer.end());

		// ���յ�url.
		if (m_settings.allow_use_meta_url)
		{
			const std::string url = e["final_url"].string();
			if (!url.empty())
			{
				m_final_url = url;
			}
		}

		// �ļ���С.
		m_file_size = e["file_size"].integer();
		m_rangefield.reset(m_file_size);
		m_downlaoded_field.reset(m_file_size);

		// ��Ƭ��С.
		m_settings.piece_size = e["piece_size"].integer();

		// ��Ƭ��.
		int piece_num = e["piece_num"].integer();

		// λͼ����.
		std::string bitfield_data = e["bitfield"].string();

		// ���쵽λͼ.
		bitfield bf(bitfield_data.c_str(), piece_num);

		// ���µ����䷶Χ.
		m_rangefield.bitfield_to_range(bf, m_settings.piece_size);
		m_downlaoded_field.bitfield_to_range(bf, m_settings.piece_size);
	}

	return true;
}

void multi_download::update_meta()
{
	if (!m_file_meta.is_open())
	{
		boost::system::error_code ec;
		m_file_meta.open(m_settings.meta_file, file::read_write, ec);
		if (ec)
		{
			return;
		}
	}

	entry e;

	e["final_url"] = m_final_url.to_string();
	e["file_size"] = m_file_size;
	e["piece_size"] = m_settings.piece_size;
	e["piece_num"] = (m_file_size / m_settings.piece_size) +
		(m_file_size % m_settings.piece_size == 0 ? 0 : 1);
	bitfield bf;
	m_downlaoded_field.range_to_bitfield(bf, m_settings.piece_size);
	std::string str(bf.bytes(), bf.bytes_size());
	e["bitfield"] = str;

	std::vector<char> buffer;
	bencode(back_inserter(buffer), e);

	m_file_meta.write(0, &buffer[0], buffer.size());
}

void multi_download::change_outstranding(bool addref/* = true*/)
{
#ifndef AVHTTP_DISABLE_THREAD
	boost::mutex::scoped_lock lock(m_outstanding_mutex);
#endif
	if (addref)
	{
		m_outstanding++;
	}
	else
	{
		m_outstanding--;
	}
}

// Ĭ�ϸ����ļ���С�Զ������Ƭ��С.
std::size_t multi_download::default_piece_size(const boost::int64_t& file_size) const
{
	const int target_size = 40 * 1024;
	std::size_t piece_size = boost::int64_t(file_size / (target_size / 20));

	std::size_t i = 16 * 1024;
	for (; i < 16 * 1024 * 1024; i *= 2)
	{
		if (piece_size > i) continue;
		break;
	}
	piece_size = i;

	return piece_size;
}

} // namespace avhttp

#endif // AVHTTP_MULTI_DOWNLOAD_IPP
