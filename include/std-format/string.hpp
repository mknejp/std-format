//
//  string.hpp
//  std-format
//
//  Created by knejp on 23.1.14.
//  Copyright (c) 2014 Miro Knejp. All rights reserved.
//

#ifndef std_format_string_hpp
#define std_format_string_hpp

#include <string>

#include <std-format/detail/string_view.hpp>

namespace std { namespace experimental
{
	string to_string(char ch) { return { ch }; }
	wstring to_string(wchar_t ch) { return { ch }; }
	u16string to_string(char16_t ch) { return { ch }; }
	u32string to_string(char32_t ch) { return { ch }; }
	
}} // namespace std::experimental

#endif
