//
//  formatter.hpp
//  std-format
//
//  Created by knejp on 17.1.14.
//  Copyright (c) 2014 Miro Knejp. All rights reserved.
//

#ifndef std_format_detail_formatter_hpp
#define std_format_detail_formatter_hpp

namespace std { namespace experimental
{
	namespace detail
	{
		template<class FormatSource, class Output, class... Args>
		class preformatter;
	}
}} // namespace std::experimental

////////////////////////////////////////////////////////////////////////////
// preformatter

template<class FormatSource, class Output, class... Args>
class std::experimental::detail::preformatter
	: public formatter_base<FormatSource, preformatter<FormatSource, Output, Args...>>
{
	using Base = formatter_base<FormatSource, preformatter<FormatSource, Output, Args...>>;
		
public:
	using format_iterator = typename Base::format_iterator;
	using traits_type = typename Base::traits_type;
	using value_type = typename Base::value_type;
	using format_type = typename Base::format_type;
	
private:
	using tuple_type = tuple<const typename remove_reference<Args>::type&...>;
	using flags_type = basic_string_view<value_type, traits_type>;
	using string_type = basic_string<value_type, traits_type>;
	using formatters_type = vector<function<void(tuple_type&, const string_type&, Output&)>>;
	
	preformatter(const FormatSource& fmt) : _fmt(fmt)
	{
		this->parse(fmt, sizeof...(Args));
	}
	
	formatters_type& formatters() { return _formatters; }

private:
	friend Base;
	friend class formatter<FormatSource, Args...>;
	
	// CRTP callbacks from formatter_base
	void copy_to_output(format_iterator first, format_iterator last);
	void do_format(size_t n, size_t index, format_iterator options_first, format_iterator options_last);
	
	const FormatSource& _fmt;
	formatters_type _formatters;
	format_delegates<tuple_type, Output, flags_type, make_index_sequence<sizeof...(Args)>> _delegates;
};

template<class FormatSource, class Output, class... Args>
void std::experimental::detail::preformatter<FormatSource, Output, Args...>
	::copy_to_output(format_iterator first, format_iterator last)
{
	if(first == last)
		return;
		
	auto start = distance(_fmt.begin(), first);
	auto size = distance(first, last);
	
	_formatters.emplace_back([=] (tuple_type& args, const string_type& fmt, Output& out)
	{
		if(out.sputn(fmt.data() + start, size) != size)
			throw ios_base::failure{"Error writing to output streambuf."};
	});
}

template<class FormatSource, class Output, class... Args>
void std::experimental::detail::preformatter<FormatSource, Output, Args...>
	::do_format(size_t n, size_t index, format_iterator options_first, format_iterator options_last)
{
	auto start = distance(_fmt.begin(), options_first);
	auto size = distance(options_first, options_last);
	auto f = _delegates.value[index];
	
	// TODO: check for custom type preformatters
	_formatters.emplace_back([=] (tuple_type& args, const string_type& fmt, Output& out)
	{
		auto flags = flags_type{ fmt.data() + start, size };
		f(args, out, flags);
	});
}

////////////////////////////////////////////////////////////////////////////
// formatter

template<class FormatSource, class... Args>
class std::experimental::formatter
{
public:
	using traits_type = typename FormatSource::traits_type;
	using value_type = typename FormatSource::value_type;
	using streambuf_type = basic_streambuf<value_type, traits_type>;
	
private:
	using preformatter = detail::preformatter<FormatSource, streambuf_type, Args...>;
	
public:
	using flags_type = typename preformatter::flags_type;
	using format_type = FormatSource;
	using format_iterator = typename FormatSource::const_iterator;
	using result_type = basic_string<value_type, traits_type>;
	using size_type = size_t;

	explicit formatter(format_type fmt)
	{
		rebuild(move(fmt));
	}
	
	formatter(const formatter&) = default;
	formatter(formatter&) = default;
	
	~formatter() = default;
	
	static constexpr size_type size() noexcept { return sizeof...(Args); }
	
	result_type operator() (const typename remove_reference<Args>::type&... args) const;
	void operator() (streambuf_type& buf, const typename remove_reference<Args>::type&... args) const;
	auto operator() (basic_ostream<value_type, traits_type>& os, const typename remove_reference<Args>::type&... args) const -> decltype(os);
	template<class Allocator>
	void operator() (basic_string<value_type, traits_type, Allocator>& str, const typename remove_reference<Args>::type&... args) const;
	
private:
	using tuple_type = typename preformatter::tuple_type;
	using string_type = typename preformatter::string_type;
	using formatters_type = typename preformatter::formatters_type;

	void rebuild(format_type fmt);
	
	format_type _fmt;
	formatters_type _formatters;
};

template<class FormatSource, class... Args>
auto std::experimental::formatter<FormatSource, Args...>::operator() (const typename remove_reference<Args>::type&... args) const
	-> result_type
{
	basic_stringbuf<value_type, traits_type> buf;
	this->operator()(buf, args...);
	return buf.str();
}

template<class FormatSource, class... Args>
auto std::experimental::formatter<FormatSource, Args...>
	::operator() (basic_ostream<value_type, traits_type>& os, const typename remove_reference<Args>::type&... args) const
	-> decltype(os)
{
	this->operator()(*os.rdbuf(), args...);
	return os;
}

template<class FormatSource, class... Args>
template<class Allocator>
void std::experimental::formatter<FormatSource, Args...>
	::operator() (basic_string<value_type, traits_type, Allocator>& str, const typename remove_reference<Args>::type&... args) const
{
	basic_stringbuf<value_type, traits_type> buf;
	this->operator()(buf, args...);
	str += buf.str();
}

template<class FormatSource, class... Args>
void std::experimental::formatter<FormatSource, Args...>::operator() (streambuf_type& buf, const typename remove_reference<Args>::type&... args) const
{
	auto values = tie(args...);
	for(const auto& f : _formatters)
		f(values, _fmt, buf);
}

template<class FormatSource, class... Args>
void std::experimental::formatter<FormatSource, Args...>::rebuild(format_type fmt)
{
	preformatter pre{fmt};
	swap(_fmt, fmt);
	swap(_formatters, pre.formatters());
}

#endif // std_format_detail_formatter_hpp
