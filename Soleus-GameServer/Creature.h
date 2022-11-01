#ifndef CREATURE_H
#define CREATURE_H

#include "Entity.h"

class Creature : public Entity {
public:
	Creature() : Entity() { load(); } // empty constructor
	// constructor with attributes, sends to parent
	Creature(int health, int maxHealth, int speed, sf::Vector2f initialPos) :
		Entity(health, maxHealth, speed, initialPos) {
		// loads creature
		load();
	}

private:
	int load(); // loads creature
};

#endif