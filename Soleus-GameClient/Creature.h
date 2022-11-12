#ifndef CREATURE_H
#define CREATURE_H

#include "Entity.h"

class Creature : public Entity {
public:
	Creature() : Entity() {} // empty constructor
	// constructor with attributes, sends to parent
	Creature(int health, int maxHealth, int speed, sf::Vector2f initialPos) :
		Entity(health, maxHealth, speed, initialPos) {
		// loads creature
	}

};

#endif