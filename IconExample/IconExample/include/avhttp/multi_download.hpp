//
// multi_download.hpp
// ~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2013 Jack (jack dot wgm at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// path LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef AVHTTP_MULTI_DOWNLOAD_HPP
#define AVHTTP_MULTI_DOWNLOAD_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <vector>
#include <list>
#include <algorithm>    // for std::min/std::max

#include <boost/assert.hpp>
#include <boost/noncopyable.hpp>
#include <boost/cstdint.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>
#include <boost/thread.hpp>
#include <boost/thread/condition.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/date_time.hpp>
#include <boost/format.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/crc.hpp>  // for boost::crc_32_type

#include "avhttp/http_stream.hpp"
#include "avhttp/rangefield.hpp"
#include "avhttp/entry.hpp"
#include "avhttp/settings.hpp"
#include "file.hpp"


namespace avhttp {

// multi_download��ľ���ʵ��.
class multi_download : public boost::noncopyable
{
	// �ض���http_stream_ptrָ��.
	typedef boost::shared_ptr<http_stream> http_stream_ptr;

	// ����http_stream_obj.
	struct http_stream_object;

	// �ض���http_object_ptrָ��.
	typedef boost::shared_ptr<http_stream_object> http_object_ptr;

	// ���ڼ�����������.
	struct byte_rate;
	typedef boost::shared_ptr<byte_rate> byte_rate_ptr;

	// ���ڰ���multi_download�Զ�����outstranding.
	struct auto_outstanding;
	friend struct auto_outstanding;

public:

	/// Constructor.
	AVHTTP_DECL explicit multi_download(boost::asio::io_service& io);

	/// Destructor.
	AVHTTP_DECL ~multi_download();

public:

	///��ʼmulti_download��ʼ����.
	// @param uָ����url.
	// @param ec����������ʱ, ������ϸ�Ĵ�����Ϣ.
	// @��ע: ֱ��ʹ���ڲ���file.hpp�������ݵ��ļ�, �����Լ�ָ���������ص�ָ���ĵط�
	// ����ͨ��������һ��open�����, �������һ��open����ϸ˵��.
	AVHTTP_DECL void start(const std::string& u, boost::system::error_code& ec);

	///��ʼmulti_download��ʼ����, ��ʧ���׳�һ���쳣.
	// @param uָ����url.
	// @��ע: ֱ��ʹ���ڲ���file.hpp�������ݵ��ļ�, �����Լ�ָ���������ص�ָ���ĵط�
	// ����ͨ��������һ��open�����, �������һ��open����ϸ˵��.
	AVHTTP_DECL void start(const std::string& u);

	///��ʼmulti_download��ʼ����.
	// @param uָ����url.
	// @param sָ����������Ϣ.
	// @ʧ���׳�һ��boost::system::system_error�쳣, ������ϸ�Ĵ�����Ϣ.
	AVHTTP_DECL void start(const std::string& u, const settings& s);

	///��ʼmulti_download��ʼ����.
	// @param uָ����url.
	// @param sָ����������Ϣ.
	// @����error_code, ������ϸ�Ĵ�����Ϣ.
	AVHTTP_DECL void start(const std::string& u, const settings& s, boost::system::error_code& ec);

	///�첽��������, ������ɽ��ص���Ӧ��Handler.
	// @param u ��Ҫ���ص�URL.
	// @param handler �����������������ʱ. ������������������:
	// @begin code
	//  void handler(
	//    const boost::system::error_code& ec // ���ڷ��ز���״̬.
	//  );
	// @end code
	// @begin example
	//  void start_handler(const boost::system::error_code& ec)
	//  {
	//    if (!ec)
	//    {
	//      // �������سɹ�!
	//    }
	//  }
	//  ...
	//  avhttp::multi_download h(io_service);
	//  h.async_start("http://www.boost.org", start_handler);
	// @end example
	// @��ע: handlerҲ����ʹ��boost.bind����һ�����Ϲ涨�ĺ�����
	// Ϊasync_start�Ĳ���handler.
	template <typename Handler>
	void async_start(const std::string& u, Handler handler);

	///�첽��������, ������ɽ��ص���Ӧ��Handler.
	// @param u ��Ҫ���ص�URL.
	// @param s �������ò�����Ϣ.
	// @param handler �����������������ʱ. ������������������:
	// @begin code
	//  void handler(
	//    const boost::system::error_code& ec // ���ڷ��ز���״̬.
	//  );
	// @end code
	// @begin example
	//  void start_handler(const boost::system::error_code& ec)
	//  {
	//    if (!ec)
	//    {
	//      // �������سɹ�!
	//    }
	//  }
	//  ...
	//  avhttp::multi_download h(io_service);
	//  settings s;
	//  h.async_open("http://www.boost.org", s, start_handler);
	// @end example
	// @��ע: handlerҲ����ʹ��boost.bind����һ�����Ϲ涨�ĺ�����
	// Ϊasync_start�Ĳ���handler.
	template <typename Handler>
	void async_start(const std::string& u, const settings& s, Handler handler);

	// stop��ǰ��������, ֹͣ����.
	AVHTTP_DECL void stop();

	///��ȡָ��������, ���ı����ص��λ��.
	// @param buffers ָ�������ݻ���. ������ͱ�������MutableBufferSequence�Ķ���,
	//          MutableBufferSequence�Ķ�����boost.asio�ĵ���.
	// @param offset ��ȡ���ݵ�ָ��ƫ��λ��, ע��: offsetӰ���ڲ�����λ�ô�offset��ʼ����.
	// ���ض�ȡ���ݵĴ�С.
	template <typename MutableBufferSequence>
	std::size_t fetch_data(const MutableBufferSequence& buffers,
		boost::int64_t offset);

	///���ص�ǰ������Ϣ.
	AVHTTP_DECL const settings& set() const;

	///�Ƿ�ֹͣ����.
	AVHTTP_DECL bool stopped() const;

	///�ȴ�ֱ���������.
	// @��ɷ���true, ���򷵻�false.
	AVHTTP_DECL bool wait_for_complete();

	///�����Ƿ���֤��, Ĭ�ϼ��֤��.
	// @param checkָ���Ƿ���ssl֤��.
	AVHTTP_DECL void check_certificate(bool check);

	///���ص�ǰ���ص��ļ���С.
	// @�����������֧�ֶ������, ������ļ���СΪ-1.
	AVHTTP_DECL boost::int64_t file_size() const;

	///����url�������Ӧ��meta�ļ���.
	// @param url��ָ����url��ַ.
	// @����һ����crc32����url���16�����ַ���meta�ļ���.
	AVHTTP_DECL std::string meta_name(const std::string& url) const;

	///�õ���ǰ���ص��ļ���.
	// @��������url��̫����, ����ܷ��ش�����ļ���.
	AVHTTP_DECL std::string file_name() const;

	///��ǰ�Ѿ����ص��ֽ�����.
	AVHTTP_DECL boost::int64_t bytes_download() const;

	///��ǰ��������, ��λbyte/s.
	AVHTTP_DECL int download_rate() const;

	///������������, -1Ϊ������, ��λbyte/s.
	AVHTTP_DECL void download_rate_limit(int rate);

	///���ص�ǰ����.
	AVHTTP_DECL int download_rate_limit() const;

protected:

	AVHTTP_DECL void handle_open(const int index,
		http_object_ptr object_ptr, const boost::system::error_code& ec);

	AVHTTP_DECL void handle_read(const int index,
		http_object_ptr object_ptr, int bytes_transferred, const boost::system::error_code& ec);

	AVHTTP_DECL void handle_request(const int index,
		http_object_ptr object_ptr, const boost::system::error_code& ec);

	template <typename Handler>
	void handle_start(Handler handler, http_object_ptr object_ptr, const boost::system::error_code& ec);

	AVHTTP_DECL void on_tick(const boost::system::error_code& e);

	AVHTTP_DECL bool allocate_range(range& r);

	AVHTTP_DECL bool open_meta(const fs::path& file_path);

	AVHTTP_DECL void update_meta();

private:

	AVHTTP_DECL void change_outstranding(bool addref = true);

	// Ĭ�ϸ����ļ���С�Զ������Ƭ��С.
	AVHTTP_DECL std::size_t default_piece_size(const boost::int64_t& file_size) const;

private:
	// io_service����.
	boost::asio::io_service& m_io_service;

	// ÿһ��http_stream_obj��һ��http����.
	// ע��: �����е�http_object_ptrֻ����on_tickһ������д����, ����ȷ�������ط�
	// ���µĸ���, ����Ҫ�����ڷ����µ��첽������ʱ��http_object_ptr��Ϊ������ʽ
	// ����, �������첽�ص���ֻ��Ҫ����http_object_ptr�ĸ���ָ��, ������ֱ�ӷ���
	// m_streams!!!
	std::vector<http_object_ptr> m_streams;

#ifndef AVHTTP_DISABLE_THREAD
	// Ϊm_streams�ڶ��̻߳������̰߳�ȫ.
	mutable boost::mutex m_streams_mutex;
#endif

	// ���յ�url, �������ת�Ļ�, ����ת�����Ǹ�url.
	url m_final_url;

	// �Ƿ�֧�ֶ������.
	bool m_accept_multi;

	// �Ƿ�֧�ֳ�����.
	bool m_keep_alive;

	// �ļ���С, ���û���ļ���СֵΪ-1.
	boost::int64_t m_file_size;

	// ������ļ���.
	mutable std::string m_file_name;

	// ��ǰ�û�����.
	settings m_settings;

	// ��ʱ��, ���ڶ�ʱִ��һЩ����, �����������Ƿ�ʱ֮��.
	boost::asio::deadline_timer m_timer;

	// ��̬��������.
	byte_rate_ptr m_byte_rate;

	// ʵ��������.
	int m_number_of_connections;

	// ���ؼ�ʱ.
	int m_time_total;

	// �������ݴ洢�ӿ�ָ��, �����û�����, ����openʱָ��.
	boost::scoped_ptr<storage_interface> m_storage;

	// meta�ļ�, ��������.
	file m_file_meta;

	// ���ص�λ��.
	boost::int64_t m_download_point;

	// �ļ�����ͼ, ÿ��������m_rangefield������ռ�����.
	rangefield m_rangefield;

	// �Ѿ����ص��ļ�����.
	rangefield m_downlaoded_field;

	// ��֤������������Ψһ��.
#ifndef AVHTTP_DISABLE_THREAD
	boost::mutex m_rangefield_mutex;
#endif

	// ��������.
	int m_drop_size;

	// �����첽��������.
	int m_outstanding;

#ifndef AVHTTP_DISABLE_THREAD
	mutable boost::mutex m_outstanding_mutex;
#endif

	// ����֪ͨwait_for_complete�˳�.
	mutable boost::mutex m_quit_mtx;
	mutable boost::condition m_quit_cond;

	// �Ƿ���ֹ����.
	bool m_abort;
};

} // avhttp

#if defined(AVHTTP_HEADER_ONLY)
#	include "avhttp/impl/multi_download.cpp"
#endif

#endif // AVHTTP_MULTI_DOWNLOAD_HPP
