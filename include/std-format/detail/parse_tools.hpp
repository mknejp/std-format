//
//  parse_tools.hpp
//  std-format
//
//  Created by knejp on 24.1.14.
//  Copyright (c) 2014 Miro Knejp. All rights reserved.
//

#ifndef std_format_parse_tools_hpp
#define std_format_parse_tools_hpp

#include <cassert>
#include <limits>
#include <stdexcept>
#include <std-format/optional.hpp>

// All the C/C++ methods operate on null-terminated strings, which is useless for most of what we do

namespace std { namespace experimental
{
	template<class Int, class Iter, class CharT = typename Iter::value_type, class Traits = char_traits<CharT>>
	pair<optional<Int>, Iter> parse_integer(Iter first, Iter last, int radix = 10);
	
	template<class Int, class Iter, class CharT = typename Iter::value_type, class Traits = char_traits<CharT>>
	pair<optional<Int>, Iter> parse_prefixed_integer(Iter first, Iter last, int radix = 10);
	
	template<class Iter, class CharT = typename Iter::value_type, class Traits = char_traits<CharT>>
	pair<int, Iter> parse_radix_prefix(Iter first, Iter last);
	
	template<class Int, class CharT, class Traits = char_traits<CharT>>
	optional<Int> parse_digit(CharT ch, int radix);
	
	template<class Int, class CharT, class Traits = char_traits<CharT>>
	optional<Int> parse_digit10(CharT ch);
	
	template<class Int, class CharT, class Traits = char_traits<CharT>>
	optional<Int> parse_digit16(CharT ch);
	
	template<class Int, class CharT, class Traits>
	optional<Int> parse_integer(basic_string_view<CharT, Traits> s, int radix = 10)
	{ return parse_integer<Int, decltype(s.begin()), CharT, Traits>(s.begin(), s.end(), radix).first; }
	
	template<class Int, class CharT, class Traits>
	optional<Int> parse_prefixed_integer(basic_string_view<CharT, Traits> s, int radix = 10)
	{ return parse_prefixed_integer<Int, decltype(s.begin()), CharT, Traits>(s.begin(), s.end(), radix).first; }
	
	template<class CharT, class Traits>
	int parse_radix_prefix(basic_string_view<CharT, Traits> s)
	{ return parse_radix_prefix<decltype(s.begin()), CharT, Traits>(s.begin(), s.end()).first; }
	
	namespace detail
	{
		template<class Int, class CharT, class Traits, class Iter>
		pair<optional<Int>, Iter> parse_integer(Iter first, Iter last, int radix, true_type /*signed*/);
		template<class Int, class CharT, class Traits, class Iter>
		pair<optional<Int>, Iter> parse_integer(Iter first, Iter last, int radix, false_type /*signed*/);
		template<class Int, class CharT, class Traits, class Iter>
		pair<optional<Int>, Iter> parse_raw_integer(Iter first, Iter last, int radix);
		template<class Int, class CharT, class Traits, class Iter>
		pair<optional<Int>, Iter> parse_prefixed_integer(Iter first, Iter last, int radix, true_type /*signed*/);
		template<class Int, class CharT, class Traits, class Iter>
		pair<optional<Int>, Iter> parse_prefixed_integer(Iter first, Iter last, int radix, false_type /*signed*/);
	}
	
}} // namespace std::experimental

template<class Int, class Iter, class CharT, class Traits>
auto std::experimental::parse_integer(Iter first, Iter last, int radix) -> pair<optional<Int>, Iter>
{
	static_assert(is_integral<Int>::value, "only integral types supported");
	assert(radix >= 2 && radix <= 36 && "invalid radix");
	return detail::parse_integer<Int, CharT, Traits>(first, last, radix, is_signed<Int>());
}

template<class Int, class Iter, class CharT, class Traits>
auto std::experimental::parse_prefixed_integer(Iter first, Iter last, int radix) -> pair<optional<Int>, Iter>
{
	static_assert(is_integral<Int>::value, "only integral types supported");
	assert(radix >= 2 && radix <= 36 && "invalid radix");
	
	return detail::parse_prefixed_integer<Int, CharT, Traits>(first, last, radix, is_signed<Int>());
}

template<class Iter, class CharT, class Traits>
auto std::experimental::parse_radix_prefix(Iter first, Iter last) -> pair<int, Iter>
{
	if(first == last || !Traits::eq(*first, CharT('0')))
		return { 0, first };
	
	++first;
	if(first != last)
	{
		if(Traits::eq(*first, CharT('x')) || Traits::eq(*first, CharT('X')))
			return { 16, ++first, };
		
		if(Traits::eq(*first, CharT('b')) || Traits::eq(*first, CharT('B')))
			return { 2, ++first, };
	}
	return { 8, first };
}

template<class Int, class CharT, class Traits>
auto std::experimental::parse_digit10(CharT ch) -> optional<Int>
{
	if(Traits::lt(ch, CharT('0')) || Traits::lt(CharT('9'), ch))
	   return nullopt;
	else
	   return Int(ch - CharT('0'));
}

template<class Int, class CharT, class Traits>
auto std::experimental::parse_digit16(CharT ch) -> optional<Int>
{
	if(Traits::lt(ch, CharT('0')) || Traits::lt(CharT('9'), ch))
	{
		if(Traits::lt(ch, CharT('a')) || Traits::lt(CharT('f'), ch))
		{
			if(Traits::lt(ch, CharT('A')) || Traits::lt(CharT('F'), ch))
				return nullopt;
			else
				return Int(ch - CharT('A') + 10);
		}
		else
			return Int(ch - CharT('a') + 10);
	}
	else
		return Int(ch - CharT('0'));
}

template<class Int, class CharT, class Traits>
auto std::experimental::parse_digit(CharT ch, int radix) -> optional<Int>
{
	assert(radix >= 2 && radix <= 36 && "Unsupported radix");
	if(Traits::lt(ch, CharT('0')) || Traits::lt(CharT('0') + radix - 1, ch))
	{
		if(radix > 10)
		{
			if(Traits::lt(ch, CharT('a')) || Traits::lt(CharT('a') + radix - 11, ch))
			{
				if(Traits::lt(ch, CharT('A')) || Traits::lt(CharT('A') + radix - 11, ch))
					return nullopt;
				else
					return Int(ch - CharT('A') + 10);
			}
			else
				return Int(ch - CharT('a') + 10);
		}
		else
			return nullopt;
	}
	else
		return Int(ch - CharT('0'));
}

template<class Int, class CharT, class Traits, class Iter>
auto std::experimental::detail::parse_integer(Iter first, Iter last, int radix, true_type /*signed*/) -> pair<optional<Int>, Iter>
{
	if(first == last)
		return { nullopt, first };
	
	bool neg = false;
	if(Traits::eq(*first, CharT('-')))
	{
		neg = true;
		++first;
	}
	else
		if(Traits::eq(*first, CharT('+')))
		++first;
	
	using UInt = make_unsigned_t<Int>;
	auto i = parse_raw_integer<UInt, CharT, Traits>(first, last, radix);
	if(i.first)
	{
		if(neg)
		{
			return { *i.first > -static_cast<UInt>(numeric_limits<Int>::min())
				? optional<Int>(nullopt) : optional<Int>(-static_cast<Int>(*i.first)), i.second };
		}
		else
		{
			return { *i.first > numeric_limits<Int>::max()
				? optional<Int>(nullopt) : optional<Int>(static_cast<Int>(*i.first)), i.second };
		}
	}
	else
		return { nullopt, i.second };
}

template<class Int, class CharT, class Traits, class Iter>
auto std::experimental::detail::parse_integer(Iter first, Iter last, int radix, false_type /*signed*/) -> pair<optional<Int>, Iter>
{
	if(first == last)
		return { nullopt, first };
	
	if(Traits::eq(*first, CharT('+')))
		++first;
	
	return parse_raw_integer<Int, CharT, Traits>(first, last, radix);
}

template<class Int, class CharT, class Traits, class Iter>
auto std::experimental::detail::parse_raw_integer(Iter first, Iter last, int radix) -> pair<optional<Int>, Iter>
{
	static_assert(is_unsigned<Int>::value, "something went horribly wrong");

	if(first == last)
		return { nullopt, first };
	
	auto i = parse_digit<Int, CharT, Traits>(*first, radix);
	if(!i)
		return { nullopt, first };
	++first;
	
	bool overflow = false; // Consume the entire range of valid characters, even on overflow
	while(first != last)
	{
		auto digit = parse_digit<Int, CharT, Traits>(*first, radix);
		if(!digit)
			break;
		++first;
		auto k = *i * radix + *digit;
		overflow |= k < *i;
		i = k;
	}
	return { overflow || !i ? nullopt : i, first };
}

template<class Int, class CharT, class Traits, class Iter>
auto std::experimental::detail::parse_prefixed_integer(Iter first, Iter last, int radix, true_type /*signed*/) -> pair<optional<Int>, Iter>
{
	if(first == last)
		return { nullopt, first };
	
	bool neg = false;
	if(Traits::eq(*first, CharT('-')))
	{
		neg = true;
		++first;
	}
	else if(Traits::eq(*first, CharT('+')))
		++first;
	
	auto base = parse_radix_prefix<Iter, CharT, Traits>(first, last);
	if(base.first != 0)
		radix = base.first;
	first = base.second;
	
	using UInt = make_unsigned_t<Int>;
	auto i = parse_raw_integer<UInt, CharT, Traits>(first, last, radix);
	if(i.first)
	{
		if(neg)
		{
			return { *i.first > -static_cast<UInt>(numeric_limits<Int>::min())
				? optional<Int>(nullopt) : optional<Int>(-static_cast<Int>(*i.first)), i.second };
		}
		else
		{
			return { *i.first > numeric_limits<Int>::max()
				? optional<Int>(nullopt) : optional<Int>(static_cast<Int>(*i.first)), i.second };
		}
	}
	else
		return { nullopt, i.second };
}

template<class Int, class CharT, class Traits, class Iter>
auto std::experimental::detail::parse_prefixed_integer(Iter first, Iter last, int radix, false_type /*signed*/) -> pair<optional<Int>, Iter>
{
	if(first == last)
		return { nullopt, first };
	
	if(Traits::eq(*first, CharT('+')))
		++first;
	
	auto base = parse_radix_prefix<Iter, CharT, Traits>(first, last);
	if(base.first != 0)
		radix = base.first;
	first = base.second;

	return parse_raw_integer<Int, CharT, Traits>(first, last, radix);
}

#endif
