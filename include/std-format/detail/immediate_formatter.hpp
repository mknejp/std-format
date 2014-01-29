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
		using format_signature = function<size_t(const Args&, Appender&, const FmtFlags&)> ;
		
		template<class Args, class Appender, class FmtFlags, size_t N>
		constexpr format_signature<Args, Appender, FmtFlags> make_format_delegate()
		{
			return [] (const Args& args, Appender& app, const FmtFlags& flags)
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
	size_t operator() (Appender& app, const Args&... args) const
	{
		basic_string<CharT, Traits> temp; // This buffer is used for all temporaries we need, thus hopefully minimizing the number of reallocations
		
		format_delegates<Tuple, Appender, format_type, make_index_sequence<sizeof...(Args)>> delegates1;
		format_delegates<Tuple, decltype(make_format_appender(temp)), format_type, make_index_sequence<sizeof...(Args)>> delegates2;
		
		size_t printed = 0;
		
		auto values = std::tie(args...);
		for(auto component : parse_format(_fmt, sizeof...(Args)))
		{
			if(component.type == format_component_type::static_substring)
				app.append(component.substring);
			else if(component.type == format_component_type::format_argument)
			{
				if(component.width > 0)
					printed += right_align(app, component, values, temp, delegates2);
				else if(component.width < 0)
					printed += left_align(app, component, values, delegates1);
				else
					printed += delegates1[component.index](values, app, component.substring);
			}
		}
		return printed;
	}
	
private:
	template<class Appender, class Component, class Values, class Delegates>
	static size_t right_align(Appender& app, Component& component, Values& values, basic_string<CharT, Traits>& temp, Delegates& delegates)
	{
		// Format the substring first to determine its length and prepend the padding if necessary.
		// Padding is done using the space ' ' character.
		temp.clear();
		auto app2 = make_format_appender(temp);
		auto n = delegates[component.index](values, app2, component.substring);
		for ( ; n < component.width; ++n)
			app.append(CharT(' '));
		app.append(temp);
		return n;
	}
	
	template<class Appender, class Component, class Values, class Delegates>
	static size_t left_align(Appender& app, Component& component, Values& values, Delegates& delegates)
	{
		// Fill the destination directly and append padding if necessary.
		// Padding is done using the space ' ' character.
		component.width = -component.width;
		auto n = delegates[component.index](values, app, component.substring);
		for ( ; n < component.width; ++n)
			app.append(CharT(' '));
		return n;
	}
	
	format_type _fmt;
};

#endif // std_format_detail_immediate_formatter_hpp
