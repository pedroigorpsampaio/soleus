#include "date.h"

#ifndef UTIL_H
#define UTIL_H

static const int TILESIZE = 32;
static const int INIT_X = 300;
static const int INIT_Y = 300;
static const int PLAYER_SPEED = 100;
static const int SNAPSHOT_TICKRATE = 4; // SNAPSHOT (SERVER STATE) SENT PER SECOND

/// format timestamp into a string of date and time
std::string getDateTime(std::chrono::time_point<std::chrono::system_clock> timestamp)
{
	using namespace date;
	using namespace std::chrono;

	std::stringstream ss;
	std::string s = format("%F %T", floor<milliseconds>(timestamp));
	return s;
}


namespace util {
	// Function to calculate distance between two points
	float distance(float x1, float y1, float x2, float y2)
	{
		// Calculating distance
		return sqrt(pow(x2 - x1, 2) + pow(y2 - y1, 2) * 1.0);
	}

	// normalize 2d Vector
	sf::Vector2f normalize(sf::Vector2f v) {
		float length;
		float safeSum = v.x * v.x + v.y * v.y;
		if (safeSum != 0) {
			length = sqrt(v.x * v.x + v.y * v.y);
			sf::Vector2f nV = sf::Vector2f(v.x / length, v.y / length);
			return nV;
		}
		return v;
	}

	/// interpolate two vectors 2d with the average of the sum (50% interpolation)
	sf::Vector2<float> interpolate2v(sf::Vector2<float> v1, sf::Vector2<float> v2)
	{
		sf::Vector2<float> iV;
		iV = (v1 + v2) / 2.f;
		return iV;
	}
}

#endif