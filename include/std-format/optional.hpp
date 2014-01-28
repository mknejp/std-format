//
//  optional.hpp
//  std-format
//
//  Created by knejp on 28.1.14.
//  Copyright (c) 2014 Miro Knejp. All rights reserved.
//

#ifndef std_format_optional_hpp
#define std_format_optional_hpp

#include <utility>

namespace std { namespace experimental
{

	template<class T>
	class optional;
	
	static constexpr struct nullopt_t { } nullopt{};
	
}} // namespace std::experimental


// Very minimalistic implementation of optional to fullfil our needs
template<class T>
class std::experimental::optional
{
public:
	optional() : optional(nullopt) { }
	optional(nullopt_t) : _engaged(false) { }
	optional(const T& value) : _engaged(true) { new (&_value) T(value); }
	optional(T&& value) : _engaged(true) { new (&_value) T(move(value)); }
	optional(const optional& other);
	optional(optional&& other);
	~optional();
	
	optional& operator= (const optional<T>& val);
	optional& operator= (optional<T>&& val);
	optional& operator= (const T& val);
	optional& operator= (T&& val);
	optional& operator= (nullopt_t);
	
	T& operator* () noexcept { return _value; }
	const T& operator* () const noexcept { return _value; }
	
	T* operator-> () noexcept { return &_value; }
	const T* operator-> () const noexcept { return &_value; }
	
	explicit operator bool() const noexcept { return _engaged; }
	
private:
	union
	{
		T _value;
		char _dummy[1];
	};
	bool _engaged;
};

template<class T>
std::experimental::optional<T>::optional(const optional& other) : _engaged(other._engaged)
{
	if(*this)
		new (&_value) T(*other);
}

template<class T>
std::experimental::optional<T>::optional(optional&& other) : _engaged(other._engaged)
{
	if(*this)
	{
		new (&_value) T(move(*other));
		other._value.~T();
		other._engaged = false;
	}
}

template<class T>
std::experimental::optional<T>::~optional()
{
	if(*this)
		_value.~T();
}

template<class T>
auto std::experimental::optional<T>::operator= (const optional<T>& other) -> optional<T>&
{
	if(*this && other)
		_value = *other;
	else if(*this)
	{
		_value.~T();
		_engaged = false;
	}
	else if(other)
	{
		new (&_value) T(*other);
		_engaged = true;
	}
	return *this;
}

template<class T>
auto std::experimental::optional<T>::operator= (optional<T>&& other) -> optional<T>&
{
	if(*this && other)
	{
		_value = move(*other);
		other._value.~T();
		other._engaged = false;
	}
	else if(*this)
	{
		_value.~T();
		_engaged = false;
	}
	else if(other)
	{
		new (&_value) T(*other);
		other._value.~T();
		other._engaged = false;
		_engaged = true;
	}
	return *this;
}

template<class T>
auto std::experimental::optional<T>::operator= (const T& val) -> optional<T>&
{
	if(_engaged)
		_value = val;
	else
	{
		new (&_value) T(val);
		_engaged = true;
	}
	return *this;
}

template<class T>
auto std::experimental::optional<T>::operator= (T&& val) -> optional<T>&
{
	if(_engaged)
		_value = move(val);
	else
	{
		new (&_value) T(move(val));
		_engaged = true;
	}
	return *this;
}

template<class T>
auto std::experimental::optional<T>::operator= (nullopt_t) -> optional<T>&
{
	if(_engaged)
	{
		_value.~T();
		_engaged = false;
	}
	return *this;
}

#endif
