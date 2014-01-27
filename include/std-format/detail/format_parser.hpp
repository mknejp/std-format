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
	 
	 \p tempBuffer points to a string which can be used for temporary storage where necessary to reduce the number of reallocations. If the \p options_in_temp argument to \p formatCallback is true then the \p offset and \p length parameters are relative to \p tempBuffer, not the input range! The buffer may be invalidated and rewritten as soon as the callback returns. If the data needs to be persisted it must be saved out of the temporary stirage.
	 
	 \p formatCallback can rely on the following preconditions:
	 - \p arg is the positional index read from the format argument and is guaranteed to be in the range `[0, nargs)`
	 */
	template<class FormatIter, class Fn, class Gn>
	void operator() (FormatIter first, FormatIter last, size_t nargs, Fn copyCallback, Gn formatCallback);
	
private:
	static constexpr bool is_bidirectional(bidirectional_iterator_tag) { return true; }
	static constexpr bool is_bidirectional(...) { return false; }
	
	/// Copy all characters to the stream buffer using `Derived::copy_to_output()`, transforming escaped braces until an unescaped brace is encountered and return its iterator or \p last if none was found.
	template<class FormatIter, class Fn>
	FormatIter copy_until_unescaped(FormatIter first, FormatIter last, Fn copyCallback);
	/// Skip all characters until an unescaped brace is encountered and return its iterator or \p last if none was found.
	template<class FormatIter>
	FormatIter skip_until_unescaped(FormatIter first, FormatIter last);
	/// Parse the unsigned integer starting at the given position and also return the iterator to the first position past the number.
	template<class FormatIter>
	pair<unsigned int, FormatIter> parseIndex(FormatIter first, FormatIter last, FormatIter start, size_t n);
	/// Parse the signed(!) integer starting at the given position and also return the iterator to the first position past the number.
	template<class FormatIter>
	pair<int, FormatIter> parseWidth(FormatIter first, FormatIter last, FormatIter start, size_t n);
	/// Find the next brace and return its iterator or \p last if none was found.
	template<class FormatIter>
	FormatIter nextBrace(FormatIter first, FormatIter last);
	/// Throw if we unexpectedly reached the end
	template<class FormatIter>
	void throwIfEof(FormatIter pos, FormatIter end, FormatIter lbrace, FormatIter start, size_t n);
};

template<class CharT, class Traits>
template<class FormatIter, class Fn, class Gn>
void std::experimental::detail::format_parser<CharT, Traits>
	::operator() (FormatIter first, FormatIter last, size_t nargs, Fn copyCallback, Gn formatCallback)
{
	static_assert(is_bidirectional(typename iterator_traits<FormatIter>::iterator_category()), "format_parser requires bidirectional iterators");
	
	basic_string<CharT, Traits> temp; // This buffer is used for all temporaries we need, thus hopefully minimizing the number of reallocations
	auto start = first;
	int n = 0;
	
	for( ; first != last; ++n)
	{
		auto lbrace = copy_until_unescaped(first, last, copyCallback);
		if(lbrace == last)
			break;
		if(CharT('{') != *lbrace)
			throw runtime_error{format("{0}: Invalid nesting of braces in format string.", lbrace - start)};
		auto pos = ++lbrace;
		auto rbrace = skip_until_unescaped(pos, last);
		throwIfEof(rbrace, last, lbrace, start, n);
		unsigned int index;
		std::tie(index, pos) = parseIndex(pos, rbrace, start, n);
		if(index >= nargs)
		{
			throw runtime_error{format("{0}: Index in format string parameter #{1} out of bounds ({2} specified, max allowed is {3}).",
									   first - start, n, index, nargs - 1)};
		}
		int width = 0;
		if(CharT(',') == *pos)
		{
			++pos;
			std::tie(width, pos) = parseWidth(pos, rbrace, start, n);
			
		}
		if(CharT(':') == *pos)
			++pos;
		else if(CharT('}') != *pos)
		{
			// If index/align is not followed by a colon it must be closed immediately
			throw runtime_error{format("{0}: Unexpected character '{1}' after index/alignment in format string parameter #{2}.",
									   pos - start, *pos, n)};
		}

		auto next_brace = nextBrace(pos, rbrace);
		if(next_brace != rbrace)
		{
			// The format options contain escaped braces.
			// We need to skip over all those and assemble the escaped string in our temporary buffer before passing to formatCallback
			temp.clear();
			copy_until_unescaped(pos, rbrace, [&] (basic_string_view<CharT, Traits> s) { temp.append(s.begin(), s.end()); });
			formatCallback(n, index, width, { temp.data(), temp.size() });
		}
		else
			formatCallback(n, index, width, { pos, rbrace - pos });
		first = ++rbrace;
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
		copyCallback(basic_string_view<CharT, Traits>{ first, brace - first });
		if(brace == last)
			return last;
		else
		{
			if(*(brace + 1) == *brace)
			{
				copyCallback(basic_string_view<CharT, Traits>{ brace, 1 });
				first = brace + 2;
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
		else if(*(brace + 1) == *brace)
			first = brace + 2;
			else
				return brace;
	}
}

template<class CharT, class Traits>
template<class FormatIter>
auto std::experimental::detail::format_parser<CharT, Traits>
	::parseIndex(FormatIter first, FormatIter last, FormatIter start, size_t n) -> pair<unsigned int, FormatIter>
{
	if(first == last || CharT('0') > *first || CharT('9') < *first)
		throw runtime_error{format("{0}: Unexpected character '{1}' in format string parameter #{2} (index expected).", first - start, *first, n)};

	unsigned int i = 0;
	for( ; first != last; ++first)
	{
		auto ch = *first;
		if(CharT('0') > ch || CharT('9') < ch)
			break;
		i = i * 10 + (ch - CharT('0'));
	}
	return { i, first };
}

template<class CharT, class Traits>
template<class FormatIter>
auto std::experimental::detail::format_parser<CharT, Traits>
	::parseWidth(FormatIter first, FormatIter last, FormatIter start, size_t n) -> pair<int, FormatIter>
{
	if(first == last)
		throw runtime_error{format("{0}: Unexpected character '{1}' in format string parameter #{2} (alignment expected).", first - start, *first, n)};
	
	bool neg = false;
	if(CharT('-') == *first)
	{
		neg = true;
		++first;
	}
	if(first == last || CharT('0') > *first || CharT('9') < *first)
		throw runtime_error{format("{0}: Unexpected character '{1}' in format string parameter #{2} (alignment expected).", first - start, *first, n)};
	
	int i = 0;
	for( ; first != last; ++first)
	{
		auto ch = *first;
		if(CharT('0') > ch || CharT('9') < ch)
			break;
		i = i * 10 + (ch - CharT('0'));
	}
	return { neg ? -i : i, first };
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
void std::experimental::detail::format_parser<CharT, Traits>
	::throwIfEof(FormatIter pos, FormatIter end, FormatIter lbrace, FormatIter start, size_t n)
{
	if(pos == end)
		throw runtime_error{format("{0}: Reached unexpected end of format string while parsing format parameter #{1} (opening brace at {2})",
								   pos - start, n, lbrace - start)};
}


#endif // std_format_detail_parser_hpp
