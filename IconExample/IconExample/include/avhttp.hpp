//
// avhttp.hpp
// ~~~~~~~~~~
//
// Copyright (c) 2013 Jack (jack dot wgm at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// path LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef AVHTTP_HPP
#define AVHTTP_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "avhttp/detail/abi_prefix.hpp"

#include <boost/version.hpp>
#include <boost/static_assert.hpp>

namespace
{
	// ����avhttpʹ����boost.locale, ����ʹ��boost�汾1.48�����ϰ汾!!!
	// ԭ����boost.locale����boost-1.48����boost��.
	BOOST_STATIC_ASSERT_MSG(BOOST_VERSION >= 104800, "You must use boost-1.48 or later!!!");
}

#include <boost/format.hpp>
#include <boost/algorithm/string.hpp>

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <boost/noncopyable.hpp>


// Default to a header-only implementation.
#if !defined(AVHTTP_HEADER_ONLY)
#	if !defined(AVHTTP_SEPARATE_COMPILATION)
#		define AVHTTP_HEADER_ONLY 1
#	endif // !defined(AVHTTP_SEPARATE_COMPILATION)
#endif // !defined(AVHTTP_HEADER_ONLY)

#if defined(AVHTTP_HEADER_ONLY)
#	define AVHTTP_DECL inline
# else
#	define AVHTTP_DECL
#endif // defined(AVHTTP_HEADER_ONLY)

// If AVHTTP_DECL isn't defined yet define it now.
#if !defined(AVHTTP_DECL)
# define AVHTTP_DECL
#endif // !defined(AVHTTP_DECL)

#include "avhttp/version.hpp"
#include "avhttp/logging.hpp"
#include "avhttp/detail/error_codec.hpp"
#include "avhttp/url.hpp"
#include "avhttp/http_stream.hpp"
#ifndef AVHTTP_DISABLE_FILE_UPLOAD
# if (BOOST_VERSION >= 105400)
#  include "avhttp/file_upload.hpp"
# endif
#endif
#ifndef AVHTTP_DISABLE_MULTI_DOWNLOAD
# include "avhttp/entry.hpp"
# include "avhttp/bencode.hpp"
# include "avhttp/rangefield.hpp"
# include "avhttp/bitfield.hpp"
# include "avhttp/multi_download.hpp"
#endif
#if (BOOST_VERSION >= 105400)
# include "avhttp/async_read_body.hpp"
#endif

#include "avhttp/detail/abi_suffix.hpp"


#endif // AVHTTP_HPP
