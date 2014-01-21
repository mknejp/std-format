//
//  appender.hpp
//  std-format
//
//  Created by knejp on 21.1.14.
//  Copyright (c) 2014 Miro Knejp. All rights reserved.
//

#ifndef std_format_detail_appender_hpp
#define std_format_detail_appender_hpp

namespace std { namespace experimental
{
	template<class OutIter>
	class appender : public OutIter
	{
	public:
		static_assert(std::is_convertible<typename iterator_traits<OutIter>::iterator_category, output_iterator_tag>::value, "appender<T> supports only output iterators.");
		using OutIter::OutIter;
		
		appender& append(typename OutIter::value_type val)
		{
			*this = std::move(val);
			return ++*this;
		}
		
		appender append(const typename OutIter::value_type* val, size_t count)
		{
			assert(val && "NULL passed to appender");
			*this = copy_n(val, count, *this);
			return *this;
		}
		
	private:
		OutIter _iter;
	};
	
	template<class CharT, class Traits>
	class appender<ostreambuf_iterator<CharT, Traits>>
	{
	public:
		appender(ostreambuf_iterator<CharT, Traits> iter) : _iter(move(iter)) { }
		
		appender append(CharT val)
		{
			_iter = std::move(val);
			if(_iter.failed())
				throw runtime_error{"Error writing to ostreambuf."};
			return *this;
		}
		
		appender append(const CharT* start, size_t count)
		{
			assert(start && "NULL passed to appender");
			_iter = copy_n(start, count, _iter);
			if(_iter.failed())
				throw runtime_error{"Error writing to ostreambuf."};
			return *this;
		}
		
	private:
		ostreambuf_iterator<CharT, Traits> _iter;
	};
	
	template<class CharT, class Traits>
	class appender<basic_streambuf<CharT, Traits>>
	{
	public:
		appender(basic_streambuf<CharT, Traits>& buf) : _buf(&buf) { }
		
		appender append(CharT val)
		{
			if(_buf->sputc(val) == Traits::eof())
				throw runtime_error{"Error writing to ostreambuf."};
			return *this;
		}
		
		appender append(const CharT* start, size_t count)
		{
			assert(start && "NULL passed to appender");
			if(_buf->sputn(start, count) != count)
				throw runtime_error{"Error writing to ostreambuf."};
			return *this;
		}
		
	private:
		basic_streambuf<CharT, Traits>* _buf;
	};
	
	template<class CharT, class Traits, class Allocator>
	class appender<basic_string<CharT, Traits, Allocator>>
	{
	public:
		appender(basic_string<CharT, Traits, Allocator>& str) : _str(&str) { }
		
		appender append(CharT val)
		{
			_str->append(val);
			return *this;
		}
		
		appender append(const CharT* start, size_t count)
		{
			_str->append(start, count);
			return *this;
		}
		
	private:
		basic_string<CharT, Traits, Allocator>* _str;
	};
	
	// Enabled only for OutputIterator-like types
	template<class T, bool = is_convertible<typename iterator_traits<T>::iterator_category, output_iterator_tag>::value>
	auto appender_for(T& t) -> appender<T> { return { t }; }
	
	// Force conversion of derived classes
	template<class CharT, class Traits>
	auto appender_for(basic_streambuf<CharT, Traits>& buf) -> appender<basic_streambuf<CharT, Traits>>{ return { buf }; }
	template<class CharT, class Traits, class Allocator>
	auto appender_for(basic_string<CharT, Traits, Allocator>& str) -> appender<basic_string<CharT, Traits, Allocator>>{ return { str }; }
	
	// For ostreams we use the streambuf instead
	template<class CharT, class Traits>
	auto appender_for(basic_ostream<CharT, Traits>& os) -> appender<basic_streambuf<CharT, Traits>> { return { *os.rdbuf() }; }
	
	// Don't nest appenders
	template<class T>
	auto appender_for(appender<T> app) -> appender<T> { return app; }
}} // namespace std::experimental

#endif // std_format_detail_appender_hpp
