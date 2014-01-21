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
#include <std-format/type_traits.hpp>

#include <algorithm>
#include <array>
#include <cassert>
#include <functional>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <tuple>
#include <vector>

namespace std { namespace experimental
{
	template<class FormatSource, class... Args>
	class formatter;

	template<class... Args>
	using sformatter = formatter<string, Args...>;
	template<class... Args>
	using wformatter = formatter<wstring, Args...>;
	template<class... Args>
	using u16formatter = formatter<u16string, Args...>;
	template<class... Args>
	using u32formatter = formatter<u32string, Args...>;
	
	template<class... Args>
	using svformatter = formatter<string_view, Args...>;
	template<class... Args>
	using wvformatter = formatter<wstring_view, Args...>;
	template<class... Args>
	using u16vformatter = formatter<u16string_view, Args...>;
	template<class... Args>
	using u32vformatter = formatter<u32string_view, Args...>;
	
	constexpr struct in_place_t { } in_place{};
	
	namespace detail
	{
		// Unevaluated helper methods
		template<class T>
		auto char_type_impl(T) -> typename T::value_type;
		wchar_t char_type_impl(wchar_t*);
		char16_t char_type_impl(char16_t*);
		char32_t char_type_impl(char32_t*);
		char char_type_impl(...);
		template<class T>
		using char_type = decltype(detail::char_type_impl(declval<decay_t<T>>()));
		
		template<class T>
		auto traits_type_impl(T, typename T::traits_type* = 0) -> typename T::traits_type;
		template<class T>
		auto traits_type_impl(T, ...) -> char_traits<char_type<T>>;
		template<class T>
		using traits_type = decltype(detail::traits_type_impl(declval<decay_t<T>>()));
		
		template<class T>
		auto allocator_type_impl(T, typename T::allocator_type* = 0) -> typename T::allocator_type;
		template<class T>
		auto allocator_type_impl(T, ...) -> allocator<char_type<T>>;
		template<class T>
		using allocator_type = decltype(detail::allocator_type_impl(declval<decay_t<T>>()));
		
		template<class T, class Allocator>
		using string_type = basic_string<char_type<T>, traits_type<T>, Allocator>;
	}
	
	/// \name Overloads accepting `formatter`
	//@{
	
	template<class FormatSource, class... Args>
	auto format(const FormatSource& fmt, const Args&... args)
		-> detail::string_type<FormatSource, detail::allocator_type<FormatSource>>;
	
	template<class Allocator, class FormatSource, class... Args>
	auto format(allocator_arg_t, const Allocator& alloc, const FormatSource& fmt, const Args&... args)
		-> detail::string_type<FormatSource, Allocator>;
	
	template<class Destination, class FormatSource, class... Args>
	auto format(in_place_t, Destination& dest, const FormatSource& fmt, const Args&... args)
		-> Destination&;
/*
	template<class FormatSource, class... Args1, class... Args2>
	auto format(const formatter<FormatSource, Args1...>& fmt, const Args2&... args)
		-> typename formatter<FormatSource, Args1...>::result_type
	{
		static_assert(sizeof...(Args1) == sizeof...(Args2), "Number of arguments does not match formatter type.");
		// Diagnose incompatible types
		return fmt(args...);
	}
	
	template<class CharT, class Traits, class FormatSource, class... Args1, class... Args2>
	void format(basic_streambuf<CharT, Traits>& buf, const formatter<FormatSource, Args1...>& fmt, const Args2&... args)
	{
		static_assert(sizeof...(Args1) == sizeof...(Args2), "Number of arguments does not match formatter type.");
		return fmt(buf, args...);
	}
	
	template<class CharT, class Traits, class FormatSource, class... Args1, class... Args2>
	auto format(basic_ostream<CharT, Traits>& os, const formatter<FormatSource, Args1...>& fmt, const Args2&... args)
		-> basic_ostream<CharT, Traits>&
	{
		static_assert(sizeof...(Args1) == sizeof...(Args2), "Number of arguments does not match formatter type.");
		return fmt(os, args...);
	}
	
	template<class CharT, class Traits, class Allocator, class FormatSource, class... Args1, class... Args2>
	void format(basic_string<CharT, Traits, Allocator>& str, const formatter<FormatSource, Args1...>& fmt, const Args2&... args)
	{
		static_assert(sizeof...(Args1) == sizeof...(Args2), "Number of arguments does not match formatter type.");
		return fmt(str, args...);
	}
	
	/// \name Overloads accepting `const char*`-like formats
	//@{
	
	template<class CharT, class... Args>
	auto format(const CharT* fmt, const Args&... args) -> basic_string<CharT>;
	
	template<class CharT, class Traits, class... Args>
	void format(basic_streambuf<CharT, Traits>& buf, const CharT* fmt, const Args&... args);
	
	template<class CharT, class Traits, class... Args>
	basic_ostream<CharT, Traits>& format(basic_ostream<CharT, Traits>& os, const CharT* fmt, const Args&... args);
	
	//@}
	/// \name Overloads accepting `basic_string` formats
	//@{
	
	template<class CharT, class Traits, class Allocator, class... Args>
	auto format(const basic_string<CharT, Traits, Allocator>& fmt, const Args&... args) -> basic_string<CharT, Traits, Allocator>;
	
	template<class CharT, class Traits, class Allocator, class... Args>
	void format(basic_streambuf<CharT, Traits>& buf, const basic_string<CharT, Traits, Allocator>& fmt, const Args&... args);
	
	template<class CharT, class Traits, class Allocator, class... Args>
	basic_ostream<CharT, Traits>&
		format(basic_ostream<CharT, Traits>& os, const basic_string<CharT, Traits, Allocator>& fmt, const Args&... args);
	
	//@}
	/// \name Overloads accepting `basic_string_view` formats
	//@{
	
	template<class CharT, class Traits, class... Args>
	auto format(basic_string_view<CharT, Traits> fmt, const Args&... args) -> basic_string<CharT, Traits>;
	
	template<class CharT, class Traits, class... Args>
	void format(basic_streambuf<CharT, Traits>& buf, basic_string_view<CharT, Traits> fmt, const Args&... args);
	
	template<class CharT, class Traits, class... Args>
	basic_ostream<CharT, Traits>&
		format(basic_ostream<CharT, Traits>& os, basic_string_view<CharT, Traits> fmt, const Args&... args);
	*/
	//@}
	
	// Strangely these aren't defined in std
	string to_string(char ch) { return { ch }; }
	wstring to_string(wchar_t ch) { return { ch }; }
	u16string to_string(char16_t ch) { return { ch }; }
	u32string to_string(char32_t ch) { return { ch }; }
}} // namespace std::experimental

// Must be inclued after defining the std_format::to_string() and to_string() methods for primitive types not findable by ADL
#include <std-format/detail/dispatch_to_string.hpp>

// Require previous declarations of public names.
#include <std-format/detail/appender.hpp>
#include <std-format/detail/format_parser.hpp>
#include <std-format/detail/immediate_formatter.hpp>
//#include <std-format/detail/formatter.hpp>

namespace std { namespace experimental
{
	namespace detail
	{
		using std::begin;
		using std::end;
		
		template<class Appender, class FormatSource, class... Args>
		auto format_impl(Appender& app, const FormatSource& fmt, const Args&... args)
			-> decltype(app = fmt(app, args...))
		{
			app = fmt(app, args...);
		}
		template<class Appender, class FormatSource, class... Args>
		void format_impl(Appender& app, const FormatSource& fmt, const Args&... args)
		{
			using CharT = detail::char_type<FormatSource>;
			using Traits = detail::traits_type<FormatSource>;
			
			detail::immediate_formatter<CharT, Traits, Args...> formatter{fmt};
			app = formatter(app, args...);
		}
	}
	
}} // namespace std::experimental

template<class FormatSource, class... Args>
auto std::experimental::format(const FormatSource& fmt, const Args&... args)
	-> detail::string_type<FormatSource, detail::allocator_type<FormatSource>>
{
	using Allocator = detail::allocator_type<FormatSource>;
	return format(allocator_arg, Allocator{}, fmt, args...);
}

template<class Allocator, class FormatSource, class... Args>
auto std::experimental::format(allocator_arg_t, const Allocator& alloc, const FormatSource& fmt, const Args&... args)
	-> detail::string_type<FormatSource, Allocator>
{
	detail::string_type<FormatSource, Allocator> out{alloc};
	format(in_place, out, fmt, args...);
	return out;
}

template<class Destination, class FormatSource, class... Args>
auto std::experimental::format(in_place_t, Destination& dest, const FormatSource& fmt, const Args&... args)
	-> Destination&
{
	auto app = appender_for(dest);
	detail::format_impl(app, fmt, args...);
	return dest;
}
/*
template<class CharT, class... Args>
auto std::experimental::format(const CharT* fmt, const Args&... args) -> basic_string<CharT>
{
	assert(fmt && "Format string is NULL");
	using String = basic_string_view<CharT>;
	detail::immediate_formatter<String, Args...> formatter{fmt};
	return formatter(args...);
}

template<class CharT, class Traits, class... Args>
void std::experimental::format(basic_streambuf<CharT, Traits>& buf, const CharT* fmt, const Args&... args)
{
	assert(fmt && "Format string is NULL");
	using String = basic_string_view<CharT, Traits>;
	detail::immediate_formatter<String, Args...> formatter{fmt};
	formatter(buf, args...);
}

template<class CharT, class Traits, class... Args>
auto std::experimental::format(basic_ostream<CharT, Traits>& os, const CharT* fmt, const Args&... args)
	-> basic_ostream<CharT, Traits>&
{
	assert(fmt && "Format string is NULL");
	using String = basic_string_view<CharT, Traits>;
	detail::immediate_formatter<String, Args...> formatter{fmt};
	return formatter(os, args...);
}

template<class CharT, class Traits, class Allocator, class... Args>
auto std::experimental::format(const basic_string<CharT, Traits, Allocator>& fmt, const Args&... args)
	-> basic_string<CharT, Traits, Allocator>
{
	using String = basic_string_view<CharT, Traits>;
	detail::immediate_formatter<String, Args...> formatter{fmt};
	return formatter(args...);
}

template<class CharT, class Traits, class Allocator, class... Args>
void std::experimental::format(basic_streambuf<CharT, Traits>& buf, const basic_string<CharT, Traits, Allocator>& fmt, const Args&... args)
{
	using String = basic_string_view<CharT, Traits>;
	detail::immediate_formatter<String, Args...> formatter{fmt};
	formatter(buf, args...);
}

template<class CharT, class Traits, class Allocator, class... Args>
auto std::experimental::format(basic_ostream<CharT, Traits>& os, const basic_string<CharT, Traits, Allocator>& fmt, const Args&... args)
	-> basic_ostream<CharT, Traits>&
{
	using String = basic_string_view<CharT, Traits>;
	detail::immediate_formatter<String, Args...> formatter{fmt};
	return formatter(os, args...);
}

template<class CharT, class Traits, class... Args>
auto std::experimental::format(basic_string_view<CharT, Traits> fmt, const Args&... args)
	-> basic_string<CharT, Traits>
{
	using String = basic_string_view<CharT, Traits>;
	detail::immediate_formatter<String, Args...> formatter{fmt};
	return formatter(args...);
}

template<class CharT, class Traits, class... Args>
void std::experimental::format(basic_streambuf<CharT, Traits>& buf, basic_string_view<CharT, Traits> fmt, const Args&... args)
{
	using String = basic_string_view<CharT, Traits>;
	detail::immediate_formatter<String, Args...> formatter{fmt};
	formatter(buf, args...);
}

template<class CharT, class Traits, class... Args>
auto std::experimental::format(basic_ostream<CharT, Traits>& os, basic_string_view<CharT, Traits> fmt, const Args&... args)
	-> basic_ostream<CharT, Traits>&
{
	using String = basic_string_view<CharT, Traits>;
	detail::immediate_formatter<String, Args...> formatter{fmt};
	return formatter(os, args...);
}
*/
#endif // std_format_format_hpp
