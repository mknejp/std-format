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

using namespace std_format;
using std::cout;

int main()
{
	format(cout, u8"asd,{0}, {3}, {0}, {2}, {1}, {4}\n", 1, 2, 3.45, 4.0f, std::string("hi"));
	
	using Fmt = sformatter<int, int, float, double, std::string>;

	std::string s;
	Fmt fmt{u8"asd,{0}, {3}, {0}, {2}, {1}, {4}\n"};
	format(*cout.rdbuf(), fmt, 1, 2, 3.45, 4.0f, std::string("hi"));
	format(s, fmt, 1, 2, 3.45, 4.0f, std::string("hi"));
	format(cout, fmt, 2, 1, 2.45, 5.0f, std::string("ho"));
}
