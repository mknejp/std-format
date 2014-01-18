//
//  format.hpp
//  std-format
//
//  Created by knejp on 15.1.14.
//  Copyright (c) 2014 Miro Knejp. All rights reserved.
//

#ifndef std_format_format_hpp
#define std_format_format_hpp

#include <std-format/integer_sequence.hpp>
#include <std-format/string_view.hpp>

#include <algorithm>
#include <array>
#include <cassert>
#include <functional>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <tuple>
#include <type_traits>
#include <vector>

namespace std_format
{
	template<class FormatSource, class... Args>
	class formatter;

	template<class... Args>
	using sformatter = formatter<std::string, Args...>;
	template<class... Args>
	using wformatter = formatter<std::wstring, Args...>;
	template<class... Args>
	using u16formatter = formatter<std::u16string, Args...>;
	template<class... Args>
	using u32formatter = formatter<std::u32string, Args...>;
	
	template<class... Args>
	using svformatter = formatter<string_view, Args...>;
	template<class... Args>
	using wvformatter = formatter<wstring_view, Args...>;
	template<class... Args>
	using u16vformatter = formatter<u16string_view, Args...>;
	template<class... Args>
	using u32vformatter = formatter<u32string_view, Args...>;
	
	/// \name Overloads accepting `formatter`
	//@{
	
	template<class FormatSource, class... Args1, class... Args2>
	auto format(const formatter<FormatSource, Args1...>& fmt, const Args2&... args)
		-> typename formatter<FormatSource, Args1...>::result_type
	{
		static_assert(sizeof...(Args1) == sizeof...(Args2), "Number of arguments does not match formatter type.");
		// Diagnose incompatible types
		return fmt(args...);
	}
	
	template<class CharT, class Traits, class FormatSource, class... Args1, class... Args2>
	void format(std::basic_streambuf<CharT, Traits>& buf, const formatter<FormatSource, Args1...>& fmt, const Args2&... args)
	{
		static_assert(sizeof...(Args1) == sizeof...(Args2), "Number of arguments does not match formatter type.");
		return fmt(buf, args...);
	}
	
	template<class CharT, class Traits, class FormatSource, class... Args1, class... Args2>
	auto format(std::basic_ostream<CharT, Traits>& os, const formatter<FormatSource, Args1...>& fmt, const Args2&... args)
		-> std::basic_ostream<CharT, Traits>&
	{
		static_assert(sizeof...(Args1) == sizeof...(Args2), "Number of arguments does not match formatter type.");
		return fmt(os, args...);
	}
	
	template<class CharT, class Traits, class Allocator, class FormatSource, class... Args1, class... Args2>
	void format(std::basic_string<CharT, Traits, Allocator>& str, const formatter<FormatSource, Args1...>& fmt, const Args2&... args)
	{
		static_assert(sizeof...(Args1) == sizeof...(Args2), "Number of arguments does not match formatter type.");
		return fmt(str, args...);
	}
	
	/// \name Overloads accepting `const char*`-like formats
	//@{
	
	template<class CharT, class... Args>
	auto format(const CharT* fmt, const Args&... args) -> std::basic_string<CharT>;
	
	template<class CharT, class Traits, class... Args>
	void format(std::basic_streambuf<CharT, Traits>& buf, const CharT* fmt, const Args&... args);
	
	template<class CharT, class Traits, class... Args>
	std::basic_ostream<CharT, Traits>& format(std::basic_ostream<CharT, Traits>& os, const CharT* fmt, const Args&... args);
	
	//@}
	/// \name Overloads accepting `basic_string` formats
	//@{
	
	template<class CharT, class Traits, class Allocator, class... Args>
	auto format(const std::basic_string<CharT, Traits, Allocator>& fmt, const Args&... args) -> std::basic_string<CharT, Traits, Allocator>;
	
	template<class CharT, class Traits, class Allocator, class... Args>
	void format(std::basic_streambuf<CharT, Traits>& buf, const std::basic_string<CharT, Traits, Allocator>& fmt, const Args&... args);
	
	template<class CharT, class Traits, class Allocator, class... Args>
	std::basic_ostream<CharT, Traits>&
		format(std::basic_ostream<CharT, Traits>& os, const std::basic_string<CharT, Traits, Allocator>& fmt, const Args&... args);
	
	//@}
	/// \name Overloads accepting `basic_string_view` formats
	//@{
	
	template<class CharT, class Traits, class... Args>
	auto format(basic_string_view<CharT, Traits> fmt, const Args&... args) -> std::basic_string<CharT, Traits>;
	
	template<class CharT, class Traits, class... Args>
	void format(std::basic_streambuf<CharT, Traits>& buf, basic_string_view<CharT, Traits> fmt, const Args&... args);
	
	template<class CharT, class Traits, class... Args>
	std::basic_ostream<CharT, Traits>&
		format(std::basic_ostream<CharT, Traits>& os, basic_string_view<CharT, Traits> fmt, const Args&... args);
	
	//@}
	
	// Strangely these aren't defined in std
	std::string to_string(char ch) { return { ch }; }
	std::wstring to_string(wchar_t ch) { return { ch }; }
	std::u16string to_string(char16_t ch) { return { ch }; }
	std::u32string to_string(char32_t ch) { return { ch }; }
}

// Must be inclued after defining the std_format::to_string() and std::to_string() methods for primitive types not findable by ADL
#include <std-format/detail/dispatch_to_string.hpp>

// Require previous declarations of public names.
#include <std-format/detail/formatter_base.hpp>
#include <std-format/detail/immediate_formatter.hpp>
#include <std-format/detail/formatter.hpp>

template<class CharT, class... Args>
auto std_format::format(const CharT* fmt, const Args&... args) -> std::basic_string<CharT>
{
	assert(fmt && "Format string is NULL");
	using String = basic_string_view<CharT>;
	detail::immediate_formatter<String, Args...> formatter{fmt};
	return formatter(args...);
}

template<class CharT, class Traits, class... Args>
void std_format::format(std::basic_streambuf<CharT, Traits>& buf, const CharT* fmt, const Args&... args)
{
	assert(fmt && "Format string is NULL");
	using String = basic_string_view<CharT, Traits>;
	detail::immediate_formatter<String, Args...> formatter{fmt};
	formatter(buf, args...);
}

template<class CharT, class Traits, class... Args>
std::basic_ostream<CharT, Traits>&
std_format::format(std::basic_ostream<CharT, Traits>& os, const CharT* fmt, const Args&... args)
{
	assert(fmt && "Format string is NULL");
	using String = basic_string_view<CharT, Traits>;
	detail::immediate_formatter<String, Args...> formatter{fmt};
	return formatter(os, args...);
}

template<class CharT, class Traits, class Allocator, class... Args>
std::basic_string<CharT, Traits, Allocator>
std_format::format(const std::basic_string<CharT, Traits, Allocator>& fmt, const Args&... args)
{
	using String = basic_string_view<CharT, Traits>;
	detail::immediate_formatter<String, Args...> formatter{fmt};
	return formatter(args...);
}

template<class CharT, class Traits, class Allocator, class... Args>
void std_format::format(std::basic_streambuf<CharT, Traits>& buf, const std::basic_string<CharT, Traits, Allocator>& fmt, const Args&... args)
{
	using String = basic_string_view<CharT, Traits>;
	detail::immediate_formatter<String, Args...> formatter{fmt};
	formatter(buf, args...);
}

template<class CharT, class Traits, class Allocator, class... Args>
std::basic_ostream<CharT, Traits>&
std_format::format(std::basic_ostream<CharT, Traits>& os, const std::basic_string<CharT, Traits, Allocator>& fmt, const Args&... args)
{
	using String = basic_string_view<CharT, Traits>;
	detail::immediate_formatter<String, Args...> formatter{fmt};
	return formatter(os, args...);
}

template<class CharT, class Traits, class... Args>
std::basic_string<CharT, Traits>
std_format::format(basic_string_view<CharT, Traits> fmt, const Args&... args)
{
	using String = basic_string_view<CharT, Traits>;
	detail::immediate_formatter<String, Args...> formatter{fmt};
	return formatter(args...);
}

template<class CharT, class Traits, class... Args>
void std_format::format(std::basic_streambuf<CharT, Traits>& buf, basic_string_view<CharT, Traits> fmt, const Args&... args)
{
	using String = basic_string_view<CharT, Traits>;
	detail::immediate_formatter<String, Args...> formatter{fmt};
	formatter(buf, args...);
}

template<class CharT, class Traits, class... Args>
std::basic_ostream<CharT, Traits>&
std_format::format(std::basic_ostream<CharT, Traits>& os, basic_string_view<CharT, Traits> fmt, const Args&... args)
{
	using String = basic_string_view<CharT, Traits>;
	detail::immediate_formatter<String, Args...> formatter{fmt};
	return formatter(os, args...);
}

#endif // std_format_format_hpp
