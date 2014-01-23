//
//  string_view.hpp
//  std-format
//
//  Created by knejp on 17.1.14.
//  Copyright (c) 2014 Miro Knejp. All rights reserved.
//

#ifndef std_format_detail_string_view_hpp
#define std_format_detail_string_view_hpp

#include <string>

namespace std { namespace experimental
{
	// Very minimalistic implementation of basic_string_view to fullfil our needs
	template<class CharT, class Traits = char_traits<CharT>>
	class basic_string_view
	{
	public:
		using const_iterator = const CharT*;
		using iterator = const_iterator;
		using traits_type = Traits;
		using value_type = CharT;
		
		basic_string_view(const value_type* str) : _str(str), _len(strlen(str)) { }
		basic_string_view(const value_type* str, size_t len) : _str(str), _len(len) { }
		basic_string_view(const_iterator begin, const_iterator end) : _str(begin), _len(end - begin) { }
		template<class Allocator>
		basic_string_view(const basic_string<CharT, Traits, Allocator>& str) : _str(str.data()), _len(str.size()) { }
		
		iterator begin() const noexcept { return _str; }
		iterator end() const noexcept { return _str + _len; }
		const_iterator cbegin() const noexcept { return _str; }
		const_iterator cend() const noexcept { return _str + _len; }
		
		const CharT* data() const noexcept { return _str; }
		size_t size() const noexcept { return _len; }
		
	private:
		const CharT* _str;
		size_t _len;
	};
	
	using string_view = basic_string_view<char>;
	using wstring_view = basic_string_view<wchar_t>;
	using u16string_view = basic_string_view<char16_t>;
	using u32string_view = basic_string_view<char32_t>;
}} // namespace std::experimental

#endif // std_format_detail_string_view_hpp
