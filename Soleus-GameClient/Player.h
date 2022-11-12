#ifndef PLAYER_H
#define PLAYER_H
#include "Entity.h"

class Player : public Entity {
	public:
		Player() : Entity() { } // empty constructor
		// constructor with attributes, sends to parent
		Player(int health, int maxHealth, int speed, sf::Vector2f initialPos) :
			Entity(health, maxHealth, speed, initialPos) {
		}

		int getLowestVisibleFloor(); // gets player current lowest visible floor
		int getHighestVisibleFloor(int arraySize); // gets player current highest visible floor
		void update(float dt); // updates player
};

#endif
