//
//  immediate_formatter.hpp
//  std-format
//
//  Created by knejp on 17.1.14.
//  Copyright (c) 2014 Miro Knejp. All rights reserved.
//

#ifndef std_format_detail_immediate_formatter_hpp
#define std_format_detail_immediate_formatter_hpp

namespace std { namespace experimental
{
	namespace detail
	{
		template<class CharT, class Traits, class... Args>
		class immediate_formatter;
		
		template<class Args, class Appender, class FmtFlags>
		using format_signature = function<Appender(const Args&, Appender, const FmtFlags&)> ;
		
		template<class Args, class Appender, class FmtFlags, size_t N>
		constexpr format_signature<Args, Appender, FmtFlags> make_format_delegate()
		{
			return [] (const Args& args, Appender app, const FmtFlags& flags)
			{
				return dispatch_to_string(get<N>(args), app, flags);
			};
		}
		
		template<class Args, class Appender, class FmtFlags, class I>
		struct format_delegates;
		
		template<class Args, class Appender, class FmtFlags, size_t... I>
		struct format_delegates<Args, Appender, FmtFlags, integer_sequence<size_t, I...>>
		{
			array<format_signature<Args, Appender, FmtFlags>, sizeof...(I)> value
			{{
				make_format_delegate<Args, Appender, FmtFlags, I>()...
			}};
			constexpr const format_signature<Args, Appender, FmtFlags>& operator[] (size_t n)
			{
				return value[n];
			}
		};
	} // namespace detail
}} // namespace std::experimental

// This could be made a public class if people think it would be useful
template<class CharT, class Traits, class... Args>
class std::experimental::detail::immediate_formatter
{
	using Tuple = tuple<const Args&...>;

public:
	using format_type = basic_string_view<CharT, Traits>;
	using format_iterator = typename format_type::const_iterator;
	using traits_type = typename format_type::traits_type;
	using value_type = typename format_type::value_type;
	
	template<class T>
	explicit immediate_formatter(T&& fmt) : _fmt(std::forward<T>(fmt)) { }

	template<class Appender>
	Appender operator() (Appender app, const Args&... args)
	{
		format_delegates<Tuple, Appender, format_type, make_index_sequence<sizeof...(Args)>> delegates;
		auto values = std::tie(args...);
		detail::format_parser<value_type, traits_type> parser;
		basic_string<CharT, Traits> tempBuffer; // This buffer is used for all temporaries we need, thus hopefully minimizing the number of reallocations
		parser(_fmt.begin(), _fmt.end(), sizeof...(Args), tempBuffer,
			   [&] (format_iterator first, format_iterator last) { app.append(format_type{first, distance(first, last)}); },
			   [&] (size_t n, size_t arg, size_t offset, size_t length, bool options_in_temp)
			   {
				   auto flags = format_type{ options_in_temp ? tempBuffer.data() + offset : _fmt.data() + offset, length };
				   app = delegates[arg](values, app, flags);
			   }
		);
		return app;
	}
	
private:
	format_type _fmt;
};

#endif // std_format_detail_immediate_formatter_hpp
