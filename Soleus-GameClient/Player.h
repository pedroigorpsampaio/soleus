#ifndef PLAYER_H
#define PLAYER_H
#include "Entity.h"

class Player : public Entity {
	public:
		Player() : Entity() { load(); } // empty constructor
		// constructor with attributes, sends to parent
		Player(int health, int maxHealth, int speed, sf::Vector2f initialPos) :
			Entity(health, maxHealth, speed, initialPos) {
			// loads player
			load();
		}

	private:
		int load(); // loads player
};

#endif
