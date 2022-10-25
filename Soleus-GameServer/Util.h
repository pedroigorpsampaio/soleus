#include "date.h"

#ifndef UTIL_H
#define UTIL_H

/// format timestamp into a string of date and time
std::string getDateTime(std::chrono::time_point<std::chrono::system_clock> timestamp)
{
	using namespace date;
	using namespace std::chrono;

	std::stringstream ss;
	std::string s = format("%F %T", floor<milliseconds>(timestamp));
	return s;
}

#endif