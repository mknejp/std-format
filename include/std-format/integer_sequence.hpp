//
//  integer_sequence.hpp
//  SystemGen
//
//  Created by knejp on 17.1.14.
//  Copyright (c) 2014 Miroslav Knejp. All rights reserved.
//

#ifndef std_format_integer_sequence_hpp
#define std_format_integer_sequence_hpp

#include <cstddef>

namespace std_format
{
	template<class T, T... Ints>
	struct integer_sequence
	{
		using value_type = T;
		static constexpr std::size_t size() noexcept { return sizeof...(Ints); }
	};
	
	template<std::size_t... Ints>
	using index_sequence = integer_sequence<std::size_t, Ints...>;
	
	namespace detail
	{
		template<class T, T N, bool Zero, T... Tail>
		struct unfold_index_sequence
		{
			using type = typename unfold_index_sequence<T, N-1, N-1 == 0, N-1, Tail...>::type;
		};
		template<class T, T N, T... Tail>
		struct unfold_index_sequence<T, N, true, Tail...>
		{
			using type = integer_sequence<T, Tail...>;
		};
	}
	
	template<class T, T N>
	using make_integer_sequence = typename detail::unfold_index_sequence<T, N, N==0>::type;
	
	template<std::size_t N>
	using make_index_sequence = make_integer_sequence<std::size_t, N>;
	
	template<class... T>
	using index_sequence_for = make_index_sequence<sizeof...(T)>;
} // namespace std_format

#endif // std_format_integer_sequence_hpp
