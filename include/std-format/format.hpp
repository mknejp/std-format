//
//  format.hpp
//  std-format
//
//  Created by knejp on 15.1.14.
//  Copyright (c) 2014 Miro Knejp. All rights reserved.
//

#ifndef std_format_format_hpp
#define std_format_format_hpp

#include <std-format/detail/format_appender.hpp>
#include <std-format/integer_sequence.hpp>
#include <std-format/string.hpp>
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
	
	/// \name Format string validation
	//@{
	
	template<class CharT, class Traits>
	void validate_format(basic_string_view<CharT, Traits> fmt, size_t nargs);
	
	template<class... Args, class CharT, class Traits>
	void validate_format(basic_string_view<CharT, Traits> fmt) { validate_format(fmt, sizeof...(Args)); }
	
	template<class CharT>
	void validate_format(const CharT* fmt, size_t nargs) { validate_format(basic_string_view<CharT>{fmt}, nargs); }
	
	template<class... Args, class CharT>
	void validate_format(const CharT* fmt) { validate_format<Args...>(basic_string_view<CharT>{fmt}); }
	
	template<class CharT, class Traits>
	bool validate_format(basic_string_view<CharT, Traits> fmt, size_t nargs, nothrow_t) noexcept;
	
	template<class... Args, class CharT, class Traits>
	bool validate_format(basic_string_view<CharT, Traits> fmt, nothrow_t) noexcept { return validate_format(fmt, sizeof...(Args), nothrow); }
	
	template<class CharT>
	bool validate_format(const CharT* fmt, size_t nargs, nothrow_t) noexcept { return validate_format(basic_string_view<CharT>{fmt}, nargs, nothrow); }
	
	template<class... Args, class CharT>
	bool validate_format(const CharT* fmt, nothrow_t) noexcept { return validate_format<Args...>(basic_string_view<CharT>{fmt}, nothrow); }
	
	//@}
	/// \name Main format method
	//@{
	
	template<class Result, class FormatSource, class... Args>
	auto format(const FormatSource& fmt, const Args&... args) -> Result;
	
	template<class FormatSource, class... Args>
	auto format(const FormatSource& fmt, const Args&... args)
		-> detail::string_type<FormatSource, detail::allocator_type<FormatSource>>
	{
		return format<detail::string_type<FormatSource, detail::allocator_type<FormatSource>>>(fmt, args...);
	}
	
	template<class Result, class Allocator, class FormatSource, class... Args>
	auto format(allocator_arg_t, const Allocator& alloc, const FormatSource& fmt, const Args&... args) -> Result;
	
	template<class Allocator, class FormatSource, class... Args>
	auto format(allocator_arg_t, const Allocator& alloc, const FormatSource& fmt, const Args&... args)
		-> detail::string_type<FormatSource, Allocator>
	{
		return format<detail::string_type<FormatSource, Allocator>>(allocator_arg, alloc, fmt, args...);
	}
	
	template<class Destination, class FormatSource, class... Args>
	size_t format(in_place_t, Destination& dest, const FormatSource& fmt, const Args&... args);

	//@}
}} // namespace std::experimental

// Must be inclued after defining the std::experimental::to_string() and std::to_string() methods for primitive types not findable by ADL
#include <std-format/detail/dispatch_to_string.hpp>

// Require previous declarations of public names.
#include <std-format/detail/format_parser.hpp>
#include <std-format/detail/immediate_formatter.hpp>
//#include <std-format/detail/formatter.hpp> // Currently disabled until the inline format case is mature enough and has a more or less stable implementation and interface

namespace std { namespace experimental
{
	namespace detail
	{
		using std::begin;
		using std::end;
		
		template<class Appender, class FormatSource, class... Args>
		auto format_impl(Appender&& app, const FormatSource& fmt, const Args&... args)
			-> decltype(fmt(app, args...))
		{
			return fmt(app, args...);
		}
		template<class Appender, class FormatSource, class... Args>
		size_t format_impl(Appender&& app, const FormatSource& fmt, const Args&... args)
		{
			using CharT = detail::char_type<FormatSource>;
			using Traits = detail::traits_type<FormatSource>;
			
			detail::immediate_formatter<CharT, Traits, Args...> formatter{fmt};
			return formatter(app, args...);
		}
	}
	
}} // namespace std::experimental

template<class Result, class FormatSource, class... Args>
auto std::experimental::format(const FormatSource& fmt, const Args&... args) -> Result
{
	using Allocator = detail::allocator_type<FormatSource>;
	return format<Result>(allocator_arg, Allocator{}, fmt, args...);
}

template<class Result, class Allocator, class FormatSource, class... Args>
auto std::experimental::format(allocator_arg_t, const Allocator& alloc, const FormatSource& fmt, const Args&... args) -> Result
{
	Result out{alloc};
	format(in_place, out, fmt, args...);
	return out;
}

template<class Destination, class FormatSource, class... Args>
size_t std::experimental::format(in_place_t, Destination& dest, const FormatSource& fmt, const Args&... args)
{
	return detail::format_impl(make_format_appender(dest), fmt, args...);
}

template<class CharT, class Traits>
void std::experimental::validate_format(basic_string_view<CharT, Traits> fmt, size_t nargs)
{
	// All we do here is let the parser do it's job.
	// If it doesn't throw the format string doesn't violate any syntactic rules and all indices are valid.
	for(auto part : parse_format<CharT, Traits>(fmt, nargs))
		;
}

template<class CharT, class Traits>
bool std::experimental::validate_format(basic_string_view<CharT, Traits> fmt, size_t nargs, nothrow_t) noexcept
{
	try { validate_format(fmt, nargs); }
	catch(...) { return false; }
	return true;
}

#endif // std_format_format_hpp
