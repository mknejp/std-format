//
//  format_parser.hpp
//  std-format
//
//  Created by knejp on 17.1.14.
//  Copyright (c) 2014 Miro Knejp. All rights reserved.
//

#ifndef std_format_detail_parser_hpp
#define std_format_detail_parser_hpp

namespace std { namespace experimental
{
	namespace detail
	{
		template<class CharT, class Traits>
		class format_parser;
	}
}} // namespace std::experimental

////////////////////////////////////////////////////////////////////////////
// format_parser

// This could be made a public class if people think it would be useful
template<class CharT, class Traits>
class std::experimental::detail::format_parser
{
public:
	/**
	 Parse the input and invoke the callbacks when necessary.
	 
	 `copyCallback(first, last)` for the substrings which are copied 1:1 to the output
	 `formatCallback(index, argindex, first, last)` for the components to be formatted where
	 - \p index is the running index of the argument,
	 - \p argindex is the positional argument in the range [0; nargs) and
	 - \p first, \p last the format flags substring.
	 */
	template<class FormatIter, class Fn, class Gn>
	void operator() (FormatIter first, FormatIter last, size_t nargs, Fn copyCallback, Gn formatCallback);
	
private:
	/// Copy all characters to the stream buffer using `Derived::copy_to_output()`, transforming escaped braces until an unescaped brace is encountered and return its iterator or \p last if none was found.
	template<class FormatIter, class Fn>
	FormatIter copy_until_unescaped(FormatIter first, FormatIter last, Fn copyCallback);
	/// Skip all characters until an unescaped brace is encountered and return its iterator or \p last if none was found.
	template<class FormatIter>
	FormatIter skip_until_unescaped(FormatIter first, FormatIter last);
	/// Parse the integer starting at the given position and also return the iterator to the first position past the number.
	template<class FormatIter>
	pair<size_t, FormatIter> parseIndex(FormatIter first, FormatIter last);
	/// Find the next brace and return its iterator or \p last if none was found.
	template<class FormatIter>
	FormatIter nextBrace(FormatIter first, FormatIter last);
};

template<class CharT, class Traits>
template<class FormatIter, class Fn, class Gn>
void std::experimental::detail::format_parser<CharT, Traits>
	::operator() (FormatIter first, FormatIter last, size_t nargs, Fn copyCallback, Gn formatCallback)
{
	auto start = first;
	int n = 0;
	
	for( ; first != last; ++n)
	{
		first = copy_until_unescaped(first, last, copyCallback);
		if(first == last)
			break;
		if(CharT('{') != *first)
		{
			throw runtime_error{format("{0}: Invalid nesting of braces in format string.",
									   first - start)};
		}
		auto rbrace = skip_until_unescaped(next(first), last);
		if(rbrace == last || CharT('}') != *rbrace)
		{
			throw runtime_error{format("{0}: Missing brace to close parameter #{1} in format string (opening brace at {2}).",
									   rbrace - start, n, first - start)};
		}
		first = next(first);
		auto index = parseIndex(first, rbrace);
		if(index.second == first)
		{
			throw runtime_error{format("{0}: Unrecognized index in format string parameter #{1}.",
									   first - start, n)};
		}
		if(index.first >= nargs)
		{
			throw runtime_error{format("{0}: Index in format string parameter #{1} out of bounds ({2} specified, max allowed is {3}).",
									   first - start, n, index.first, nargs - 1)};
		}
		if(CharT(':') == *index.second)
			index.second = next(index.second);
		else if(CharT('}') != *index.second)
		{
			throw runtime_error{format("{0}: Invalid character '{1}' after index in format string parameter #{2}.",
									   index.second - start, *index.second, n)};
		}
		first = next(rbrace);
		formatCallback(n, index.first, index.second, rbrace);
	}
}

template<class CharT, class Traits>
template<class FormatIter, class Fn>
auto std::experimental::detail::format_parser<CharT, Traits>
	::copy_until_unescaped(FormatIter first, FormatIter last, Fn copyCallback) -> FormatIter
{
	while(true)
	{
		auto brace = nextBrace(first, last);
		copyCallback(first, brace);
		if(brace == last)
			return last;
		else
		{
			if(*next(brace) == *brace)
			{
				copyCallback(brace, next(brace));
				first = next(brace, 2);
			}
			else
				return brace;
		}
	}
}

template<class CharT, class Traits>
template<class FormatIter>
auto std::experimental::detail::format_parser<CharT, Traits>
	::skip_until_unescaped(FormatIter first, FormatIter last) -> FormatIter
{
	while(true)
	{
		auto brace = nextBrace(first, last);
		if(brace == last)
			return last;
		else if(*next(brace) == *brace)
			first = next(brace, 2);
			else
				return brace;
	}
}

template<class CharT, class Traits>
template<class FormatIter>
auto std::experimental::detail::format_parser<CharT, Traits>
	::nextBrace(FormatIter first, FormatIter last) -> FormatIter
{
	auto chars = { CharT('{'), CharT('}') };
	return find_first_of(first, last, begin(chars), end(chars), Traits::eq);
}

template<class CharT, class Traits>
template<class FormatIter>
auto std::experimental::detail::format_parser<CharT, Traits>
	::parseIndex(FormatIter first, FormatIter last) -> pair<size_t, FormatIter>
{
	size_t i = 0;
	for( ; first != last; ++first)
	{
		auto ch = *first;
		if(CharT('0') > ch || CharT('9') < ch)
			break;
		i = i * 10 + (ch - CharT('0'));
	}
	return { i, first };
}

#endif // std_format_detail_parser_hpp
