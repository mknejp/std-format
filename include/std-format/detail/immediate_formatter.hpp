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
		basic_string<CharT, Traits> temp; // This buffer is used for all temporaries we need, thus hopefully minimizing the number of reallocations
		
		format_delegates<Tuple, Appender, format_type, make_index_sequence<sizeof...(Args)>> delegates1;
		format_delegates<Tuple, decltype(make_format_appender(temp)), format_type, make_index_sequence<sizeof...(Args)>> delegates2;
		
		auto values = std::tie(args...);
		detail::format_parser<value_type, traits_type> parser;
		parser(_fmt.begin(), _fmt.end(), sizeof...(Args),
			   [&] (format_type string) { app.append(string); },
			   [&] (unsigned int n, unsigned int arg, int width, format_type options)
			   {
				   // TODO: Right now aligning is completely ignorant of multibyte characters. This needs to be supported somehow.
				   if(width > 0)
				   {
					   // Positive width means right alignment.
					   // In that case we need to format the substring first to determine its length so we can add the padding.
					   // Padding is done using the space ' ' character.
					   temp.clear();
					   delegates2[arg](values, make_format_appender(temp), options);
					   for (auto i = temp.size(); i < width; ++i)
						   app.append(CharT(' '));
					   app.append(temp);
				   }
				   else if(width < 0)
				   {
					   // Otherwise we can fill the destination directly and append padding if necessary
					   // Padding is done using the space ' ' character.
					   auto start = app.write_count();
					   app = delegates1[arg](values, app, options);
					   auto end = app.write_count();
					   while(end < start - width)
					   {
						   app.append(CharT(' '));
						   ++end;
					   }
				   }
				   else
					   app = delegates1[arg](values, app, options);
			   }
		);
		return app;
	}
	
private:
	format_type _fmt;
};

#endif // std_format_detail_immediate_formatter_hpp
