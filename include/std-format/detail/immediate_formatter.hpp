//
//  immediate_formatter.hpp
//  SystemGen
//
//  Created by knejp on 17.1.14.
//  Copyright (c) 2014 Miroslav Knejp. All rights reserved.
//

#ifndef std_format_detail_immediate_formatter_hpp
#define std_format_detail_immediate_formatter_hpp

namespace std_format
{
	namespace detail
	{
		template<class FormatSource, class... Args>
		class immediate_formatter;
		
		template<class Args, class Output, class FmtFlags>
		using format_signature = std::function<void(Args&, Output&, const FmtFlags&)> ;
		
		template<class Args, class Output, class FmtFlags, std::size_t N>
		format_signature<Args, Output, FmtFlags> make_format_delegate()
		{
			return [] (Args& args, Output& out, const FmtFlags& flags)
			{
				dispatch_to_string(std::get<N>(args), out, flags);
			};
		}
		
		template<class Args, class Output, class FmtFlags, class I>
		struct format_delegates;
		
		template<class Args, class Output, class FmtFlags, std::size_t... I>
		struct format_delegates<Args, Output, FmtFlags, integer_sequence<std::size_t, I...>>
		{
			std::array<format_signature<Args, Output, FmtFlags>, sizeof...(I)> value
			{{
				make_format_delegate<Args, Output, FmtFlags, I>()...
			}};
		};
	}
}

template<class FormatSource, class... Args>
class std_format::detail::immediate_formatter
	: public formatter_base<FormatSource, immediate_formatter<FormatSource, Args...>>
{
	using Base = formatter_base<FormatSource, immediate_formatter<FormatSource, Args...>>;
	
public:
	using format_iterator = typename Base::format_iterator;
	using traits_type = typename Base::traits_type;
	using value_type = typename Base::value_type;
	using format_type = typename Base::format_type;
	
	/// The type of stream buffer used in operator() overloads where none is provided as argument
	using streambuf_type = std::basic_streambuf<value_type, traits_type>;
	
	template<class... FormatArgs>
	explicit immediate_formatter(FormatArgs&&... fmt) : _fmt(std::forward<FormatArgs>(fmt)...) { }
	
	std::basic_string<value_type, traits_type> operator() (const typename std::remove_reference<Args>::type&... args)
	{
		std::basic_stringbuf<value_type, traits_type> streambuf;
		this->operator()(streambuf, args...);
		return streambuf.str();
	}
	
	auto operator() (std::basic_ostream<value_type, traits_type>& os, const typename std::remove_reference<Args>::type&... args) -> decltype(os)
	{
		this->operator()(*os.rdbuf(), args...);
		return os;
	}
	
	void operator() (streambuf_type& streambuf, const typename std::remove_reference<Args>::type&... args);
	
	static constexpr std::size_t size() noexcept { return sizeof...(Args); }
	
private:
	using Tuple = std::tuple<const typename std::remove_reference<Args>::type&...>;
	using FmtFlags = basic_string_view<value_type, traits_type>;
	
	// CRTP callbacks from formatter_base
	friend Base;
	void copy_to_output(format_iterator first, format_iterator last);
	void do_format(std::size_t n, std::size_t index, format_iterator options_first, format_iterator options_last);
	
	format_type _fmt;
	format_delegates<Tuple, streambuf_type, FmtFlags, make_index_sequence<size()>> _delegates;
	Tuple* _values;
	streambuf_type* _streambuf;
};

template<class FormatSource, class... Args>
void std_format::detail::immediate_formatter<FormatSource, Args...>
	::operator()(streambuf_type& streambuf, const typename std::remove_reference<Args>::type&... args)
{
	auto values = std::tie(args...);
	_values = &values;
	_streambuf = &streambuf;
	this->parse(_fmt, sizeof...(Args));
}

template<class FormatSource, class... Args>
void std_format::detail::immediate_formatter<FormatSource, Args...>
	::copy_to_output(format_iterator first, format_iterator last)
{
	auto size = std::distance(first, last);
	if(_streambuf->sputn(std::addressof(*first), size) != size)
	   throw std::ios_base::failure{"Error writing to output streambuf."};
}

template<class FormatSource, class... Args>
void std_format::detail::immediate_formatter<FormatSource, Args...>
	::do_format(std::size_t n, std::size_t index, format_iterator options_first, format_iterator options_last)
{
	auto flags = FmtFlags{ options_first, std::distance(options_first, options_last) };
	this->_delegates.value[index](*_values, *_streambuf, flags);
}

#endif // std_format_detail_immediate_formatter_hpp
