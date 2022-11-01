#include "Creature.h"
#include <iostream>

int Creature::load() {
	// loads player texture

	if (!texture.loadFromFile("assets/sprites/creature_tst.png")) {
		std::cout << "Could not load creature texture" << std::endl;
		return -1;
	}

	texture.setSmooth(true);
	sprite.setTexture(texture);
	sprite.scale(sf::Vector2f(0.2, 0.2));

	return 0;
}