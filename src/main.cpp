//
//  main.cpp
//  std-format
//
//  Created by knejp on 18.1.14.
//  Copyright (c) 2014 Miro Knejp. All rights reserved.
//

#include <std-format/format.hpp>

#include <iostream>
#include <sstream>

using namespace std;
using namespace experimental;

namespace test
{
	struct Foo
	{
		int i;
	};
	
	string to_string(const Foo& f)
	{
		return std::to_string(f.i * f.i);
	}
	template<class T>
	format_appender<T> to_string(const Foo& f, format_appender<T> app)
	{
		auto s = std::to_string(f.i + f.i);
		return app.append(s.data(), s.size());
	}
	template<class T, class CharT, class Traits>
	format_appender<T> to_string(const Foo& f, format_appender<T> app, basic_string_view<CharT, Traits> options)
	{
		return app.append(options.data(), options.size());
	}
}

int main()
{
//	std::cout << f(1) << endl;
//	std::cout << f<int>(1) << endl;
	
/*	foo::bar b{1};
	format(cout, u8"asd,{0}, {3}, {0}, {2}, {1}, {4}\n", b, 2, 3.45, 4.0f, string("hi"));
	
	using Fmt = sformatter<int, int, float, double, string>;

	string s;
	Fmt fmt{u8"asd,{0}, {3}, {0}, {2}, {1}, {4}\n"};
	format(*cout.rdbuf(), fmt, 1, 2, 3.45, 4.0f, string("hi"));
	format(s, fmt, 1, 2, 3.45, 4.0f, string("hi"));
	format(cout, fmt, 2, 1, 2.45, 5.0f, string("ho"));*/
	
	std::vector<char> v;
	
	string dest = "4";
	auto app = make_format_appender(back_inserter(v));
	
	app = format(in_place, app, u8"{0:asdf} {2} {1}\n", test::Foo{5}, 2, 3);
	app = format(in_place, app, u8"{0} {1} {2}\n", 1, 2, 3);
	copy(v.begin(), v.end(), ostream_iterator<char>(cout));
}
