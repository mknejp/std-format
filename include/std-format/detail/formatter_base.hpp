//
//  formatter_base.hpp
//  SystemGen
//
//  Created by knejp on 17.1.14.
//  Copyright (c) 2014 Miroslav Knejp. All rights reserved.
//

#ifndef std_format_detail_formatter_base_hpp
#define std_format_detail_formatter_base_hpp

namespace std_format
{
	namespace detail
	{
		template<class FormatSource>
		struct formatter_base_impl;
		
		template<class FormatSource, class Derived>
		class formatter_base;
	}
}

////////////////////////////////////////////////////////////////////////////
// formatter_base_impl

template<class FormatSource>
struct std_format::detail::formatter_base_impl
{
	using format_iterator = typename FormatSource::const_iterator;
	using traits_type = typename FormatSource::traits_type;
	using value_type = typename FormatSource::value_type;
	using format_type = FormatSource;
	
	/// Skip all characters until an unescaped brace is encountered and return its iterator or \p last if none was found.
	format_iterator skip_until_unescaped(format_iterator first, format_iterator last);
	/// Parse the integer starting at the given position and also return the iterator to the first position past the number.
	std::pair<std::size_t, format_iterator> parseIndex(format_iterator first, format_iterator last);
	/// Find the next brace and return its iterator or \p last if none was found.
	format_iterator nextBrace(format_iterator first, format_iterator last);
	
};

template<class FormatSource>
auto std_format::detail::formatter_base_impl<FormatSource>
	::skip_until_unescaped(format_iterator first, format_iterator last) -> format_iterator
{
	using namespace std;
	while(true)
	{
		auto brace = this->nextBrace(first, last);
		if(brace == last)
			return last;
		else if(*next(brace) == *brace)
			first = next(brace, 2);
			else
				return brace;
	}
}

template<class FormatSource>
auto std_format::detail::formatter_base_impl<FormatSource>
::nextBrace(format_iterator first, format_iterator last) -> format_iterator
{
	auto chars = { value_type('{'), value_type('}') };
	return std::find_first_of(first, last, begin(chars), end(chars), traits_type::eq);
}

template<class FormatSource>
auto std_format::detail::formatter_base_impl<FormatSource>
	::parseIndex(format_iterator first, format_iterator last) -> std::pair<std::size_t, format_iterator>
{
	std::size_t i = 0;
	for( ; first != last; ++first)
	{
		auto ch = *first;
		if(value_type('0') > ch || value_type('9') < ch)
			break;
		i = i * 10 + (ch - value_type('0'));
	}
	return { i, first };
}

////////////////////////////////////////////////////////////////////////////
// formatter_base

template<class FormatSource, class Derived>
class std_format::detail::formatter_base : public formatter_base_impl<FormatSource>
{
	using Base = formatter_base_impl<FormatSource>;
	
public:
	using format_iterator = typename Base::format_iterator;
	using traits_type = typename Base::traits_type;
	using value_type = typename Base::value_type;
	using format_type = typename Base::format_type;
	
protected:
	void parse(const format_type& fmt, std::size_t size);
	
private:
	/// Copy all characters to the stream buffer using `Derived::copy_to_output()`, transforming escaped braces until an unescaped brace is encountered and return its iterator or \p last if none was found.
	format_iterator copy_until_unescaped(format_iterator first, format_iterator last);
};

template<class FormatSource, class Derived>
void std_format::detail::formatter_base<FormatSource, Derived>
	::parse(const format_type& fmt, std::size_t size)
{
	using namespace std;
	auto first = fmt.cbegin();
	auto last = fmt.cend();
	auto start = first;
	int n = 0;
	
	for( ; first != last; ++n)
	{
		first = this->copy_until_unescaped(first, last);
		if(first == last)
			break;
		if(value_type('{') != *first)
		{
			throw runtime_error{format("{0}: Invalid nesting of braces in format string.",
									   first - start)};
		}
		auto rbrace = this->skip_until_unescaped(next(first), last);
		if(rbrace == last || value_type('}') != *rbrace)
		{
			throw runtime_error{format("{0}: Missing brace to close parameter #{1} in format string (opening brace at {2}).",
									   rbrace - start, n, first - start)};
		}
		first = next(first);
		auto index = this->parseIndex(first, rbrace);
		if(index.second == first)
		{
			throw runtime_error{format("{0}: Unrecognized index in format string parameter #{1}.",
									   first - start, n)};
		}
		if(index.first >= size)
		{
			throw runtime_error{format("{0}: Index in format string parameter #{1} out of bounds ({2} specified, max allowed is {3}).",
									   first - start, n, index.first, size - 1)};
		}
		if(value_type(':') == *index.second)
			index.second = next(index.second);
		else if(value_type('}') != *index.second)
		{
			throw runtime_error{format("{0}: Invalid character '{1}' after index in format string parameter #{2}.",
									   index.second - start, *index.second, n)};
		}
		first = next(rbrace);
		static_cast<Derived*>(this)->do_format(n, index.first, index.second, rbrace);
	}
}

template<class FormatSource, class Derived>
auto std_format::detail::formatter_base<FormatSource, Derived>
	::copy_until_unescaped(format_iterator first, format_iterator last) -> format_iterator
{
	using namespace std;
	while(true)
	{
		auto brace = this->nextBrace(first, last);
		static_cast<Derived*>(this)->copy_to_output(first, brace);
		if(brace == last)
			return last;
		else
		{
			if(*next(brace) == *brace)
			{
				static_cast<Derived*>(this)->copy_to_output(brace, next(brace));
				first = next(brace, 2);
			}
			else
				return brace;
		}
	}
}

#endif // std_format_detail_formatter_base_hpp
