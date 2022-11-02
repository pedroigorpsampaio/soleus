#ifndef UTIL_H
#define UTIL_H

#include "date.h"
#include <SFML\System\Vector2.hpp>

static const int TILESIZE = 32;
static const int INIT_X = 300;
static const int INIT_Y = 300;
static const int PLAYER_SPEED = 100;
static const int SNAPSHOT_TICKRATE = 10; // SNAPSHOT (SERVER STATE) SENT PER SECOND

/// format timestamp into a string of date and time
extern std::string getDateTime(std::chrono::time_point<std::chrono::system_clock> timestamp);

namespace util {
	// Function to calculate distance between two points
	float distance(float x1, float y1, float x2, float y2);

	// normalize 2d Vector
	sf::Vector2f normalize(sf::Vector2f v);

	/// interpolate two vectors 2d with the average of the sum (50% interpolation)
	sf::Vector2<float> interpolate2v(sf::Vector2<float> v1, sf::Vector2<float> v2);
}

#endif