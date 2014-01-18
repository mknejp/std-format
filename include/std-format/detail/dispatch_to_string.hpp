//
//  dispatch_to_string.hpp
//  SystemGen
//
//  Created by knejp on 17.1.14.
//  Copyright (c) 2014 Miroslav Knejp. All rights reserved.
//

#ifndef std_format_detail_dispatch_to_string_hpp
#define std_format_detail_dispatch_to_string_hpp

namespace std_format
{
	namespace detail
	{
		using std::result_of;
		using std::to_string;
		
		template<class T>
		using enable_for = typename std::conditional<false, T, void>::type;
		
		// Single argument overload returning a string without options
		template<class Arg, class Output, class FmtFlags>
		using overload1_sig =
			decltype(to_string(std::declval<const Arg&>()));
		
		template<class Arg, class Output, class FmtFlags>
		std::true_type has_overload1(enable_for<overload1_sig<Arg, Output, FmtFlags>>*) { return { }; }
		
		template<class Arg, class Output, class FmtFlags>
		std::false_type has_overload1(...) { return { }; }
		
		// Single argument overload returning a string with options
		template<class Arg, class Output, class FmtFlags>
		using overload1_opt_sig =
			decltype(to_string(std::declval<const Arg&>(), std::declval<const FmtFlags&>()));
		
		template<class Arg, class Output, class FmtFlags>
		std::true_type has_overload1_opt(enable_for<overload1_opt_sig<Arg, Output, FmtFlags>>*) { return { }; }
		
		template<class Arg, class Output, class FmtFlags>
		std::false_type has_overload1_opt(...) { return { }; }
		
		// Writes directly to out without options
		template<class Arg, class Output, class FmtFlags>
		using overload2_sig =
			decltype(to_string(std::declval<const Arg&>(), std::declval<Output&>()));
		
		template<class Arg, class Output, class FmtFlags>
		std::true_type has_overload2(enable_for<overload2_sig<Arg, Output, FmtFlags>>*) { return { }; }
		
		template<class Arg, class Output, class FmtFlags>
		std::false_type has_overload2(...) { return { }; }
		
		// Writes directly to out with options
		template<class Arg, class Output, class FmtFlags>
		using overload2_opt_sig =
			decltype(to_string(std::declval<const Arg&>(), std::declval<Output&>(), std::declval<const FmtFlags&>()));
		
		template<class Arg, class Output, class FmtFlags>
		std::true_type has_overload2_opt(enable_for<overload2_opt_sig<Arg, Output, FmtFlags>>*) { return { }; }
		
		template<class Arg, class Output, class FmtFlags>
		std::false_type has_overload2_opt(...) { return { }; }
		
		// Overloads listed in order of priority, sorted by specialization first, efficiency second.
		// Those accepting format arguments are considered more important (independent of efficiency) to ensure correct output.
		// Within each group (with or without format options) overloads are sorted by (possible) efficiency of signature.
		template<class Arg, class Output, class FmtFlags, bool b1, bool b2, bool b3>
		void dispatch_to_string(const Arg& arg, Output& out, const FmtFlags& flags,
								std::integral_constant<bool, true> has_overload2_opt,
								std::integral_constant<bool, b1> has_overload1_opt,
								std::integral_constant<bool, b2> has_overload2,
								std::integral_constant<bool, b3> has_overload1)
		{
			to_string(arg, out, flags);
		}
		
		template<class Arg, class Output, class FmtFlags, bool b1, bool b2>
		void dispatch_to_string(const Arg& arg, Output& out, const FmtFlags& flags,
								std::integral_constant<bool, false> has_overload2_opt,
								std::integral_constant<bool, true> has_overload1_opt,
								std::integral_constant<bool, b1> has_overload2,
								std::integral_constant<bool, b2> has_overload1)
		{
			auto string = to_string(arg, flags);
			if(out.sputn(string.data(), string.size()) != string.size())
				throw std::ios_base::failure{"Error writing to output streambuf."};
		}
		
		// Below do not accept format arguments
		template<class Arg, class Output, class FmtFlags, bool b1>
		void dispatch_to_string(const Arg& arg, Output& out, const FmtFlags& flags,
								std::integral_constant<bool, false> has_overload2_opt,
								std::integral_constant<bool, false> has_overload1_opt,
								std::integral_constant<bool, true> has_overload2,
								std::integral_constant<bool, b1> has_overload1)
		{
			to_string(arg, out);
		}
		
		template<class Arg, class Output, class FmtFlags>
		void dispatch_to_string(const Arg& arg, Output& out, const FmtFlags& flags,
								std::integral_constant<bool, false> has_overload2_opt,
								std::integral_constant<bool, false> has_overload1_opt,
								std::integral_constant<bool, false> has_overload2,
								std::integral_constant<bool, true> has_overload1)
		{
			auto string = to_string(arg);
			if(out.sputn(string.data(), string.size()) != string.size())
				throw std::ios_base::failure{"Error writing to output streambuf."};
		}
		
		// There is no to_string for basic_string and basic_string_view, handle it internally
		template<class CharT, class Traits, class Output, class FmtFlags>
		void dispatch_to_string(const std::basic_string<CharT, Traits>& arg, Output& out, const FmtFlags& flags)
		{
			if(out.sputn(arg.data(), arg.size()) != arg.size())
				throw std::ios_base::failure{"Error writing to output streambuf."};
		}
		
		template<class CharT, class Traits, class Output, class FmtFlags>
		void dispatch_to_string(const basic_string_view<CharT, Traits>& arg, Output& out, const FmtFlags& flags)
		{
			if(out.sputn(arg.data(), arg.size()) != arg.size())
				throw std::ios_base::failure{"Error writing to output streambuf."};
		}
		
		template<class Arg, class Output, class FmtFlags>
		void dispatch_to_string(const Arg& arg, Output& out, const FmtFlags& flags)
		{
			dispatch_to_string(arg, out, flags,
							   has_overload2_opt<Arg, Output, FmtFlags>(0),
							   has_overload1_opt<Arg, Output, FmtFlags>(0),
							   has_overload2    <Arg, Output, FmtFlags>(0),
							   has_overload1    <Arg, Output, FmtFlags>(0));
		}
	}
}


#endif // std_format_detail_dispatch_to_string_hpp
