//
//  type_traits.hpp
//  std-format
//
//  Created by knejp on 21.1.14.
//  Copyright (c) 2014 Miro Knejp. All rights reserved.
//

#ifndef std_format_type_traits_hpp
#define std_format_type_traits_hpp

#include <type_traits>

namespace std { namespace experimental
{
	template<class T> using decay_t = typename decay<T>::type;
	template<class T> using make_unsigned_t = typename make_unsigned<T>::type;
	
}} // namespace std::experimental

#endif
