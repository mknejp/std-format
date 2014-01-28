//
//  format_appender.hpp
//  std-format
//
//  Created by knejp on 21.1.14.
//  Copyright (c) 2014 Miro Knejp. All rights reserved.
//

#ifndef std_format_detail_format_appender_hpp
#define std_format_detail_format_appender_hpp

#include <std-format/detail/string_view.hpp>

#include <cassert>
#include <iterator>
#include <stdexcept>
#include <string>
#include <std-format/type_traits.hpp>

namespace std { namespace experimental
{
	/**
	 A simple wrapper around a character sink that allows only appending operations.
	 
	 This type serves two purposes:
	 - Enable block-writing operations for types which support it and where it may be more efficient than writing every single character separate.
	 - Serve as tag type to disambiguate conflicting to_string() overloads.
	 
	 Every specialization of \p appender has the following four methods:
	 - `appender& append(CharT ch)`: appends a single character to the underlying sink.
	 - `appender& append(const CharT* str, size_t len)`: appends an array of characters to the sink, possibly performing a block-operation that may be more efficient than a looped per-character append.
	 - `appender& append(basic_string_view<...> str)`: Convenience overload, same as for arrays but takes it's data from a \p basic_string_view.
	 - `appender& append(const basic_string<...>& str)`: Convenience overload, same as for arrays but takes it's data from a \p basic_string.

	 Every append method returns a reference to itself to allow chaining or convenient returning from a function.
	 The *exact* type of \p CharT is not defined and depends on \p Sink, however it should be one of the builtin character types.
	 
	 Various specializations of \p appender are predefined to be usable with as many existing types as possible (\p sink is a placeholder for the actual instance of the \p sink type):
	 - If \p Sink is an iterator for which `iterator_traits<Sink>::iterator_category` is convertible to \p output_iterator_tag then appending equals writing to the underlying \p OutputIterator.
	 - If both `begin(sink)` and `end(sink)` are well-formed the appender fills the range specified by the two calls and throws if appendending would cause overflow.
	 - If \p Sink is a \p basic_string the then data is appended to the string using the \p basic_string::append() overloads.
	 - If \p Sink is a \p ostream_iterator then behavior is the same as for an ordinary \p OutputIterator except that if \p ostream_iterator::failed() signals \p true then \p append() throws.
	 - If \p Sink is implicitly convertible to \p basic_streambuf then streambuf::sputn() and streambuf::sputc() are used for appending, throwing if they signal _EOF_ conditions.
	 - If \p Sink is implicitly convertible to \p basic_ostream then appending writes to `*basic_ostream::rdbuf()` and behaves the same as above.
	 - If none of the above apply the user is required to specialize the \p appender class for the given \p Sink type.
	 
	 It is encouraged to use make_format_appender() for creating appenders to save oneself the hassle of specifying template parameters.
	 */
	template<class Sink>
	class format_appender;
	
	namespace detail
	{
		class write_counter
		{
		public:
			size_t write_count() const { return _count; }
			void increment(size_t n = 1) { _count += n; }
			
		private:
			size_t _count = 0;
		};
		
		template<class Derived, class OutIter>
		class iterator_appender
		{
		public:
			using value_type = typename OutIter::value_type;
			
			iterator_appender(OutIter iter) : _iter(move(iter)) { }
			
			// Since OutputIterator can have value_type=void accept whatever the assignment accepts
			template<class CharT>
			Derived& append(CharT ch)
			{
				*_iter++ = move(ch);
				static_cast<Derived&>(*this).increment_write_counter(1);
				return static_cast<Derived&>(*this);
			}
			
			template<class CharT>
			Derived& append(const CharT* str, size_t len)
			{
				assert(str && "NULL buffer passed to append()");
				_iter = copy_n(str, len, _iter);
				static_cast<Derived&>(*this).increment_write_counter(len);
				return static_cast<Derived&>(*this);
			}
			
			template<class CharT, class Traits>
			Derived& append(basic_string_view<CharT, Traits> str) { return append(str.data(), str.size()); }
			template<class CharT, class Traits, class Allocator>
			Derived& append(const basic_string<CharT, Traits, Allocator>& str) { return append(str.data(), str.size()); }
			
		protected:
			iterator_appender(iterator_appender&&) = default;
			iterator_appender& operator= (iterator_appender&&) = default;
			
		private:
			OutIter _iter;
		};
		
		template<class Derived, class Container>
		class range_checked_appender
		{
			using Iter = decltype(declval<Container>().begin());
			
		public:
			using value_type = decay_t<decltype(*declval<Iter>())>;
			
			range_checked_appender(Container& c) : _first(begin(c)), _last(end(c)) { }
			
			Derived& append(value_type ch)
			{
				if(_first == _last)
					throw runtime_error{"buffer overflow in format_appender"};
				*_first++ = move(ch);
				static_cast<Derived&>(*this).increment_write_counter(1);
				return static_cast<Derived&>(*this);
			}
			
			Derived& append(const value_type* str, size_t len)
			{
				assert(str && "NULL buffer passed to append()");
				append_block(str, len, typename iterator_traits<Iter>::iterator_category());
				static_cast<Derived&>(*this).increment_write_counter(len);
				return static_cast<Derived&>(*this);
			}
			template<class CharT, class Traits>
			Derived& append(const basic_string_view<CharT, Traits>& str) { return append(str.data(), str.size()); }
			template<class CharT, class Traits, class Allocator>
			Derived& append(const basic_string<CharT, Traits, Allocator>& str) { return append(str.data(), str.size()); }

		protected:
			range_checked_appender(range_checked_appender&&) = default;
			range_checked_appender& operator= (range_checked_appender&&) = default;
			
		private:
			void append_block(const value_type* str, size_t len, random_access_iterator_tag)
			{
				if(distance(_first, _last) < len)
					throw runtime_error{"buffer overflow in format_appender"};
				static_cast<Derived&>(*this).increment_write_counter(1);
				_first = copy_n(str, len, _first);
			}
			void append_block(const value_type* str, size_t len, ...)
			{
				size_t i = 0;
				for(auto s_iter = str; i < len && _first != _last; ++i)
					*_first++ = *s_iter++;
				if(i < len)
					throw runtime_error{"buffer overflow in format_appender"};
			}
			
			Iter _first;
			Iter _last;
		};
		
		template<class Derived, class CharT, class Traits>
		class ostreambuf_iterator_appender
		{
		public:
			using value_type = CharT;
			
			ostreambuf_iterator_appender(ostreambuf_iterator<CharT, Traits> iter) : _iter(iter) { }
			
			Derived& append(value_type ch)
			{
				*_iter++ = std::move(ch);
				if(_iter.failed())
					throw runtime_error{"buffer overflow in format_appender"};
				static_cast<Derived&>(*this).increment_write_counter(1);
				return static_cast<Derived&>(*this);
			}
			
			Derived& append(const value_type* str, size_t len)
			{
				assert(str && "NULL buffer passed to append()");
				_iter = copy_n(str, len, _iter);
				if(_iter.failed())
					throw runtime_error{"buffer overflow in format_appender"};
				static_cast<Derived&>(*this).increment_write_counter(len);
				return static_cast<Derived&>(*this);
			}
			Derived& append(const basic_string_view<CharT, Traits>& str) { return append(str.data(), str.size()); }
			template<class Allocator>
			Derived& append(const basic_string<CharT, Traits, Allocator>& str) { return append(str.data(), str.size()); }
			
		protected:
			ostreambuf_iterator_appender(ostreambuf_iterator_appender&&) = default;
			ostreambuf_iterator_appender& operator= (ostreambuf_iterator_appender&&) = default;
			
		private:
			ostreambuf_iterator<CharT, Traits> _iter;
		};
		
		template<class Derived, class CharT, class Traits>
		class streambuf_appender
		{
		public:
			using value_type = CharT;
			
			streambuf_appender(basic_streambuf<CharT, Traits>& buf) : _buf(&buf) { }
			streambuf_appender(basic_ostream<CharT, Traits>& os) : _buf(os.rdbuf()) { }
			
			Derived& append(CharT ch)
			{
				if(Traits::eq_int_type(_buf->sputc(ch), Traits::eof()))
					throw runtime_error{"Error writing to ostreambuf."};
				static_cast<Derived&>(*this).increment_write_counter(1);
				return static_cast<Derived&>(*this);
			}
			
			Derived& append(const value_type* str, size_t len)
			{
				assert(str && "NULL buffer passed to append()");
				if(_buf->sputn(str, len) != len)
					throw runtime_error{"buffer overflow in format_appender"};
				static_cast<Derived&>(*this).increment_write_counter(len);
				return static_cast<Derived&>(*this);
			}
			Derived& append(const basic_string_view<CharT, Traits>& str) { return append(str.data(), str.size()); }
			template<class Allocator>
			Derived& append(const basic_string<CharT, Traits, Allocator>& str) { return append(str.data(), str.size()); }

		protected:
			streambuf_appender(streambuf_appender&&) = default;
			streambuf_appender& operator= (streambuf_appender&&) = default;
			
		private:
			basic_streambuf<CharT, Traits>* _buf;
		};
		
		template<class Derived, class CharT, class Traits, class Allocator>
		class string_appender : public write_counter
		{
		public:
			using value_type = CharT;
			
			string_appender(basic_string<CharT, Traits, Allocator>& str) : _str(&str) { }

			Derived& append(CharT ch)
			{
				_str->append(1, ch);
				static_cast<Derived&>(*this).increment_write_counter(1);
				return static_cast<Derived&>(*this);
			}
			
			Derived& append(const value_type* str, size_t len)
			{
				assert(str && "NULL buffer passed to append()");
				_str->append(str, len);
				static_cast<Derived&>(*this).increment_write_counter(len);
				return static_cast<Derived&>(*this);
			}
			Derived& append(const basic_string_view<CharT, Traits>& str) { return append(str.data(), str.size()); }
			template<class Allocator2>
			Derived& append(const basic_string<CharT, Traits, Allocator2>& str) { return append(str.data(), str.size()); }
			
		protected:
			string_appender(string_appender&&) = default;
			string_appender& operator= (string_appender&&) = default;
			
		private:
			basic_string<CharT, Traits, Allocator>* _str;
		};

		using std::begin;
		using std::end;
		
		// Accept output iterators only
		template<class Derived, class OutIter>
		auto select_appender(OutIter it, output_iterator_tag) -> iterator_appender<Derived, OutIter>;
		template<class Derived, class OutIter>
		auto select_appender(OutIter it) -> decltype(select_appender<Derived>(it, typename iterator_traits<OutIter>::iterator_category()));
		// Destinations with begin() and end() are range checked to prevent overflow
		template<class Derived, class Container>
		auto select_appender(Container c, decltype(begin(c))* = 0, decltype(end(c))* = 0) -> range_checked_appender<Derived, Container>;
		// Special treatment for ostreambuf_iterator because we can check for errors
		template<class Derived, class CharT, class Traits>
		auto select_appender(ostreambuf_iterator<CharT, Traits> buf) -> ostreambuf_iterator_appender<Derived, CharT, Traits>;
		// If it is derived from streambuf use the streambuf base class
		template<class Derived, class CharT, class Traits>
		auto select_appender(basic_streambuf<CharT, Traits> buf) -> streambuf_appender<Derived, CharT, Traits>;
		// If it is derived from ostream use the readbuffer
		template<class Derived, class CharT, class Traits>
		auto select_appender(basic_ostream<CharT, Traits> buf) -> streambuf_appender<Derived, CharT, Traits>;
		// strings are special as they can be appended to
		template<class Derived, class CharT, class Traits, class Allocator>
		auto select_appender(basic_string<CharT, Traits, Allocator> s) -> string_appender<Derived, CharT, Traits, Allocator>;

		// Cannot append to the requested type
		template<class Derived, class T>
		void select_appender();
	}
	
	template<class Sink>
	class format_appender : public decltype(detail::select_appender<format_appender<Sink>>(declval<Sink>()))
	{
		using Base = decltype(detail::select_appender<format_appender<Sink>>(declval<Sink>()));
		
	public:
		using Base::Base;
		
		size_t write_count() const { return _count.write_count(); }
		
	private:
		friend Base;
		
		void increment_write_counter(size_t n) { _count.increment(n); }
		
		detail::write_counter _count;
	};
	
	// Prevent format_appender<format_appender<...>> nesting
	template<class Sink>
	format_appender<Sink>& make_format_appender(format_appender<Sink>& app) { return app; }
	template<class Sink>
	format_appender<Sink>& make_format_appender(format_appender<Sink>&& app) { return app; }
	
	template<class Sink>
	format_appender<decay_t<Sink>> make_format_appender(Sink& t) { return { t }; }
	template<class Sink>
	format_appender<decay_t<Sink>> make_format_appender(Sink&& t) { return { t }; }
	
}} // namespace std::experimental

#endif // std_format_detail_format_appender_hpp
