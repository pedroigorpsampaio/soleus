#ifndef UTIL_H
#define UTIL_H

#include "date.h"
#include <SFML\System\Vector2.hpp>

static const int TILESIZE = 32;
static const int INIT_X = 2080;	/// new player x
static const int INIT_Y = 3008; /// new player y
static const int INIT_FLOOR = 0; /// new player floor
static const int MIN_FLOOR = -1; /// lowest floor of map
static const int MAX_FLOOR = 1; /// highest floor of map
static const int N_FLOOR_DOWN_VISIBLE = 1; /// number of floors visible below
static const int N_FLOOR_UP_VISIBLE = 0; /// number of floors visible above
static const int PLAYER_SPEED = 100; /// new player speed
static const int SNAPSHOT_TICKRATE = 10; // SNAPSHOT (SERVER STATE) SENT PER SECOND
static const bool DRAW_COLLIDERS = true; /// draw object colliders in screen (debug)
static const bool SERVER_RECONCILIATION = false; /// reconciliate player position with server by interpolation

/// format timestamp into a string of date and time
extern std::string getDateTime(std::chrono::time_point<std::chrono::system_clock> timestamp);

namespace util {
	// Function to calculate distance between two points
	float distance(float x1, float y1, float x2, float y2);

	// normalize 2d Vector
	sf::Vector2f normalize(sf::Vector2f v);

	/// interpolate two vectors 2d with the average of the sum (50% interpolation)
	sf::Vector2<float> interpolate2v(sf::Vector2<float> v1, sf::Vector2<float> v2);

	/// checks collision between two rectangles
	bool checkRectCollision(float x1, float y1, float w1, float h1, float x2, float y2, float w2, float h2);
}

#endif