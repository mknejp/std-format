//
//  format_validator.hpp
//  std-format
//
//  Created by knejp on 21.1.14.
//  Copyright (c) 2014 Miro Knejp. All rights reserved.
//

#ifndef std_format_format_validator_hpp
#define std_format_format_validator_hpp

namespace std { namespace experimental
{
	namespace detail
	{
		template<class CharT, class Traits>
		class format_validator;
	}
	
}} // namespace std::experimental

template<class CharT, class Traits>
class std::experimental::detail::format_validator
{
public:
	using format_type = basic_string_view<CharT, Traits>;
	using format_iterator = typename format_type::const_iterator;
	using traits_type = typename format_type::traits_type;
	using value_type = typename format_type::value_type;

	format_validator(format_type fmt) : _fmt(fmt)
	{
	}
	
	bool operator() (size_t nargs, nothrow_t) noexcept
	{
		try
		{
			this->operator() (nargs);
			return true;
		}
		catch(...) { return false; }
	}
	
	void operator() (size_t nargs)
	{
		detail::format_parser<value_type, traits_type> parser;
		basic_string<CharT, Traits> tempBuffer;
		// All we do here is let the parser do it's job.
		// If it doesn't throw the format string doesn't violate any syntactic rules and all indices are valid.
		parser(_fmt.begin(), _fmt.end(), nargs, tempBuffer,
			   [&] (format_iterator first, format_iterator last) { },
			   [&] (unsigned int n, unsigned int arg, int width, size_t offset, size_t length, bool options_in_temp) { }
			   );
	}

private:
	basic_string_view<CharT, Traits> _fmt;
};

#endif
