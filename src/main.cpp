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
	struct PrintOptions
	{
	};
	
	template<class T, class CharT, class Traits>
	format_appender<T> to_string(PrintOptions, format_appender<T> app, basic_string_view<CharT, Traits> options)
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
	
	cout << u8"{0456789876543456789876543}" << " - " << boolalpha << validate_format(u8"{0456789876543456789876543}", 1, nothrow) << "\n";
	
	cout << u8"{0,123:}}as{{df{{}}} {2} {1}" << " - " << boolalpha << validate_format(u8"{0,123:}}as{{df{{}}} {2} {1}", 1, nothrow) << "\n";
	cout << u8"{0,123:}}as{{df{{}}} {2} {1}" << " - " << boolalpha << validate_format(u8"{0,123:}}as{{df{{}}} {2} {1}", 2, nothrow) << "\n";
	cout << u8"{0,123:}}as{{df{{}}} {2} {1}" << " - " << boolalpha << validate_format(u8"{0,123:}}as{{df{{}}} {2} {1}", 3, nothrow) << "\n";

	cout << u8"{0,123:}}as{{df{{}}} {2} {1}" << " - " << boolalpha << validate_format<int>(u8"{0,123:}}as{{df{{}}} {2} {1}", nothrow) << "\n";
	cout << u8"{0,123:}}as{{df{{}}} {2} {1}" << " - " << boolalpha << validate_format<int, int>(u8"{0,123:}}as{{df{{}}} {2} {1}", nothrow) << "\n";
	cout << u8"{0,123:}}as{{df{{}}} {2} {1}" << " - " << boolalpha << validate_format<int, int, int>(u8"{0,123:}}as{{df{{}}} {2} {1}", nothrow) << "\n";

	auto app = make_format_appender(back_inserter(v));
	
	app = format(in_place, app, u8"{0,10:}}as{{df{{}}} {2} {1}\n", test::PrintOptions(), 2, 3);
	app = format(in_place, app, u8"{0,-10:a{{sdf} {2} {1}\n", test::PrintOptions(), 2, 3);
	app = format(in_place, app, u8"{0,-10} {1:dfgh} {2}\n", 1, test::PrintOptions(), 3);
//	app = format(in_place, app, u8"{0456789876543456789876543}\n", 1);
	copy(v.begin(), v.end(), ostream_iterator<char>(cout));
}
