//
//  to_string.hpp
//  std-format
//
//  Created by knejp on 23.1.14.
//  Copyright (c) 2014 Miro Knejp. All rights reserved.
//

#ifndef std_format_detail_to_string_hpp
#define std_format_detail_to_string_hpp

// included from <string.hpp>

#include <std-format/detail/appender.hpp>

namespace std { namespace experimental
{
	
	template<class Sink, class CharT, class Traits>
	format_appender<Sink> to_string(int i, format_appender<Sink> app, basic_string<CharT, Traits> options)
	{
		
	}
	
}} // namespace std::experimental

#endif // std_format_detail_to_string_hpp
