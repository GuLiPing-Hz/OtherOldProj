//
// settings.hpp
// ~~~~~~~~~~~~
//
// Copyright (c) 2013 Jack (jack dot wgm at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// path LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef AVHTTP_SETTINGS_HPP
#define AVHTTP_SETTINGS_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <vector>
#include <map>
#include <string>
#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include <boost/date_time.hpp>

#include "avhttp/storage_interface.hpp"

namespace avhttp {

// ���û�ж�������ض������, ��Ĭ��Ϊ5������ض���.
#ifndef AVHTTP_MAX_REDIRECTS
#define AVHTTP_MAX_REDIRECTS 5
#endif

// ����������httpѡ��.
namespace http_options {

	// ����Ϊavhttp�ڶ���һЩѡ��.
	static const std::string request_method("_request_method"); // ����ʽ(GET/POST)
	static const std::string http_version("_http_version");		// HTTP/1.0|HTTP/1.1
	static const std::string request_body("_request_body");		// һ������POSTһЩ�������֮��ʱʹ��.
	static const std::string status_code("_status_code");	// HTTP״̬��.
	static const std::string path("_path");		// �����path, ��http://abc.ed/v2/cma.txt�е�/v2/cma.txt.
	static const std::string url("_url");		// ������keep-alive��ʱ��, ����host�ϲ�ͬ��urlʱʹ��.
	// �����ǳ��õı�׼http headѡ��.
	static const std::string host("Host");
	static const std::string accept("Accept");
	static const std::string range("Range");
	static const std::string cookie("Cookie");
	static const std::string referer("Referer");
	static const std::string user_agent("User-Agent");
	static const std::string content_type("Content-Type");
	static const std::string content_length("Content-Length");
	static const std::string content_range("Content-Range");
	static const std::string connection("Connection");
	static const std::string proxy_connection("Proxy-Connection");
	static const std::string accept_encoding("Accept-Encoding");
	static const std::string transfer_encoding("Transfer-Encoding");
	static const std::string content_encoding("Content-Encoding");

} // namespace http_options


// �����http��optionѡ��ʵ��.

class option
{
public:
	// ����option_item����.
	typedef std::pair<std::string, std::string> option_item;
	// ����option_item_list����.
	typedef std::vector<option_item> option_item_list;
	// for boost::assign::insert
	typedef option_item value_type;
	// ����ѡ��ص�.
	typedef boost::function<void (boost::system::error_code&)> body_callback_func;

public:

	option()
		: m_fake_continue(false)
	{}

	~option()
	{}

public:

	// ����������������Ӧ��:
	// http_stream s;
	// s.request_options(request_opts()("cookie","XXXXXX"));
	option& operator()(const std::string& key, const std::string& val)
	{
		insert(key, val);
		return *this;
	}

	// ���ѡ��, ��key/value��ʽ���.
	void insert(const std::string& key, const std::string& val)
	{
		m_opts.push_back(option_item(key, val));
	}

	// ���ѡ��� std::part ��ʽ.
	void insert(value_type& item)
	{
		m_opts.push_back(item);
	}

	// ɾ��ѡ��.
	option& remove(const std::string& key)
	{
		for (option_item_list::iterator i = m_opts.begin(); i != m_opts.end(); i++)
		{
			if (i->first == key)
			{
				m_opts.erase(i);
				return *this;
			}
		}
		return *this;
	}

	// ����ָ��key��value.
	bool find(const std::string& key, std::string& val) const
	{
		std::string s = key;
		boost::to_lower(s);
		for (option_item_list::const_iterator f = m_opts.begin(); f != m_opts.end(); f++)
		{
			std::string temp = f->first;
			boost::to_lower(temp);
			if (temp == s)
			{
				val = f->second;
				return true;
			}
		}
		return false;
	}

	// ����ָ���� key �� value. û�ҵ����� ""�������Ǹ�͵���İ���.
	std::string find(const std::string& key) const
	{
		std::string v;
		find(key,v);
		return v;
	}

	// �õ�Header�ַ���.
	std::string header_string() const
	{
		std::string str;
		for (option_item_list::const_iterator f = m_opts.begin(); f != m_opts.end(); f++)
		{
			if (f->first != http_options::status_code)
				str += (f->first + ": " + f->second + "\r\n");
		}
		return str;
	}

	// ���.
	void clear()
	{
		m_opts.clear();
	}

	// ��������option.
	option_item_list& option_all()
	{
		return m_opts;
	}

	// ���ص�ǰoption����.
	int size() const
	{
		return m_opts.size();
	}

	// ����fake_continue.
	bool fake_continue() const
	{
		return m_fake_continue;
	}

	// ����fake_continue.
	void fake_continue(bool b)
	{
		m_fake_continue = b;
	}

protected:
	// ѡ���б�.
	option_item_list m_opts;

	// �Ƿ����ü�100 continue��Ϣ, �������, ���ڷ������http request head
	// ֮��, ����һ��fake continue��Ϣ.
	bool m_fake_continue;
};

// ����ʱ��httpѡ��.
// _http_version, ȡֵ "HTTP/1.0" / "HTTP/1.1", Ĭ��Ϊ"HTTP/1.1".
// _request_method, ȡֵ "GET/POST/HEAD", Ĭ��Ϊ"GET".
// _request_body, �����е�body����, ȡֵ����, Ĭ��Ϊ��.
// Host, ȡֵΪhttp������, Ĭ��Ϊhttp������.
// Accept, ȡֵ����, Ĭ��Ϊ"*/*".
// ��Щ�Ƚϳ��õ�ѡ�������http_options��.
typedef option request_opts;

// http���������ص�httpѡ��.
// һ���������¼���ѡ��:
// _status_code, http����״̬.
// Server, ����������.
// Content-Length, �������ݳ���.
// Connection, ����״̬��ʶ.
typedef option response_opts;



// Http����Ĵ�������.

struct proxy_settings
{
	proxy_settings()
		: type (none)
	{}

	std::string hostname;
	int port;

	std::string username;
	std::string password;

	enum proxy_type
	{
		// û�����ô���.
		none,
		// socks4����, ��Ҫusername.
		socks4,
		// ����Ҫ�û������socks5����.
		socks5,
		// ��Ҫ�û�������֤��socks5����.
		socks5_pw,
		// http����, ����Ҫ��֤.
		http,
		// http����, ��Ҫ��֤.
		http_pw,
	};

	proxy_type type;
};


// һЩĬ�ϵ�ֵ.
static const int default_request_piece_num = 10;
static const int default_time_out = 11;
static const int default_connections_limit = 5;
static const int default_buffer_size = 1024;

// multi_download��������.

struct settings
{
	settings ()
		: download_rate_limit(-1)
		, connections_limit(default_connections_limit)
		, piece_size(-1)
		, time_out(default_time_out)
		, request_piece_num(default_request_piece_num)
		, allow_use_meta_url(true)
		, disable_multi_download(false)
		, check_certificate(true)
		, storage(NULL)
	{}

	// ������������, -1Ϊ������, ��λΪ: byte/s.
	int download_rate_limit;

	// ����������, -1ΪĬ��.
	int connections_limit;

	// �ֿ��С, Ĭ�ϸ����ļ���С�Զ�����.
	int piece_size;

	// ��ʱ�Ͽ�, Ĭ��Ϊ11��.
	int time_out;

	// ÿ������ķ�Ƭ��, Ĭ��Ϊ10.
	int request_piece_num;

	// meta_file·��, Ĭ��Ϊ��ǰ·����ͬ�ļ�����.meta�ļ�.
	fs::path meta_file;

	// ����ʹ��meta�б����url, Ĭ��Ϊ����. ���һЩ�䶯��url, ����Ӧ�ý���.
	bool allow_use_meta_url;

	// ��ֹʹ�ò�������.
	// NOTE: �������ڶ�̬ҳ������, ��Ϊ��̬ҳ�治��ʹ�ò�������, ����������ʹ��
	// multi_download��������, ����Ҫ�����������, ���򲢷����ض�̬���ݵ���Ϊ, ��
	// δ�����, ����������Ϊ���ݳ��Ȳ�һ�����¶���, Ҳ�������ݴ���.
	// NOTE: ���Ƽ�ʹ��multi_download��������, ��Ӧ��ʹ��http_stream��������,
	// multi_download��ҪӦ���ڴ��ļ�, ��̬ҳ������!
	bool disable_multi_download;

	// �����ļ�·��, Ĭ��Ϊ��ǰĿ¼.
	fs::path save_path;

	// �����Ƿ���֤��, Ĭ�ϼ��֤��.
	bool check_certificate;

	// �洢�ӿڴ�������ָ��, Ĭ��Ϊmulti_download�ṩ��file.hppʵ��.
	storage_constructor_type storage;

	// ����ѡ��.
	request_opts opts;

	// ��������.
	proxy_settings proxy;
};

} // namespace avhttp

#endif // AVHTTP_SETTINGS_HPP
