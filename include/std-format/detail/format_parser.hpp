//
//  format_parser.hpp
//  std-format
//
//  Created by knejp on 17.1.14.
//  Copyright (c) 2014 Miro Knejp. All rights reserved.
//

#ifndef std_format_detail_parser_hpp
#define std_format_detail_parser_hpp

#include <std-format/detail/parse_tools.hpp>

namespace std { namespace experimental
{
	template<class CharT, class Traits, class FormatIter>
	class format_parser;
	
	enum class format_component_type
	{
		static_substring,
		format_argument,
	};
	template<class CharT, class Traits>
	struct format_component
	{
		format_component_type type;
		basic_string_view<CharT, Traits> substring;
		int counter;
		size_t index;
		int width;
	};

	template<class CharT, class Traits, class FormatIter>
	format_parser<CharT, Traits, FormatIter> parse_format(FormatIter first, FormatIter last, size_t nargs);
	
	template<class CharT, class Traits>
	auto parse_format(basic_string_view<CharT, Traits> fmt, size_t nargs)
		-> decltype(parse_format<CharT, Traits>(fmt.begin(), fmt.end(), nargs))
	{
		return parse_format<CharT, Traits>(fmt.begin(), fmt.end(), nargs);
	}

}} // namespace std::experimental

////////////////////////////////////////////////////////////////////////////
// format_parser

// This could be made a public class if people think it would be useful
template<class CharT, class Traits, class FormatIter>
class std::experimental::format_parser
{
public:
	class iterator;
	using const_iterator = iterator;
	
	format_parser(FormatIter first, FormatIter last, size_t nargs) : _first(first), _last(last), _nargs(nargs) { }
	
	iterator begin();
	iterator end();
	
private:
	using component = format_component<CharT, Traits>;
	
	static constexpr bool is_random_access(bidirectional_iterator_tag) { return true; }
	static constexpr bool is_random_access(...) { return false; }
	
	static_assert(is_random_access(typename iterator_traits<FormatIter>::iterator_category()), "format_parser requires random access iterators");
	
	// Extract the next substring out of the format source
	pair<component, FormatIter> parse_next(FormatIter iter, int n);
	// Process a format argument
	pair<component, FormatIter> parse_argument(FormatIter lbrace, int n);
	// Mnemonic for creating a static substring object
	pair<component, FormatIter> static_substring(FormatIter first, FormatIter last, FormatIter next, int n);
	// Skip all characters until an unescaped brace is encountered and return its iterator or \p last if none was found.
	FormatIter skip_until_unescaped(FormatIter first, FormatIter last);
	/// Parse the unsigned integer starting at the given position and also return the iterator to the first position past the number.
	pair<size_t, FormatIter> parseIndex(FormatIter first, FormatIter last, FormatIter start, size_t n);
	// Parse the signed(!) integer starting at the given position and also return the iterator to the first position past the number.
	pair<int, FormatIter> parseWidth(FormatIter first, FormatIter last, FormatIter start, size_t n);
	// Find the next brace and return its iterator or \p last if none was found.
	FormatIter nextBrace(FormatIter first, FormatIter last);

	bool is_escaped(FormatIter brace);

	FormatIter _first;
	FormatIter _last;
	size_t _nargs;
	basic_string<CharT, Traits> _temp; // This buffer is used for all temporaries we need, thus hopefully minimizing the number of reallocations
};

template<class CharT, class Traits, class FormatIter>
auto std::experimental::parse_format(FormatIter first, FormatIter last, size_t nargs)
	-> format_parser<CharT, Traits, FormatIter>
{
	return { first, last, nargs };
}

template<class CharT, class Traits, class FormatIter>
class std::experimental::format_parser<CharT, Traits, FormatIter>::iterator
	: ::std::iterator<forward_iterator_tag, component, void, const component*, const component&>
{
public:
	const component& operator* () const { return _value; }
	const component* operator-> () const { return &_value; }
	
	iterator& operator++ ()
	{
		auto substring = _parser->parse_next(_next, _value.counter);
		_current = _next;
		_next = substring.second;
		_value = substring.first;
		return *this;
	}
	iterator operator++ (int) { auto temp = *this; ++*this; return temp; }
	
	friend bool operator== (const iterator& a, const iterator& b) { return a._current == b._current; }
	friend bool operator!= (const iterator& a, const iterator& b) { return a._current != b._current; }
	
private:
	friend class format_parser;
	
	iterator(FormatIter next, format_parser* parser) : _current(next), _next(next), _parser(parser) { }
	iterator(FormatIter current, FormatIter next, format_parser* parser, component value)
		: _current(current), _next(next), _parser(parser), _value(value) { }

	FormatIter _current;
	FormatIter _next;
	format_parser* _parser;
	component _value;
};

template<class CharT, class Traits, class FormatIter>
auto std::experimental::format_parser<CharT, Traits, FormatIter>
	::begin() -> iterator
{
	auto substring = parse_next(_first, -1);
	return { _first, substring.second, this, substring.first, };
}

template<class CharT, class Traits, class FormatIter>
auto std::experimental::format_parser<CharT, Traits, FormatIter>
	::end() -> iterator
{
	return { _last, this };
}


template<class CharT, class Traits, class FormatIter>
auto std::experimental::format_parser<CharT, Traits, FormatIter>::parse_next(FormatIter iter, int n)
	-> pair<component, FormatIter>
{
	if(iter == _last)
		return static_substring(_last, _last, _last, n);
	
	auto lbrace = nextBrace(iter, _last);
	if(lbrace == _last)
		return static_substring(iter, _last, _last, n);
	else if(lbrace == iter) // When detecting a format argument we always place the next iterator at the brace character
	{
		if(!is_escaped(lbrace))
			return parse_argument(lbrace, ++n);
	}
	
	if(is_escaped(lbrace))
		// Return the substring including the brace character but set the iterator to the character following it
		return static_substring(iter, lbrace + 1, lbrace + 2, n);
	else
		// Make the next iterator point to the brace itself to signal an argument in the next iteration
		return static_substring(iter, lbrace, lbrace, n);
}

template<class CharT, class Traits, class FormatIter>
auto std::experimental::format_parser<CharT, Traits, FormatIter>::parse_argument(FormatIter lbrace, int n)
	-> pair<component, FormatIter>
{
	if(CharT('{') != *lbrace)
		throw runtime_error{format("{0}: Invalid nesting of braces in format string.", lbrace - _first)};

	auto pos = ++lbrace;
	auto rbrace = skip_until_unescaped(pos, _last);
	if(pos == _last)
	{
		throw runtime_error{format("{0}: Reached unexpected end of format string while parsing format argument #{1} (opening brace at {2})",
								   pos - _first, n, lbrace - _first)};
	}
	unsigned index;
	std::tie(index, pos) = parseIndex(pos, rbrace, _first, n);
	if(index >= _nargs)
	{
		throw runtime_error{format("{0}: Index in format argument #{1} out of bounds ({2} specified, max allowed is {3}).",
								   pos - _first, n, index, _nargs - 1)};
	}
	int width = 0;
	if(CharT(',') == *pos)
	{
		++pos;
		std::tie(width, pos) = parseWidth(pos, rbrace, _first, n);
		
	}
	if(CharT(':') == *pos)
		++pos;
	else if(CharT('}') != *pos)
	{
		// If index/align is not followed by a colon it must be closed immediately
		throw runtime_error{format("{0}: Unexpected character '{1}' after index/alignment in format argument #{2}.",
								   pos - _first, *pos, n)};
	}
	auto next_brace = nextBrace(pos, rbrace);
	if(next_brace != rbrace)
	{
		// The format options contain escaped braces.
		// We need to assemble the escaped string in our temporary buffer and return that
		_temp.clear();
		for(auto part : format_parser{pos, rbrace, 0})
			_temp.append(part.substring.data(), part.substring.size());
		return { { format_component_type::format_argument, _temp, n, index, width }, ++rbrace };
	}
	else
		return { { format_component_type::format_argument, { pos, rbrace }, n, index, width }, ++rbrace };
}

template<class CharT, class Traits, class FormatIter>
auto std::experimental::format_parser<CharT, Traits, FormatIter>
	::static_substring(FormatIter first, FormatIter last, FormatIter next, int n) -> pair<component, FormatIter>
{
	return { { format_component_type::static_substring, { first, last }, n, 0, 0 }, next };
}

template<class CharT, class Traits, class FormatIter>
auto std::experimental::format_parser<CharT, Traits, FormatIter>
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

template<class CharT, class Traits, class FormatIter>
bool std::experimental::format_parser<CharT, Traits, FormatIter>::is_escaped(FormatIter brace)
{
	assert(brace != _last);
	return (brace + 1) != _last && *(brace + 1) == *brace;
}

template<class CharT, class Traits, class FormatIter>
auto std::experimental::format_parser<CharT, Traits, FormatIter>
	::parseIndex(FormatIter first, FormatIter last, FormatIter start, size_t n) -> pair<size_t, FormatIter>
{
	auto i = parse_integer<size_t, FormatIter, CharT, Traits>(first, last, 10);
	if(!i.first)
		throw runtime_error{format("{0}: Invalid index in format string parameter #{1}.", first - start, n)};
	return { *i.first, i.second };
}

template<class CharT, class Traits, class FormatIter>
auto std::experimental::format_parser<CharT, Traits, FormatIter>
	::parseWidth(FormatIter first, FormatIter last, FormatIter start, size_t n) -> pair<int, FormatIter>
{
	auto i = parse_integer<int, FormatIter, CharT, Traits>(first, last, 10);
	if(!i.first)
		throw runtime_error{format("{0}: Invalid width in format string parameter #{1}.", first - start, n)};
	return { *i.first, i.second };
}

template<class CharT, class Traits, class FormatIter>
auto std::experimental::format_parser<CharT, Traits, FormatIter>
	::nextBrace(FormatIter first, FormatIter last) -> FormatIter
{
	auto chars = { CharT('{'), CharT('}') };
	return find_first_of(first, last, chars.begin(), chars.end(), Traits::eq);
}

#endif // std_format_detail_parser_hpp
