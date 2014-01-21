//
//  dispatch_to_string.hpp
//  std-format
//
//  Created by knejp on 17.1.14.
//  Copyright (c) 2014 Miro Knejp. All rights reserved.
//

#ifndef std_format_detail_dispatch_to_string_hpp
#define std_format_detail_dispatch_to_string_hpp

namespace std { namespace experimental
{
	namespace detail
	{
		template<class T>
		using enable_for = typename conditional<false, T, void>::type;
		
		using std::to_string; // Required to find overloads for fundamental types
		
		// Single argument overload returning a string without options
		template<class Arg, class Appender, class FmtFlags>
		using overload1_sig =
			decltype(to_string(declval<const Arg&>()));
		
		template<class Arg, class Appender, class FmtFlags>
		true_type has_overload1(enable_for<overload1_sig<Arg, Appender, FmtFlags>>*) { return { }; }
		
		template<class Arg, class Appender, class FmtFlags>
		false_type has_overload1(...) { return { }; }
		
		// Single argument overload returning a string with options
		template<class Arg, class Appender, class FmtFlags>
		using overload1_opt_sig =
			decltype(to_string(declval<const Arg&>(), declval<FmtFlags>()));
		
		template<class Arg, class Appender, class FmtFlags>
		true_type has_overload1_opt(enable_for<overload1_opt_sig<Arg, Appender, FmtFlags>>*) { return { }; }
		
		template<class Arg, class Appender, class FmtFlags>
		false_type has_overload1_opt(...) { return { }; }
		
		// Writes directly to app without options
		template<class Arg, class Appender, class FmtFlags>
		using overload2_sig =
			decltype(to_string(declval<const Arg&>(), declval<Appender&>()));
		
		template<class Arg, class Appender, class FmtFlags>
		true_type has_overload2(enable_for<overload2_sig<Arg, Appender, FmtFlags>>*) { return { }; }
		
		template<class Arg, class Appender, class FmtFlags>
		false_type has_overload2(...) { return { }; }
		
		// Writes directly to app with options
		template<class Arg, class Appender, class FmtFlags>
		using overload2_opt_sig =
			decltype(to_string(declval<const Arg&>(), declval<Appender&>(), declval<FmtFlags>()));
		
		template<class Arg, class Appender, class FmtFlags>
		true_type has_overload2_opt(enable_for<overload2_opt_sig<Arg, Appender, FmtFlags>>*) { return { }; }
		
		template<class Arg, class Appender, class FmtFlags>
		false_type has_overload2_opt(...) { return { }; }
		
		// Overloads listed in order of priority, sorted by specialization first, efficiency second.
		// Those accepting format arguments are considered more important (independent of efficiency) to ensure correct output.
		// Within each group (with or without format options) overloads are sorted by (possible) efficiency of signature.
		template<class Arg, class Appender, class FmtFlags, bool b1, bool b2, bool b3>
		Appender dispatch_to_string(const Arg& arg, Appender& app, FmtFlags flags,
									integral_constant<bool, true> has_overload2_opt,
									integral_constant<bool, b1> has_overload1_opt,
									integral_constant<bool, b2> has_overload2,
									integral_constant<bool, b3> has_overload1)
		{
			return to_string(arg, app, flags);
		}
		
		template<class Arg, class Appender, class FmtFlags, bool b1, bool b2>
		Appender dispatch_to_string(const Arg& arg, Appender& app, FmtFlags flags,
									integral_constant<bool, false> has_overload2_opt,
									integral_constant<bool, true> has_overload1_opt,
									integral_constant<bool, b1> has_overload2,
									integral_constant<bool, b2> has_overload1)
		{
			auto string = to_string(arg, flags);
			return app.append(string.data(), string.size());
		}
		
		// Below do not accept format arguments
		template<class Arg, class Appender, class FmtFlags, bool b1>
		Appender dispatch_to_string(const Arg& arg, Appender& app, FmtFlags flags,
									integral_constant<bool, false> has_overload2_opt,
									integral_constant<bool, false> has_overload1_opt,
									integral_constant<bool, true> has_overload2,
									integral_constant<bool, b1> has_overload1)
		{
			return to_string(arg, app);
		}
		
		template<class Arg, class Appender, class FmtFlags>
		Appender dispatch_to_string(const Arg& arg, Appender& app, FmtFlags flags,
									integral_constant<bool, false> has_overload2_opt,
									integral_constant<bool, false> has_overload1_opt,
									integral_constant<bool, false> has_overload2,
									integral_constant<bool, true> has_overload1)
		{
			auto string = to_string(arg);
			return app.append(string.data(), string.size());
		}
		
		// There is no to_string for basic_string and basic_string_view, handle it internally
		template<class CharT, class Traits, class Appender, class FmtFlags>
		Appender dispatch_to_string(const basic_string<CharT, Traits>& arg, Appender& app, FmtFlags flags)
		{
			return app.append(arg.data(), arg.size());
		}
		
		template<class CharT, class Traits, class Appender, class FmtFlags>
		Appender dispatch_to_string(const basic_string_view<CharT, Traits>& arg, Appender& app, FmtFlags flags)
		{
			return app.append(arg.data(), arg.size());
		}
		
		template<class Arg, class Appender, class FmtFlags>
		Appender dispatch_to_string(const Arg& arg, Appender& app, FmtFlags flags)
		{
			return dispatch_to_string(arg, app, flags,
									  has_overload2_opt<Arg, Appender, FmtFlags>(0),
									  has_overload1_opt<Arg, Appender, FmtFlags>(0),
									  has_overload2    <Arg, Appender, FmtFlags>(0),
									  has_overload1    <Arg, Appender, FmtFlags>(0));
		}
	} // namespace detail
}} // namespace std::experimental


#endif // std_format_detail_dispatch_to_string_hpp
