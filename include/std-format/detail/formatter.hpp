//
//  formatter.hpp
//  SystemGen
//
//  Created by knejp on 17.1.14.
//  Copyright (c) 2014 Miroslav Knejp. All rights reserved.
//

#ifndef std_format_detail_formatter_hpp
#define std_format_detail_formatter_hpp

namespace std_format
{
	namespace detail
	{
		template<class FormatSource, class Output, class... Args>
		class preformatter;
	}
}

////////////////////////////////////////////////////////////////////////////
// preformatter

template<class FormatSource, class Output, class... Args>
class std_format::detail::preformatter
	: public formatter_base<FormatSource, preformatter<FormatSource, Output, Args...>>
{
	using Base = formatter_base<FormatSource, preformatter<FormatSource, Output, Args...>>;
		
public:
	using format_iterator = typename Base::format_iterator;
	using traits_type = typename Base::traits_type;
	using value_type = typename Base::value_type;
	using format_type = typename Base::format_type;
	
private:
	using tuple_type = std::tuple<const typename std::remove_reference<Args>::type&...>;
	using flags_type = basic_string_view<value_type, traits_type>;
	using string_type = std::basic_string<value_type, traits_type>;
	using formatters_type = std::vector<std::function<void(tuple_type&, const string_type&, Output&)>>;
	
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
	void do_format(std::size_t n, std::size_t index, format_iterator options_first, format_iterator options_last);
	
	const FormatSource& _fmt;
	formatters_type _formatters;
	format_delegates<tuple_type, Output, flags_type, make_index_sequence<sizeof...(Args)>> _delegates;
};

template<class FormatSource, class Output, class... Args>
void std_format::detail::preformatter<FormatSource, Output, Args...>
	::copy_to_output(format_iterator first, format_iterator last)
{
	if(first == last)
		return;
		
	auto start = std::distance(_fmt.begin(), first);
	auto size = std::distance(first, last);
	
	_formatters.emplace_back([=] (tuple_type& args, const string_type& fmt, Output& out)
	{
		if(out.sputn(fmt.data() + start, size) != size)
			throw std::ios_base::failure{"Error writing to output streambuf."};
	});
}

template<class FormatSource, class Output, class... Args>
void std_format::detail::preformatter<FormatSource, Output, Args...>
	::do_format(std::size_t n, std::size_t index, format_iterator options_first, format_iterator options_last)
{
	auto start = std::distance(_fmt.begin(), options_first);
	auto size = std::distance(options_first, options_last);
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
class std_format::formatter
{
public:
	using traits_type = typename FormatSource::traits_type;
	using value_type = typename FormatSource::value_type;
	using streambuf_type = std::basic_streambuf<value_type, traits_type>;
	
private:
	using preformatter = detail::preformatter<FormatSource, streambuf_type, Args...>;
	
public:
	using flags_type = typename preformatter::flags_type;
	using format_type = FormatSource;
	using format_iterator = typename FormatSource::const_iterator;
	using result_type = std::basic_string<value_type, traits_type>;
	using size_type = std::size_t;

	explicit formatter(format_type fmt)
	{
		rebuild(std::move(fmt));
	}
	
	formatter(const formatter&) = default;
	formatter(formatter&) = default;
	
	~formatter() = default;
	
	static constexpr size_type size() noexcept { return sizeof...(Args); }
	
	result_type operator() (const typename std::remove_reference<Args>::type&... args) const;
	void operator() (streambuf_type& buf, const typename std::remove_reference<Args>::type&... args) const;
	auto operator() (std::basic_ostream<value_type, traits_type>& os, const typename std::remove_reference<Args>::type&... args) const -> decltype(os);
	template<class Allocator>
	void operator() (std::basic_string<value_type, traits_type, Allocator>& str, const typename std::remove_reference<Args>::type&... args) const;
	
private:
	using tuple_type = typename preformatter::tuple_type;
	using string_type = typename preformatter::string_type;
	using formatters_type = typename preformatter::formatters_type;

	void rebuild(format_type fmt);
	
	format_type _fmt;
	formatters_type _formatters;
};

template<class FormatSource, class... Args>
auto std_format::formatter<FormatSource, Args...>::operator() (const typename std::remove_reference<Args>::type&... args) const -> result_type
{
	std::basic_stringbuf<value_type, traits_type> buf;
	this->operator()(buf, args...);
	return buf.str();
}

template<class FormatSource, class... Args>
auto std_format::formatter<FormatSource, Args...>
	::operator() (std::basic_ostream<value_type, traits_type>& os, const typename std::remove_reference<Args>::type&... args) const -> decltype(os)
{
	this->operator()(*os.rdbuf(), args...);
	return os;
}

template<class FormatSource, class... Args>
template<class Allocator>
void std_format::formatter<FormatSource, Args...>
	::operator() (std::basic_string<value_type, traits_type, Allocator>& str, const typename std::remove_reference<Args>::type&... args) const
{
	std::basic_stringbuf<value_type, traits_type> buf;
	this->operator()(buf, args...);
	str += buf.str();
}

template<class FormatSource, class... Args>
void std_format::formatter<FormatSource, Args...>::operator() (streambuf_type& buf, const typename std::remove_reference<Args>::type&... args) const
{
	auto values = std::tie(args...);
	for(const auto& f : _formatters)
		f(values, _fmt, buf);
}

template<class FormatSource, class... Args>
void std_format::formatter<FormatSource, Args...>::rebuild(format_type fmt)
{
	using std::swap;
	preformatter pre{fmt};
	swap(_fmt, fmt);
	swap(_formatters, pre.formatters());
}

#endif // std_format_detail_formatter_hpp
